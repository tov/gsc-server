/*
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include "LettersWidget.h"

#include <Wt/WApplication.h>
#include <Wt/WEvent.h>
#include <Wt/WPushButton.h>
#include <Wt/WTable.h>

using namespace Wt;

LettersWidget::LettersWidget() {
  impl_ = setNewImplementation<WTable>();

  impl_->resize(13 * 30, WLength::Auto);

  for (int i = 0; i < 26; ++i) {
    std::string c(1, char('A' + i));

    auto button = impl_->elementAt(i / 13, i % 13)->addNew<WPushButton>(c);
    letterButtons_.push_back(button);

    button->resize(WLength(30), WLength::Auto);

    button->clicked().connect(
        std::bind(&LettersWidget::process_key_index, this, i));
  }

  WApplication::instance()->globalKeyPressed().connect(
      this, &LettersWidget::process_key);
}

void LettersWidget::process_key_index(int i) {
  if (letterButtons_[i]->isEnabled()) {
    letterButtons_[i]->disable();
    letterPushed_.emit(char('A' + i));
  }
}

void LettersWidget::process_key(const WKeyEvent &e) {
  if (isHidden())
    return;

  if ('A' <= e.charCode() && e.charCode() <= 'Z')
    process_key_index(e.charCode() - 'A');
  else if ('a' <= e.charCode() && e.charCode() <= 'z')
    process_key_index(e.charCode() - 'a');
}

void LettersWidget::reset() {
  for (const auto &button : letterButtons_)
    button->enable();
  show();
}
