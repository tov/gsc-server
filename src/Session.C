#include "Session.h"

#include "model/auth/Auth_token.h"
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
#include "Navigate.h"

#include <Wt/Auth/AuthService.h>
#include <Wt/Auth/HashFunction.h>
#include <Wt/Auth/Identity.h>
#include <Wt/Auth/PasswordService.h>
#include <Wt/Auth/PasswordStrengthValidator.h>
#include <Wt/Auth/PasswordVerifier.h>

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

Db_session::Db_session(Wt::Dbo::SqlConnectionPool& pool)
    : users_{*this}
{
    setConnectionPool(pool);

    map_classes(*this);

    dbo::Transaction transaction(*this);
    try {
        createTables();

        create_index("users", "name", false);
        create_index("self_evals", "permalink", false);

        auto root_password = std::getenv("ADMIN_PASSWORD");
        create_user("root", root_password? root_password : "",
                    User::Role::Admin);
        create_user("jtov", "", User::Role::Admin);

        auto now = Wt::WDateTime::currentDateTime();

        auto asst1 = addNew<Assignment>(1, "Homework 1", 10,
                                        now.addDays(-10), now.addDays(-3),
                                        now.addDays(-1));
        auto asst2 = addNew<Assignment>(2, "Homework 2", 10,
                                        now.addDays(-3),
                                        now.addDays(4),
                                        now.addDays(6));
        auto asst3 = addNew<Assignment>(3, "Homework 3", 10,
                                        now.addDays(-8), now.addDays(-1),
                                        now.addDays(1));
        auto asst4 = addNew<Assignment>(4, "Homework 4", 10,
                                        now.addDays(4), now.addDays(11),
                                        now.addDays(13));

        for (auto name : std::vector<std::string>{"student", "s1", "s2", "s3"}) {
            auto user = create_user(name, "");

            auto exam1 = addNew<Exam_grade>(user, 1);
            exam1.modify()->set_points_and_possible(40, 50);
            auto exam2 = addNew<Exam_grade>(user, 2);
            exam2.modify()->set_points_and_possible(37, 50);

            for (auto asst : {asst1, asst2, asst3, asst4}) {
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

        Wt::log("info") << "Database created";
    } catch (const std::exception& e) {
        Wt::log("info") << "Using existing database: " << e.what();
    } catch (...) {
        Wt::log("info") << "Using existing database";
    }

    transaction.commit();

}

void Db_session::configureAuth()
{
    my_auth_service.setAuthTokensEnabled(true, "gsc_cookie");

    auto verifier = std::make_unique<Auth::PasswordVerifier>();
    verifier->addHashFunction(std::make_unique<Auth::BCryptHashFunction>(7));

    my_password_service.setVerifier(std::move(verifier));
    my_password_service.setStrengthValidator(
            std::make_unique<Auth::PasswordStrengthValidator>());
    my_password_service.setAttemptThrottlingEnabled(true);
}

dbo::ptr<User>
Db_session::create_user(const std::string& username,
                        const std::string& password,
                        User::Role role)
{
    Auth::User user = users_.registerNew();
    user.addIdentity(Auth::Identity::LoginName, username);
    my_password_service.updatePassword(user, password);

    auto user_obj = users_.find(user);
    user_obj.modify()->set_role(role);
    return user_obj;
}

void Db_session::set_password(const dbo::ptr<User>& user,
                              const std::string& password)
{
    my_password_service.updatePassword(users_.find(user), password);
}

Session::Session(dbo::SqlConnectionPool& pool)
        : Db_session(pool)
{ }

dbo::ptr<User> Session::user() const
{
    if (login_.loggedIn()) {
        auto login_name = login_.user().identity(Auth::Identity::LoginName);
        if (!user_ || user_->name() != login_name)
            user_ = users().find(login_.user());

        return user_;
    } else return dbo::ptr<User>();
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

    transaction.commit();
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
    transaction.commit();

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

std::unique_ptr<Wt::Dbo::SqlConnectionPool>
Db_session::createConnectionPool(const std::string& db)
{
    auto connection = std::make_unique<dbo::backend::Postgres>(db);
//    connection->setProperty("show-queries", "true");
    return std::make_unique<dbo::FixedSqlConnectionPool>(std::move(connection),
                                                         10);
}

void Db_session::create_index(const char* table, const char* field, bool unique)
{
    std::ostringstream query;
    query << "CREATE ";
    if (unique) query << "UNIQUE ";
    query << "INDEX ix_" << table << '_' << field;
    query << " ON " << table << " (" << field << ')';

    execute(query.str());
}

void Db_session::map_classes(Wt::Dbo::Session& dbo)
{
    dbo.mapClass<Assignment>("assignments");
    dbo.mapClass<Auth_token>("auth_tokens");
    dbo.mapClass<Eval_item>("eval_items");
    dbo.mapClass<Exam_grade>("exam_grades");
    dbo.mapClass<File_data>("file_data");
    dbo.mapClass<File_meta>("file_meta");
    dbo.mapClass<Grader_eval>("grader_evals");
    dbo.mapClass<Partner_request>("partner_requests");
    dbo.mapClass<Self_eval>("self_evals");
    dbo.mapClass<Submission>("submissions");
    dbo.mapClass<User>("users");
    dbo.mapClass<User_stats>("user_stats");
}

void Session::become_user(const Wt::Dbo::ptr<User>& user)
{
    login_.logout();

    auto user_id = boost::lexical_cast<std::string>(user.id());
    auto auth_user = users().findWithId(user_id);
    login_.login(auth_user);

    user_ = user;

    Navigate::to("/");
}

