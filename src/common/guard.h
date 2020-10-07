#pragma once

template <typename Guarded, typename Value, typename New_value,
          Value (Guarded::*apply)(New_value)>
class Basic_guard {
public:
  Basic_guard(Guarded &guarded, New_value new_value)
      : guarded_{guarded}, saved_{(guarded.*apply)(new_value)} {}

  ~Basic_guard() { (guarded_.*apply)(saved_); }

private:
  Guarded &guarded_;
  Value saved_;
};
