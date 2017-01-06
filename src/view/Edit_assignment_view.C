#include "Edit_assignment_view.h"
#include "Confirmation_dialog.h"
#include "../model/Assignment.h"
#include "../model/Eval_item.h"
#include "../model/Session.h"

#include <Wt/WBreak>
#include <Wt/WButtonGroup>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WRadioButton>
#include <Wt/WText>
#include <Wt/WTextArea>

#include <cstdlib>

class Edit_eval_item : public Wt::WContainerWidget
{
public:
    Edit_eval_item(const dbo::ptr<Eval_item>&,
                   Session&,
                   Wt::WContainerWidget* parent = nullptr);

private:
    dbo::ptr<Eval_item> eval_item_;
    Session& session_;

    Wt::WTextArea* prompt_;
    Wt::WLineEdit* value_;
    Wt::WButtonGroup* type_;

    void save_();

    friend class Edit_assignment_view;
};

Edit_eval_item::Edit_eval_item(const dbo::ptr<Eval_item>& eval_item,
                               Session& session,
                               Wt::WContainerWidget* parent)
        : WContainerWidget(parent),
          eval_item_(eval_item),
          session_(session)
{
    setStyleClass("edit-eval-item");

    std::ostringstream title;
    title << "<h5>Question " << eval_item->sequence() << "</h5>";
    new Wt::WText(title.str(), this);

    prompt_ = new Wt::WTextArea(eval_item_->prompt(), this);
    prompt_->setStyleClass("prompt");
    new Wt::WBreak(this);

    new Wt::WText("Relative value: ", this);
    value_ = new Wt::WLineEdit(eval_item_->relative_value_str(), this);
    value_->setStyleClass("relative-value");
    new Wt::WText(" Type: ", this);

    type_ = new Wt::WButtonGroup(this);
    type_->addButton(new Wt::WRadioButton("Boolean", this));
    type_->addButton(new Wt::WRadioButton("Scale", this));
    type_->addButton(new Wt::WRadioButton("Informational", this));

    switch (eval_item->type()) {
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

    prompt_->changed().connect(this, &Edit_eval_item::save_);
    value_->changed().connect(this, &Edit_eval_item::save_);
    type_->checkedChanged().connect(this, &Edit_eval_item::save_);
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
    auto widget = std::make_unique<Edit_eval_item>(eval_item, session_,
                                                   container_);
    items_.push_back(std::move(widget));
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

    items_.pop_back();
}
