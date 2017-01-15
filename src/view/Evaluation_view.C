#include "Evaluation_view.h"

#include "Admin_eval_item_widget.h"
#include "List_eval_item_widget.h"
#include "Review_eval_item_widget.h"
#include "Self_eval_item_widget.h"

#include "File_viewer_widget.h"
#include "Response_widget.h"

#include "../model/auth/User.h"
#include "../model/Assignment.h"
#include "../model/Eval_item.h"
#include "../model/Grader_eval.h"
#include "../model/Self_eval.h"
#include "../model/Session.h"
#include "../model/Submission.h"

#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WHBoxLayout>
#include <Wt/WPushButton>
#include <Wt/WTemplate>
#include <Wt/WText>

class Self_eval_item_widget : public Single_eval_item_widget
{
public:
    Self_eval_item_widget(const Submission::Item&,
                          Evaluation_view&,
                          Session&,
                          Wt::WContainerWidget* parent = nullptr);

private:
    Response_widget* response_widget_;
    Wt::WPushButton* save_button_;

    void save_action_();
    void validate_();
};

class Review_eval_item_widget : public Single_eval_item_widget
{
public:
    Review_eval_item_widget(const Submission::Item&,
                            Evaluation_view&,
                            Session&,
                            Wt::WContainerWidget* parent = nullptr);
};

Self_eval_item_widget::Self_eval_item_widget(
        const Submission::Item& model,
        Evaluation_view& main,
        Session& session,
        Wt::WContainerWidget* parent)
        : Single_eval_item_widget(model, main, session, parent)
{
    response_widget_ = Response_widget::create(model.eval_item->type(),
                                               this);
    if (model.self_eval) {
        response_widget_->set_value(model.self_eval->score());
        response_widget_->set_explanation(model.self_eval->explanation());
    }

    response_widget_->changed().connect(this,
                                        &Self_eval_item_widget::validate_);

    auto buttons = new Wt::WContainerWidget(this);
    buttons->setStyleClass("buttons");
    save_button_ = new Wt::WPushButton("Save", buttons);

    save_button_->clicked().connect(this,
                                    &Self_eval_item_widget::save_action_);

    if (model_.grader_eval) {
        auto score = model_.eval_item->format_score(model_.grader_eval->score());
        add_evaluation_("Grader evaluation <small>(old)</small>",
                        score,
                        model_.grader_eval->explanation());
    }

    validate_();
}

void Self_eval_item_widget::save_action_()
{
    if (!main_.can_eval()) return;

    dbo::Transaction transaction(session_);
    auto self_eval = Submission::get_self_eval(model_.eval_item,
                                               main_.submission());
    Submission::save_self_eval(self_eval, session_,
                               response_widget_->value(),
                               response_widget_->explanation());
    transaction.commit();

    main_.go_default();
}

void Self_eval_item_widget::validate_()
{
    save_button_->setEnabled(response_widget_->is_ready());
}

Review_eval_item_widget::Review_eval_item_widget(
        const Submission::Item& model,
        Evaluation_view& main,
        Session& session,
        Wt::WContainerWidget* parent)
        : Single_eval_item_widget(model, main, session, parent)
{
    auto& eval_item = model.eval_item;
    auto& self_eval = model.self_eval;
    auto& grader_eval = model.grader_eval;

    if (self_eval) {
        add_evaluation_("Self evaluation",
                        eval_item->format_score(self_eval->score()),
                        self_eval->explanation());

        if (grader_eval) {
            add_evaluation_("Grader evaluation",
                            eval_item->format_score(grader_eval->score()),
                            grader_eval->explanation());
        }
    } else {
        new Wt::WText("<h5>No self evaluation submitted!</h5>", this);
    }

    add_navigation_();
}

Evaluation_view::Evaluation_view(const dbo::ptr<Submission>& submission,
                                 Session& session,
                                 Wt::WContainerWidget* parent)
        : Abstract_file_view(submission, session, parent)
{
    load_();
    setStyleClass("evaluation-view");
}

void Evaluation_view::load_()
{
    dbo::Transaction transaction(session_);
    submission_->load_cache();
}

void Evaluation_view::go_to(unsigned int index)
{
    std::ostringstream eval_url;
    eval_url << submission_->eval_url();
    eval_url << '/' << index;
    Wt::WApplication::instance()->setInternalPath(eval_url.str());

    right_column_->clear();

    const auto& model = submission_->items().at(index);

    if (session_.user()->can_admin())
        new Admin_eval_item_widget(model, *this, session_, right_column_);
    else if (submission_->can_eval(session_.user()))
        new Self_eval_item_widget(model, *this, session_, right_column_);
    else
        new Review_eval_item_widget(model, *this, session_, right_column_);
}

void Evaluation_view::go_default()
{
    dbo::Transaction transaction(session_);

    if (session_.user()->role() == User::Role::Student && can_eval()) {
        for (auto& row : submission_->items()) {
            if (row.eval_item && !row.self_eval) {
                go_to((unsigned int) row.eval_item->sequence());
                return;
            }
        }
    }

    Wt::WApplication::instance()->setInternalPath(submission_->eval_url());

    right_column_->clear();
    for (auto& row : submission_->items()) {
        if (!row.eval_item) continue;

        new List_eval_item_widget(row, *this, session_, right_column_);
    }
}

bool Evaluation_view::can_eval()
{
    return submission_->can_eval(session_.user());
}

