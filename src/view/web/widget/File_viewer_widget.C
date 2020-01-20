#include "File_viewer_widget.h"
#include "../../../Session.h"
#include "../../../model/File_data.h"
#include "../../../model/File_meta.h"
#include "../../../model/Submission.h"
#include "../../../common/util.h"

#include <Wt/Dbo/Transaction.h>
#include <Wt/WApplication.h>
#include <Wt/WComboBox.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WEnvironment.h>
#include <Wt/WTable.h>
#include <Wt/WText.h>

#include <sstream>

class Base_file_viewer : public WContainerWidget
{
protected:
    Base_file_viewer(dbo::ptr<File_meta> const&,
                     const char* style_class,
                     bool numbered);

    string contents_() const;
    WTableRow* row_at(int row_no);
    WTableCell* element_at(int row_no, int col_no);

private:
    dbo::ptr<File_meta> file_meta_;
    WTable* table_;
};

Base_file_viewer::Base_file_viewer(
        dbo::ptr<File_meta> const& source_file,
        const char* style_class,
        bool numbered)
        : file_meta_(source_file)
{
    auto container = addNew<WContainerWidget>();
    container->setStyleClass(style_class);

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

string Base_file_viewer::contents_() const
{
    return string(file_meta_->file_data().lock()->contents());
}

WTableRow* Base_file_viewer::row_at(int row_no)
{
    return table_->rowAt(row_no + 1);
}

WTableCell* Base_file_viewer::element_at(int row_no, int col_no)
{
    return table_->elementAt(row_no + 1, col_no);
}

class Html_file_viewer : public Base_file_viewer
{
public:
    Html_file_viewer(dbo::ptr<File_meta> const& source_file);
};

Html_file_viewer::Html_file_viewer(dbo::ptr<File_meta> const& source_file)
        : Base_file_viewer(source_file, "html-file-viewer", false)
{
    auto td = element_at(0, 0);
    td->addNew<WText>(contents_());
    td->setStyleClass("html-file-contents");
}

template <typename R>
class Line_file_viewer : public Base_file_viewer
{
public:
    using Renderer = R;

    Line_file_viewer(
            const dbo::ptr<File_meta>& source_file,
            int& line_number,
            vector<WTableRow*>& lines,
            const File_viewer_widget* viewer,
            Renderer = Renderer{});
};

template <typename R>
Line_file_viewer<R>::Line_file_viewer(
        const dbo::ptr<File_meta>& source_file,
        int& line_number,
        vector<WTableRow*>& lines,
        const File_viewer_widget* viewer,
        Renderer renderer)
        : Base_file_viewer(source_file, "line-file-viewer", true)
{
    istringstream file_stream(contents_());
    string        line;
    int           row_no = 1;

    while (getline(file_stream, line)) {
        auto tr = row_at(row_no);
        auto th = tr->elementAt(0);
        auto td = tr->elementAt(1);

        tr->setId(viewer->line_id(line_number));
        th->template addNew<WText>(to_string(line_number));
        th->setStyleClass("code-number");
        renderer(td, line);

        lines.push_back(tr);
        ++row_no;
        ++line_number;
    }
}

struct Plain_text_line_renderer
{
    Plain_text_line_renderer() = default;

    void operator()(WTableCell* td, string const& line) const
    {
        td->addNew<WText>(WString::fromUTF8(line), TextFormat::Plain);
        td->setStyleClass("code-line");
    }
};

struct Hlog_line_renderer
{
    Hlog_line_renderer() = default;

    void operator()(WTableCell* td, string const& line) const
    {
        td->addNew<WText>(WString::fromUTF8(line), TextFormat::XHTML);
        td->setStyleClass("hlog-line");
    }
};

using Plain_text_file_viewer = Line_file_viewer<Plain_text_line_renderer>;

using Hlog_file_viewer = Line_file_viewer<Hlog_line_renderer>;

File_viewer_widget::File_viewer_widget(Submission_context& context)
        : Submission_context{context}
{
    impl_ = setNewImplementation<WContainerWidget>();

    file_selector_ = impl_->addNew<WComboBox>();
    file_selector_->changed().connect(this,
                                      &File_viewer_widget::scroll_to_selected_file_);
    if (!WApplication::instance()->environment().ajax())
        file_selector_->hide();

    auto scroll_area = impl_->addNew<WContainerWidget>();
    file_contents_ = scroll_area->addNew<WContainerWidget>();
    scroll_area->setOverflow(Overflow::Auto, Orientation::Vertical);
    scroll_area->setId(WCompositeWidget::id() + "-area");

    setStyleClass("file-viewer");
    scroll_area->setStyleClass("file-viewer-area");

    reload_();
}

void File_viewer_widget::on_change()
{
    reload_();
}

void File_viewer_widget::reload_()
{
    file_selector_->clear();
    file_contents_->clear();

    lines_.clear();
    lines_.push_back(nullptr); // 1-based indexing

    dbo::Transaction transaction(session().dbo());
    Source_file_vec files = submission()->source_files_sorted();

    int line_number = 1;

    for (const auto& file : files) {
        if (file->media_type() == "text/plain") {
            file_selector_->addItem(file->name());
            file_contents_->addNew<Plain_text_file_viewer>(
                    file, line_number, lines_, this);
        }

        else if (file->media_type() == "text/x-html-log") {
            file_selector_->addItem(file->name());
            file_contents_->addNew<Hlog_file_viewer>(
                    file, line_number, lines_, this);
        }

        else if (file->media_type() == "text/html") {
            file_selector_->addItem(file->name());
            file_contents_->addNew<Html_file_viewer>(file);
        }
    }
}

void File_viewer_widget::scroll_to_line(int line_number) const
{
    scroll_to_id_(line_id(line_number));
}

void File_viewer_widget::scroll_to_file(int file_number) const
{
    scroll_to_id_(file_contents_->children().at(file_number)->id());
}

string File_viewer_widget::line_id(int line_number) const
{
    return WCompositeWidget::id() + "-L" + to_string(line_number);
}

void File_viewer_widget::set_line_style(int line, const WString& style)
{
    if (0 < line && line < lines_.size())
        lines_[line]->setStyleClass(style);
}

void File_viewer_widget::scroll_to_id_(const string& target) const
{
    ostringstream code;

    code << "var target = $('#" << target << "');";
    code << "if (target && target.position() && target.position().top) {";
    code << "$('#" << WCompositeWidget::id() << "-area').scrollTop(0);";
    code << "$('#" << WCompositeWidget::id() << "-area').scrollTop(target.position().top)";
    code << "}";

    WApplication::instance()->doJavaScript(code.str());
}

void File_viewer_widget::scroll_to_selected_file_()
{
    scroll_to_file(file_selector_->currentIndex());
}
