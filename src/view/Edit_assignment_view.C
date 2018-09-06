#include "Edit_assignment_view.h"
#include "Confirmation_dialog.h"
#include "../model/Assignment.h"
#include "../model/Eval_item.h"
#include "../model/Session.h"

#include <Wt/WBreak.h>
#include <Wt/WButtonGroup.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WRadioButton.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>
#include <Wt/WTextArea.h>

#include <cstdlib>

class Edit_eval_item : public Wt::WContainerWidget
{
public:
    Edit_eval_item(const dbo::ptr<Eval_item>&,
                   Edit_assignment_view& main,
                   Session&,
                   Wt::WContainerWidget* parent = nullptr);

    void reload();

private:
    dbo::ptr<Eval_item> eval_item_;
    Edit_assignment_view& main_;
    Session& session_;
    bool in_edit_mode_;

    Wt::WTextArea* prompt_;
    Wt::WLineEdit* value_;
    Wt::WButtonGroup* type_;

    void view_mode_();
    void edit_mode_();

    void save_action_();
    void edit_action_();

    void save_();

    friend class Edit_assignment_view;
};

Edit_eval_item::Edit_eval_item(const dbo::ptr<Eval_item>& eval_item,
                               Edit_assignment_view& main,
                               Session& session,
                               Wt::WContainerWidget* parent)
        : WContainerWidget(parent),
          eval_item_(eval_item),
          main_(main),
          session_(session),
          in_edit_mode_(false)
{
    setStyleClass("edit-eval-item");

    if (eval_item->prompt().empty())
        edit_mode_();
    else
        view_mode_();
}

void Edit_eval_item::view_mode_()
{
    in_edit_mode_ = false;
    clear();

    auto templ = new Wt::WTemplate(
            "<h5>Question ${sequence} <small>(${type}, ${value})</small></h5>"
            "<p>${prompt}</p>",
            this);

    std::string sequence =
            boost::lexical_cast<std::string>(eval_item_->sequence());
    templ->bindWidget("sequence", new Wt::WText(sequence));

    std::string type = boost::lexical_cast<std::string>(eval_item_->type());
    templ->bindWidget("type", new Wt::WText(type));

    templ->bindWidget("value", new Wt::WText(eval_item_->absolute_value_str()));

    templ->bindWidget("prompt", new Wt::WText(eval_item_->prompt()));

    auto edit_button = new Wt::WPushButton("Edit", this);
    edit_button->clicked().connect(this, &Edit_eval_item::edit_action_);
}

void Edit_eval_item::edit_mode_()
{
    in_edit_mode_ = true;
    clear();

    std::ostringstream title;
    title << "<h5>Question " << eval_item_->sequence() << "</h5>";
    new Wt::WText(title.str(), this);

    prompt_ = new Wt::WTextArea(eval_item_->prompt(), this);
    prompt_->setStyleClass("prompt");
    new Wt::WBreak(this);

    new Wt::WText("Relative value: ", this);
    value_ = new Wt::WLineEdit(eval_item_->relative_value_str(), this);
    value_->setStyleClass("relative-value");

    new Wt::WBreak(this);
    new Wt::WText("Type: ", this);
    type_ = new Wt::WButtonGroup(this);
    type_->addButton(new Wt::WRadioButton("Boolean", this));
    type_->addButton(new Wt::WRadioButton("Scale", this));
    type_->addButton(new Wt::WRadioButton("Informational", this));

    switch (eval_item_->type()) {
        case Eval_item::Type::Boolean:
            type_->setSelectedButtonIndex(0);
            break;
        case Eval_item::Type::Scale:
            type_->setSelectedButtonIndex(1);
            break;
        case Eval_item::Type::Informational:
            type_->setSelectedButtonIndex(2);
            break;
    }

    new Wt::WBreak(this);

    auto save_button = new Wt::WPushButton("Save", this);
    save_button->clicked().connect(this, &Edit_eval_item::save_action_);
}

void Edit_eval_item::save_action_()
{
    save_();

    dbo::Transaction transaction(session_);
    view_mode_();
    main_.reload_all();
}

void Edit_eval_item::edit_action_()
{
    dbo::Transaction transaction(session_);
    edit_mode_();
}

void Edit_eval_item::save_()
{
    Eval_item::Type type = Eval_item::Type::Boolean;
    switch (type_->selectedButtonIndex()) {
        case 1: type = Eval_item::Type::Scale; break;
        case 2: type = Eval_item::Type::Informational; break;
        default: break;
    }

    dbo::Transaction transaction(session_);
    auto eval_item = eval_item_.modify();
    eval_item->set_prompt(prompt_->text().toUTF8());
    eval_item->set_relative_value(value_->text().toUTF8());
    eval_item->set_type(type);
    transaction.commit();

    value_->setText(eval_item->relative_value_str());
}

void Edit_eval_item::reload()
{
    if (!in_edit_mode_) view_mode_();
}

Edit_assignment_view::Edit_assignment_view(
        const Wt::Dbo::ptr<Assignment>& assignment,
        Session& session,
        Wt::WContainerWidget* parent)
        : WContainerWidget(parent),
          assignment_(assignment),
          session_(session)
{
    setStyleClass("edit-assignment-view");

    container_ = new Wt::WContainerWidget(this);

    dbo::Transaction transaction(session_);
    for (const auto& eval_item : assignment_->eval_items())
        add_item_(eval_item);
    transaction.commit();

    auto buttons = new Wt::WContainerWidget(this);
    buttons->setStyleClass("buttons");
    auto more = new Wt::WPushButton("More", buttons);
    auto fewer = new Wt::WPushButton("Fewer", buttons);

    more->clicked().connect(this, &Edit_assignment_view::more_);
    fewer->clicked().connect(this, &Edit_assignment_view::fewer_);
}

void Edit_assignment_view::more_()
{
    dbo::Transaction transaction(session_);
    int sequence = (int)assignment_->eval_items().size() + 1;
    auto eval_item = session_.add(new Eval_item(assignment_, sequence));
    add_item_(eval_item);
}

void Edit_assignment_view::add_item_(const dbo::ptr<Eval_item>& eval_item)
{
    auto widget = new Edit_eval_item(eval_item, *this, session_, container_);
    items_.push_back(widget);
}

void Edit_assignment_view::fewer_()
{
    if (items_.empty()) return;

    auto eval_item = items_.back()->eval_item_;

    dbo::Transaction transaction(session_);
    auto self_eval_count = eval_item->self_evals().size();
    transaction.commit();

    if (self_eval_count == 0) {
        real_fewer_();
    } else {
        std::ostringstream message;
        message << "Question " << eval_item->sequence() << " currently has "
                << self_eval_count << " self eval(s). "
                << "Are you sure you want to delete it?";;

        auto dialog = new Confirmation_dialog(message.str(), this);
        dialog->accepted().connect(this, &Edit_assignment_view::real_fewer_);
    }
}

void Edit_assignment_view::real_fewer_()
{
    if (items_.empty()) return;

    dbo::Transaction transaction(session_);
    items_.back()->eval_item_.remove();
    transaction.commit();

    delete items_.back();
    items_.pop_back();
}

void Edit_assignment_view::reload_all()
{
    for (auto each : items_) each->reload();
}
