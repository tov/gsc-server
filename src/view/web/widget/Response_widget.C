#include "Response_widget.h"

#include <Wt/WButtonGroup.h>
#include <Wt/WRadioButton.h>
#include <Wt/WSlider.h>
#include <Wt/WTextArea.h>

#include <optional>

Response_widget::Response_widget() {
  container_ = setNewImplementation<Wt::WContainerWidget>();
}

void Response_widget::freeze() { disable(); }

class Explanation_response_widget : public Response_widget {
public:
  Explanation_response_widget();

  bool has_changed() const override;

  std::string explanation() const override;
  void set_explanation(const std::string &string) override;

  void set_initial_value(double) override;
  void set_initial_explanation(const std::string &) override;

protected:
  bool explanation_has_changed() const;
  bool explanation_is_empty() const;
  void toggle_explanation();

  virtual std::optional<double> optional_value() const = 0;

  template <class T, class... Args> T *addNew(Args &&... args) {
    return score_->addNew<T>(std::forward<Args>(args)...);
  }

private:
  Wt::WContainerWidget *score_;
  Wt::WContainerWidget *explanation_holder_;
  Wt::WTextArea *explanation_;

  std::optional<double> initial_value_;
  Wt::WString initial_explanation_;
};

Explanation_response_widget::Explanation_response_widget() : Response_widget() {
  score_ = container_->addNew<Wt::WContainerWidget>();
  score_->setStyleClass("score");

  explanation_holder_ = container_->addNew<Wt::WContainerWidget>();

  explanation_holder_->addNew<Wt::WText>(
      "<p>Explain (using line references like <em>L14</em>):</p>");

  explanation_ = explanation_holder_->addNew<Wt::WTextArea>();
  explanation_->setStyleClass("explanation");
  explanation_->setInline(false);
  explanation_->changed().connect([=] { signal_change(); });
  explanation_->keyWentUp().connect([=] { signal_change(); });
}

bool Explanation_response_widget::has_changed() const {
  return initial_value_ != optional_value() || explanation_has_changed();
}

bool Explanation_response_widget::explanation_has_changed() const {
  return initial_explanation_ != explanation_->text();
}

std::string Explanation_response_widget::explanation() const {
  return explanation_->text().toUTF8();
}

void Explanation_response_widget::set_explanation(const std::string &s) {
  explanation_->setText(s);
}

void Explanation_response_widget::set_initial_value(double v) {
  set_value(v);
  initial_value_ = value();
}

void Explanation_response_widget::set_initial_explanation(
    const std::string &expl) {
  set_explanation(expl);
  initial_explanation_ = explanation_->text();
}

bool Explanation_response_widget::explanation_is_empty() const {
  return explanation_->text().empty();
}

void Explanation_response_widget::toggle_explanation() {
  explanation_holder_->setHidden(value() == 0.0);
}

class Boolean_response_widget : public Explanation_response_widget {
public:
  Boolean_response_widget();

  bool is_valid() const override;
  double value() const override;
  void set_value(double d) override;
  void freeze_value() override;

protected:
  std::optional<double> optional_value() const override;

private:
  std::shared_ptr<Wt::WButtonGroup> no_yes_;
  Wt::WRadioButton *no_;
  Wt::WRadioButton *yes_;
};

Boolean_response_widget::Boolean_response_widget() {
  no_yes_ = std::make_shared<Wt::WButtonGroup>();
  no_ = addNew<Wt::WRadioButton>("No");
  yes_ = addNew<Wt::WRadioButton>("Yes");
  no_yes_->addButton(no_);
  no_yes_->addButton(yes_);

  no_->setFocus();

  no_yes_->checkedChanged().connect([=] { toggle_explanation(); });
  no_yes_->checkedChanged().connect([=] { signal_change(); });

  toggle_explanation();
}

bool Boolean_response_widget::is_valid() const {
  auto checked = no_yes_->checkedButton();
  return checked == no_ || (checked == yes_ && !explanation_is_empty());
}

double Boolean_response_widget::value() const {
  return optional_value().value_or(0.0);
}

void Boolean_response_widget::set_value(double d) {
  no_yes_->setCheckedButton(d == 1.0 ? yes_ : no_);
  toggle_explanation();
}

void Boolean_response_widget::freeze_value() {
  no_->disable();
  yes_->disable();
}

std::optional<double> Boolean_response_widget::optional_value() const {
  auto checked = no_yes_->checkedButton();

  if (checked == yes_)
    return {1.0};

  if (checked == no_)
    return {0.0};

  return std::nullopt;
}

class Scale_response_widget : public Explanation_response_widget {
public:
  Scale_response_widget();

  bool is_valid() const override;

  double value() const override;
  void set_value(double d) override;
  void freeze_value() override;

protected:
  std::optional<double> optional_value() const override;

private:
  Wt::WSlider *slider_;
  bool has_changed_ = false;

  void slider_change_();
};

Scale_response_widget::Scale_response_widget() {
  slider_ = addNew<Wt::WSlider>();
  slider_->resize(200, 50);
  slider_->setTickPosition(Wt::WSlider::TickPosition::TicksAbove);
  slider_->setTickInterval(20);
  slider_->setMinimum(0);
  slider_->setMaximum(100);
  slider_->setValue(50);

  slider_->valueChanged().connect([=] { slider_change_(); });
}

bool Scale_response_widget::is_valid() const {
  return has_changed_ || explanation_has_changed();
}

double Scale_response_widget::value() const { return slider_->value() / 100.0; }

void Scale_response_widget::set_value(double d) {
  has_changed_ = true;
  slider_->setValue(int(100 * d));
}

void Scale_response_widget::freeze_value() { slider_->disable(); }

void Scale_response_widget::slider_change_() {
  has_changed_ = true;
  signal_change();
}

std::optional<double> Scale_response_widget::optional_value() const {
  if (has_changed_)
    return {value()};
  else
    return std::nullopt;
}

class Informational_response_widget : public Response_widget {
public:
  Informational_response_widget();

  bool is_valid() const override;
  bool has_changed() const override;

  double value() const override;
  void set_value(double) override;
  std::string explanation() const override;
  void set_explanation(const std::string &) override;
  void freeze_value() override;
};

Informational_response_widget::Informational_response_widget()
    : Response_widget() {}

bool Informational_response_widget::has_changed() const { return true; }

bool Informational_response_widget::is_valid() const { return true; }

double Informational_response_widget::value() const { return 1.0; }

void Informational_response_widget::set_value(double) {}

std::string Informational_response_widget::explanation() const { return ""; }

void Informational_response_widget::set_explanation(const std::string &) {}

void Informational_response_widget::freeze_value() {}

std::unique_ptr<Response_widget> Response_widget::create(Eval_item::Type type) {
  switch (type) {
  case Eval_item::Type::Boolean:
    return std::make_unique<Boolean_response_widget>();
  case Eval_item::Type::Scale:
    return std::make_unique<Scale_response_widget>();
  case Eval_item::Type::Informational:
    return std::make_unique<Informational_response_widget>();
  }
}

void Response_widget::signal_change() { changed().emit(); }
