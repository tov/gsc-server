#pragma once

#include <Wt/WCompositeWidget.h>
#include <Wt/WContainerWidget.h>

#include <string>

class File_viewer;

// Displays a student's self-eval explanations, with links to line tags.
class Explanation_view_widget : public Wt::WCompositeWidget {
public:
  explicit Explanation_view_widget(const std::string &content);

  Explanation_view_widget(const std::string &content, File_viewer *viewer,
                          const std::string &highlight_style);

private:
  File_viewer *const viewer_;
  const std::string highlight_style_;

  void initialize_viewer_(const std::string &content);
};