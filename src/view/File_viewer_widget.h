#pragma once

#include <Wt/WCompositeWidget>
#include <Wt/Dbo/ptr>

class Session;

class Submission;

namespace Wt {

class WComboBox;

}

class File_viewer_widget : public Wt::WCompositeWidget
{
public:
    File_viewer_widget(const Wt::Dbo::ptr<Submission>&,
                       Session&,
                       Wt::WContainerWidget* parent = nullptr);

    void reload();

    void scroll_to_line(int line_number) const;
    void scroll_to_file(int file_number) const;

    std::string line_id(int line_number) const;
    std::string file_id(int file_number) const;

private:
    Session& session_;
    Wt::Dbo::ptr<Submission> submission_;

    Wt::WContainerWidget* impl_;
    Wt::WComboBox* file_selector_;
    Wt::WContainerWidget* file_contents_;

    void scroll_to_id_(const std::string& id) const;
    void scroll_to_selected_file_();
};
