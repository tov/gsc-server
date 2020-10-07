/*
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include "HangmanWidget.h"

#include "../../Session.h"
#include "../../model/auth/User.h"
#include "../../model/game/Dictionary.h"
#include "ImagesWidget.h"
#include "LettersWidget.h"
#include "WordWidget.h"

#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WComboBox.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

using namespace Wt;

namespace {
const int MaxGuesses = 9;
}

HangmanWidget::HangmanWidget(Session &session)
    : session_(session), badGuesses_(0) {
  setStyleClass("hangman-widget");
  setContentAlignment(AlignmentFlag::Center);

  title_ = addNew<WText>(tr("hangman.readyToPlay"));

  word_ = addNew<WordWidget>();
  statusText_ = addNew<WText>();
  images_ = addNew<ImagesWidget>(MaxGuesses);

  letters_ = addNew<LettersWidget>();
  letters_->letterPushed().connect(this, &HangmanWidget::registerGuess);

  buttons_ = addNew<WContainerWidget>();
  buttons_->setStyleClass("buttons");

  language_ = buttons_->addNew<WComboBox>();
  language_->addItem(tr("hangman.englishWords").arg(18957));
  language_->addItem(tr("hangman.dutchWords").arg(1688));

  buttons_->addNew<WBreak>();

  auto high_scores = buttons_->addNew<WPushButton>("High scores");
  high_scores->clicked().connect(std::bind([=]() {
    WApplication::instance()->setInternalPath("/game/high_scores", true);
  }));

  auto new_game = buttons_->addNew<WPushButton>(tr("hangman.newGame"));
  new_game->clicked().connect(this, &HangmanWidget::newGame);
  new_game->setAttributeValue("accesskey", "n");

  letters_->hide();
}

void HangmanWidget::newGame() {
  WString title(tr("hangman.guessTheWord"));
  title_->setText(title.arg(session_.user_name()));

  buttons_->hide();

  /*
   * Choose a new secret word and reset the game
   */
  Dictionary dictionary = (Dictionary)language_->currentIndex();
  word_->init(RandomWord(dictionary));
  letters_->reset();
  badGuesses_ = 0;
  images_->showImage(badGuesses_);
  statusText_->setText("");
}

void HangmanWidget::registerGuess(char c) {
  if (badGuesses_ < MaxGuesses) {
    bool correct = word_->guess(c);

    if (!correct) {
      ++badGuesses_;
      images_->showImage(badGuesses_);
    }
  }

  if (badGuesses_ == MaxGuesses) {
    WString status(tr("hangman.youHang"));
    statusText_->setText(status.arg(word_->word()));

    letters_->hide();
    buttons_->show();

    session_.add_to_score(-10);
  } else if (word_->won()) {
    statusText_->setText(tr("hangman.youWin"));
    images_->showImage(ImagesWidget::HURRAY);

    letters_->hide();
    buttons_->show();

    session_.add_to_score(20 - badGuesses_);
  }
}
