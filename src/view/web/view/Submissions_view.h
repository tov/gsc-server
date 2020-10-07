#pragma once

#include "../../../common/autovector.h"
#include "../../../model/Submission.h"
#include "../Submission_context.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WSignal.h>

#include <memory>

class Exam_grade;

class Session;

class User;

class Submissions_view : public Wt::WContainerWidget {
public:
  Submissions_view(const Wt::Dbo::ptr<User> &, Session &);

  struct Model {
    struct Item {
      Item() = default;
      Item(Wt::Dbo::ptr<Submission> const &submission,
           Wt::Dbo::ptr<User> const &principal);

      Wt::Dbo::ptr<Submission> submission;
      size_t file_count = 0;
      Submission::Eval_status eval_status = Submission::Eval_status::empty;
      Wt::Dbo::ptr<User> principal;
    };

    auto_vector<Item> submissions;
    std::vector<Wt::Dbo::ptr<Exam_grade>> exams;
    Wt::Dbo::ptr<User> principal;

    Model(Wt::Dbo::ptr<User> const &principal, Session &);
  };

  class Row_view;

private:
  Session &session_;
  Model model_;
  std::vector<std::unique_ptr<Row_view>> rows_;
  Submission_change_signal changed_;

  void reload_();
  void on_change_(Submission_change_message);
};

class Submissions_view::Row_view : public Wt::WObject {
public:
  enum columns {
    NAME,
    STATUS,
    DUE_DATE,
    EVAL_DATE,
    GRADE,
    ACTION,
  };

  using Row_model = Submissions_view::Model::Item;

  static std::unique_ptr<Row_view>
  construct(Row_model const &model, Session &session, Wt::WTableRow *row);

  static void add_headings(Wt::WTableRow *);

protected:
  Row_view(Row_model const &model, Session &session, Wt::WTableRow *row);

  Row_model const &model_;
  Session &session_;
  Wt::WTableRow *row_;

  virtual void update();

  virtual void set_files_action(const char[]);
  virtual void set_eval_action(const char[]);
  virtual void set_action_style_class(const char[]);

private:
  Wt::WText *status_;
  Wt::WText *grade_;
  Wt::WPushButton *action_;
  std::string action_url_;

  void action();
};
