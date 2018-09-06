#include "Explanation_view_widget.h"
#include "File_viewer_widget.h"

#include <Wt/WText.h>

#include <cctype>
#include <cstdlib>

Explanation_view_widget::Explanation_view_widget(const std::string& content,
                                                 Wt::WContainerWidget* parent)
        : Explanation_view_widget(content, nullptr, {}, parent)
{ }

Explanation_view_widget::Explanation_view_widget(const std::string& content,
                                                 File_viewer_widget* viewer,
                                                 const std::string& highlight_style,
                                                 Wt::WContainerWidget* parent)
        : WCompositeWidget(parent),
          viewer_(viewer),
          highlight_style_(highlight_style)
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
    int jump_line = 0;

    auto emit_text = [&]() {
        new Wt::WText(buf, Wt::PlainText, impl);
        buf.clear();
    };

    auto emit_link = [&]() {
        auto line_no = atoi(&buf[1]);
        viewer_->set_line_style(line_no, highlight_style_);
        auto link = new Wt::WText(buf, impl);
        buf.clear();

        if (jump_line == 0) jump_line = line_no;

        link->setStyleClass("line-link");
        link->clicked().connect(viewer_->scroller(line_no));
    };

    for (char c : content) {
        if (c == 'L') {
            if (in_L && buf.size() > 1)
                emit_link();
            else
                emit_text();
            in_L = true;
        } else if (isdigit(c) && in_L) {
            // Nothing -- still in_L
        } else if (in_L && buf.size() > 1) {
            emit_link();
            in_L = false;
        } else {
            in_L = false;
        }
        buf.push_back(c);
    }

    if (in_L && buf.size() > 1)
        emit_link();
    else
        emit_text();

    if (jump_line != 0)
        viewer_->scroll_to_line(jump_line);
}
