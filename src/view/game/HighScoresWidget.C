/*
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include "../../Session.h"
#include "../../model/auth/User.h"
#include "../../model/game/User_stats.h"
#include "HighScoresWidget.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/WApplication.h>
#include <Wt/WPushButton.h>
#include <Wt/WTable.h>
#include <Wt/WText.h>

using namespace Wt;

HighScoresWidget::HighScoresWidget(Session &session) : session_(session) {
  setContentAlignment(AlignmentFlag::Center);
  setStyleClass("highscores");
  update();
}

void HighScoresWidget::update() {
  clear();

  int ranking = session_.find_ranking();

  std::string yourScore;
  if (ranking == 1)
    yourScore = "Congratulations! You are currently leading the pack.";
  else {
    yourScore = "You are currently ranked number " + std::to_string(ranking) +
                ". Almost there !";
  }

  WText *score = addNew<WText>("<p>" + yourScore + "</p>");
  score->addStyleClass("score");

  WTable *table = addNew<WTable>();

  table->elementAt(0, 0)->addNew<WText>("Rank");
  table->elementAt(0, 1)->addNew<WText>("User");
  table->elementAt(0, 2)->addNew<WText>("Games");
  table->elementAt(0, 3)->addNew<WText>("Score");
  table->elementAt(0, 4)->addNew<WText>("Last game");
  table->setHeaderCount(1);

  long long formerScore = -1;
  int rank = 0;
  auto top = session_.top_users(20);

  for (auto &stats : top) {
    if (stats->score() != formerScore) {
      formerScore = stats->score();
      ++rank;
    }

    int row = table->rowCount();
    table->elementAt(row, 0)->addNew<WText>(std::to_string(rank));
    table->elementAt(row, 1)->addNew<WText>(stats->user()->name());
    table->elementAt(row, 2)->addNew<WText>(
        std::to_string(stats->games_played()));
    table->elementAt(row, 3)->addNew<WText>(std::to_string(stats->score()));
    if (!stats->last_game().isNull())
      table->elementAt(row, 4)->addNew<WText>(
          stats->last_game().timeTo(WDateTime::currentDateTime()) + " ago");
    else
      table->elementAt(row, 4)->addNew<WText>("---");

    if (session_.user_name() == stats->user()->name())
      table->rowAt(row)->setId("self");
  }

  WText *fineprint = addNew<WText>(tr("highscore.info"));
  fineprint->addStyleClass("fineprint");

  WPushButton *play_game = addNew<WPushButton>("Back to game");
  play_game->clicked().connect(std::bind(
      [=]() { WApplication::instance()->setInternalPath("/game", true); }));
}
