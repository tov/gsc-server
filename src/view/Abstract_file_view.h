#pragma once

#include "Reloadable.h"

#include <Wt/Dbo/ptr.h>
#include <Wt/WCompositeWidget.h>
#include <Wt/WContainerWidget.h>

class File_viewer_widget;

class Submission;
class Session;

class Abstract_file_view
        : public Wt::WCompositeWidget
        , public Reloadable
{
public:
    Abstract_file_view(const Wt::Dbo::ptr<Submission>&, Session&);

    void reload() override;

    File_viewer_widget* file_viewer() { return viewer_; }

protected:
    Wt::Dbo::ptr<Submission> submission_;
    Session& session_;

    File_viewer_widget* viewer_;
    Wt::WContainerWidget* right_column_;

    // Sub-widgets will be passed a reference to this signal, and should emit
    // it to trigger a reload.
    Wt::Signal<> changed_;
};
