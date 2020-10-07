#pragma once

#include "../../../model/Abstract_evaluation.h"

#include <Wt/WCompositeWidget.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WSignal.h>
#include <Wt/WWidget.h>

#include <vector>

class Unit_line_edit;
class Explanation_edit_widget;

class Admin_response_widget : public Wt::WCompositeWidget {
public:
  explicit Admin_response_widget(Abstract_evaluation * = nullptr);

  void load(const Abstract_evaluation *);
  bool save(Abstract_evaluation *);

  bool is_valid();
  bool is_saved();

  Wt::WContainerWidget *buttons() const { return buttons_; }

  Wt::Signal<> &changed() { return changed_; }

  void setFocus(bool focus) override;

  // Avoid hiding this by overriding.
  void load() override;

private:
  struct {
    std::string explanation;
    double score;
  } model_;

  void set_model_(const Abstract_evaluation *);

  Explanation_edit_widget *explanation_;
  Unit_line_edit *grade_;

  Wt::WContainerWidget *buttons_;

  Wt::Signal<> changed_;

  void handle_change_();
};
