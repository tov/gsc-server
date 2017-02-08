#pragma once

#include <Wt/Dbo/Dbo>
#include <Wt/WContainerWidget>

#include <memory>

class Session;
class User;

struct Submissions_view_model;
struct Submissions_view_row;

class Submissions_view : public Wt::WContainerWidget
{
public:
    Submissions_view(const Wt::Dbo::ptr<User>&, Session&,
                     Wt::WContainerWidget* parent = nullptr);

private:
    Session& session_;
    std::unique_ptr<Submissions_view_model> model_;
    std::vector<std::unique_ptr<Submissions_view_row>> rows_;
};