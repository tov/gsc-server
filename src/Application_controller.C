#include "Application_controller.h"

#include "Config.h"
#include "common/format.h"
#include "common/util.h"
#include "model/Assignment.h"
#include "model/Grader_eval.h"
#include "model/Self_eval.h"
#include "model/Submission.h"
#include "model/auth/User.h"
#include "view/game/HangmanWidget.h"
#include "view/game/HighScoresWidget.h"
#include "view/web/PageTitle.h"
#include "view/web/view/Admin_view.h"
#include "view/web/view/Assignments_view.h"
#include "view/web/view/Edit_assignment_view.h"
#include "view/web/view/Error_view.h"
#include "view/web/view/Evaluation_view.h"
#include "view/web/view/Grading_stats_view.h"
#include "view/web/view/Grading_view.h"
#include "view/web/view/Held_back_view.h"
#include "view/web/view/Main_view.h"
#include "view/web/view/Profile_view.h"
#include "view/web/view/Submissions_view.h"

#include <Wt/Date/tz.h>
#include <Wt/WBootstrapTheme.h>
#include <Wt/WEnvironment.h>
#include <Wt/WLocale.h>
#include <Wt/WWidget.h>

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

struct Load_error : std::exception {
  Load_error(const std::string &title, const std::string &message)
      : title(title), message(message) {}

  std::string title;
  std::string message;
};

void not_found [[noreturn]] (const std::string &message) {
  throw Load_error("Not found", message);
}

const std::string denied_message = "You aren't allowed to access that.";

void permission_denied
    [[noreturn]] (const std::string &message = denied_message) {
  throw Load_error("Permission denied", message);
}

} // namespace

std::unique_ptr<Wt::WApplication>
Application_controller::create(Wt::Dbo::SqlConnectionPool &pool,
                               const Wt::WEnvironment &env) {
  return std::make_unique<Application_controller>(pool, env);
}

static std::string hashedResource(char const *filename) {
  std::ostringstream result_path;
  result_path << filename;

  std::ostringstream hash_filename;
  hash_filename << "html/" << filename << ".hash";

  std::string hash_value;
  std::ifstream hash_file(hash_filename.str());

  if (hash_file && hash_file >> hash_value) {
    result_path << "?";
    result_path << hash_value;
  }

  return result_path.str();
}

static date::time_zone const *find_time_zone(WEnvironment const &env) {
  if (env.javaScript()) {
    if (string tz_name = env.timeZoneName(); !tz_name.empty())
      if (auto tz = date::locate_zone(tz_name))
        return tz;

    // Or this for old IE:
    // chrono::minutes tz_offset = env.timeZoneOffset();
  }

  if (auto tz = date::locate_zone(CONFIG.server_time_zone))
    return tz;

  return date::zoned_traits<date::time_zone const *>::default_zone();
}

Application_controller::Application_controller(Wt::Dbo::SqlConnectionPool &pool,
                                               const Wt::WEnvironment &env)
    : WApplication(env), session_(pool) {
  setTitle("gsc homework server");

  messageResourceBundle().use(appRoot() + "strings");
  messageResourceBundle().use(appRoot() + "templates");

  auto theme = std::make_shared<Wt::WBootstrapTheme>();
  theme->setVersion(Wt::BootstrapVersion::v3);
  setTheme(theme);

  useStyleSheet(hashedResource("css/gsc.css"));

  requireJQuery("js/jquery-3.5.1.min.js");
  require("js/jquery-ui-1.12.1.min.js");
  require(hashedResource("js/loader.js"));
  require(hashedResource("js/pre-gsc.js"));
  require(hashedResource("js/gsc.js"));
  // require("https://cdnjs.cloudflare.com/ajax/libs/markdown-it/10.0.0/markdown-it.min.js");

  auto locale = this->locale();
  auto time_zone = find_time_zone(environment());
  locale.setTimeZone(time_zone);

  auto now = chrono::system_clock::now();
  string abbrev = time_zone->get_info(now).abbrev;

  if (any_of(abbrev.begin(), abbrev.end(), [](char c) { return isalpha(c); })) {
    abbrev = "'" + abbrev + "'";
  } else {
    abbrev = "Z";
  }

  locale.setDateTimeFormat("ddd dd MMM '@' h:mmap (" + abbrev + ")");
  setLocale(locale);

  // Try to use OpenAM login info.
  session_.authenticate_from_environment();

  main_ = root()->addNew<Main_view>(session_);

  internalPathChanged().connect(this,
                                &Application_controller::handle_internal_path);
  session_.login().changed().connect(this,
                                     &Application_controller::on_auth_event);

  on_auth_event();
}

