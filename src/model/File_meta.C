#include "../common/Media_type_registry.h"
#include "../common/format.h"
#include "../common/paths.h"
#include "../common/util.h"
#include "File_data.h"
#include "File_meta.h"
#include "Submission.h"
#include "auth/User.h"

#include <Wt/Dbo/Impl.h>
#include <Wt/Json/Object.h>
#include <Wt/Json/Value.h>
#include <Wt/Utils.h>

#include <algorithm>
#include <cctype>
#include <regex>

namespace J = Json;

DBO_INSTANTIATE_TEMPLATES(File_meta)

char const* Enum<File_purpose>::show(File_purpose purpose)
{
    switch (purpose) {
    case File_purpose::config: return "config";
    case File_purpose::log: return "log";
    case File_purpose::resource: return "resource";
    case File_purpose::source: return "source";
    case File_purpose::test: return "test";
    case File_purpose::forbidden: return "forbidden";
    }
}

static regex const config_re("config", regex_constants::icase);
static regex const log_re("log", regex_constants::icase);
static regex const resource_re("resource", regex_constants::icase);
static regex const source_re("source", regex_constants::icase);
static regex const test_re("test", regex_constants::icase);

File_purpose Enum<File_purpose>::read(const char* purpose)
{
    auto match = [=](auto re) { return regex_match(purpose, re); };

    if (match(config_re)) return File_purpose::config;

    if (match(log_re)) return File_purpose::log;

    if (match(resource_re)) return File_purpose::resource;

    if (match(test_re)) return File_purpose::test;

    if (match(source_re)) return File_purpose::source;

    return File_purpose::forbidden;
}

static int count_lines(const Bytes& bytes)
{
    int result = 0;

    for (unsigned char c : bytes) {
        if (c == '\n') { ++result; }
    }

    if (! bytes.empty() && bytes.back() != '\n') ++result;

    return result;
}

#define OR "|"
#define BASE(X)                                                                \
    "((" X ".*"                                                                \
    ")" OR "("                                                                 \
    "[^.].*" X "\\.[^.]*"                                                      \
    "))"
#define EXT(X) "([^.].*[.]" X ")"

namespace re {
namespace {

using namespace regex_constants;

constexpr syntax_option_type opt = icase | nosubs;

regex const config_file("Makefile" OR "CMakeLists\\.txt" OR
                        ".gitignore" OR EXT("md"),
                        opt);

regex const
log_file(BASE("screenshot") OR EXT("h?log") OR "[^.].*[-.]log[.][^.]*", opt);

regex const resource_file(EXT("in") OR EXT("out") OR EXT("err") OR EXT("txt"),
                          opt);

regex const test_file(BASE("test"), opt);

regex const forbidden_file("[.].*"  // starts with dot
                           OR EXT("o") OR EXT("exe") OR "[^.]*",  // no dot
                           opt);

}  // namespace
}  // namespace re

static File_purpose classify_file_type(string const& media_type,
                                       string const& filename)
{
    if (regex_match(filename, re::config_file)) return File_purpose::config;

    if (regex_match(filename, re::log_file)) return File_purpose::log;

    if (regex_match(filename, re::forbidden_file)) {
        return File_purpose::forbidden;
    }

    if (regex_match(filename, re::resource_file) || media_type != "text/plain")
        return File_purpose::resource;

    if (regex_match(filename, re::test_file)) return File_purpose::test;

    return File_purpose::source;
}

const int File_meta::max_byte_count = 5 * 1024 * 1024;

File_meta::File_meta(std::string const& name, std::string const& media_type,
                     File_purpose purpose, dbo::ptr<User> const& uploader,
                     int line_count, int byte_count,
                     dbo::ptr<Submission> const& submission)
    : name_{name}
    , time_stamp_{WDateTime::currentDateTime()}
    , line_count_{line_count}
    , byte_count_{byte_count}
    , submission_{submission}
    , media_type_{media_type}
    , purpose_{purpose}
    , uploader_{uploader}
{}

