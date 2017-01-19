#include "Explanation_view_widget.h"
#include "File_viewer_widget.h"

#include <Wt/WText>

#include <cctype>
#include <cstdlib>

Explanation_view_widget::Explanation_view_widget(const std::string& content,
                                                 Wt::WContainerWidget* parent)
        : Explanation_view_widget(content, nullptr, parent)
{ }

Explanation_view_widget::Explanation_view_widget(const std::string& content,
                                                 File_viewer_widget* viewer,
                                                 Wt::WContainerWidget* parent)
        : WCompositeWidget(parent),
          viewer_(viewer)
{
    if (viewer_)
        initialize_viewer_(content);
    else
        setImplementation(new Wt::WText(content, Wt::PlainText));
}

void Explanation_view_widget::initialize_viewer_(const std::string& content)
{
    auto impl = new Wt::WContainerWidget;
    impl->setInline(true);
    setImplementation(impl);

    std::string buf;
    bool in_L = false;

    auto emit_text = [&]() {
        new Wt::WText(buf, Wt::PlainText, impl);
        buf.clear();
    };

    auto emit_link = [&]() {
        auto line_no = atoi(&buf[1]);
        viewer_->set_line_style(line_no, "student-highlight");
        auto link = new Wt::WText(buf, impl);
        buf.clear();

        link->setStyleClass("line-link");
        link->clicked().connect(viewer_->scroller(line_no));
    };

    for (char c : content) {
        if (c == 'L') {
            emit_text();
            buf.push_back(c);
            in_L = true;
        } else if (isdigit(c) && in_L) {
            buf.push_back(c);
        } else if (in_L && buf.size() > 1) {
            emit_link();
        } else {
            buf.push_back(c);
            in_L = false;
        }
    }

    if (in_L && buf.size() > 1)
        emit_link();
    else
        emit_text();
}
