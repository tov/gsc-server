#include "Abstract_file_view.h"
#include "File_viewer_widget.h"

#include <Wt/WHBoxLayout>

Abstract_file_view::Abstract_file_view(
        const Wt::Dbo::ptr<Submission>& submission,
        Session& session,
        Wt::WContainerWidget* parent)
        : WCompositeWidget(parent),
          submission_(submission),
          session_(session),
          viewer_(new File_viewer_widget(submission_, session_)),
          right_column_(new Wt::WContainerWidget)
{
    auto container = new Wt::WContainerWidget;
    setImplementation(container);

    auto hbox = new Wt::WHBoxLayout();
    container->setLayout(hbox);

    hbox->addWidget(viewer_);
    hbox->addWidget(right_column_, 1);

    right_column_->setStyleClass("right-column");
}
