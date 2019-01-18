#include "Base_eval_item_widget.h"
#include "Evaluation_view.h"
#include "Explanation_view_widget.h"
#include "../model/Eval_item.h"
#include "../Session.h"

#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>

#include <boost/lexical_cast.hpp>

Base_eval_item_widget::Base_eval_item_widget(const Submission::Item& model, Evaluation_view& main,
                                             Session& session)
        : model_(model),
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
    auto h4 = addNew<Wt::WTemplate>("<h4>Question ${number} "
                                    "<small>(${value})</small></h4>");

    std::string number = boost::lexical_cast<std::string>(
            model_.eval_item->sequence());
    std::string value = Eval_item::pct_string(
            model_.eval_item->relative_value()
            / main_.submission()->point_value());

    h4->bindWidget("number", std::make_unique<Wt::WText>(number));
    h4->bindWidget("value", std::make_unique<Wt::WText>(value));
}

void Base_eval_item_widget::add_question_()
{
    auto p = addNew<Wt::WTemplate>("<div class='question'>${question}</div>");
    p->bindWidget("question",
                  std::make_unique<Wt::WText>(model_.eval_item->prompt()));
}

void Base_eval_item_widget::add_evaluation_(const std::string& heading,
                                            const std::string& score,
                                            const std::string& explanation,
                                            const std::string& highlight_style)
{
    auto p = addNew<Wt::WTemplate>(
            "<h5>${heading}</h5>"
            "<p class='answer'>"
              "<strong>${score}.</strong>"
              " ${explanation}"
            "</p>");

    p->bindWidget("heading", std::make_unique<Wt::WText>(heading));
    p->bindWidget("score", std::make_unique<Wt::WText>(score));
    p->bindWidget("explanation",
                  std::make_unique<Explanation_view_widget>(
                          explanation, main_.file_viewer(), highlight_style));
}

void Base_eval_item_widget::add_navigation_(bool focus)
{
    auto buttons = addNew<Wt::WContainerWidget>();
    buttons->setStyleClass("buttons");

    auto prev_btn = buttons->addNew<Wt::WPushButton>("Prev");
    auto list_btn = buttons->addNew<Wt::WPushButton>("List");
    auto next_btn = buttons->addNew<Wt::WPushButton>("Next");

    auto sequence = model_.eval_item->sequence();

    if (sequence > 1) {
        prev_btn->clicked().connect(std::bind([=]() {
            main_.go_to((unsigned) (sequence - 1));
        }));
    } else prev_btn->disable();

    list_btn->setFocus(focus);
    list_btn->clicked().connect(std::bind([=]() { main_.go_default(); }));

    if (sequence < main_.submission()->item_count()) {
        next_btn->clicked().connect(std::bind([=]() {
            main_.go_to((unsigned) (sequence + 1));
        }));
    } else next_btn->disable();
}

