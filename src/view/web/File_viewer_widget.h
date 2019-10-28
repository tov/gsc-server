#pragma once

#include "Submission_context.h"

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

    void scroll_to_line(int line_number) const;
    void scroll_to_file(int file_number) const;

    std::string line_id(int line_number) const;

    void set_line_style(int line, const Wt::WString& style);

    class Scroller {
    public:
        Scroller(File_viewer_widget* viewer, int line)
                : viewer_(viewer), line_(line)
        { }

        void operator()() { viewer_->scroll_to_line(line_); }

    private:
        File_viewer_widget* viewer_;
        int line_;
    };

    Scroller scroller(int line)
    {
        return Scroller(this, line);
    }

protected:
    void on_change() final override;

private:
    Wt::WContainerWidget* impl_;
    Wt::WComboBox* file_selector_;
    Wt::WContainerWidget* file_contents_;
    // INVARIANT: each child of file_contents_ is a Single_file_viewer
    // corresponding to each item in file_selector_.

    std::vector<Wt::WTableRow*> lines_;

    void reload_();

    void scroll_to_id_(const std::string& id) const;
    void scroll_to_selected_file_();
};
