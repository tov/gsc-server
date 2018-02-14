#include "File_viewer_widget.h"
#include "../model/Session.h"
#include "../model/File_data.h"
#include "../model/File_meta.h"
#include "../model/Submission.h"

#include <Wt/Dbo/Transaction>
#include <Wt/WApplication>
#include <Wt/WComboBox>
#include <Wt/WContainerWidget>
#include <Wt/WEnvironment>
#include <Wt/WScrollArea>
#include <Wt/WTable>
#include <Wt/WText>

#include <sstream>

class Single_file_viewer : Wt::WContainerWidget
{
public:
    Single_file_viewer(const Wt::Dbo::ptr<File_meta>& source_file,
                       int file_number,
                       int& first_line_number,
                       std::vector<Wt::WTableRow*>& lines,
                       const File_viewer_widget* viewer,
                       Wt::WContainerWidget* parent = nullptr);
};

Single_file_viewer::Single_file_viewer(
        const Wt::Dbo::ptr<File_meta>& source_file,
        int file_number,
        int& line_number,
        std::vector<Wt::WTableRow*>& lines,
        const File_viewer_widget* viewer,
        Wt::WContainerWidget* parent)
        : WContainerWidget(parent)
{
    setId(viewer->file_id(file_number));

    auto file_name = new Wt::WText("<h4>" + source_file->name() + "</h4>", this);

    auto               table = new Wt::WTable(this);
    std::istringstream file_stream(source_file->file_data().lock()->contents());
    std::string        line;
    int                row   = 0;

    while (std::getline(file_stream, line)) {
        table->elementAt(row, 0)->addWidget(new Wt::WText
                                                    (boost::lexical_cast<std::string>
                                                             (line_number)));
        table->elementAt(row, 0)->setStyleClass("code-number");
        table->elementAt(row, 1)->addWidget(
                new Wt::WText(Wt::WString::fromUTF8(line), Wt::PlainText));
        table->elementAt(row, 1)->setStyleClass("code-line");
        table->rowAt(row)->setId(viewer->line_id(line_number));
        lines.push_back(table->rowAt(row));
        ++row;
        ++line_number;
    }
}

File_viewer_widget::File_viewer_widget(
        const Wt::Dbo::ptr<Submission>& submission,
        Session& session,
        Wt::WContainerWidget* parent)
        : WCompositeWidget(parent),
          session_(session),
          submission_(submission)
{
    setImplementation(impl_ = new Wt::WContainerWidget);

    file_selector_ = new Wt::WComboBox(impl_);
    file_selector_->changed().connect(this,
                                      &File_viewer_widget::scroll_to_selected_file_);
    if (!Wt::WApplication::instance()->environment().ajax())
        file_selector_->hide();

    file_contents_ = new Wt::WContainerWidget;
    auto scroll_area = new Wt::WScrollArea(impl_);
    scroll_area->setWidget(file_contents_);
    scroll_area->setStyleClass("file-viewer");
    scroll_area->setId(id() + "-area");

    reload();
}

void File_viewer_widget::reload()
{
    file_selector_->clear();
    file_contents_->clear();

    lines_.clear();
    lines_.push_back(nullptr); // 1-based indexing

    Wt::Dbo::Transaction transaction(session_.dbo());
    Source_file_vec      files = submission_->source_files_sorted();

    for (const auto& file : files) {
        file_selector_->addItem(file->name());
    }

    int file_number = 0;
    int line_number = 1;

    for (const auto& file : files) {
        new Single_file_viewer(file, file_number++, line_number, lines_,
                               this, file_contents_);
    }
}

void File_viewer_widget::scroll_to_line(int line_number) const
{
    scroll_to_id_(line_id(line_number));
}

void File_viewer_widget::scroll_to_file(int file_number) const
{
    scroll_to_id_(file_id(file_number));
}

std::string File_viewer_widget::line_id(int line_number) const
{
    return id() + "-L" + boost::lexical_cast<std::string>(line_number);
}

std::string File_viewer_widget::file_id(int file_number) const
{
    return id() + "-F" + boost::lexical_cast<std::string>(file_number);
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
    code << "if (target) {";
    code << "$('#" << id() << "-area').scrollTop(0);";
    code << "$('#" << id() << "-area').scrollTop(target.position().top)";
    code << "}";

    Wt::WApplication::instance()->doJavaScript(code.str());
}

void File_viewer_widget::scroll_to_selected_file_()
{
    scroll_to_file(file_selector_->currentIndex());
}

