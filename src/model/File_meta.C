#include "File_meta.h"
#include "File_data.h"
#include "Submission.h"
#include "auth/User.h"
#include "../common/Media_type_registry.h"
#include "../common/date_time.h"

#include <Wt/Dbo/Impl.h>
#include <Wt/Json/Value.h>
#include <Wt/Json/Object.h>
#include <Wt/Utils.h>

#include <algorithm>
#include <cctype>
#include <regex>
#include <sstream>

namespace J = Wt::Json;

DBO_INSTANTIATE_TEMPLATES(File_meta)

char const* Enum<File_purpose>::show(File_purpose purpose)
{
    switch (purpose) {
        case File_purpose::log: return "log";
        case File_purpose::resource: return "resource";
        case File_purpose::test: return "test";
        case File_purpose::source: return "source";
    }
}

static std::regex const log_re("log", std::regex_constants::icase);
static std::regex const resource_re("resource",
                                    std::regex_constants::icase);
static std::regex const source_re("source", std::regex_constants::icase);
static std::regex const test_re("test", std::regex_constants::icase);

File_purpose Enum<File_purpose>::read(char const* purpose)
{
    if (std::regex_match(purpose, log_re))
        return File_purpose::log;

    if (std::regex_match(purpose, resource_re))
        return File_purpose::resource;

    if (std::regex_match(purpose, test_re))
        return File_purpose::test;

    return File_purpose::source;
}

static int count_lines(const Bytes& bytes)
{
    int result = 0;

    for (unsigned char c : bytes) {
        if (c == '\n') {
            ++result;
        }
    }

    if (!bytes.empty() && bytes[bytes.size() - 1] != '\n')
        ++result;

    return result;
}

static std::regex const log_file_re(".*\\.(?:log|out)", std::regex_constants::icase);
static std::regex const test_file_re("test.*|.*test\\.[^.]*",
                                     std::regex_constants::icase);

static File_purpose classify_file_type(std::string const& media_type,
                                       std::string const& filename)
{
    std::cerr << "*** media_type: " << media_type
            << " filename: " << filename << "\n";
    if (media_type != "text/plain")
        return File_purpose::resource;

    if (std::regex_match(filename, log_file_re)) {
        std::cerr << "*** Matched log_file_re\n";
        return File_purpose::log;
    }

    if (std::regex_match(filename, test_file_re))
        return File_purpose::test;

    return File_purpose::source;
}

const int File_meta::max_byte_count = 5 * 1024 * 1024;

File_meta::File_meta(const std::string& name, const std::string& media_type,
                     const dbo::ptr<Submission>& submission, int line_count,
                     int byte_count)
        : name_{name}
        , media_type_{media_type}
        , purpose_{classify_file_type(media_type, name)}
        , submission_{submission}
        , line_count_{line_count}
        , byte_count_{byte_count}
        , time_stamp_{Wt::WDateTime::currentDateTime()}
{ }

dbo::ptr<File_meta>
File_meta::upload(const std::string& name,
                  const Bytes& contents,
                  const dbo::ptr<Submission>& submission)
{
    dbo::Session& session = *submission.session();
    dbo::Transaction transaction(session);

    for (auto file : submission->source_files()) {
        if (file->name_ == name) file.remove();
    }

    auto media_type = Media_type_registry::instance().lookup(name);
    auto line_count = media_type == "text/plain"? count_lines(contents) : 0;

    dbo::ptr<File_meta> result =
            session.addNew<File_meta>(name, media_type, submission,
                                      line_count, contents.size());
    session.addNew<File_data>(result, contents);

    submission.modify()->touch();

    transaction.commit();

    return result;
}

void File_meta::rename(const std::string& new_name)
{
    name_ = new_name;
}

void File_meta::re_own(const dbo::ptr <Submission>& new_owner)
{
    Source_file_vec avoid = new_owner->source_files_sorted();

    auto exists = [&](const std::string& name) {
        for (const auto& each : avoid) {
            if (name == each->name()) return true;
        }
        return false;
    };

    if (exists(name())) {
        std::string alternate = name() + "." + submission()->user1()->name();

        while (exists(alternate)) {
            alternate += "~";
        }

        rename(alternate);
    }

    submission_ = new_owner;
}

bool File_meta::is_out_file() const
{
    return purpose() == File_purpose::log;
}

std::string const& File_meta::media_type() const
{
    return media_type_;
}

File_purpose const& File_meta::purpose() const
{
    return purpose_;
}

bool sorts_before(const File_meta& a, const File_meta& b, bool name_only)
{
    if (!name_only) {
        auto a_type = a.purpose();
        auto b_type = b.purpose();

        // sort by type first:
        if (a_type < b_type) return true;
        if (a_type > b_type) return false;
    }

    return std::lexicographical_compare(
            a.name().begin(), a.name().end(),
            b.name().begin(), b.name().end(),
            [](char c1, char c2) {
                return std::toupper(c1) < std::toupper(c2);
            }
    );
}

std::string File_meta::rest_uri() const
{
    std::ostringstream os;
    os << "/api/submissions/" << submission().id()
            << "/files/"
            << Wt::Utils::urlEncode(name());
    return os.str();
}

J::Object File_meta::to_json(bool brief) const
{
    J::Object result;
    result["uri"] = J::Value(rest_uri());
    result["name"] = J::Value(name());
    result["media_type"] = J::Value(media_type());
    result["purpose"] = J::Value(stringify(purpose()));
//    result["submission"] = J::Value(submission()->to_json(true));
    result["byte_count"] = J::Value(byte_count());
    result["upload_time"] = J::Value(json_format(time_stamp_));
    return result;
}

namespace Wt {
namespace Dbo {

char const* sql_value_traits<File_purpose, void>::type(
        SqlConnection* conn, int size)
{
    return repr_trait::type(conn, size);
}

void sql_value_traits<File_purpose, void>::bind(
        const File_purpose& v, SqlStatement* statement, int column, int size)
{
    int value = static_cast<int>(v);
    repr_trait::bind(value, statement, column, size);
}

bool
sql_value_traits<File_purpose, void>::read(File_purpose& v, SqlStatement* statement,
                                        int column, int size)
{
    int value;
    bool result = repr_trait::read(value, statement, column, size);
    v = static_cast<File_purpose>(value);
    return result;
}


} // end namespace Dbo
} // end namespace Wt
