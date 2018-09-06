#include "Abstract_file_view.h"
#include "File_viewer_widget.h"

#include <Wt/WHBoxLayout.h>

Abstract_file_view::Abstract_file_view(const Wt::Dbo::ptr<Submission>& submission,
                                       Session& session)
        : submission_(submission),
          session_(session)
{
    auto container = setNewImplementation<Wt::WContainerWidget>();
    auto hbox = container->setLayout(std::make_unique<Wt::WHBoxLayout>());

    auto viewer = std::make_unique<File_viewer_widget>(submission_, session_);
    viewer_ = hbox->addWidget(std::move(viewer));

    auto right_column = std::make_unique<Wt::WContainerWidget>();
    right_column_ = hbox->addWidget(std::move(right_column), 1);
    right_column_->setStyleClass("right-column");
}
