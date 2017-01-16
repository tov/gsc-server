#include "Application_controller.h"

#include "model/auth/User.h"
#include "model/Assignment.h"
#include "model/Grader_eval.h"
#include "model/Self_eval.h"
#include "model/Submission.h"
#include "view/Admin_view.h"
#include "view/Assignments_view.h"
#include "view/Edit_assignment_view.h"
#include "view/Error_view.h"
#include "view/Grading_view.h"
#include "view/Held_back_view.h"
#include "view/Main_view.h"
#include "view/Evaluation_view.h"
#include "view/Submissions_view.h"
#include "view/game/HangmanWidget.h"
#include "view/game/HighScoresWidget.h"

#include <Wt/WBootstrapTheme>
#include <Wt/WLocale>
#include <Wt/WWidget>

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
static const regex grade_N("/grade/([[:alnum:]]+)");
static const regex user("/~([^/]+)");
static const regex user_hw("/~([^/]+)/hw");
static const regex user_hw_N("/~([^/]+)/hw/(\\d+)");
static const regex user_hw_N_eval("/~([^/]+)/hw/(\\d+)/eval");
static const regex user_hw_N_eval_M("/~([^/]+)/hw/(\\d+)/eval/(\\d+)");
static const regex trailing_slash("(.*)/");

static const string hw("/hw");
static const string held_back("/held_back");
static const string grade("/grade");
static const string root("/");
static const string game("/game");
static const string high_scores("/game/high_scores");
static const string admin("/admin");

}

namespace Title
{

using namespace std;

inline string user_hw(const dbo::ptr<User>& user)
{
    return "~" + user->name();
}

inline string
user_hw_N(const dbo::ptr<Submission>& submission)
{
    return user_hw(submission->user1()) +
            ": " + submission->assignment()->name();
}

inline string
user_hw_N_eval(const dbo::ptr<Submission>& submission)
{
    return user_hw_N(submission) + " evaluation";
}

}

