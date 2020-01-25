#pragma once

#include "../Submission_context.h"

#include <Wt/WCompositeWidget.h>
#include <Wt/Dbo/ptr.h>

#include <vector>

class Session;

class Submission;

namespace Wt {

class WComboBox;

}

class File_viewer_widget : public Wt::WCompositeWidget,
                           private Submission_context
{
public:
    explicit File_viewer_widget(Submission_context&);

    void show_line(int line_number) const;
    void show_file(int file_number) const;

    void set_line_style(int line, const Wt::WString& style);

    class Scroller;
    Scroller scroller(int line);

protected:
    void on_change() final override;

private:
    Wt::WContainerWidget* impl_;
    Wt::WComboBox* file_selector_;
    Wt::WContainerWidget* scroll_area_;
    Wt::WContainerWidget* file_contents_;
    // INVARIANT: each child of file_contents_ is a Single_file_viewer
    // corresponding to each item in file_selector_.

    std::vector<Wt::WTableRow*> lines_;

    void reload_();
};

class File_viewer_widget::Scroller {
public:
    Scroller(File_viewer_widget* viewer, int line)
            : viewer_(viewer), line_(line)
    { }

    void operator()();

private:
    File_viewer_widget* viewer_;
    int line_;
};

