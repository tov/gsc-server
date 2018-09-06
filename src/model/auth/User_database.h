#pragma once

#include <Wt/Auth/AbstractUserDatabase.h>
#include <Wt/Dbo/Dbo.h>

namespace dbo = Wt::Dbo;

class User;

class User_database : public Wt::Auth::AbstractUserDatabase
{
public:
    User_database(dbo::Session&);

    static User_database* make(dbo::Session& session) {
        return new User_database(session);
    }

    virtual Transaction *startTransaction() override;

    dbo::ptr<User> find(const Wt::Auth::User& user) const;
    Wt::Auth::User find(dbo::ptr<User> user) const;

    virtual Wt::Auth::User findWithId(const std::string& id) const override;

    virtual Wt::Auth::User findWithIdentity(const std::string& provider,
                                            const Wt::WString& identity) const override;

    virtual void addIdentity(const Wt::Auth::User& user,
                             const std::string& provider,
                             const Wt::WString& identity) override;

    virtual Wt::WString identity(const Wt::Auth::User& user,
                                 const std::string& provider) const override;

    virtual void removeIdentity(const Wt::Auth::User& user,
                                const std::string& provider) override;

    virtual Wt::Auth::PasswordHash password(const Wt::Auth::User& user) const override;
    virtual void setPassword(const Wt::Auth::User& user,
                             const Wt::Auth::PasswordHash& password) override;

    virtual Wt::Auth::User::Status status(const Wt::Auth::User& user) const override;
    virtual void setStatus(const Wt::Auth::User& user,
                           Wt::Auth::User::Status status) override;

    virtual Wt::Auth::User registerNew() override;

    virtual void addAuthToken(const Wt::Auth::User& user,
                              const Wt::Auth::Token& token) override;
    virtual int updateAuthToken(const Wt::Auth::User& user,
                                const std::string& hash,
                                const std::string& newHash) override;
    virtual void removeAuthToken(const Wt::Auth::User& user,
                                 const std::string& hash) override;
    virtual Wt::Auth::User
    findWithAuthToken(const std::string& hash) const override;

    virtual int failedLoginAttempts(const Wt::Auth::User& user) const override;
    virtual void setFailedLoginAttempts(const Wt::Auth::User& user,
                                        int count) override;

    virtual Wt::WDateTime lastLoginAttempt(const Wt::Auth::User& user) const override;
    virtual void setLastLoginAttempt(const Wt::Auth::User& user,
                                     const Wt::WDateTime& t) override;

private:
    dbo::Session& session_;
    mutable dbo::ptr<User> user_cache_;

    void load_user_(const std::string& id) const;
    Wt::Auth::User cached_user_() const;

    struct With_user {
        With_user(const User_database&, const Wt::Auth::User&);
        ~With_user();

        dbo::Transaction transaction_;
    };
};