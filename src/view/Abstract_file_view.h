#pragma once

#include <Wt/dbo/ptr>
#include <Wt/WCompositeWidget>
#include <Wt/WContainerWidget>

class File_viewer_widget;

class Submission;
class Session;

class Abstract_file_view : public Wt::WCompositeWidget
{
public:
    Abstract_file_view(const Wt::Dbo::ptr<Submission>&,
                       Session&,
                       Wt::WContainerWidget* parent = nullptr);

protected:
    Wt::Dbo::ptr<Submission> submission_;
    Session& session_;

    File_viewer_widget* const viewer_;
    Wt::WContainerWidget* const right_column_;
};
