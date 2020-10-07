// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#ifndef LETTERS_WIDGET_H_
#define LETTERS_WIDGET_H_

#include <Wt/WCompositeWidget.h>

namespace Wt {
class WContainerWidget;

class WPushButton;

class WTable;
} // namespace Wt

class LettersWidget : public Wt::WCompositeWidget {
public:
  LettersWidget();

  void reset();

  Wt::Signal<char> &letterPushed() { return letterPushed_; }

private:
  Wt::WTable *impl_;
  std::vector<Wt::WPushButton *> letterButtons_;

  Wt::Signal<char> letterPushed_;

  void process_key_index(int);
  void process_key(const Wt::WKeyEvent &);
};

#endif // LETTERS_WIDGET_H_
