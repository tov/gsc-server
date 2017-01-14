#include "Application_controller.h"

#include "model/auth/User.h"
#include "model/Assignment.h"
#include "model/Submission.h"
#include "view/Admin_view.h"
#include "view/Assignments_view.h"
#include "view/Edit_assignment_view.h"
#include "view/Error_view.h"
#include "view/Main_view.h"
#include "view/Evaluation_view.h"
#include "view/Submissions_view.h"
#include "view/game/HangmanWidget.h"
#include "view/game/HighScoresWidget.h"

#include <Wt/WBootstrapTheme>
#include <Wt/WLocale>

#include <cstdlib>
#include <regex>
#include <stdexcept>
#include <string>

namespace {

struct Load_error : std::exception
{
    Load_error(const std::string& title,
               const std::string& message)
            : title(title), message(message) {}

    std::string title;
    std::string message;
};

void not_found(const std::string& message)
{
    throw Load_error("Not found", message);
}

const std::string denied_message =
        "You aren't allowed to access that.";

void permission_denied(const std::string& message = denied_message)
{
    throw Load_error("Permission denied", message);
}

}

Application_controller*
Application_controller::create(Wt::Dbo::SqlConnectionPool* pool,
                               const Wt::WEnvironment& env)
{
    return new Application_controller(pool, env);
}

Application_controller::Application_controller(Wt::Dbo::SqlConnectionPool* pool,
                                               const Wt::WEnvironment& env)
        : WApplication(env), session_(*pool)
{
    setTitle("gsc homework server");

    messageResourceBundle().use(appRoot() + "strings");
    messageResourceBundle().use(appRoot() + "templates");

    setTheme(new Wt::WBootstrapTheme());
    useStyleSheet("css/gsc.css");

    // There ought to be a way to pick this up from the environment.
    auto locale = this->locale();
    locale.setTimeZone("CST-6CDT,M3.2.0/2,M11.1.0/2");
    setLocale(locale);

    main_ = new Main_view(session_, root());

    internalPathChanged().connect(
            this, &Application_controller::handle_internal_path);
    session_.login().changed().connect(
            this, &Application_controller::on_auth_event);

    on_auth_event();
}

void Application_controller::on_auth_event()
{
    handle_internal_path(internalPath());
}

namespace Path {

using namespace std;

static const regex hw_N("/hw/(\\d+)");
static const regex hw_N_eval("/hw/(\\d+)/eval");
static const regex hw_N_eval_M("/hw/(\\d+)/eval/(\\d+)");
static const regex user_hw("/~([^/]+)/hw");
static const regex user_hw_N("/~([^/]+)/hw/(\\d+)");
static const regex user_hw_N_eval("/~([^/]+)/hw/(\\d+)/eval");
static const regex user_hw_N_eval_M("/~([^/]+)/hw/(\\d+)/eval/(\\d+)");

static const string hw("/hw");
static const string root("/");
static const string game("/game");
static const string high_scores("/game/high_scores");
static const string admin("/admin");

}

