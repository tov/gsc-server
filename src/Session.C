#include "Session.h"

#include "Navigate.h"
#include "common/env_var.h"
#include "model/auth/Auth_token.h"
#include "model/auth/Environment.h"
#include "model/auth/User.h"
#include "model/game/User_stats.h"
#include "model/Self_eval.h"
#include "model/Assignment.h"
#include "model/Eval_item.h"
#include "model/Exam_grade.h"
#include "model/File_data.h"
#include "model/File_meta.h"
#include "model/Grader_eval.h"
#include "model/Partner_request.h"
#include "model/Submission.h"

#include <Wt/Auth/AuthService.h>
#include <Wt/Auth/HashFunction.h>
#include <Wt/Auth/Identity.h>
#include <Wt/Auth/PasswordService.h>
#include <Wt/Auth/PasswordStrengthValidator.h>
#include <Wt/Auth/PasswordVerifier.h>
#include <Wt/Auth/Dbo/AuthInfo.h>

#include <Wt/Dbo/backend/Postgres.h>
#include <Wt/Dbo/FixedSqlConnectionPool.h>
#include <Wt/Dbo/Dbo.h>

#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>
#include <Wt/WLogger.h>

#include <cstdlib>

using namespace Wt;
namespace dbo = Wt::Dbo;

namespace {

Auth::AuthService     my_auth_service;
Auth::PasswordService my_password_service(my_auth_service);

}

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

        auto root_pw = get_env_var("ADMIN_PASSWORD");
        create_user("jat489", root_pw, User::Role::Admin);

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
    create_user("jtov", "", User::Role::Admin);

    auto now = WDateTime::currentDateTime();

    std::vector<dbo::ptr<Assignment>> assts;

    for (int i = 1; i <= 4; ++i) {
        std::string title  = "Homework " + std::to_string(i);
        int points = 10;
        auto due   = now.addDays(-7 * i + 8),
             avail = due.addDays(-7),
             eval  = due.addDays(2);

        auto asst = addNew<Assignment>(i, title, points, avail, due, eval);
        assts.push_back(asst);
    }

    for (auto const& name : std::vector{"s1", "s2", "s3", "s4"}) {
        auto user = create_user(name).first;

        auto exam1 = addNew<Exam_grade>(user, 1);
        exam1.modify()->set_points_and_possible(40, 50);
        auto exam2 = addNew<Exam_grade>(user, 2);
        exam2.modify()->set_points_and_possible(37, 50);

        for (auto asst : assts) {
            auto submission = addNew<Submission>(user, asst);
            File_meta::upload("file.h",
                              Bytes("#pragma once\n"),
                              submission, user);
            File_meta::upload("file.C",
                              Bytes("#include \"file.h\"\n\n"
                                    "namespace meh {\n\n}\n"),
                              submission, user);
        }
    }
}

void Db_session::configure_auth()
{
    my_auth_service.setAuthTokensEnabled(true, "gsc_cookie");

    auto verifier = std::make_unique<Auth::PasswordVerifier>();
    verifier->addHashFunction(std::make_unique<Auth::BCryptHashFunction>(7));

    my_password_service.setVerifier(std::move(verifier));
    my_password_service.setStrengthValidator(
            std::make_unique<Auth::PasswordStrengthValidator>());
    my_password_service.setAttemptThrottlingEnabled(true);
}

std::pair<Dbo::ptr<User>, Auth::User>
Db_session::create_user(const std::string& username,
                        const std::string& password,
                        User::Role role)
{
    auto wt_user = users().registerNew();

    wt_user.addIdentity(Auth::Identity::LoginName, username);

    if (!password.empty() || role == User::Role::Admin)
        my_password_service.updatePassword(wt_user, password);

    auto user = addNew<User>(username, role);
    users().find(wt_user).modify()->setUser(user);

    return {user, wt_user};
}

