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

namespace {

inline std::string pct_string(double ratio)
{
    std::ostringstream fmt;
    fmt << std::setprecision(2) << 100 * ratio << '%';
    return fmt.str();
}

}

void Base_eval_item_widget::retract_action_()
{
    if (!main_.can_eval()) return;

    dbo::Transaction transaction(session_);
    Submission::retract_self_eval(model_.self_eval);
    transaction.commit();

    main_.go_default();
}

void Base_eval_item_widget::add_item_heading_()
{
    auto h4 = new Wt::WTemplate("<h4>Question ${number} "
                                        "<small>${value}</small></h4>",
                                this);

    std::string number = boost::lexical_cast<std::string>(
            model_.eval_item->sequence());
    std::string value = pct_string(model_.eval_item->relative_value()
                                   / main_.submission()->point_value());

    h4->bindWidget("number", new Wt::WText(number));
    h4->bindWidget("value", new Wt::WText(value));
}

void Base_eval_item_widget::add_question_()
{
    auto p = new Wt::WTemplate("<p>${question}</p>", this);
    p->bindWidget("question", new Wt::WText(model_.eval_item->prompt()));
}

std::string Base_eval_item_widget::format_score_(
        const Eval_item::Type& type, double score) const
{
    if ((score != 0 && score != 1) || type == Eval_item::Type::Scale) {
        return pct_string(score);
    } else if (type == Eval_item::Type::Boolean) {
        if (score == 1.0) return "Yes";
        else return "No";
    } else {
        return "Okay";
    }
}
