#include "model/auth/User.h"
#include "model/Assignment.h"
#include "model/Eval_item.h"
#include "model/File_data.h"
#include "model/File_meta.h"
#include "model/Grader_eval.h"
#include "model/Session.h"
#include "model/Submission.h"

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/backend/Postgres>

#include <iostream>
#include <cstdlib>
#include <cstring>

namespace dbo = Wt::Dbo;

class Gsc_admin
{
public:
    Gsc_admin();

    void list_submissions(int asst_no);
    void get_submission(int asst_no, const std::string& username);
    void upload_file(int asst_no, const std::string& username,
                     const std::string& filename);
    void record_grade(int asst_no, const std::string& username, double score,
                      const std::string& comment);

    dbo::Session& session() { return dbo_; }

private:
    dbo::backend::Postgres connection_;
    dbo::Session dbo_;

    dbo::ptr<User> find_user(const std::string&);
    dbo::ptr<Assignment> find_assignment(int);
    dbo::ptr<Submission> find_submission(const dbo::ptr<User>&,
                                         const dbo::ptr<Assignment>&);

    const char* get_db_string();
};

void assert_argc(bool okay, const char* argv[], const char* message)
{
    if (!okay) {
        std::cerr << "Usage: " << argv[0] << ' '
                  << argv[1] << ' ' << message << '\n';
        exit(2);
    }
}

int main(int argc, const char* argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " COMMAND OPTIONS...\n";
        exit(1);
    }

    Gsc_admin app;
    dbo::Transaction transaction(app.session());

    if (strcmp("get", argv[1]) == 0) {
        assert_argc(argc == 4, argv, "HW_NUMBER USERNAME");
        app.get_submission(atoi(argv[2]), argv[3]);
    }

    else if (strcmp("list", argv[1]) == 0) {
        assert_argc(argc == 3, argv, "HW_NUMBER");
        app.list_submissions(atoi(argv[2]));
    }

    else if (strcmp("upload", argv[1]) == 0) {
        assert_argc(argc == 5, argv, " HW_NUMBERUSERNAME FILENAME");
        app.upload_file(atoi(argv[2]), argv[3], argv[4]);
    }

    else if (strcmp("grade", argv[1]) == 0) {
        assert_argc(argc == 5 || argc == 6, argv,
                    " HW_NUMBERUSERNAME SCORE [COMMENT]");
        const char* comment = argc == 6 ? argv[5] : "";
        app.record_grade(atoi(argv[2]), argv[3], atof(argv[4]), comment);
    }

    else {
        std::cerr << "Unknown command: " << argv[1] << '\n';
        exit(3);
    }
}

Gsc_admin::Gsc_admin()
        : connection_(get_db_string())
{
    dbo_.setConnection(connection_);
    Session::map_classes(dbo_);
}

void Gsc_admin::get_submission(int asst_no, const std::string& username)
{
    auto assignment = find_assignment(asst_no);
    auto user       = find_user(username);
    auto submission = find_submission(user, assignment);

    for (auto file : submission->source_files()) {
        auto text = file->file_data()->contents();
        std::cout << "Writing " << file->name() << "...";
        std::ofstream of(file->name(), std::ios::binary);
        of.write(text.data(), text.size());
        std::cout << "done\n";
    }
}

void Gsc_admin::list_submissions(int asst_no)
{
    auto assignment = find_assignment(asst_no);

    for (auto submission : assignment->submissions()) {
        std::cout << submission->user1()->name() << '\n';
    }
}

void Gsc_admin::upload_file(int asst_no, const std::string& username,
                            const std::string& filename)
{
    auto assignment = find_assignment(asst_no);
    auto user       = find_user(username);
    auto submission = find_submission(user, assignment);

    std::cout << "Uploading " << filename << "...";
    std::cin >> std::noskipws;
    std::string contents{std::istream_iterator<char>(std::cin),
                         std::istream_iterator<char>()};
    File_meta::upload(filename, contents, submission);
    std::cout << "done\n";
}

void Gsc_admin::record_grade(int asst_no, const std::string& username, double score,
                             const std::string& comment)
{
    if (score < 0 || score > 1) {
        std::cerr << "Score must be in the interval [0, 1]\n";
        exit(20);
    }

    auto assignment = find_assignment(asst_no);
    auto user = find_user(username);
    auto submission = find_submission(user, assignment);

    auto root = find_user("root");

    dbo::ptr<Eval_item> eval_item;
    for (const auto& item : submission->items())
        if (item.eval_item &&
            item.eval_item->type() == Eval_item::Type::Informational)
            eval_item = item.eval_item;

    if (!eval_item) {
        std::cerr << "Couldn’t find informational item to add score to.\n";
        exit(13);
    }

    std::cout << "Setting item " << eval_item->sequence() << " to "
              << score << "...";

    auto self_eval   = Submission::get_self_eval(eval_item, submission);
    auto grader_eval = Submission::get_grader_eval(self_eval, root);

    auto grader_eval_m = grader_eval.modify();
    grader_eval_m->set_score(score);
    grader_eval_m->set_explanation(comment);
    grader_eval_m->set_status(Grader_eval::Status::ready);
    grader_eval_m->set_grader(root);

    std::cout << "done.\n";
}

dbo::ptr<User> Gsc_admin::find_user(const std::string& username)
{
    auto user = User::find_by_name(dbo_, username);
    if (!user) {
        std::cerr << "User not found: " << username << '\n';
        exit(10);
    }
    return user;
}

dbo::ptr<Assignment> Gsc_admin::find_assignment(int number)
{
    auto assignment = Assignment::find_by_number(dbo_, number);
    if (!assignment) {
        std::cerr << "Assignment not found: " << number << '\n';
        exit(11);
    }
    return assignment;
}

dbo::ptr<Submission> Gsc_admin::find_submission(const dbo::ptr<User>& user,
                                            const dbo::ptr<Assignment>& asst)
{
    auto submission = Submission::find_by_assignment_and_user(dbo_, asst, user);
    if (!submission) {
        std::cerr << "Submission not found: " << user->name() << '/'
                  << asst->number() << '\n';
        exit(12);
    }
    return submission;
}

const char* Gsc_admin::get_db_string()
{
    auto db_string = std::getenv("POSTGRES_CONNINFO");
    return db_string ? db_string : "dbname=gsc";
}
