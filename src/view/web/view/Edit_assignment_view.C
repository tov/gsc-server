#include "../../../Session.h"
#include "../../../common/util.h"
#include "../../../model/Assignment.h"
#include "../Confirmation_dialog.h"
#include "../widget/Eval_item_type_chooser.h"
#include "Edit_assignment_view.h"

#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>
#include <Wt/WTextArea.h>

#include <cstdlib>

class Edit_eval_item : public Wt::WContainerWidget {
public:
  Edit_eval_item(const dbo::ptr<Eval_item> &, Edit_assignment_view &main,
                 Session &);

  void reload();

private:
  dbo::ptr<Eval_item> eval_item_;
  Edit_assignment_view &main_;
  Session &session_;
  bool in_edit_mode_;

  Wt::WTextArea *prompt_;
  Wt::WLineEdit *value_;
  Eval_item_type_chooser *type_;

  void view_mode_();
  void edit_mode_();

  void cancel_action_();
  void save_action_();
  void edit_action_();

  void save_();

  friend class Edit_assignment_view;
};

Edit_eval_item::Edit_eval_item(const dbo::ptr<Eval_item> &eval_item,
                               Edit_assignment_view &main, Session &session)
    : eval_item_(eval_item), main_(main), session_(session),
      in_edit_mode_(false) {
  if (eval_item->prompt().empty())
    edit_mode_();
  else
    view_mode_();
}

void Edit_eval_item::view_mode_() {
  in_edit_mode_ = false;
  clear();

  auto templ = addNew<WTemplate>(
      "<div class='edit-eval-item'>"
      "<h3>Item ${sequence} <small>(${type}, ${value})</small></h3>"
      "<table class='edit-eval-item-table'>"
      "<tr><th>&nbsp;</th><td>${prompt}</td></tr>"
      "<tr><th>&nbsp;</th><td class='buttons-inline'>"
      "${edit}"
      "</td></tr>"
      "</table>"
      "</div>");

  templ->bindInt("sequence", eval_item_->sequence());
  templ->bindString("type", stringify(eval_item_->type()));
  templ->bindString("value", eval_item_->absolute_value_str());
  templ->bindString("prompt", eval_item_->prompt());
  templ->bindNew<WPushButton>("edit", "Edit")
      ->clicked()
      .connect(this, &Edit_eval_item::edit_action_);
}

void Edit_eval_item::edit_mode_() {
  in_edit_mode_ = true;
  clear();

  auto templ =
      addNew<WTemplate>("<div class='edit-eval-item'>"
                        "<h3>Item ${sequence}</h3>"
                        "<table class='edit-eval-item-table'>"
                        "<tr><th>Type</th><td>${type}</td></tr>"
                        "<tr><th>Value</th><td>${value}</td></tr>"
                        "<tr><th>Prompt</th><td>${prompt}</td></tr>"
                        "<tr><th>&nbsp;</th><td class='buttons-inline'>"
                        "${save} ${cancel}"
                        "</td></tr>"
                        "</table>"
                        "</div>");

  templ->bindInt("sequence", eval_item_->sequence());

  prompt_ = templ->bindNew<WTextArea>("prompt", eval_item_->prompt());
  prompt_->setStyleClass("prompt-edit");

  value_ = templ->bindNew<WLineEdit>("value", eval_item_->relative_value_str());
  value_->setStyleClass("relative-value-edit");

  type_ = templ->bindNew<Eval_item_type_chooser>("type", eval_item_->type());

  templ->bindNew<WPushButton>("save", "Save")
      ->clicked()
      .connect(this, &Edit_eval_item::save_action_);

  templ->bindNew<WPushButton>("cancel", "Cancel")
      ->clicked()
      .connect(this, &Edit_eval_item::cancel_action_);
}

void Edit_eval_item::cancel_action_() {
  dbo::Transaction transaction(session_);

  view_mode_();
  main_.reload_all();
}

void Edit_eval_item::save_action_() {
  dbo::Transaction transaction(session_);

  save_();

  view_mode_();
  main_.reload_all();
}

void Edit_eval_item::edit_action_() {
  dbo::Transaction transaction(session_);

  edit_mode_();
}

void Edit_eval_item::save_() {
  Eval_item::Type type = type_->get_value();
  auto eval_item = eval_item_.modify();
  eval_item->set_prompt(prompt_->text().toUTF8());
  eval_item->set_relative_value(value_->text().toUTF8());
  eval_item->set_type(type);

  value_->setText(eval_item->relative_value_str());
}

void Edit_eval_item::reload() {
  if (!in_edit_mode_)
    view_mode_();
}

Edit_assignment_view::Edit_assignment_view(
    const dbo::ptr<Assignment> &assignment, Session &session)
    : assignment_(assignment), session_(session) {
  setStyleClass("edit-assignment-view");

  container_ = addNew<Wt::WContainerWidget>();

  for (const auto &eval_item : assignment_->eval_items())
    add_item_(eval_item);

  auto buttons = addNew<Wt::WContainerWidget>();
  buttons->setStyleClass("buttons");

  auto more = buttons->addNew<Wt::WPushButton>("More");
  auto fewer = buttons->addNew<Wt::WPushButton>("Fewer");

  more->clicked().connect(this, &Edit_assignment_view::more_);
  fewer->clicked().connect(this, &Edit_assignment_view::fewer_);
}

void Edit_assignment_view::more_() {
  dbo::Transaction transaction(session_);

  int sequence = (int)assignment_->eval_items().size() + 1;
  auto eval_item = session_.addNew<Eval_item>(assignment_, sequence);
  add_item_(eval_item);
}

void Edit_assignment_view::add_item_(const dbo::ptr<Eval_item> &eval_item) {
  auto widget = container_->addNew<Edit_eval_item>(eval_item, *this, session_);
  items_.push_back(widget);
}

void Edit_assignment_view::fewer_() {
  dbo::Transaction transaction(session_);

  if (items_.empty())
    return;

  auto eval_item = items_.back()->eval_item_;
  auto self_eval_count = eval_item->self_evals().size();

  transaction.commit();

  if (self_eval_count == 0) {
    real_fewer_();
  } else {
    std::ostringstream message;
    message << "Eval item " << eval_item->sequence() << " currently has "
            << self_eval_count << " self eval(s). "
            << "Are you sure you want to delete it?";
    ;

    Confirmation_dialog::create(message.str())
        .accepted()
        .connect(this, &Edit_assignment_view::real_fewer_);
  }
}

void Edit_assignment_view::real_fewer_() {
  dbo::Transaction transaction(session_);

  if (items_.empty())
    return;

  items_.back()->eval_item_.remove();
  container_->removeChild(items_.back());
  items_.pop_back();
}

void Edit_assignment_view::reload_all() {
  for (auto each : items_)
    each->reload();
}
