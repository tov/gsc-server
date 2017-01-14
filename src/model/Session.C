#include "Session.h"

#include "auth/Auth_token.h"
#include "auth/User.h"
#include "game/User_stats.h"
#include "Self_eval.h"
#include "Assignment.h"
#include "Eval_item.h"
#include "File_data.h"
#include "File_meta.h"
#include "Grader_eval.h"
#include "Partner_request.h"
#include "Submission.h"

#include <Wt/Auth/AuthService>
#include <Wt/Auth/HashFunction>
#include <Wt/Auth/Identity>
#include <Wt/Auth/PasswordService>
#include <Wt/Auth/PasswordStrengthValidator>
#include <Wt/Auth/PasswordVerifier>

#include <Wt/Dbo/backend/Postgres>
#include <Wt/Dbo/FixedSqlConnectionPool>
#include <Wt/Dbo/Dbo>

#include <Wt/WApplication>
#include <Wt/WEnvironment>
#include <Wt/WLogger>

#include <cstdlib>

using namespace Wt;
namespace dbo = Wt::Dbo;

namespace {

Auth::AuthService     my_auth_service;
Auth::PasswordService my_password_service(my_auth_service);

}

void Session::configureAuth()
{
    my_auth_service.setAuthTokensEnabled(true, "gsc_cookie");

    Auth::PasswordVerifier* verifier = new Auth::PasswordVerifier();
    verifier->addHashFunction(new Auth::BCryptHashFunction(7));

    my_password_service.setVerifier(verifier);
    my_password_service.setStrengthValidator(
            new Auth::PasswordStrengthValidator());
    my_password_service.setAttemptThrottlingEnabled(true);
}

Session::Session(dbo::SqlConnectionPool& pool)
        : users_(*this)
{
    setConnectionPool(pool);

    mapClass<Assignment>("assignments");
    mapClass<Auth_token>("auth_tokens");
    mapClass<Eval_item>("eval_items");
    mapClass<File_data>("file_data");
    mapClass<File_meta>("file_meta");
    mapClass<Grader_eval>("grader_evals");
    mapClass<Partner_request>("partner_requests");
    mapClass<Self_eval>("self_evals");
    mapClass<Submission>("submissions");
    mapClass<User>("users");
    mapClass<User_stats>("user_stats");

    dbo::Transaction transaction(*this);
    try {
        createTables();

        create_index("users", "name", false);
        create_index("grader_evals", "permalink");

        Auth::User root_user = users_.registerNew();
        root_user.addIdentity(Auth::Identity::LoginName, "root");
        auto root_password = std::getenv("ADMIN_PASSWORD");
        if (root_password)
            my_password_service.updatePassword(root_user, root_password);
        users_.find(root_user).modify()->set_role(User::Role::Admin);

        Auth::User jtov = users_.registerNew();
        jtov.addIdentity(Auth::Identity::LoginName, "jtov");
        my_password_service.updatePassword(jtov, "");
        users_.find(jtov).modify()->set_role(User::Role::Admin);

        auto now = Wt::WDateTime::currentDateTime();

        auto asst1 = add(new Assignment(1, "Homework 1", 10,
                                        now.addDays(-10), now.addDays(-3),
                                        now.addDays(-1)));
        auto asst2 = add(new Assignment(2, "Homework 2", 10,
                                        now.addDays(-3),
                                        now.addDays(4),
                                        now.addDays(6)));
        auto asst3 = add(new Assignment(3, "Homework 3", 10,
                                        now.addDays(-8), now.addDays(-1),
                                        now.addDays(1)));
        auto asst4 = add(new Assignment(4, "Homework 4", 10,
                                        now.addDays(4), now.addDays(11),
                                        now.addDays(13)));

        for (auto name : std::vector<std::string>{"student", "s1", "s2", "s3"}) {
            Auth::User student = users_.registerNew();
            student.addIdentity(Auth::Identity::LoginName, name);
            my_password_service.updatePassword(student, "");

            for (auto asst : {asst1, asst2, asst3, asst4}) {
                auto submission =
                        add(new Submission(users_.find(student), asst));
                File_meta::upload("file.h", "#pragma once\n", submission);
                File_meta::upload("file.C",
                                  "#include \"file.h\"\n\nnamespace meh {\n\n}\n",
                                  submission);
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

dbo::ptr<User> Session::user() const
{
    if (login_.loggedIn()) {
        auto login_name = login_.user().identity(Auth::Identity::LoginName);
        if (!user_ || user_->name() != login_name)
            user_ = users_.find(login_.user());

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
    if (!stats) stats = add(new User_stats(u));

    stats.modify()->record_game(s);

    transaction.commit();
}

std::vector<dbo::ptr<User_stats>>
Session::top_users(int limit)
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

Auth::AbstractUserDatabase& Session::users()
{
    return users_;
}

const Auth::AuthService& Session::auth()
{
    return my_auth_service;
}

const Auth::AbstractPasswordService& Session::passwordAuth()
{
    return my_password_service;
}

dbo::SqlConnectionPool*
Session::createConnectionPool(const std::string& db)
{
    auto connection = std::make_unique<dbo::backend::Postgres>(db);
    connection->setProperty("show-queries", "true");

    return new dbo::FixedSqlConnectionPool(connection.release(), 10);
}

void Session::create_index(const char* table, const char* field, bool unique)
{
    std::ostringstream query;
    query << "CREATE ";
    if (unique) query << "UNIQUE ";
    query << "INDEX ix_" << table << '_' << field;
    query << " ON " << table << " (" << field << ')';

    execute(query.str());
}

