#include "../../../Session.h"
#include "../../../common/format.h"
#include "../../../model/File_data.h"
#include "../../../model/File_meta.h"
#include "../../../model/Submission.h"
#include "File_list_widget.h"
#include "Glyph_button.h"

#include <Wt/Dbo/Transaction.h>
#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>
#include <Wt/WAnchor.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLink.h>
#include <Wt/WTable.h>
#include <Wt/WText.h>

// A resource for downloading a source file.
class File_resource : public Wt::WResource {
public:
  explicit File_resource(const Wt::Dbo::ptr<File_meta> &);
  ~File_resource();

  void handleRequest(const Wt::Http::Request &, Wt::Http::Response &) override;

private:
  Wt::Dbo::ptr<File_meta> source_file_;
};

// Action for deleting a source file.
class File_deleter : public Wt::WObject {
public:
  File_deleter(const Wt::Dbo::ptr<File_meta> &, Submission_context &);

  void go();

private:
  Wt::Dbo::ptr<File_meta> source_file_;
  Submission_context &context_;
};

File_list_widget::File_list_widget(const Wt::Dbo::ptr<Submission> &submission,
                                   bool can_modify, Session &session,
                                   Submission_change_signal &changed)
    : Submission_context{changed, session, submission},
      can_modify_(can_modify) {
  auto impl = setNewImplementation<Wt::WContainerWidget>();
  inner_ = impl->addNew<Wt::WTable>();
  inner_->setHeaderCount(1, Wt::Orientation::Horizontal);
  inner_->setHeaderCount(1, Wt::Orientation::Vertical);
  setStyleClass("file-list");
  reload_();
}

void File_list_widget::reload_() {
  inner_->clear();

  Wt::Dbo::Transaction transaction(session());

  auto const source_files = submission()->source_files_sorted();

  if (source_files.empty())
    return;

  cell_(0, 0)->addNew<Wt::WText>("Filename");
  cell_(0, 1)->addNew<Wt::WText>("Type");
  cell_(0, 2)->addNew<Wt::WText>("Bytes");
  if (can_modify_)
    cell_(0, 3)->addNew<Wt::WText>("X")->setAttributeValue("aria-label",
                                                           "Delete file");

  int row = 1;

  for (auto const &file : source_files) {
    auto download = std::make_shared<File_resource>(file);

    auto anchor =
        cell_(row, 0)->addNew<Wt::WAnchor>(Wt::WLink(download), file->name());
    auto type = cell_(row, 1)->addNew<Wt::WText>(stringify(file->purpose()));
    auto bytes =
        cell_(row, 2)->addNew<Wt::WText>(with_commas(file->byte_count()));

    cell_(row, 0)->setStyleClass("filename");
    cell_(row, 1)->setStyleClass("file-list-type");
    cell_(row, 2)->setStyleClass("numeric");

    anchor->setToolTip("view or download");
    type->setToolTip("file purpose");
    bytes->setToolTip("file size in bytes");

    if (can_modify_) {
      auto remove =
          cell_(row, 3)->addNew<Glyph_button>("trash", "Delete file", true);
      remove->setStyleClass("btn btn-danger");

      auto deleter =
          remove->addChild(std::make_unique<File_deleter>(file, context()));
      remove->clicked().connect(deleter, &File_deleter::go);
    }

    ++row;
  }
}

Wt::WTableCell *File_list_widget::cell_(int row, int column) {
  return inner_->elementAt(row, column);
}

File_resource::File_resource(const Wt::Dbo::ptr<File_meta> &source_file)
    : source_file_(source_file) {
  suggestFileName(source_file_->name(), Wt::ContentDisposition::Inline);
}

File_resource::~File_resource() { beingDeleted(); }

void File_resource::handleRequest(const Wt::Http::Request &request,
                                  Wt::Http::Response &response) {
  Wt::Dbo::Transaction transaction(*source_file_.session());
  Wt::Dbo::ptr<File_data> text = source_file_->file_data().lock();
  transaction.commit();

  response.setMimeType(source_file_->media_type());
  text->contents().write(response.out());
}

File_deleter::File_deleter(const Wt::Dbo::ptr<File_meta> &source_file,
                           Submission_context &context)
    : source_file_(source_file), context_(context) {}

void File_deleter::go() {
  Wt::Dbo::Transaction transaction(context_.session());
  if (source_file_->submission()->can_submit(context_.session().user())) {
    source_file_->submission().modify()->touch();
    source_file_.remove();
    transaction.commit();

    context_.notify();
  }
}
