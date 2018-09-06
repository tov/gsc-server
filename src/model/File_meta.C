#include "File_meta.h"
#include "File_data.h"
#include "Submission.h"
#include "auth/User.h"

#include <Wt/Dbo/Impl.h>

DBO_INSTANTIATE_TEMPLATES(File_meta);

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

dbo::ptr <File_meta>
File_meta::upload(const std::string& name, const std::string& contents,
                  const dbo::ptr<Submission>& submission)
{
    dbo::Session& session = *submission.session();
    dbo::Transaction transaction(session);

    for (auto file : submission->source_files()) {
        if (file->name_ == name) file.remove();
    }

    File_meta* meta = new File_meta;
    meta->name_       = name;
    meta->submission_ = submission;
    meta->time_stamp_ = Wt::WDateTime::currentDateTime();
    meta->line_count_ = count_lines(contents);
    dbo::ptr<File_meta> result = session.add(meta);

    session.add(new File_data(result, contents));

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
