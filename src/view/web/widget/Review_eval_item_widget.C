#include "Review_eval_item_widget.h"
#include "../../../Session.h"
#include "../../../common/util.h"
#include "../../../model/Grader_eval.h"
#include "../../../model/Self_eval.h"
#include "../../../model/Submission.h"

Review_eval_item_widget::Review_eval_item_widget(
        const Submission::Item& model,
        Evaluation_view& main,
        Session& session)
        : Single_eval_item_widget(model, main, session)
{
    Viewing_context cxt {session.user()};

    if (auto view = model.view_self_eval(cxt, "No self evaluation submitted")) {
        add_evaluation_("Self evaluation",
                        view->score,
                        view->explanation);
    }

    if (auto view = model.view_grader_eval(cxt)) {
        add_evaluation_("Grader evaluation",
                        view->score,
                        view->explanation,
                        "grader-highlight");
    }

    add_navigation_();
}