void Application_controller::on_auth_event() {
  handle_internal_path(internalPath());
}

namespace Path {

using namespace std;

static const regex hw_N("/hw/(\\d+)");
static const regex hw_N_eval("/hw/(\\d+)/eval");
static const regex hw_N_eval_M("/hw/(\\d+)/eval/(\\d+)");
static const regex grade_N("/grade/([[:alnum:]]+)");
static const regex user("/~([^/]+)");
static const regex user_hw("/~([^/]+)/hw");
static const regex user_hw_N("/~([^/]+)/hw/(\\d+)");
static const regex user_hw_N_eval("/~([^/]+)/hw/(\\d+)/eval");
static const regex user_hw_N_eval_M("/~([^/]+)/hw/(\\d+)/eval/(\\d+)");
static const regex user_profile("/~([^/]+)/profile");
static const regex trailing_slash("(.*)/");

static const string hw("/hw");
static const string held_back("/held_back");
static const string grade("/grade");
static const string grading_stats("/grading_stats");
static const string profile("/profile");
static const string root("/");
static const string game("/game");
static const string high_scores("/game/high_scores");
static const string admin("/admin");

} // namespace Path

void Application_controller::handle_internal_path(std::string internal_path) {
  if (!session_.login().loggedIn()) {
    set_widget(nullptr);
    return;
  }

  if (internal_path.substr(0, 4) == "/gsc") {
    internal_path = internal_path.substr(4);
  }

  if (internal_path.empty()) {
    internal_path = "/";
  }

  try {
    std::smatch sm;

    Wt::Dbo::Transaction transaction(session_);

    auto current_user = session_.user();

    auto add_tilde_user = [&]() {
      std::ostringstream url;
      url << "~" << current_user->name() << internal_path;
      setInternalPath(url.str(), true);
    };

    if (false) {
    }

    // /hw
    else if (internal_path == Path::hw) {
      transaction.commit();
      switch (current_user->role()) {
      case User::Role::Student:
        add_tilde_user();
        break;

      case User::Role::Grader:
        permission_denied("There's nothing for you here.");
        break;

      case User::Role::Admin:
        set_page_title("Edit Assignments");
        set_new_widget<Assignments_view>(session_);
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
        setInternalPath(Path::grade, true);
        break;

      case User::Role::Admin:
        setInternalPath(Path::admin, true);
        break;
      }

      // /profile
    } else if (internal_path == Path::profile) {
      transaction.commit();
      add_tilde_user();

      // /grade
    } else if (internal_path == Path::grade) {
      current_user->check_can_grade();

      auto permalink =
          Self_eval::find_ungraded_permalink(session_, current_user);
      transaction.commit();

      if (permalink.empty()) {
        setInternalPath("/game", true);
      } else {
        setInternalPath("/grade/" + permalink, true);
      }

      // /grading_stats
    } else if (internal_path == Path::grading_stats) {
      transaction.commit();
      current_user->check_can_grade();

      set_page_title("Grading Stats");
      set_new_widget<Grading_stats_view>(session_);

      // /grade/:permalink
    } else if (std::regex_match(internal_path, sm, Path::grade_N)) {
      current_user->check_can_grade();

      std::string permalink{sm[1].first, sm[1].second};
      auto self_eval = Self_eval::find_by_permalink(session_, permalink);
      if (!self_eval)
        not_found("No such evaluation.");
      transaction.commit();

      set_page_title("Grading");
      set_new_widget<Grading_view>(self_eval, session_);

      // /~:user/hw
    } else if (std::regex_match(internal_path, sm, Path::user_hw)) {
      auto user = find_user(sm[1], current_user);
      transaction.commit();

      current_user->check_can_view(user);

      set_page_title(PageTitle::user_home(user));
      set_new_widget<Submissions_view>(user, session_);

      // /~:user/hw/:n
    } else if (std::regex_match(internal_path, sm, Path::user_hw_N)) {
      auto assignment = find_assignment(&*sm[2].first);
      auto user = find_user(sm[1], current_user);
      auto submission =
          Submission::find_by_assignment_and_user(session_, assignment, user);

      submission->check_can_view(current_user);

      set_page_title(PageTitle::user_hwN(submission, current_user));
      set_new_widget<File_manager_view>(submission, session_);

      // /~:user/hw/:n/eval
    } else if (std::regex_match(internal_path, sm, Path::user_hw_N_eval)) {
      auto assignment = find_assignment(&*sm[2].first);
      auto user = find_user(sm[1], current_user);
      auto submission =
          Submission::find_by_assignment_and_user(session_, assignment, user);

      submission->check_can_view_eval(current_user);

      auto view = std::make_unique<Evaluation_view>(submission, session_);
      view->go_default();

      set_page_title(PageTitle::user_hwN_eval(submission, current_user));
      set_widget(std::move(view));

      // /~:user/hw/:n/eval/:m
    } else if (std::regex_match(internal_path, sm, Path::user_hw_N_eval_M)) {
      auto assignment = find_assignment(&*sm[2].first);
      auto user = find_user(sm[1], current_user);
      auto submission =
          Submission::find_by_assignment_and_user(session_, assignment, user);
      auto m = find_eval_item(assignment, &*sm[3].first);

      submission->check_can_view_eval(current_user);
      transaction.commit();

      auto view = std::make_unique<Evaluation_view>(submission, session_);
      view->go_to((unsigned)m);

      set_page_title(PageTitle::user_hwN_eval(submission, current_user));
      set_widget(std::move(view));

      // /~:user/profile
    } else if (std::regex_match(internal_path, sm, Path::user_profile)) {
      auto user = find_user(sm[1], current_user);
      transaction.commit();

      current_user->check_can_view(user);

      set_page_title(PageTitle::user_profile(user));
      set_new_widget<Profile_view>(user, session_);

      // /game
    } else if (internal_path == Path::game) {
      transaction.commit();
      set_page_title("Gaming Ground");
      set_new_widget<HangmanWidget>(session_);

      // /game/high_scores
    } else if (internal_path == Path::high_scores) {
      set_page_title("High Scores");
      set_new_widget<HighScoresWidget>(session_);

      // /hw/:n
    } else if (std::regex_match(internal_path, sm, Path::hw_N)) {
      if (current_user->can_admin()) {
        auto assignment = find_assignment(&*sm[1].first);
        set_page_title(assignment->slug_string() + ": Edit");
        set_new_widget<Edit_assignment_view>(assignment, session_);
      } else {
        add_tilde_user();
      }

      // /hw/:n/eval
    } else if (std::regex_match(internal_path, sm, Path::hw_N_eval)) {
      add_tilde_user();

      // /hw/:n/eval/:m
    } else if (std::regex_match(internal_path, sm, Path::hw_N_eval_M)) {
      add_tilde_user();

      // /admin
    } else if (internal_path == Path::admin) {
      transaction.commit();
      if (!current_user->can_admin())
        permission_denied();

      set_page_title("Admin");
      set_new_widget<Admin_view>(session_);

      // /held_back
    } else if (internal_path == Path::held_back) {
      transaction.commit();
      if (!current_user->can_admin())
        permission_denied();

      set_page_title("Held-Back Evaluations");
      set_new_widget<Held_back_view>(session_);

      // /~:user
    } else if (std::regex_match(internal_path, sm, Path::user)) {
      setInternalPath(internal_path + "/hw", true);

      // .*/
    } else if (std::regex_match(internal_path, sm, Path::trailing_slash)) {
      setInternalPath({sm[1].first, sm[1].second}, true);

    } else {
      not_found("No such page.");
    }
  } catch (const Load_error &e) {
    set_page_title(e.title);
    set_new_widget<Error_view>(e.message);
  }
}

