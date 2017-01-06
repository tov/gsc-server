#pragma once

#include "model/Session.h"

#include <Wt/Dbo/ptr>
#include <Wt/Dbo/SqlConnectionPool>
#include <Wt/WApplication>

#include <string>

class Main_view;

class User;
class Assignment;

class Application_controller : public Wt::WApplication
{
public:
    static Application_controller*
    create(Wt::Dbo::SqlConnectionPool*,
               const Wt::WEnvironment&);

private:
    Session session_;
    Main_view* main_;

    Application_controller(Wt::Dbo::SqlConnectionPool*,
                               const Wt::WEnvironment&);

    void handle_internal_path(const std::string&);
    void on_auth_event();

    Wt::Dbo::ptr<User> find_user(const std::string&);
    Wt::Dbo::ptr<Assignment> find_assignment(const std::string&);
};
