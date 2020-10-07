#pragma once

#include "Abstract_file_view.h"

class Date_list;
class File_list_widget;
class Quota_display;

class File_manager_view : public Abstract_file_view {
public:
  File_manager_view(const Wt::Dbo::ptr<Submission> &, Session &);

protected:
  void on_change() override;

private:
  void on_change_();

  Date_list *date_list_;
  File_list_widget *file_list_;
#ifdef GSC_SHOW_QUOTA
  Quota_display *quota_display_;
#endif // GSC_SHOW_QUOTA
};
