#include "Explanation_view_widget.h"
#include "File_viewer_widget.h"

#include <Wt/WText.h>

#include <cctype>
#include <cstdlib>

Explanation_view_widget::Explanation_view_widget(const std::string& content)
        : Explanation_view_widget(content, nullptr, {})
{ }

Explanation_view_widget::Explanation_view_widget(const std::string& content,
                                                 File_viewer_widget* viewer,
                                                 const std::string& highlight_style)
        : viewer_(viewer),
          highlight_style_(highlight_style)
{
    if (viewer_)
        initialize_viewer_(content);
    else
        setNewImplementation<Wt::WText>(content, Wt::TextFormat::Plain);
}

void Explanation_view_widget::initialize_viewer_(const std::string& content)
{
    auto impl = setNewImplementation<Wt::WContainerWidget>();
    impl->setInline(true);

    std::string buf;
    bool in_L = false;
    int min_line = -1;

    auto emit_text = [&] {
        impl->addNew<Wt::WText>(buf, Wt::TextFormat::Plain);
        buf.clear();
    };

    auto emit_link = [&] {
        auto line = atoi(&buf[1]);

        if (min_line < 0 || line < min_line)
            min_line = line;

        viewer_->set_line_style(line, highlight_style_);

        impl->addNew<Wt::WText>(buf)->setStyleClass("line-link");
        buf.clear();
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

    if (min_line > 0)
        viewer_->show_line(min_line);
}
