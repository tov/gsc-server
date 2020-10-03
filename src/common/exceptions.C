#include "../model/Assignment.h"
#include "../model/File_meta.h"
#include "../model/Submission.h"
#include "../model/auth/User.h"
#include "exceptions.h"
#include "format.h"
#include "util.h"


static string file_too_large_msg_(string const& filename, int needed,
                                  int allowed)
{
    ostringstream oss;

    oss << "File Too Large Error\n\n";
    oss << "Problem: File is too large to upload:\n";
    oss << " - filename:     " << filename << "\n";
    oss << " - size of file: " << with_commas(needed) << "\n";
    oss << " - size limit:   " << with_commas(allowed) << "\n";

    return oss.str();
}

static int fix_file_size_limit_(int given)
{
    return given >= 0 ? given : File_meta::max_byte_count;
}

File_too_large_error::File_too_large_error(string filename, int needed,
                                           int allowed)
    : Html_error{file_too_large_msg_(filename, needed,
                                     fix_file_size_limit_(allowed))}
    , filename_{move(filename)}
    , needed_{needed}
    , allowed_{fix_file_size_limit_(allowed)}
{}


const char* Join_collision_error::title() const
{
    return "Cannot Join Submissions";
}

ostream& Join_collision_error::write_body_html(ostream& note) const
{
    note << "<p>";

    bool singular = filenames().size() == 1;

    note << "File submitted by you and " << submissions_[1]->user1()->name()
         << " have the same " << (singular ? "name" : "names") << ":</p><ul>";

    for (auto const& name : filenames()) {
        note << "<li>" << html::Filename{name} << "</li>";
    }

    note << "</ul><p>Before this partnership can be registered, "
         << "you and your prospective partner must ensure that your "
         << "submissions do not have any filenames in common. ";

    if (submissions_[0]->assignment()->web_allowed()) {
        note << "You should delete or rename the files, ";
    } else {
        note << "You should rename the files using <code>gsc mv</code> "
             << "or delete them using <code>gsc rm</code>, ";
    }

    note << "then try again.</p>";

    return note;
}

static std::string join_collision_msg_(dbo::ptr<Submission> const& o1,
                                       dbo::ptr<Submission> const& o2,
                                       vector<string> const& names)
{
    ostringstream oss;

    auto fmt = [&](auto const& o) {
        oss << " - " << o->owner_string() << " hw" << o->assignment_number()
            << "\n";
    };

    oss << "Submission Collision Error\n\n";
    oss << "Problem: These submissions can’t be unified:\n";
    fmt(o1);
    fmt(o2);
    oss << "Reason:  These file names are common to both submissions:\n";
    for (auto const& name : names) { oss << " - " << name << "\n"; }

    return oss.str();
}

const char* Move_collision_error::title() const
{
    return "Cannot Move File";
}

ostream& Move_collision_error::write_body_html(ostream& note) const
{
    auto fmt = [&](auto const& descr, auto const& o, auto const& s) {
        note << descr << " file hw" << o->assignment_number() << ':'
             << html::Filename{s};
    };

    note << "<p>Will not rename ";
    fmt("source", submissions_[0], filenames_[0]);
    note << " over existing ";
    fmt("target", submissions_[1], filenames_[1]);
    return note << ".</p>";
}

static std::string move_collision_msg_(dbo::ptr<Submission> const& o1,
                                       string const& s1,
                                       dbo::ptr<Submission> const& o2,
                                       string const& s2)
{
    ostringstream oss;

    auto fmt = [&](auto const& descr, auto const& o, auto const& s) {
        oss << " - " << descr << ": hw" << o->assignment_number() << ':' << s
            << '\n';
    };

    oss << "Submission Collision Error\n\n";
    oss << "Problem: File cannot be renamed:\n";
    fmt("source", o1, s1);
    fmt("target", o2, s2);
    oss << "Reason:  The target already exists.\n";

    return oss.str();
}

const char* Generic_html_error::title() const
{
    return "Runtime error";
}

ostream& Generic_html_error::write_body_html(ostream& note) const
{
    return note << html::Escape{what()};
}

const char* No_partner_to_separate::title() const
{
    return "Separation Error";
}

ostream& No_partner_to_separate::write_body_html(ostream& note) const
{
    return note << "<p>"
                << "The submission’s only owner is " << username_ << ".</p>";
}

static std::string no_partner_to_separate_msg_(string const& username)
{
    ostringstream oss;

    oss << "Partnership Separation Error\n\n";
    oss << "The submission's only owner is" << username << ".\n";

    return oss.str();
}

const char* Whose_files_are_these::title() const
{
    return "Separation Error";
}

ostream& Whose_files_are_these::write_body_html(ostream& note) const
{
    using namespace html;
    using namespace elt;

    note << "<p>";
    note << "These files do not belong to either partner:";
    note << "</p>";

    note << "<ul>";

    for (auto const& file : lost_files_) {
        note << "<li>" << Filename{file->name()} << "</li>";
    }

    return note << "</ul>";
}

