#include "../../../common/util.h"
#include "Eval_item_type_chooser.h"

#include <Wt/WRadioButton.h>

Eval_item_type_chooser::Eval_item_type_chooser()
    : WTemplate{"${bool} ${scale} ${info}"}, type_{
                                                 make_shared<WButtonGroup>()} {
  type_->addButton(bindNew<WRadioButton>("bool", "Boolean"));
  type_->addButton(bindNew<WRadioButton>("scale", "Scale"));
  type_->addButton(bindNew<WRadioButton>("info", "Informational"));
}

Eval_item_type_chooser::Eval_item_type_chooser(Eval_item::Type type0)
    : Eval_item_type_chooser{} {
  set_value(type0);
}

void Eval_item_type_chooser::set_value(Eval_item::Type new_type) {
  switch (new_type) {
  default:
  case Eval_item::Type::Boolean:
    type_->setSelectedButtonIndex(0);
    break;
  case Eval_item::Type::Scale:
    type_->setSelectedButtonIndex(1);
    break;
  case Eval_item::Type::Informational:
    type_->setSelectedButtonIndex(2);
    break;
  }
}

Eval_item::Type Eval_item_type_chooser::get_value() const {
  switch (type_->selectedButtonIndex()) {
  default:
  case 0:
    return Eval_item::Type::Boolean;
  case 1:
    return Eval_item::Type::Scale;
  case 2:
    return Eval_item::Type::Informational;
  }
}