void Application_controller::set_page_title(PageTitle &&title) {
  setTitle(title.get_text());
  main_->set_title(title.build_widget());
}

void Application_controller::set_page_title(std::string const &text) {
  setTitle(text);
  main_->set_title(text);
}

void Application_controller::set_widget(std::unique_ptr<Wt::WWidget> widget) {
  main_->set_widget(std::move(widget));
}

Wt::Dbo::ptr<User>
Application_controller::find_user(ssubmatch const &sub,
                                  Wt::Dbo::ptr<User> const &current) {
  std::string user_name(sub.first, sub.second);

  if (auto user = User::find_by_name(session_, user_name)) {
    return user;
  }

  if (current && current->can_admin()) {
    not_found("No such user: " + user_name);
  } else {
    permission_denied();
  }
}

Wt::Dbo::ptr<Assignment>
Application_controller::find_assignment(const char *number_s) {
  int number = std::atoi(number_s);
  auto assignment = Assignment::find_by_number(session_, number);
  if (!assignment)
    not_found("No such assignment");
  return assignment;
}

int Application_controller::find_eval_item(
    const dbo::ptr<Assignment> &assignment, const char *number_s) {
  int m = std::atoi(number_s);

  if (m < 1 || m > assignment->eval_items().size()) {
    permission_denied("Question number out of range.");
  }

  return m;
}
