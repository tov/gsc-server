#include "File_viewer_widget.h"
#include "../../../Session.h"
#include "../../../model/File_data.h"
#include "../../../model/File_meta.h"
#include "../../../model/Submission.h"

#include <Wt/Dbo/Transaction.h>
#include <Wt/WApplication.h>
#include <Wt/WComboBox.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WEnvironment.h>
#include <Wt/WTable.h>
#include <Wt/WText.h>

#include <sstream>

class Single_file_viewer : public Wt::WContainerWidget
{
public:
    Single_file_viewer(
                const Wt::Dbo::ptr<File_meta>& source_file, int& first_line_number,
                std::vector<Wt::WTableRow*>& lines,
                const File_viewer_widget* viewer);

    ~Single_file_viewer();
};

Single_file_viewer::~Single_file_viewer()
{
//    setId("");
}

Single_file_viewer::Single_file_viewer(
            const Wt::Dbo::ptr<File_meta>& source_file, int& line_number,
            std::vector<Wt::WTableRow*>& lines,
            const File_viewer_widget* viewer)
{
    auto file_name = addNew<Wt::WText>("<h4>" + source_file->name() + "</h4>");

    auto               table = addNew<Wt::WTable>();
    std::istringstream file_stream(
            std::string(source_file->file_data().lock()->contents()));
    std::string        line;
    int                row   = 0;

    while (std::getline(file_stream, line)) {
        table->elementAt(row, 0)->addNew<Wt::WText>(
                std::to_string(line_number));
        table->elementAt(row, 0)->setStyleClass("code-number");
        table->elementAt(row, 1)->addNew<Wt::WText>(
                Wt::WString::fromUTF8(line), Wt::TextFormat::Plain);
        table->elementAt(row, 1)->setStyleClass("code-line");
        table->rowAt(row)->setId(viewer->line_id(line_number));
        lines.push_back(table->rowAt(row));
        ++row;
        ++line_number;
    }
}

File_viewer_widget::File_viewer_widget(Submission_context& context)
        : Submission_context{context}
{
    impl_ = setNewImplementation<Wt::WContainerWidget>();

    file_selector_ = impl_->addNew<Wt::WComboBox>();
    file_selector_->changed().connect(this,
                                      &File_viewer_widget::scroll_to_selected_file_);
    if (!Wt::WApplication::instance()->environment().ajax())
        file_selector_->hide();

    auto scroll_area = impl_->addNew<Wt::WContainerWidget>();
    file_contents_ = scroll_area->addNew<Wt::WContainerWidget>();
    scroll_area->setOverflow(Wt::Overflow::Auto);
    scroll_area->setStyleClass("file-viewer");
    scroll_area->setId(Wt::WCompositeWidget::id() + "-area");

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

    Wt::Dbo::Transaction transaction(session().dbo());
    Source_file_vec files = submission()->source_files_sorted();

    int line_number = 1;

    for (const auto& file : files) {
        if (file->line_count() > 0) {
            file_selector_->addItem(file->name());
            file_contents_->addNew<Single_file_viewer>(
                    file, line_number, lines_, this);
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

std::string File_viewer_widget::line_id(int line_number) const
{
    return Wt::WCompositeWidget::id() + "-L" + std::to_string(line_number);
}

void File_viewer_widget::set_line_style(int line, const Wt::WString& style)
{
    if (0 < line && line < lines_.size())
        lines_[line]->setStyleClass(style);
}

void File_viewer_widget::scroll_to_id_(const std::string& target) const
{
    std::ostringstream code;

    code << "var target = $('#" << target << "');";
    code << "if (target && target.position() && target.position().top) {";
    code << "$('#" << Wt::WCompositeWidget::id() << "-area').scrollTop(0);";
    code << "$('#" << Wt::WCompositeWidget::id() << "-area').scrollTop(target.position().top)";
    code << "}";

    Wt::WApplication::instance()->doJavaScript(code.str());
}

void File_viewer_widget::scroll_to_selected_file_()
{
    scroll_to_file(file_selector_->currentIndex());
}
