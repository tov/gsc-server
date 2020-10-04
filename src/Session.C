#include "Session.h"

#include "Config.h"
#include "Navigate.h"
#include "common/env_var.h"
#include "model/Assignment.h"
#include "model/Eval_item.h"
#include "model/Exam_grade.h"
#include "model/auth/Api_key.h"
#include "model/auth/User.h"
#include "model/game/User_stats.h"
class index;
#include "model/File_data.h"
#include "model/File_meta.h"
#include "model/Grader_eval.h"
#include "model/Partner_request.h"
#include "model/Self_eval.h"
#include "model/Submission.h"

#include <Wt/Auth/AuthService.h>
#include <Wt/Auth/Dbo/AuthInfo.h>
#include <Wt/Auth/HashFunction.h>
#include <Wt/Auth/Identity.h>

#ifdef GSC_AUTH_PASSWORD
#    include <Wt/Auth/PasswordService.h>
#    include <Wt/Auth/PasswordStrengthValidator.h>
#    include <Wt/Auth/PasswordVerifier.h>
#endif  // GSC_AUTH_PASSWORD

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/FixedSqlConnectionPool.h>
#include <Wt/Dbo/backend/Postgres.h>

#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>

#include <cstdlib>

using namespace Wt;
namespace dbo = Wt::Dbo;

namespace {

struct Auth_services {
    // Member functions

    Auth_services();
    void configure();
    void update_user_auth(Wt::Auth::User, User_auth_params const&);

    //
    // Member variables
    //

    Auth::AuthService auth;
#ifdef GSC_AUTH_PASSWORD
    Auth::PasswordService password;
#endif  // GSC_AUTH_PASSWORD
};

Auth_services services;

}  // namespace

Db_session::Db_session(dbo::SqlConnectionPool& pool)
{
    dbo().setConnectionPool(pool);
    initialize_session_();
}

Db_session::Db_session(std::unique_ptr<Dbo::SqlConnection> conn)
{
    dbo().setConnection(std::move(conn));
    initialize_session_();
}

void Db_session::initialize_session_()
{
    map_classes();
    users_ = std::make_unique<User_database>(*this);
}

void Db_session::initialize_db(bool test_data)
{
    dbo::Transaction transaction(*this);

    try {
        dbo().createTables();

        create_index_("gsc_user", "name", false);
        create_index_("self_eval", "permalink", false);

        User_auth_params params{"jat489", User::Role::Admin};
#ifdef GSC_AUTH_PASSWORD
        params.password = get_env_var("ADMIN_PASSWORD");
#endif  // GSC_AUTH_PASSWORD
        create_user(params);

        if (test_data) populate_test_data_();

        log("info") << "Database created";
    } catch (const std::exception& e) {
        log("info") << "Using existing database: " << e.what();
    } catch (...) {
        log("info") << "Using existing database";
    }
}

void Db_session::populate_test_data_()
{
    create_user({"jtov", User::Role::Admin});

    auto now = WDateTime::currentDateTime();

    std::vector<dbo::ptr<Assignment>> assts;

    for (int i = 1; i <= 4; ++i) {
        std::string title = "Homework " + std::to_string(i);
        int points        = 10;
        auto due = now.addDays(-7 * i + 8), avail = due.addDays(-7),
             eval = due.addDays(2);

        auto asst = addNew<Assignment>(i, title, points, avail, due, eval);
        assts.push_back(asst);
    }

    for (auto const& name : std::vector{"s1", "s2", "s3", "s4"}) {
        auto user = create_user({name}).first;

        auto exam1 = addNew<Exam_grade>(user, 1);
        exam1.modify()->set_points_and_possible(40, 50);
        auto exam2 = addNew<Exam_grade>(user, 2);
        exam2.modify()->set_points_and_possible(37, 50);

        for (auto const& asst : assts) {
            auto submission = addNew<Submission>(user, asst);
            File_meta::upload("file.h", Bytes("#pragma once\n"), submission,
                              user);
            File_meta::upload("file.C",
                              Bytes("#include \"file.h\"\n\n"
                                    "namespace meh {\n\n}\n"),
                              submission, user);
        }
    }
}

void Db_session::configure_auth()
{
    services.configure();
}

std::pair<Dbo::ptr<User>, Auth::User>
Db_session::create_user(const User_auth_params& params)
{
    auto wt_user = users().registerNew();

    services.update_user_auth(wt_user, params);

#ifdef GSC_AUTH_API_KEY
    set_api_key_identity(wt_user);
#endif  // GSC_AUTH_API_KEY

    auto user = addNew<User>(params.username, params.role);
    users().find(wt_user).modify()->setUser(user);

    return {user, wt_user};
}

#ifdef GSC_AUTH_API_KEY
void Db_session::set_api_key(dbo::ptr<User> const& user)
{
    auto auth_user = find_by_login<auth_user_t>(user->name(), false);
    set_api_key_identity(auth_user);
}

std::string Db_session::get_api_key(dbo::ptr<User> const& user)
{
    auto auth_user = find_by_login<auth_user_t>(user->name(), false);
    return get_api_key_identity(auth_user);
}
#endif  // GSC_AUTH_API_KEY

#ifdef GSC_AUTH_PASSWORD
void Db_session::set_password(dbo::ptr<User> const& user,
                              std::string const& password) const
{
    auth_user_t auth_user = find_by_login(user->name(), false);
    services.password.updatePassword(auth_user, password);
}
#endif  // GSC_AUTH_PASSWORD

