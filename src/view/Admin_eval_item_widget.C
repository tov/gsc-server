#include "Admin_eval_item_widget.h"
#include "Admin_response_widget.h"
#include "Evaluation_view.h"
#include "../model/Grader_eval.h"
#include "../model/Self_eval.h"
#include "../model/Session.h"
#include "../model/Submission.h"

#include <Wt/WText>

Admin_eval_item_widget::Admin_eval_item_widget(
        const Submission::Item& model,
        Evaluation_view& main,
        Session& session,
        Wt::WContainerWidget* parent)
        : Single_eval_item_widget(model, main, session, parent)
{
    load_();
    self_update_button_();
    grader_update_button_();
    add_navigation_(false);
}

void Admin_eval_item_widget::load_() {
    new Wt::WText("<h5>Self evaluation</h5>", this);
    self_response_ = new Admin_response_widget(this);
    self_response_->setFocus(true);
    auto self_buttons = self_response_->buttons();

    self_response_->changed().connect(this, &This::self_update_button_);

    self_save_button_ = new Wt::WPushButton("Save", self_buttons);
    self_save_button_->clicked().connect(this, &This::self_save_action_);

    if (!model_.self_eval) {
        grader_response_ = nullptr;
        grader_save_button_ = nullptr;
        grader_hold_button_ = nullptr;
        return;
    }

    auto self_retract_button = new Wt::WPushButton("Retract", self_buttons);
    self_retract_button->clicked().connect(this, &This::self_retract_action_);

    self_response_->load(&*model_.self_eval);

    new Wt::WText("<h5>Grader evaluation</h5>", this);
    grader_response_ = new Admin_response_widget(this);
    grader_response_->setFocus(true);
    auto grader_buttons = grader_response_->buttons();

    grader_response_->changed().connect(this, &This::grader_update_button_);

    grader_save_button_ = new Wt::WPushButton("Save", grader_buttons);
    grader_save_button_->clicked().connect(this, &This::grader_save_action_);

    grader_hold_button_ = new Wt::WPushButton("Hold", grader_buttons);
    grader_hold_button_->clicked().connect(this, &This::grader_hold_action_);

    if (!model_.grader_eval) return;

    auto grader_retract_button = new Wt::WPushButton("Retract", grader_buttons);
    grader_retract_button->clicked().connect(
            this, &This::grader_retract_action_);

    grader_response_->load(&*model_.grader_eval);
}

void Admin_eval_item_widget::reload_()
{
    main_.go_to((unsigned) model_.eval_item->sequence());
}

void Admin_eval_item_widget::self_update_button_()
{
    if (self_response_->is_saved()) {
        self_save_button_->setText("Saved");
        self_save_button_->disable();
    } else {
        self_save_button_->setText("Save");
        self_save_button_->setEnabled(self_response_->is_valid());
    }
}

void Admin_eval_item_widget::self_save_action_()
{
    dbo::Transaction transaction(session_);
    auto self_eval = Submission::get_self_eval(model_.eval_item,
                                               main_.submission());
    self_response_->save(&*self_eval.modify());
    transaction.commit();

    reload_();
}

void Admin_eval_item_widget::self_retract_action_()
{
    if (!model_.self_eval) return;

    dbo::Transaction transaction(session_);
    Submission::retract_self_eval(model_.self_eval);
    transaction.commit();

    reload_();
}

void Admin_eval_item_widget::grader_update_button_()
{
    if (!grader_response_) return;

    grader_save_button_->setEnabled(grader_response_->is_valid());
    grader_hold_button_->setEnabled(grader_response_->is_valid());

    auto status = model_.grader_eval
                  ? model_.grader_eval->status()
                  : Grader_eval::Status::editing;

    switch (status) {
        case Grader_eval::Status::editing:
            grader_save_button_->setText("Save");
            grader_hold_button_->setText("Hold");
            break;

        case Grader_eval::Status::held_back:
            grader_save_button_->setText("Save");

            if (grader_response_->is_saved()) {
                grader_hold_button_->setText("Held");
                grader_hold_button_->disable();
            } else {
                grader_hold_button_->setText("Hold");
            }
            break;

        case Grader_eval::Status::ready:
            grader_hold_button_->setText("Hold");

            if (grader_response_->is_saved()) {
                grader_save_button_->setText("Saved");
                grader_save_button_->disable();
            } else {
                grader_save_button_->setText("Save");
            }
            break;
    }
}

void Admin_eval_item_widget::grader_save_action_()
{
    grader_save_status_(Grader_eval::Status::ready);
}

void Admin_eval_item_widget::grader_hold_action_()
{
    grader_save_status_(Grader_eval::Status::held_back);
}

void Admin_eval_item_widget::grader_retract_action_()
{
    dbo::Transaction transaction(session_);
    Submission::retract_grader_eval(model_.grader_eval);
    transaction.commit();

    reload_();
}

void Admin_eval_item_widget::grader_save_status_(Grader_eval::Status status)
{
    dbo::Transaction transaction(session_);
    auto grader_eval = Submission::get_grader_eval(model_.self_eval,
                                                   session_.user());
    auto grader_eval_m = grader_eval.modify();
    grader_response_->save(&*grader_eval_m);
    grader_eval_m->set_status(status);
    grader_eval_m->set_grader(session_.user());
    transaction.commit();

    reload_();
}

