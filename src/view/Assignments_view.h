#pragma once

#include "../model/Session.h"

#include <Wt/Dbo/ptr>
#include <Wt/WContainerWidget>
#include <Wt/WDialog>
#include <Wt/WTable>

#include <memory>
#include <vector>

class Assignments_view_row;

class Assignments_view : public Wt::WContainerWidget
{
public:
    Assignments_view(Session& session,
                     Wt::WContainerWidget* parent = nullptr);

private:
    Session& session_;
    std::vector<std::unique_ptr<Assignments_view_row>> rows_;
    Wt::WTable* table_;

    void more_();

    void fewer_();
    void real_fewer_();
};