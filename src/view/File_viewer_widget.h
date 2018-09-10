#pragma once

#include "Reloadable.h"

#include <Wt/WCompositeWidget.h>
#include <Wt/Dbo/ptr.h>

#include <vector>

class Session;

class Submission;

namespace Wt {

class WComboBox;

}

class File_viewer_widget : public Wt::WCompositeWidget, public Reloadable
{
public:
    File_viewer_widget(const Wt::Dbo::ptr<Submission>&, Session&);

    void reload() override;

    void scroll_to_line(int line_number) const;
    void scroll_to_file(int file_number) const;

    std::string line_id(int line_number) const;
    std::string file_id(int file_number) const;

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

private:
    Session& session_;
    Wt::Dbo::ptr<Submission> submission_;

    Wt::WContainerWidget* impl_;
    Wt::WComboBox* file_selector_;
    Wt::WContainerWidget* file_contents_;

    std::vector<Wt::WTableRow*> lines_;

    void scroll_to_id_(const std::string& id) const;
    void scroll_to_selected_file_();
};