dbo::ptr<File_meta> File_meta::upload(const string& name, const Bytes& contents,
                                      const dbo::ptr<Submission>& submission,
                                      const dbo::ptr<User>& uploader)
{
    dbo::Session& session = *submission.session();
    dbo::Transaction transaction(session);

    auto byte_count = contents.size();
    auto media_type = Media_type_registry::instance().lookup(name);
    auto purpose    = classify_file_type(media_type, name);

    if (purpose == File_purpose::forbidden) throw Bad_file_type_error(name);

    if (byte_count > File_meta::max_byte_count)
        throw File_too_large_error(name, byte_count, File_meta::max_byte_count);

    if (! submission->has_sufficient_space(byte_count, name))
        throw Would_exceed_quota_error(name, byte_count,
                                       submission->remaining_space());

    for (auto file : submission->source_files()) {
        if (file->name_ == name) { file.remove(); }
    }

    auto line_count = media_type == "text/plain" ? count_lines(contents) : 0;

    dbo::ptr<File_meta> result =
            session.addNew<File_meta>(name, media_type, purpose, uploader,
                                      line_count, byte_count, submission);
    std::unique_ptr<File_data> file_data =
            std::make_unique<File_data>(result, contents);

    if (file_data->write_and_commit()) { throw File_operation_error(); }

    submission.modify()->touch();

    transaction.commit();

    return result;
}

void File_meta::move(const dbo::ptr<Submission>& dst_owner,
                     const string& dst_name, bool overwrite)
{
    if (submission() == dst_owner && name() == dst_name) return;

    if (auto file = dst_owner->find_file_by_name(dst_name)) {
        if (! overwrite)
            throw Move_collision_error(submission(), name(), dst_owner,
                                       dst_name);
        std::unique_ptr<File_data> file_data =
                std::make_unique<File_data>(file);
        if (file_data->delete_and_commit()) { throw File_operation_error(); }
        file.remove();
    }

    submission_ = dst_owner;
    name_       = dst_name;
}

void File_meta::set_media_type(const string& media_type)
{
    media_type_ = media_type;
}

void File_meta::reclassify()
{
    reclassify(classify_file_type(media_type(), name()));
}

void File_meta::reclassify(File_purpose purpose)
{
    purpose_ = purpose;
}

bool File_meta::is_line_numbered() const
{
    return line_count_ > 0 && ! is_automatically_deletable();
}

bool File_meta::is_automatically_deletable() const
{
    return purpose() == File_purpose::log;
}

static bool display_before(File_purpose a, File_purpose b)
{
    auto first = File_purpose::log;
    return b != first && (a < b || a == first);
}

bool operator<(const File_meta& a, const File_meta& b)
{
    auto a_type = a.purpose();
    auto b_type = b.purpose();

    // sort by type first:
    if (display_before(a_type, b_type)) return true;
    if (display_before(b_type, a_type)) return false;

    return lexicographical_compare(
            a.name().begin(), a.name().end(), b.name().begin(), b.name().end(),
            [](char c1, char c2) { return toupper(c1) < toupper(c2); });
}

string File_meta::rest_uri() const
{
    return api::paths::Submissions_1_files_2(submission().id(), name());
}

J::Object File_meta::to_json(bool brief) const
{
    J::Object result;
    result["uri"]               = J::Value(rest_uri());
    result["name"]              = J::Value(name());
    result["media_type"]        = J::Value(media_type());
    result["purpose"]           = J::Value(stringify(purpose()));
    result["assignment_number"] = J::Value(submission()->assignment_number());
    result["byte_count"]        = J::Value(byte_count());
    result["upload_time"]       = J::Value(json_format(time_stamp_));
    return result;
}

namespace Wt {
namespace Dbo {

char const* sql_value_traits<File_purpose, void>::type(SqlConnection* conn,
                                                       int size)
{
    return repr_trait::type(conn, size);
}

void sql_value_traits<File_purpose, void>::bind(const File_purpose& v,
                                                SqlStatement* statement,
                                                int column, int size)
{
    int value = static_cast<int>(v);
    repr_trait::bind(value, statement, column, size);
}

bool sql_value_traits<File_purpose, void>::read(File_purpose& v,
                                                SqlStatement* statement,
                                                int column, int size)
{
    int value;
    bool result = repr_trait::read(value, statement, column, size);
    v           = static_cast<File_purpose>(value);
    return result;
}


}  // end namespace Dbo
}  // end namespace Wt
