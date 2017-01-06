#pragma once

#include "../model/Session.h"

#include <Wt/WContainerWidget>

class Submission;

class File_manager_view : public Wt::WContainerWidget
{
public:
    File_manager_view(const Wt::Dbo::ptr<Submission>&,
                      Session&,
                      WContainerWidget* parent = nullptr);

private:
    Session& session_;
    Wt::Dbo::ptr<Submission> submission_;
};
