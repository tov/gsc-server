#include "model/auth/User.h"
#include "model/Assignment.h"
#include "model/File_data.h"
#include "model/File_meta.h"
#include "model/Session.h"
#include "model/Submission.h"

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/backend/Postgres>

#include <iostream>
#include <cstdlib>
#include <cstring>

namespace dbo = Wt::Dbo;

class Gscdb
{
public:
    Gscdb();

    void list_submissions(int assignment);
    void get_submission(int assignment, const std::string& username);

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

    Gscdb app;
    dbo::Transaction transaction(app.session());

    if (strcmp("get", argv[1]) == 0) {
        assert_argc(argc == 4, argv, "HW_NUMBER USERNAME");
        app.get_submission(std::atoi(argv[2]), argv[3]);
    }

    if (strcmp("list", argv[1]) == 0) {
        assert_argc(argc == 3, argv, "HW_NUMBER");
        app.list_submissions(std::atoi(argv[2]));
    }

    else {
        std::cerr << "Unknown command: " << argv[1];
        exit(3);
    }
}

Gscdb::Gscdb()
        : connection_(get_db_string())
{
    dbo_.setConnection(connection_);
    Session::map_classes(dbo_);
}

void Gscdb::get_submission(int asst_no, const std::string& username)
{
    auto assignment = find_assignment(asst_no);
    auto user       = find_user(username);
    auto submission = find_submission(user, assignment);

    for (auto file : submission->source_files()) {
        const auto& text = file->file_data()->contents();
        std::cout << "Writing " << file->name() << "...";
        std::ofstream of(file->name(), std::ios::binary);
        of.write(text.data(), text.size());
        std::cout << "done\n";
    }
}

void Gscdb::list_submissions(int asst_no)
{
    auto assignment = find_assignment(asst_no);

    for (auto submission : assignment->submissions()) {
        std::cout << submission->user1()->name() << '\n';
    }
}

dbo::ptr<User> Gscdb::find_user(const std::string& username)
{
    auto user = User::find_by_name(dbo_, username);
    if (!user) {
        std::cerr << "User not found: " << username << '\n';
        exit(10);
    }
    return user;
}

dbo::ptr<Assignment> Gscdb::find_assignment(int number)
{
    auto assignment = Assignment::find_by_number(dbo_, number);
    if (!assignment) {
        std::cerr << "Assignment not found: " << number << '\n';
        exit(11);
    }
    return assignment;
}

dbo::ptr<Submission> Gscdb::find_submission(const dbo::ptr<User>& user,
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

const char* Gscdb::get_db_string()
{
    auto db_string = std::getenv("POSTGRES_CONNINFO");
    return db_string ? db_string : "dbname=gsc";
}
