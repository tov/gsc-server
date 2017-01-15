#include "Base_eval_item_widget.h"
#include "Evaluation_view.h"
#include "../model/Eval_item.h"
#include "../model/Session.h"

#include <Wt/WTemplate>
#include <Wt/WText>

Base_eval_item_widget::Base_eval_item_widget(const Submission::Item& model,
                                             Evaluation_view& main,
                                             Session& session,
                                             Wt::WContainerWidget* parent)
        : WContainerWidget(parent),
          model_(model),
          main_(main),
          session_(session)
{ }

void Base_eval_item_widget::retract_action_()
{
    if (!main_.can_eval()) return;

    if (model_.self_eval) {
        dbo::Transaction transaction(session_);
        Submission::retract_self_eval(model_.self_eval);
        transaction.commit();

        main_.go_default();
    }
}

void Base_eval_item_widget::add_item_heading_()
{
    auto h4 = new Wt::WTemplate("<h4>Question ${number} "
                                        "<small>${value}</small></h4>",
                                this);

    std::string number = boost::lexical_cast<std::string>(
            model_.eval_item->sequence());
    std::string value = Eval_item::pct_string(
            model_.eval_item->relative_value()
            / main_.submission()->point_value());

    h4->bindWidget("number", new Wt::WText(number));
    h4->bindWidget("value", new Wt::WText(value));
}

void Base_eval_item_widget::add_question_()
{
    auto p = new Wt::WTemplate("<p>${question}</p>", this);
    p->bindWidget("question", new Wt::WText(model_.eval_item->prompt()));
}

