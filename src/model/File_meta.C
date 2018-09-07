#include "File_meta.h"
#include "File_data.h"
#include "../Media_type.h"
#include "Submission.h"
#include "auth/User.h"
#include "../view/api/common.h"

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

namespace {

static int count_lines(const std::string& str)
{
    int result = 0;

    for (char c : str) {
        if (c == '\n') {
            ++result;
        }
    }

    if (!str.empty() && str[str.size() - 1] != '\n')
        ++result;

    return result;
}

}

const int File_meta::max_byte_count = 5 * 1024 * 1024;

File_meta::File_meta(const std::string& name, const dbo::ptr<Submission>& submission,
                     int line_count, int byte_count)
        : name_{name}
        , submission_{submission}
        , line_count_{line_count}
        , byte_count_{byte_count}
        , time_stamp_{Wt::WDateTime::currentDateTime()}
{ }

dbo::ptr<File_meta>
File_meta::upload(const std::string& name, const std::string& contents,
                  const dbo::ptr<Submission>& submission)
{
    dbo::Session& session = *submission.session();
    dbo::Transaction transaction(session);

    for (auto file : submission->source_files()) {
        if (file->name_ == name) file.remove();
    }

    dbo::ptr<File_meta> result =
            session.addNew<File_meta>(name, submission,
                                      count_lines(contents), contents.size());
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

static std::regex out_file_re(".*\\.out");

bool File_meta::is_out_file() const
{
    return std::regex_match(name(), out_file_re);
}

std::string const& File_meta::media_type() const
{
    return Media_type_registry::instance().lookup(name());
}

bool operator<(const File_meta& a, const File_meta& b)
{
    bool a_out = a.is_out_file();
    bool b_out = b.is_out_file();

    // .out files sort after all other files
    if (a_out && !b_out) return false;
    if (b_out && !a_out) return true;

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
    os << "/submissions/" << submission().id()
            << "/files/"
            << Wt::Utils::urlEncode(name());
    return os.str();
}

J::Object File_meta::to_json(bool brief) const
{
    J::Object result;
    result["uri"] = J::Value(rest_uri());
    result["name"] = J::Value(name());
    result["submission"] = J::Value(submission()->to_json(true));
    result["byte_count"] = J::Value(byte_count());
    result["upload_time"] = J::Value(json_format(time_stamp_));
    return result;
}
