#include "model/auth/User.h"
#include "model/Assignment.h"
#include "model/Eval_item.h"
#include "model/Exam_grade.h"
#include "model/File_data.h"
#include "model/File_meta.h"
#include "model/Grader_eval.h"
#include "Session.h"
#include "model/Submission.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/Postgres.h>
#include <Wt/WLocalDateTime.h>

#include <Wt/Auth/AuthService.h>
#include <Wt/Auth/HashFunction.h>
#include <Wt/Auth/Identity.h>
#include <Wt/Auth/PasswordService.h>
#include <Wt/Auth/PasswordStrengthValidator.h>
#include <Wt/Auth/PasswordVerifier.h>

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>

namespace dbo = Wt::Dbo;

using namespace std;

class Gsc_admin
{
public:
    Gsc_admin();

    void grant_extension(int asst_no, const string& username,
                         const string& timespec, bool eval);
    void upload_file(int asst_no, const string& username, const string& filename);
    void set_exam_score(int exam_no, const string& username,
                        int points, int possible);
    void set_password(const string& username, const string& password);
    void print_csv();

    dbo::Session& session() { return dbo_; }

private:
    dbo::backend::Postgres connection_;
    dbo::Session dbo_;

    dbo::ptr<User> find_user(const string&);
    dbo::ptr<Assignment> find_assignment(int);
    dbo::ptr<Submission> find_submission(const dbo::ptr<User>&,
                                         const dbo::ptr<Assignment>&);

    const char* get_db_string();
};

void assert_argc(bool okay, const char* argv[], const char* message)
{
    if (!okay) {
        cerr << "Usage: " << argv[0] << ' '
                  << argv[1] << ' ' << message << '\n';
        exit(2);
    }
}

int main(int argc, const char* argv[])
{
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " COMMAND OPTIONS...\n";
        exit(1);
    }

    Gsc_admin app;
    dbo::Transaction transaction(app.session());

    if (strcmp("extend", argv[1]) == 0) {
        assert_argc(argc == 5, argv,
                    "HW_NUMBER USERNAME TIMESPEC");
        app.grant_extension(atoi(argv[2]), argv[3], argv[4], false);
    }

    else if (strcmp("extend_eval", argv[1]) == 0) {
        assert_argc(argc == 5, argv,
                    "HW_NUMBER USERNAME TIMESPEC");
        app.grant_extension(atoi(argv[2]), argv[3], argv[4], true);
    }

    else if (strcmp("upload", argv[1]) == 0) {
        assert_argc(argc == 5, argv, "HW_NUMBER USERNAME FILENAME");
        app.upload_file(atoi(argv[2]), argv[3], argv[4]);
    }

    else if (strcmp("set_exam", argv[1]) == 0) {
        assert_argc(argc == 6, argv, "EXAM_NUMBER USERNAME POINTS POSSIBLE");
        app.set_exam_score(atoi(argv[2]), argv[3],
                           atoi(argv[4]), atoi(argv[5]));
    }

    else if (strcmp("passwd", argv[1]) == 0) {
        assert_argc(argc == 4, argv, "USERNAME NEW_PASSWORD");
        app.set_password(argv[2], argv[3]);
    }

    else if (strcmp("csv", argv[1]) == 0) {
        assert_argc(argc == 2, argv, "");
        app.print_csv();
    }

    else {
        cerr << "Unrecognized command: " << argv[1] << '\n';
        cerr << "Commands are: extend extend_eval upload set_exam";
        exit(3);
    }
}

Gsc_admin::Gsc_admin()
        : connection_(get_db_string())
{
    dbo_.setConnection(connection_);
    Session::map_classes(dbo_);
}

void Gsc_admin::grant_extension(int asst_no, const string& username,
                                string const& timespec, bool eval)
{
    auto assignment = find_assignment(asst_no);
    auto user       = find_user(username);
    auto submission = find_submission(user, assignment);

    auto locale = Wt::WLocale::currentLocale();
    locale.setTimeZone("CST-6CDT,M3.2.0/2,M11.1.0/2");
    auto local_time = Wt::WLocalDateTime::fromString(timespec, locale);

    if (eval) {
        cout << "Extending eval date to " << local_time.toString() << ".\n";
        submission.modify()->set_eval_date(local_time.toUTC());
    } else {
        cout << "Extending due date to " << local_time.toString() << ".\n";
        submission.modify()->set_due_date(local_time.toUTC());
    }

}