static std::string whose_files_are_these_msg_(Source_file_vec const& files)
{
    ostringstream oss;

    oss << "Partnership Separation Error\n\n";
    oss << "Problem: These files do not belong to either partner:\n";
    for (auto const& file : files) { oss << " - " << file->name() << "\n"; }

    return oss.str();
}

const char* Bad_file_type_error::title() const
{
    return "Bad File Type";
}

ostream& Bad_file_type_error::write_body_html(ostream& note) const
{
    return note << "<p>File " << html::Filename{filename()}
                << " cannot be uploaded because it doesn’t appear "
                   "to be an allowed file type, such as a source file or "
                   "media resource. It might be an object (.o) file, an "
                   "executable (no extension, or .exe on Windows), or a "
                   "hidden file.</p>";
}

static string bad_file_type_msg_(string const& filename)
{
    ostringstream oss;

    oss << "Bad File Type Error\n\n";
    oss << "Problem: File cannot be uploaded:\n";
    oss << " - filename: " << filename << "\n";
    oss << "Reason: It looks like an object or hidden file.\n";

    return oss.str();
}

const char* File_too_large_error::title() const
{
    return "File Too Large";
}

ostream& File_too_large_error::write_body_html(ostream& note) const
{
    return note << "<p>File " << html::Filename{filename()}
                << " cannot be uploaded because it is " << with_commas(needed())
                << " bytes, which exceeds the per-file limit of "
                << with_commas(allowed()) << " bytes.</p>";
}

const char* Would_exceed_quota_error::title() const
{
    return "File Quota Reached";
}

ostream& Would_exceed_quota_error::write_body_html(ostream& note) const
{
    return note << "<p>File " << html::Filename{filename()}
                << " cannot be uploaded because it requires "
                << with_commas(needed())
                << " bytes of storage, which exceeds your remaining quota of "
                << with_commas(available()) << " bytes.";
}

static string would_exceed_quota_msg_(string const& filename, int needed,
                                      int available)
{
    ostringstream oss;

    oss << "File Upload Would Exceed Quota Error\n\n";
    oss << "Problem: You don't have enough space left:\n";
    oss << " - filename:        " << filename << "\n";
    oss << " - size of file:    " << with_commas(needed) << "\n";
    oss << " - remaining quota: " << with_commas(available) << "\n";


    return oss.str();
}

const char* Withdrawn_partner_request_error::title() const
{
    return "Could Not Accept Partner Request";
}

std::ostream&
Withdrawn_partner_request_error::write_body_html(std::ostream& note) const
{
    return note << "Sorry, that partner request has been withdrawn.";
}

static string withdrawn_partner_request_msg_()
{
    ostringstream oss;

    oss << "Withdrawn Partner Request Error\n\n";
    oss << "Sorry, that partner request has been withdrawn.\n";

    return oss.str();
}

const char* File_operation_error::title() const
{
    return "File operation Error";
}

std::ostream& File_operation_error::write_body_html(std::ostream& note) const
{
    return note << "Something unexpected happened while performing a file "
                   "operation.";
}

static string file_operation_msg_()
{
    ostringstream oss;
    oss << "File Operation Error\n\n";
    oss << "Something unexpected happened while performing a file operation.\n";

    return oss.str();
}

Generic_html_error::Generic_html_error(const std::runtime_error& exn)
    : Html_error{exn}
{}

Join_collision_error::Join_collision_error(submission_t o1, submission_t o2,
                                           filenames_t filenames)
    : Html_error{join_collision_msg_(o1, o2, filenames)}
    , submissions_{move(o1), move(o2)}
    , filenames_{move(filenames)}
{}

Move_collision_error::Move_collision_error(submission_t o1, filename_t s1,
                                           submission_t o2, filename_t s2)
    : Html_error{move_collision_msg_(o1, s1, o2, s2)}
    , submissions_{o1, o2}
    , filenames_{s1, s2}
{}

No_partner_to_separate::No_partner_to_separate(string username)
    : Html_error{no_partner_to_separate_msg_(username)}
    , username_{move(username)}
{}

Whose_files_are_these::Whose_files_are_these(Source_file_vec files)
    : Html_error{whose_files_are_these_msg_(files)}
    , lost_files_{move(files)}
{}

Bad_file_type_error::Bad_file_type_error(string filename)
    : Html_error(bad_file_type_msg_(filename))
    , filename_{move(filename)}
{}

Would_exceed_quota_error::Would_exceed_quota_error(string filename, int needed,
                                                   int available)
    : Html_error{would_exceed_quota_msg_(filename, needed, available)}
    , filename_{move(filename)}
    , needed_{needed}
    , available_{available}
{}

Withdrawn_partner_request_error::Withdrawn_partner_request_error()
    : Html_error{withdrawn_partner_request_msg_()}
{}

File_operation_error::File_operation_error()
    : Html_error{file_operation_msg_()}
{}