dbo::ptr<User> Session::user() const
{
    if (! login_.loggedIn()) {
        auth_info_ = nullptr;
        return {};
    }

    if (! auth_info_) auth_info_ = users().find(login_.user());
    if (! auth_info_) return nullptr;

    auto user = auth_info_->user();
    (void) user.get();  // force load
    return user;
}

std::string Session::user_name() const
{
    return user()->name();
}

void Session::add_to_score(int s)
{
    dbo::Transaction transaction(*this);

    dbo::ptr<User> u = user();
    if (! u) return;

    dbo::ptr<User_stats> stats = u->user_stats().lock();
    if (! stats) stats = addNew<User_stats>(u);

    stats.modify()->record_game(s);
}

std::vector<dbo::ptr<User_stats>> Db_session::top_users(int limit)
{
    dbo::Transaction transaction(*this);

    dbo::collection<dbo::ptr<User_stats>> top =
            find<User_stats>()
                    .where("games_played > 0")
                    .orderBy("score desc")
                    .limit(limit);
    std::vector<dbo::ptr<User_stats>> result(top.begin(), top.end());

    return result;
}

int Session::find_ranking()
{
    dbo::ptr<User> u = user();
    int ranking      = -1;

    dbo::Transaction transaction(*this);
    if (u && u->user_stats()) {
        dbo::ptr<User_stats> stats = u->user_stats().lock();
        ranking = query<int>("select distinct count(score) from user_stats")
                          .where("score > ?")
                          .bind(stats->score());
    }

    return ranking + 1;
}

const Auth::AuthService& Db_session::auth()
{
    return services.auth;
}

#ifdef GSC_AUTH_PASSWORD
const Auth::AbstractPasswordService& Db_session::passwordAuth()
{
    return services.password;
}
#endif  // GSC_AUTH_PASSWORD

std::unique_ptr<dbo::SqlConnectionPool>
Db_session::createConnectionPool(const std::string& db)
{
    auto connection = std::make_unique<dbo::backend::Postgres>(db);
    if (CONFIG.show_queries) connection->setProperty("show-queries", "true");
    return std::make_unique<dbo::FixedSqlConnectionPool>(std::move(connection),
                                                         10);
}

void Db_session::create_index_(const char* table, const char* field,
                               bool unique) const
{
    std::ostringstream query;
    query << "CREATE ";
    if (unique) query << "UNIQUE ";
    query << "INDEX ix_" << table << '_' << field;
    query << " ON " << table << " (" << field << ')';

    dbo().execute(query.str());
}

void Db_session::map_classes()
{
    mapClass<Assignment>("assignment");
    mapClass<Auth_info>("auth_info");
    mapClass<Auth_info::AuthIdentityType>("auth_identity");
    mapClass<Auth_info::AuthTokenType>("auth_token");
    mapClass<Eval_item>("eval_item");
    mapClass<Exam_grade>("exam_grade");
    //mapClass<File_data>("file_data");
    mapClass<File_meta>("file_meta");
    mapClass<Grader_eval>("grader_eval");
    mapClass<Partner_request>("partner_request");
    mapClass<Self_eval>("self_eval");
    mapClass<Submission>("submission");
    mapClass<User>("gsc_user");
    mapClass<User_stats>("user_stats");
}

bool Session::authenticate_from_environment(Environment const& env)
{
    dbo::Transaction transaction(dbo());

    auto auth_user = find_from_environment<auth_user_t>(
            CONFIG.auto_create_accounts, env);
    if (! auth_user.isValid()) return false;

    std::optional<std::string> whoami;
    if (users().find(auth_user)->user()->can_admin() &&
        (whoami = param_whoami(env)).has_value()) {
        auth_user = find_by_login<auth_user_t>(whoami.value(), true);
        if (! auth_user.isValid()) return false;
    }

    login().login(auth_user);
    return true;
}

void Session::become_user(const dbo::ptr<User>& user)
{
    redirect_with_whoami(user->name(), "/");
}

auto authn_result<Auth::User>::lift(Auth::User const& wt_user,
                                    Db_session const&) -> type
{
    return wt_user;
}

auto authn_result<Auth_info>::lift(Auth::User const& wt_user,
                                   Db_session const& session) -> type
{
    if (wt_user.isValid()) return session.users().find(wt_user);
    else
        return {};
}

auto authn_result<User>::lift(Auth::User const& wt_user,
                              Db_session const& session) -> type
{
    if (auto info = authn_result<Auth_info>::lift(wt_user, session))
        return info->user();
    else
        return nullptr;
}

Auth_services::Auth_services()
#ifdef GSC_AUTH_PASSWORD
    : password(auth)
#endif  // GSC_AUTH_PASSWORD
{}

void Auth_services::configure()
{
    auth.setAuthTokensEnabled(true, "gsc_cookie");

#ifdef GSC_AUTH_PASSWORD
    auto verifier = std::make_unique<Auth::PasswordVerifier>();
    verifier->addHashFunction(std::make_unique<Auth::BCryptHashFunction>(7));

    password.setVerifier(std::move(verifier));
    password.setStrengthValidator(
            std::make_unique<Auth::PasswordStrengthValidator>());
    password.setAttemptThrottlingEnabled(true);
#endif  // GSC_AUTH_PASSWORD
}

void Auth_services::update_user_auth(Wt::Auth::User user,
                                     User_auth_params const& params)
{
    user.setIdentity(Auth::Identity::LoginName, params.username);

#ifdef GSC_AUTH_PASSWORD
    if (! params.password.empty() || params.role == User::Role::Admin)
        password.updatePassword(user, params.password);
#endif  // GSC_AUTH_PASSWORD
}