void Application_controller::handle_internal_path(
        const std::string& internal_path)
{
    if (!session_.login().loggedIn()) {
        main_->set_widget(nullptr);
        return;
    }

    try {
        std::smatch sm;

        Wt::Dbo::Transaction transaction(session_);

        auto current_user = session_.user();

        // /hw
        if (internal_path == Path::hw) {
            transaction.commit();
            switch (current_user->role()) {
                case User::Role::Student:
                    main_->set_title("Homework server");
                    main_->set_widget(
                            new Submissions_view(current_user, session_));
                    break;

                case User::Role::Grader:
                    permission_denied("There's nothing for you here.");
                    break;

                case User::Role::Admin:
                    main_->set_title("Edit assignments");
                    main_->set_widget(new Assignments_view(session_));
                    break;
            }

            // /
        } else if (internal_path == Path::root) {
            transaction.commit();
            switch (session_.user()->role()) {
                case User::Role::Student:
                    setInternalPath(Path::hw, true);
                    break;

                case User::Role::Grader:
                    setInternalPath(Path::admin, true);
                    break;

                case User::Role::Admin:
                    setInternalPath(Path::admin, true);
                    break;
            }

            // /game
        } else if (internal_path == Path::game) {
            transaction.commit();
            main_->set_title("Gaming ground");
            main_->set_widget(new HangmanWidget(session_));

            // /game/high_scores
        } else if (internal_path == Path::high_scores) {
            transaction.commit();
            main_->set_title("High scores");
            main_->set_widget(new HighScoresWidget(session_));

            // /hw/:n
        } else if (std::regex_match(internal_path, sm, Path::hw_N)) {
            auto assignment = find_assignment({sm[1].first, sm[1].second});
            switch (current_user->role()) {
                case User::Role::Student: {
                    auto submission = Submission::find_by_assignment_and_user(
                            session_, assignment, current_user);
                    transaction.commit();

                    main_->set_title(assignment->name());
                    main_->set_widget(
                            new File_manager_view(submission, session_));
                    break;
                }

                case User::Role::Grader:
                    permission_denied();
                    break;

                case User::Role::Admin:
                    transaction.commit();
                    main_->set_title("Edit " + assignment->name());
                    main_->set_widget(
                            new Edit_assignment_view(assignment, session_));
                    break;
            }

            // /hw/:n/eval
        } else if (std::regex_match(internal_path, sm, Path::hw_N_eval)) {
            auto assignment = find_assignment({sm[1].first, sm[1].second});
            switch (current_user->role()) {
                case User::Role::Student: {
                    auto submission = Submission::find_by_assignment_and_user(
                            session_, assignment, current_user);
                    auto status = submission->status();
                    transaction.commit();

                    if (status != Submission::Status::self_eval &&
                        status != Submission::Status::extended_eval &&
                        status != Submission::Status::closed)
                    {
                        permission_denied(
                                "Too early! Submission is still open.");
                    }

                    auto view = new Evaluation_view(submission, session_);
                    view->go_default();

                    main_->set_title(assignment->name() + " self evaluation");
                    main_->set_widget(view);
                    break;
                }

                default:
                    permission_denied("This doesn't make sense for you.");
            }

            // /hw/:n/eval/:m
        } else if (std::regex_match(internal_path, sm, Path::hw_N_eval_M)) {
            auto assignment = find_assignment({sm[1].first, sm[1].second});
            int m = std::atoi(std::string(sm[2].first, sm[2].second).data());

            if (m < 1 || m > assignment->eval_items().size()) {
                permission_denied("Question number out of range.");
            }

            switch (current_user->role()) {
                case User::Role::Student: {
                    auto submission = Submission::find_by_assignment_and_user(
                            session_, assignment, current_user);
                    auto status = submission->status();
                    transaction.commit();

                    if (status != Submission::Status::self_eval &&
                        status != Submission::Status::extended_eval &&
                        status != Submission::Status::closed)
                    {
                        permission_denied(
                                "Too early! Submission is still open.");
                    }

                    auto view = new Evaluation_view(submission, session_);
                    view->go_to((unsigned) m);

                    main_->set_title(assignment->name() + " self evaluation");
                    main_->set_widget(view);
                    break;
                }

                default:
                    permission_denied("This doesn't make sense for you.");
            }

            // /~:user/hw
        } else if (std::regex_match(internal_path, sm, Path::user_hw)) {
            auto user = find_user({sm[1].first, sm[1].second});
            transaction.commit();

            if (!(current_user->can_admin() || user == current_user))
                permission_denied();

            main_->set_title("~" + user->name());
            main_->set_widget(new Submissions_view(user, session_));

        // ~:user/hw/:n
        } else if (std::regex_match(internal_path, sm, Path::user_hw_N)) {
            auto user = find_user({sm[1].first, sm[1].second});
            auto assignment = find_assignment({sm[2].first, sm[2].second});
            auto submission = Submission::find_by_assignment_and_user(
                    session_, assignment, user);
            transaction.commit();

            if (!submission->can_view(current_user))
                permission_denied();

            main_->set_title(assignment->name());
            main_->set_widget(new File_manager_view(submission, session_));

            // ~:user/hw/:n/eval
        } else if (std::regex_match(internal_path, sm, Path::user_hw_N_eval)) {
            if (!current_user->can_admin()) permission_denied();

            auto user = find_user({sm[1].first, sm[1].second});
            auto assignment = find_assignment({sm[2].first, sm[2].second});
            auto submission = Submission::find_by_assignment_and_user(
                    session_, assignment, user);
            transaction.commit();

            auto view = new Evaluation_view(submission, session_);
            view->go_default();

            main_->set_title(assignment->name() + " self evaluation");
            main_->set_widget(view);

            // ~:user/hw/:n/eval/:m
        } else if (std::regex_match(internal_path, sm,
                                    Path::user_hw_N_eval_M))
        {
            if (!current_user->can_admin()) permission_denied();

            auto user = find_user({sm[1].first, sm[1].second});
            auto assignment = find_assignment({sm[2].first, sm[2].second});
            auto submission = Submission::find_by_assignment_and_user(
                    session_, assignment, user);

            auto m = std::atoi(std::string(sm[3].first, sm[3].second).data());
            if (m < 1 || m > assignment->eval_items().size()) {
                permission_denied("Question number out of range.");
            }
            transaction.commit();

            auto view = new Evaluation_view(submission, session_);
            view->go_to((unsigned) m);

            main_->set_title(assignment->name() + " self evaluation");
            main_->set_widget(view);

            // /admin
        } else if (internal_path == Path::admin) {
            transaction.commit();
            if (session_.user()->can_admin()) {
                main_->set_title("Admin");
                main_->set_widget(new Admin_view(session_));
            } else {
                main_->set_title("Permission denied");
                main_->set_widget(
                        new Error_view("You aren't allowed to access that."));
            }
        } else {
            not_found("No such page.");
        }
    } catch (const Load_error& e) {
        main_->set_title(e.title);
        main_->set_widget(new Error_view(e.message));
    }
}

Wt::Dbo::ptr<User>
Application_controller::find_user(const std::string& user_name)
{
    auto user = User::find_by_name(session_, user_name);
    if (!user) not_found("No such user: " + user_name);
    return user;
}

Wt::Dbo::ptr<Assignment>
Application_controller::find_assignment(const std::string& number_s)
{
    int number = std::atoi(number_s.data());
    auto assignment = Assignment::find_by_number(session_, number);
    if (!assignment) not_found("No such assignment: hw" + number_s);
    return assignment;
}

