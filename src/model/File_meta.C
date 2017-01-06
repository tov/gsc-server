#include "File_meta.h"
#include "File_data.h"
#include "Submission.h"

#include <Wt/Dbo/Impl>

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