#ifdef GSC_AUTH_PASSWORD
void Db_session::set_password(const dbo::ptr<User>& user,
                              const std::string& password)
{
    my_password_service.updatePassword(users().find(user)->user(), password);
}
#endif // GSC_AUTH_PASSWORD

dbo::ptr<User> Session::user() const
{
    if (!login_.loggedIn()) {
        auth_info_ = nullptr;
        return {};
    }

    if (!auth_info_) auth_info_ = users().find(login_.user());
    if (!auth_info_) return nullptr;

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
    if (!u) return;

    dbo::ptr<User_stats> stats = u->user_stats().lock();
    if (!stats) stats = addNew<User_stats>(u);

    stats.modify()->record_game(s);
}

std::vector<dbo::ptr<User_stats>>
Db_session::top_users(int limit)
{
    dbo::Transaction transaction(*this);

    dbo::collection<dbo::ptr<User_stats>> top =
        find<User_stats>()
            .where("games_played > 0")
            .orderBy("score desc").limit(limit);
    std::vector<dbo::ptr<User_stats>> result(top.begin(), top.end());

    return result;
}

int Session::find_ranking()
{
    dbo::ptr<User> u = user();
    int ranking = -1;

    dbo::Transaction transaction(*this);
    if (u && u->user_stats()) {
        dbo::ptr<User_stats> stats = u->user_stats().lock();
        ranking = query<int>("select distinct count(score) from user_stats")
                .where("score > ?").bind(stats->score());
    }

    return ranking + 1;
}

const Auth::AuthService& Db_session::auth()
{
    return my_auth_service;
}

const Auth::AbstractPasswordService& Db_session::passwordAuth()
{
    return my_password_service;
}

std::unique_ptr<dbo::SqlConnectionPool>
Db_session::createConnectionPool(const std::string& db)
{
    auto connection = std::make_unique<dbo::backend::Postgres>(db);
//    connection->setProperty("show-queries", "true");
    return std::make_unique<dbo::FixedSqlConnectionPool>(std::move(connection),
                                                         10);
}

void Db_session::create_index_(const char* table, const char* field, bool unique)
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
    mapClass<File_data>("file_data");
    mapClass<File_meta>("file_meta");
    mapClass<Grader_eval>("grader_eval");
    mapClass<Partner_request>("partner_request");
    mapClass<Self_eval>("self_eval");
    mapClass<Submission>("submission");
    mapClass<User>("gsc_user");
    mapClass<User_stats>("user_stats");
}

void Db_session::initialize_db(dbo::SqlConnectionPool& pool,
                               bool test_data)
{
    Db_session(pool).initialize_db(test_data);
}

bool Session::authenticate_from_environment()
{
    dbo::Transaction transaction(dbo());

    if (auto wt_user = find_from_environment<auth_user_t>();
            wt_user.isValid())
    {
        login().login(wt_user);
        Wt::log("info") << "authed!";
        return true;
    }

    Wt::log("info") << "didn't auth?";
    return false;
}

void Session::become_user(const dbo::ptr<User>& user)
{
    log("info") << "becoming user: " << user->name();

    login_.logout();
    auth_info_ = nullptr;

    auto user_id = std::to_string(user.id());
    auto auth_user = users().findWithId(user_id);
    login_.login(auth_user);

    Navigate::to("/");
}

auto authn_result<Auth::User>::lift(
        Auth::User const& wt_user,
        Db_session const&) -> type
{
    return wt_user;
}

auto authn_result<Auth_info>::lift(
        Auth::User const& wt_user,
        Db_session const& session) -> type
{
    if (wt_user.isValid())
        return session.users().find(wt_user);
    else
        return {};
}

auto authn_result<User>::lift(
        Auth::User const& wt_user,
        Db_session const& session) -> type
{
    if (auto info = authn_result<Auth_info>::lift(wt_user, session))
        return info->user();
    else
        return nullptr;
}
