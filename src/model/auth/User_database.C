// This is based on the blog example from the Wt source code.

#include "User_database.h"
#include "Auth_token.h"
#include "User.h"

#include <Wt/Auth/Identity.h>

#include <cstdlib>
#include <string>

using namespace Wt;

class Invalid_user : public std::runtime_error
{
public:
    Invalid_user(const std::string& id)
            : std::runtime_error("Invalid user: " + id) {}
};

User_database::User_database(dbo::Session& session)
        : session_(session) {}

void User_database::load_user_(const std::string& id_string) const
{
    long long id = std::atoll(id_string.c_str());

    if (user_cache_ && user_cache_.id() == id)
        return;

    dbo::Transaction t(session_);
    user_cache_ = session_.find<User>()
                          .where("id = ?").bind(id);
    t.commit();
}

Auth::User User_database::cached_user_() const
{
    if (user_cache_)
        return Auth::User(boost::lexical_cast<std::string>(user_cache_.id()),
                          *this);
    else
        return Auth::User();
}

User_database::With_user::With_user(const User_database& self,
                                    const Auth::User& user)
        : transaction_(self.session_)
{
    self.load_user_(user.id());
    if (!self.user_cache_) throw Invalid_user(user.id());
}

User_database::With_user::~With_user()
{
    transaction_.commit();
}

class TransactionImpl : public Auth::AbstractUserDatabase::Transaction,
                        public dbo::Transaction
{
public:
    TransactionImpl(dbo::Session& session)
            : dbo::Transaction(session) {}

    virtual ~TransactionImpl() override {}

    virtual void commit() override
    { dbo::Transaction::commit(); }

    virtual void rollback() override
    { dbo::Transaction::rollback(); }
};

Auth::AbstractUserDatabase::Transaction* User_database::startTransaction()
{
    return new TransactionImpl(session_);
}

dbo::ptr <User> User_database::find(const Auth::User& user) const
{
    load_user_(user.id());
    return user_cache_;
}

Auth::User User_database::find(const dbo::ptr<User> user) const
{
    user_cache_ = user;
    return cached_user_();
}

Auth::User User_database::findWithId(const std::string& id) const
{
    load_user_(id);
    return cached_user_();
}

Auth::PasswordHash User_database::password(const Auth::User& user) const
{
    With_user find(*this, user);
    return user_cache_->password();
}

void User_database::setPassword(const Auth::User& user,
                                const Auth::PasswordHash& password)
{
    With_user find(*this, user);
    user_cache_.modify()->set_password(password);
}

Auth::User::Status User_database::status(const Auth::User&) const
{
    return Auth::User::Status::Normal;
}

void User_database::setStatus(const Auth::User& user,
                              Auth::User::Status status)
{
    throw std::runtime_error("Changing status is not supported.");
}

void User_database::addIdentity(const Auth::User& user,
                                const std::string& provider,
                                const Wt::WString& identity)
{
    With_user find(*this, user);

    if (provider == Auth::Identity::LoginName)
        user_cache_.modify()->set_name(identity.toUTF8());
    else {
        throw std::runtime_error("OAuth is not supported.");
    }
}

Wt::WString User_database::identity(const Auth::User& user,
                                    const std::string& provider) const
{
    With_user find(*this, user);

    if (provider == Auth::Identity::LoginName)
        return user_cache_->name();
    else
        return WString::Empty;
}

void User_database::removeIdentity(const Auth::User& user,
                                   const std::string& provider)
{
    With_user find(*this, user);

    if (provider == Auth::Identity::LoginName)
        user_cache_.modify()->set_name("");
}

Auth::User User_database::findWithIdentity(const std::string& provider,
                                           const WString& identity) const
{
    dbo::Transaction t(session_);
    if (provider == Auth::Identity::LoginName &&
            (!user_cache_ || user_cache_->name() != identity))
        user_cache_ = User::find_by_name(session_, identity.toUTF8());
    t.commit();

    return cached_user_();
}

Auth::User User_database::registerNew()
{
    user_cache_ = session_.addNew<User>();
    user_cache_.flush();
    return Auth::User(boost::lexical_cast<std::string>(user_cache_.id()),
                      *this);
}

void User_database::addAuthToken(const Auth::User& user,
                                 const Auth::Token& token)
{
    With_user find(*this, user);

    if (session_.find<Auth_token>().where("value = ?")
                .bind(token.hash()).resultList().size() > 0)
        throw std::runtime_error("Token hash collision");

    user_cache_.modify()->add_auth_token(token.hash(),
                                         token.expirationTime());
}

int User_database::updateAuthToken(const Auth::User& user,
                                   const std::string& hash,
                                   const std::string& new_hash)
{
    With_user find(*this, user);
    return user_cache_.modify()->update_auth_token(hash, new_hash);
}

void User_database::removeAuthToken(const Auth::User& user,
                                    const std::string& hash)
{
    With_user find(*this, user);
    user_cache_.modify()->remove_auth_token(hash);
}

Auth::User User_database::findWithAuthToken(const std::string& hash) const
{
    dbo::Transaction t(session_);
    user_cache_ = User::find_by_auth_token(session_, hash);
    t.commit();

    return cached_user_();
}

int User_database::failedLoginAttempts(const Auth::User& user) const
{
    With_user find(*this, user);

    return user_cache_->failed_login_attempts();
}

void User_database::setFailedLoginAttempts(const Auth::User& user,
                                           int count)
{
    With_user find(*this, user);

    user_cache_.modify()->set_failed_login_attempts(count);
}

WDateTime User_database::lastLoginAttempt(const Auth::User& user) const
{
    With_user find(*this, user);

    return user_cache_->last_login_attempt();
}

void User_database::setLastLoginAttempt(const Auth::User& user,
                                        const WDateTime& t)
{
    With_user find(*this, user);

    user_cache_.modify()->set_last_login_attempt(t);
}

