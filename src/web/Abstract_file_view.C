#include "Abstract_file_view.h"
#include "File_viewer_widget.h"
#include "../Session.h"

#include <Wt/WHBoxLayout.h>
#include <Wt/Dbo/Transaction.h>

Abstract_file_view::Abstract_file_view(const Wt::Dbo::ptr<Submission>& submission,
                                       Session& session)
        : submission_(submission),
          session_(session)
{
    auto container = setNewImplementation<Wt::WContainerWidget>();
    auto layout    = std::make_unique<Wt::WHBoxLayout>();
    auto hbox      = layout.get();
    container->setLayout(std::move(layout));

    viewer_        = hbox->addWidget(
            std::make_unique<File_viewer_widget>(submission_, session_));
    right_column_  = hbox->addWidget(
            std::make_unique<Wt::WContainerWidget>(), 1);

    right_column_->setStyleClass("right-column");

    changed_.connect(this, &Abstract_file_view::reload);
}

void Abstract_file_view::reload()
{
    Wt::Dbo::Transaction transaction(session_);
    viewer_->reload();
}
