/*
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include "HangmanWidget.h"

#include "../../model/auth/User.h"
#include "../../model/game/Dictionary.h"
#include "../../model/Session.h"
#include "WordWidget.h"
#include "ImagesWidget.h"
#include "LettersWidget.h"

#include <Wt/WApplication>
#include <Wt/WBreak>
#include <Wt/WComboBox>
#include <Wt/WPushButton>
#include <Wt/WText>

#include <boost/lexical_cast.hpp>

using namespace Wt;

namespace {
const int MaxGuesses = 9;
}

HangmanWidget::HangmanWidget(Session& session, WContainerWidget* parent)
        : WContainerWidget(parent),
          session_(session),
          badGuesses_(0)
{
    setStyleClass("hangman-widget");
    setContentAlignment(AlignCenter);

    title_ = new WText(tr("hangman.readyToPlay"), this);

    word_       = new WordWidget(this);
    statusText_ = new WText(this);
    images_     = new ImagesWidget(MaxGuesses, this);

    letters_ = new LettersWidget(this);
    letters_->letterPushed().connect(this, &HangmanWidget::registerGuess);

    buttons_ = new WContainerWidget(this);
    buttons_->setStyleClass("buttons");

    language_ = new WComboBox(buttons_);
    language_->addItem(tr("hangman.englishWords").arg(18957));
    language_->addItem(tr("hangman.dutchWords").arg(1688));

    new WBreak(buttons_);

    auto high_scores = new WPushButton("High scores", buttons_);
    high_scores->clicked().connect(std::bind([=]() {
        WApplication::instance()->setInternalPath("/game/high_scores", true);
    }));

    auto new_game = new WPushButton(tr("hangman.newGame"), buttons_);
    new_game->clicked().connect(this, &HangmanWidget::newGame);
    new_game->setAttributeValue("accesskey", "n");

    letters_->hide();
}

void HangmanWidget::newGame()
{
    WString title(tr("hangman.guessTheWord"));
    title_->setText(title.arg(session_.user_name()));

    buttons_->hide();

    /*
     * Choose a new secret word and reset the game
     */
    Dictionary dictionary = (Dictionary) language_->currentIndex();
    word_->init(RandomWord(dictionary));
    letters_->reset();
    badGuesses_ = 0;
    images_->showImage(badGuesses_);
    statusText_->setText("");
}

void HangmanWidget::registerGuess(char c)
{
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