void Application_controller::handle_internal_path(
        const std::string& internal_path)
{
    if (!session_.login().loggedIn()) {
        set_widget(nullptr);
        return;
    }

    try {
        std::smatch sm;

        Wt::Dbo::Transaction transaction(session_);

        auto current_user = session_.user();

        if (false)
        { }

        // /hw
        else if (internal_path == Path::hw) {
            transaction.commit();
            switch (current_user->role()) {
                case User::Role::Student:
                case User::Role::Grader:
                    permission_denied("There's nothing for you here.");
                    break;

                case User::Role::Admin:
                    set_title("Edit assignments");
                    set_widget(new Assignments_view(session_));
                    break;
            }

            // /
        } else if (internal_path == Path::root) {
            transaction.commit();
            switch (session_.user()->role()) {
                case User::Role::Student:
                    setInternalPath(current_user->hw_url(), true);
                    break;

                case User::Role::Grader:
                    permission_denied("There's nothing for you here.");
                    break;

                case User::Role::Admin:
                    setInternalPath(Path::admin, true);
                    break;
            }

            // /grade
        } else if (internal_path == Path::grade) {
            if (!current_user->can_grade()) permission_denied();

            auto permalink = Self_eval::find_ungraded_permalink(session_,
                                                                current_user);
            transaction.commit();

            if (permalink.empty()) {
                setInternalPath("/game", true);
            } else {
                setInternalPath("/grade/" + permalink, true);
            }

            // /grade/:permalink
        } else if (std::regex_match(internal_path, sm, Path::grade_N)) {
            if (!current_user->can_grade()) permission_denied();

            std::string permalink{sm[1].first, sm[1].second};
            auto self_eval = Self_eval::find_by_permalink(session_, permalink);
            if (!self_eval) not_found("No such evaluation.");
            transaction.commit();

            auto view = new Grading_view(self_eval, session_);

            set_title("Grading");
            set_widget(view);

            // /~:user/hw
        } else if (std::regex_match(internal_path, sm, Path::user_hw)) {
            auto user = find_user({sm[1].first, sm[1].second});
            transaction.commit();

            if (!current_user->can_view(user))
                permission_denied();

            set_title(Title::user_hw(user));
            set_widget(new Submissions_view(user, session_));

        // ~:user/hw/:n
        } else if (std::regex_match(internal_path, sm, Path::user_hw_N)) {
            auto user = find_user({sm[1].first, sm[1].second});
            auto assignment = find_assignment(&*sm[2].first);
            auto submission = Submission::find_by_assignment_and_user(
                    session_, assignment, user);
            transaction.commit();

            if (!submission->can_view(current_user))
                permission_denied();

            set_title(Title::user_hw_N(submission));
            set_widget(new File_manager_view(submission, session_));

            // ~:user/hw/:n/eval
        } else if (std::regex_match(internal_path, sm, Path::user_hw_N_eval)) {
            auto user = find_user({sm[1].first, sm[1].second});
            auto assignment = find_assignment(&*sm[2].first);
            auto submission = Submission::find_by_assignment_and_user(
                    session_, assignment, user);

            check_eval_view_privileges(current_user, submission);

            auto view = new Evaluation_view(submission, session_);
            view->go_default();

            set_title(Title::user_hw_N_eval(submission));
            set_widget(view);

            // ~:user/hw/:n/eval/:m
        } else if (std::regex_match(internal_path, sm,
                                    Path::user_hw_N_eval_M))
        {
            auto user = find_user({sm[1].first, sm[1].second});
            auto assignment = find_assignment(&*sm[2].first);
            auto submission = Submission::find_by_assignment_and_user(
                    session_, assignment, user);
            auto m = find_eval_item(assignment, &*sm[3].first);

            check_eval_view_privileges(current_user, submission);
            transaction.commit();

            auto view = new Evaluation_view(submission, session_);
            view->go_to((unsigned) m);

            set_title(Title::user_hw_N_eval(submission));
            set_widget(view);

            // /game
        } else if (internal_path == Path::game) {
            transaction.commit();
            set_title("Gaming ground");
            set_widget(new HangmanWidget(session_));

            // /game/high_scores
        } else if (internal_path == Path::high_scores) {
            transaction.commit();
            set_title("High scores");
            set_widget(new HighScoresWidget(session_));

            // /hw/:n
        } else if (std::regex_match(internal_path, sm, Path::hw_N)) {
            if (!current_user->can_admin()) permission_denied();

            auto assignment = find_assignment(&*sm[1].first);
            transaction.commit();

            set_title("Edit " + assignment->name());
            set_widget(new Edit_assignment_view(assignment, session_));

            // /admin
        } else if (internal_path == Path::admin) {
            transaction.commit();
            if (!current_user->can_admin()) permission_denied();

            set_title("Admin");
            set_widget(new Admin_view(session_));

            // /held_back
        } else if (internal_path == Path::held_back) {
            transaction.commit();
            if (!current_user->can_admin()) permission_denied();

            set_title("Held-back evaluations");
            set_widget(new Held_back_view(session_));

            // /~:user
        } else if (std::regex_match(internal_path, sm, Path::user)) {
            setInternalPath(internal_path + "/hw", true);

            // .*/
        } else if (std::regex_match(internal_path, sm, Path::trailing_slash)) {
            setInternalPath({sm[1].first, sm[1].second}, true);

        } else {
            not_found("No such page.");
        }
    } catch (const Load_error& e) {
        set_title(e.title);
        set_widget(new Error_view(e.message));
    }
}

void Application_controller::set_title(const std::string& title)
{
    setTitle("gsc: " + title);
    main_->set_title(title);
}

void Application_controller::set_widget(Wt::WWidget* widget)
{
    main_->set_widget(widget);
}

Wt::Dbo::ptr<User>
Application_controller::find_user(const std::string& user_name)
{
    auto user = User::find_by_name(session_, user_name);
    if (!user) not_found("No such user: " + user_name);
    return user;
}

Wt::Dbo::ptr<Assignment>
Application_controller::find_assignment(const char* number_s)
{
    int number = std::atoi(number_s);
    auto assignment = Assignment::find_by_number(session_, number);
    if (!assignment) not_found("No such assignment");
    return assignment;
}

int
Application_controller::find_eval_item(const dbo::ptr <Assignment>& assignment,
                                       const char* number_s) {
    int m = std::atoi(number_s);

    if (m < 1 || m > assignment->eval_items().size())
        permission_denied("Question number out of range.");

    return m;
}

void Application_controller::check_eval_view_privileges(
        const Wt::Dbo::ptr<User>& current_user,
        const Wt::Dbo::ptr<Submission>& submission) const
{
    if (!submission->can_view_eval(current_user)) {
        if (!submission->can_view(current_user))
            permission_denied();

        permission_denied("Too early! Submission is still open.");
    }
}