void Gsc_admin::upload_file(int asst_no, const string& username, const string& filename)
{
    auto assignment = find_assignment(asst_no);
    auto user       = find_user(username);
    auto submission = find_submission(user, assignment);

    cout << "Uploading " << filename << "...";
    cin >> noskipws;
    string contents{istream_iterator<char>(cin), istream_iterator<char>()};
    File_meta::upload(filename, contents, submission);
    cout << "done\n";
}

void Gsc_admin::set_exam_score(int exam_no, const string& username,
                               int points, int possible)
{
    auto user = find_user(username);
    auto exam_grade = Exam_grade::find_by_user_and_number(user, exam_no);

    cout << "Setting " << username << " exam " << exam_no
         << " to " << points << " / " << possible << "...";
    exam_grade.modify()->set_points_and_possible(points, possible);
    cout << "done\n";
}


void Gsc_admin::set_password(const string& username, const string& password)
{
    User_database user_database(dbo_);
    Wt::Auth::AuthService auth_service;
    Wt::Auth::PasswordService password_service(auth_service);

    Wt::Auth::PasswordVerifier* verifier = new Wt::Auth::PasswordVerifier();
    verifier->addHashFunction(new Wt::Auth::BCryptHashFunction(7));
    password_service.setVerifier(verifier);

    auto user = find_user(username);
    auto db_user = user_database.find(user);
    password_service.updatePassword(db_user, password);
    cout << "Set password for user " << user->name() << '\n';
}

void Gsc_admin::print_csv()
{
    const auto assigns_q = dbo_.find<Assignment>().orderBy("number")
                               .resultList();
    const auto exams_q = dbo_.query<int>("SELECT number FROM exam_grades")
                             .orderBy("number")
                             .groupBy("number")
                             .resultList();

    std::vector<dbo::ptr<Assignment>> assigns(assigns_q.begin(), assigns_q.end());
    std::vector<int> exams(exams_q.begin(), exams_q.end());

    cout << "NetID";
    for (const auto& assign : assigns)
        cout << ",hw" << assign->number();
    for (int exam : exams)
        cout << ",exam" << exam;
    cout << '\n';

    const auto users = dbo_.find<User>()
                           .where("role = ?").bind(int(User::Role::Student))
                           .orderBy("name")
                           .resultList();

    for (const auto& user : users) {
        cout << user->name();

        for (const auto& assign : assigns) {
            auto submission = Submission::find_by_assignment_and_user(dbo_, assign, user);
            cout << ',' << (submission? submission->grade() : 0);
        }

        for (int exam : exams) {
            auto exam_grade = Exam_grade::find_by_user_and_number(user, exam);
            cout << ',' << (exam_grade? exam_grade->grade() : 0);
        }

        cout << endl;
    }
}

dbo::ptr<User> Gsc_admin::find_user(const string& username)
{
    auto user = User::find_by_name(dbo_, username);
    if (!user) {
        cerr << "User not found: " << username << '\n';
        exit(10);
    }
    return user;
}

dbo::ptr<Assignment> Gsc_admin::find_assignment(int number)
{
    auto assignment = Assignment::find_by_number(dbo_, number);
    if (!assignment) {
        cerr << "Assignment not found: " << number << '\n';
        exit(11);
    }
    return assignment;
}

dbo::ptr<Submission> Gsc_admin::find_submission(const dbo::ptr<User>& user,
                                            const dbo::ptr<Assignment>& asst)
{
    auto submission = Submission::find_by_assignment_and_user(dbo_, asst, user);
    if (!submission) {
        cerr << "Submission not found: " << user->name() << '/'
                  << asst->number() << '\n';
        exit(12);
    }
    return submission;
}

const char* Gsc_admin::get_db_string()
{
    auto db_string = getenv("POSTGRES_CONNINFO");
    return db_string ? db_string : "dbname=gsc";
}

