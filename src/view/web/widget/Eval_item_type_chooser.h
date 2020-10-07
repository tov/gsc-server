#pragma once

#include "../../../model/Eval_item.h"

#include <Wt/WButtonGroup.h>
#include <Wt/WTemplate.h>

#include <memory>

class Eval_item_type_chooser : public Wt::WTemplate {
public:
  Eval_item_type_chooser();
  explicit Eval_item_type_chooser(Eval_item::Type);

  void set_value(Eval_item::Type);
  Eval_item::Type get_value() const;

private:
  std::shared_ptr<Wt::WButtonGroup> type_;
};
