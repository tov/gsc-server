#include "../../../Session.h"
#include "../../../common/util.h"
#include "../../../model/File_data.h"
#include "../../../model/File_meta.h"
#include "../../../model/Submission.h"
#include "File_viewer.h"

#include <Wt/Dbo/Transaction.h>
#include <Wt/WApplication.h>
#include <Wt/WComboBox.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WEnvironment.h>
#include <Wt/WTable.h>
#include <Wt/WText.h>

#include <sstream>

class Base_file_viewer : public WContainerWidget {
protected:
  Base_file_viewer(dbo::ptr<File_meta> const &, std::string file_type,
                   bool numbered);

  string contents_() const;
  WTableRow *row_at(int row_no);
  WTableCell *element_at(int row_no, int col_no);

private:
  dbo::ptr<File_meta> file_meta_;
  WTable *table_;
};

Base_file_viewer::Base_file_viewer(dbo::ptr<File_meta> const &source_file,
                                   std::string file_type, bool numbered)
    : file_meta_(source_file) {
  auto container = addNew<WContainerWidget>();
  file_type += "-single-file-viewer single-file-viewer";
  container->setStyleClass(file_type);

  table_ = container->addNew<WTable>();
  table_->setHeaderCount(1, Orientation::Horizontal);
  if (numbered) {
    table_->setHeaderCount(1, Orientation::Vertical);
  }

  auto file_head = table_->elementAt(0, 0);
  if (numbered) {
    file_head->setColumnSpan(2);
  }

  auto file_name = file_head->addNew<WText>(source_file->name());
  file_name->setStyleClass("filename");
}

string Base_file_viewer::contents_() const {
  return string(file_meta_->file_data().lock()->contents());
}

WTableRow *Base_file_viewer::row_at(int row_no) {
  return table_->rowAt(row_no + 1);
}

WTableCell *Base_file_viewer::element_at(int row_no, int col_no) {
  return table_->elementAt(row_no + 1, col_no);
}

class Html_file_viewer : public Base_file_viewer {
public:
  Html_file_viewer(dbo::ptr<File_meta> const &source_file);
};

Html_file_viewer::Html_file_viewer(dbo::ptr<File_meta> const &source_file)
    : Base_file_viewer(source_file, "html", false) {
  auto td = element_at(0, 0);
  td->addNew<WText>(contents_());
  td->setStyleClass("html-file-contents");
}

template <typename R> class Line_file_viewer : public Base_file_viewer {
public:
  using Renderer = R;

  Line_file_viewer(const dbo::ptr<File_meta> &source_file, int &line_number,
                   vector<WTableRow *> &lines, const File_viewer *viewer,
                   Renderer = Renderer{});
};

template <typename R>
Line_file_viewer<R>::Line_file_viewer(const dbo::ptr<File_meta> &source_file,
                                      int &line_number,
                                      vector<WTableRow *> &lines,
                                      const File_viewer *viewer,
                                      Renderer renderer)
    : Base_file_viewer(source_file, "line", source_file->is_line_numbered()) {
  istringstream file_stream(contents_());
  string line;
  int row_no = 1;

  while (getline(file_stream, line)) {
    auto tr = row_at(row_no);

    if (source_file->is_line_numbered()) {
      auto th = tr->elementAt(0);
      auto td = tr->elementAt(1);

      th->template addNew<WText>(to_string(line_number));
      th->setStyleClass("code-number");
      renderer(td, line);
    } else {
      auto td = tr->elementAt(0);
      renderer(td, line);
    }

    lines.push_back(tr);
    ++row_no;
    ++line_number;
  }
}

struct Plain_text_line_renderer {
  Plain_text_line_renderer() = default;

  void operator()(WTableCell *td, string const &line) const {
    td->addNew<WText>(WString::fromUTF8(line), TextFormat::Plain);
    td->setStyleClass("code-line");
  }
};

using Plain_text_file_viewer = Line_file_viewer<Plain_text_line_renderer>;

File_viewer::File_viewer(Submission_context &context)
    : Submission_context{context} {
  impl_ = setNewImplementation<WContainerWidget>();

  file_selector_ = impl_->addNew<WComboBox>();
  if (!WApplication::instance()->environment().ajax())
    file_selector_->hide();

  scroll_area_ = impl_->addNew<WContainerWidget>();

  file_contents_ = scroll_area_->addNew<WContainerWidget>();

  setStyleClass("file-viewer");
  file_selector_->setStyleClass("file-viewer-selector");
  scroll_area_->setStyleClass("file-viewer-area");

  reload_();
}

void File_viewer::on_change() { reload_(); }

void File_viewer::reload_() {
  file_selector_->clear();
  file_contents_->clear();

  lines_.clear();
  lines_.push_back(nullptr); // 1-based indexing

  dbo::Transaction transaction(session().dbo());
  Source_file_vec files = submission()->source_files_sorted();

  int line_number = 1;

  for (const auto &file : files) {
    if (file->media_type() == "text/plain") {
      file_contents_->addNew<Plain_text_file_viewer>(file, line_number, lines_,
                                                     this);
    }

    else if (file->media_type() == "text/x-html-log" ||
             file->media_type() == "text/html") {
      file_contents_->addNew<Html_file_viewer>(file);
    }

    else
      continue;

    file_selector_->addItem(file->name());
  }

  ostringstream script;
  script << "GSC.FileViewer.forId('" << id() << "')";
  doJavaScript(script.str());
}

void File_viewer::show_line(int line_number) const {
  ostringstream script;
  script << "GSC.FileViewer.forId('" << id() << "').showLine(" << line_number
         << ")";
  WApplication::instance()->doJavaScript(script.str());
}

void File_viewer::set_line_style(int line, const WString &style) {
  if (0 < line && line < (int)lines_.size())
    lines_[line]->setStyleClass(style);
}

File_viewer::Highlighter File_viewer::highlighter(Wt::WString style) {
  return Highlighter(move(style), this);
}

File_viewer::Highlighter::Highlighter(WString style, File_viewer *viewer)
    : style_(move(style)), viewer_(viewer), first_(0) {}

File_viewer::Highlighter::~Highlighter() {
  if (first_)
    viewer_->show_line(first_);
}

void File_viewer::Highlighter::highlight(int line) {
  viewer_->set_line_style(line, style_);

  if (!first_ || line < first_)
    first_ = line;
}
