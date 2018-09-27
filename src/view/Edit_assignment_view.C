#include "Edit_assignment_view.h"
#include "Confirmation_dialog.h"
#include "../model/Assignment.h"
#include "../model/Eval_item.h"
#include "../Session.h"

#include <Wt/WBreak.h>
#include <Wt/WButtonGroup.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WRadioButton.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>
#include <Wt/WTextArea.h>

#include <boost/lexical_cast.hpp>

#include <cstdlib>

class Edit_eval_item : public Wt::WContainerWidget
{
public:
    Edit_eval_item(const dbo::ptr<Eval_item>&, Edit_assignment_view& main,
                   Session&);

    void reload();

private:
    dbo::ptr<Eval_item> eval_item_;
    Edit_assignment_view& main_;
    Session& session_;
    bool in_edit_mode_;

    Wt::WTextArea* prompt_;
    Wt::WLineEdit* value_;
    std::shared_ptr<Wt::WButtonGroup> type_;

    void view_mode_();
    void edit_mode_();

    void save_action_();
    void edit_action_();

    void save_();

    friend class Edit_assignment_view;
};

Edit_eval_item::Edit_eval_item(const dbo::ptr<Eval_item>& eval_item,
                               Edit_assignment_view& main, Session& session)
        : eval_item_(eval_item),
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

    auto templ = addNew<Wt::WTemplate>(
            "<h5>Question ${sequence} <small>(${type}, ${value})</small></h5>"
            "<p>${prompt}</p>");

    std::string sequence =
            boost::lexical_cast<std::string>(eval_item_->sequence());
    templ->bindWidget("sequence", std::make_unique<Wt::WText>(sequence));

    std::string type = boost::lexical_cast<std::string>(eval_item_->type());
    templ->bindWidget("type", std::make_unique<Wt::WText>(type));

    templ->bindWidget("value", std::make_unique<Wt::WText>(eval_item_->absolute_value_str()));

    templ->bindWidget("prompt", std::make_unique<Wt::WText>(eval_item_->prompt()));

    auto edit_button = addNew<Wt::WPushButton>("Edit");
    edit_button->clicked().connect(this, &Edit_eval_item::edit_action_);
}

void Edit_eval_item::edit_mode_()
{
    in_edit_mode_ = true;
    clear();

    std::ostringstream title;
    title << "<h5>Question " << eval_item_->sequence() << "</h5>";
    addNew<Wt::WText>(title.str());

    prompt_ = addNew<Wt::WTextArea>(eval_item_->prompt());
    prompt_->setStyleClass("prompt");
    addNew<Wt::WBreak>();

    addNew<Wt::WText>("Relative value: ");
    value_ = addNew<Wt::WLineEdit>(eval_item_->relative_value_str());
    value_->setStyleClass("relative-value");

    addNew<Wt::WBreak>();
    addNew<Wt::WText>("Type: ");
    type_ = std::make_shared<Wt::WButtonGroup>();
    type_->addButton(addNew<Wt::WRadioButton>("Boolean"));
    type_->addButton(addNew<Wt::WRadioButton>("Scale"));
    type_->addButton(addNew<Wt::WRadioButton>("Informational"));

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

    addNew<Wt::WBreak>();

    auto save_button = addNew<Wt::WPushButton>("Save");
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

Edit_assignment_view::Edit_assignment_view(const dbo::ptr<Assignment>& assignment,
                                           Session& session)
        : assignment_(assignment),
          session_(session)
{
    setStyleClass("edit-assignment-view");

    container_ = addNew<Wt::WContainerWidget>();

    dbo::Transaction transaction(session_);
    for (const auto& eval_item : assignment_->eval_items())
        add_item_(eval_item);
    transaction.commit();

    auto buttons = addNew<Wt::WContainerWidget>();
    buttons->setStyleClass("buttons");
    auto more = buttons->addNew<Wt::WPushButton>("More");
    auto fewer = buttons->addNew<Wt::WPushButton>("Fewer");

    more->clicked().connect(this, &Edit_assignment_view::more_);
    fewer->clicked().connect(this, &Edit_assignment_view::fewer_);
}

void Edit_assignment_view::more_()
{
    dbo::Transaction transaction(session_);
    int sequence = (int)assignment_->eval_items().size() + 1;
    auto eval_item = session_.addNew<Eval_item>(assignment_, sequence);
    add_item_(eval_item);
}

void Edit_assignment_view::add_item_(const dbo::ptr<Eval_item>& eval_item)
{
    auto widget = container_->addNew<Edit_eval_item>(eval_item, *this, session_);
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

        auto dialog = addNew<Confirmation_dialog>(message.str());
        dialog->accepted().connect(this, &Edit_assignment_view::real_fewer_);
    }
}

void Edit_assignment_view::real_fewer_()
{
    if (items_.empty()) return;

    dbo::Transaction transaction(session_);
    items_.back()->eval_item_.remove();
    transaction.commit();

    container_->removeChild(items_.back());
    items_.pop_back();
}

void Edit_assignment_view::reload_all()
{
    for (auto each : items_) each->reload();
}
