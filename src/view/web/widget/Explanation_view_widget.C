#include "../../../common/util.h"
#include "../../../model/util/Explanation_parser.h"
#include "Explanation_view_widget.h"
#include "File_viewer.h"

#include <xxint.hxx>

#include <Wt/WText.h>

#include <cctype>
#include <string_view>

Explanation_view_widget::Explanation_view_widget(const string &content)
    : Explanation_view_widget(content, nullptr, {}) {}

Explanation_view_widget::Explanation_view_widget(const string &content,
                                                 File_viewer *viewer,
                                                 const string &highlight_style)
    : viewer_(viewer), highlight_style_(highlight_style) {
  if (viewer_) {
    initialize_viewer_(content);
  } else {
    setNewImplementation<WText>(content, TextFormat::Plain);
  }
}

namespace {

class Html_explanation_writer : public Explanation_writer {
public:
  void plain_char(char) override;
  int link(string_view) override;
};

} // namespace

void Explanation_view_widget::initialize_viewer_(const string &content) {
  auto writer = Html_explanation_writer();
  auto highlighter = viewer_->highlighter(highlight_style_);

  parse_explanation(content, writer, highlighter);

  auto impl =
      setNewImplementation<WText>(writer.wstr(), TextFormat::UnsafeXHTML);
  impl->setInline(true);
}

void Html_explanation_writer::plain_char(char c) {
  switch (c) {
  case '<':
    raw_text_("&lt;");
    return;

  case '>':
    raw_text_("&gt;");
    return;

  case '&':
    raw_text_("&amp;");
    return;

  default:
    raw_text_(c);
    return;
  }
}

int Html_explanation_writer::link(string_view sv) {
  const char *src = sv.data();
  const char *limit = sv.data() + sv.size();
  xxint::Checked<int> result = 0;

  raw_text_("<span class=\"line-link\">");

  while (src < limit && !isdigit(*src)) {
    plain_char(*src);
    ++src;
  }

  while (src < limit && isdigit(*src)) {
    result = result * 10 + (*src - '0');
    raw_text_(*src);
    ++src;
  }

  raw_text_("</span>");

  if (src < limit) {
    plain_range(src, limit);
  }

  return result.get();
}
