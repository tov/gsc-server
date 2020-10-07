#pragma once

#include "../../../model/util/Explanation_parser.h"
#include "../Submission_context.h"

#include <Wt/Dbo/ptr.h>
#include <Wt/WCompositeWidget.h>

#include <vector>

class Session;

class Submission;

namespace Wt {

class WComboBox;

}

class File_viewer : public Wt::WCompositeWidget, private Submission_context {
public:
  explicit File_viewer(Submission_context &);

  void show_line(int line_number) const;

  void set_line_style(int line, const Wt::WString &style);

  class Highlighter;

  Highlighter highlighter(Wt::WString style);

protected:
  void on_change() final override;

private:
  Wt::WContainerWidget *impl_;
  Wt::WComboBox *file_selector_;
  Wt::WContainerWidget *scroll_area_;
  Wt::WContainerWidget *file_contents_;
  // INVARIANT: each child of file_contents_ is a Single_file_viewer
  // corresponding to each item in file_selector_.

  std::vector<Wt::WTableRow *> lines_;

  void reload_();
};

class File_viewer::Highlighter : public Code_highlighter {
public:
  void highlight(int line) override;

  ~Highlighter() override;

private:
  friend File_viewer;

  Highlighter(Wt::WString style, File_viewer *viewer);

  Wt::WString const style_;
  File_viewer *viewer_;
  int first_;
};
