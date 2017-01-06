/*
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include "HighScoresWidget.h"
#include "../../model/Session.h"
#include "../../model/auth/User.h"
#include "../../model/game/User_stats.h"

#include <Wt/WApplication>
#include <Wt/WPushButton>
#include <Wt/WText>
#include <Wt/WTable>
#include <Wt/Dbo/Dbo>

#include <boost/lexical_cast.hpp>

using namespace Wt;

HighScoresWidget::HighScoresWidget(Session& session, WContainerWidget* parent) :
        WContainerWidget(parent),
        session_(session)
{
    setContentAlignment(AlignCenter);
    setStyleClass("highscores");
    update();
}

void HighScoresWidget::update()
{
    clear();

    int ranking = session_.find_ranking();

    std::string yourScore;
    if (ranking == 1)
        yourScore = "Congratulations! You are currently leading the pack.";
    else {
        yourScore = "You are currently ranked number "
                    + boost::lexical_cast<std::string>(ranking)
                    + ". Almost there !";
    }

    WText* score = new WText("<p>" + yourScore + "</p>", this);
    score->addStyleClass("score");

    WTable* table = new WTable(this);

    new WText("Rank", table->elementAt(0, 0));
    new WText("User", table->elementAt(0, 1));
    new WText("Games", table->elementAt(0, 2));
    new WText("Score", table->elementAt(0, 3));
    new WText("Last game", table->elementAt(0, 4));
    table->setHeaderCount(1);

    long long formerScore = -1;
    int       rank        = 0;
    auto      top         = session_.top_users(20);

    for (auto& stats : top) {
        if (stats->score() != formerScore) {
            formerScore = stats->score();
            ++rank;
        }

        int row = table->rowCount();
        new WText(boost::lexical_cast<std::string>(rank),
                  table->elementAt(row, 0));
        new WText(stats->user()->name(), table->elementAt(row, 1));
        new WText(boost::lexical_cast<std::string>(stats->games_played()),
                  table->elementAt(row, 2));
        new WText(boost::lexical_cast<std::string>(stats->score()),
                  table->elementAt(row, 3));
        if (!stats->last_game().isNull())
            new WText(stats->last_game().timeTo(WDateTime::currentDateTime())
                      + " ago", table->elementAt(row, 4));
        else
            new WText("---", table->elementAt(row, 4));

        if (session_.user_name() == stats->user()->name())
            table->rowAt(row)->setId("self");
    }

    WText* fineprint = new WText(tr("highscore.info"), this);
    fineprint->addStyleClass("fineprint");

    WPushButton* play_game = new WPushButton("Back to game", this);
    play_game->clicked().connect(std::bind([=]() {
        WApplication::instance()->setInternalPath("/game", true);
    }));
}
