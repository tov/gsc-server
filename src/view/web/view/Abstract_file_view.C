#include "../widget/File_viewer.h"
#include "Abstract_file_view.h"

#include <Wt/Dbo/Transaction.h>
#include <Wt/WHBoxLayout.h>

Abstract_file_view::Abstract_file_view(
    const Wt::Dbo::ptr<Submission> &submission, Session &session)
    : Submission_context_root{session, submission} {
  auto container = setNewImplementation<Wt::WContainerWidget>();

  auto layout = std::make_unique<Wt::WHBoxLayout>();
  auto viewer = std::make_unique<File_viewer>(context());
  auto right_column = std::make_unique<Wt::WContainerWidget>();

  right_column->setStyleClass("right-column");

  viewer_ = viewer.get();
  right_column_ = right_column.get();

  layout->addWidget(std::move(viewer));
  layout->addWidget(std::move(right_column), 1);
  container->setLayout(std::move(layout));
}
