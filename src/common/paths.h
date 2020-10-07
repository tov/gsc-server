#pragma once

#include <iostream>
#include <memory>
#include <regex>
#include <string>

namespace api {

namespace paths {

struct Uri_base {
  // Writes the URI to an output stream
  virtual void format(std::ostream &) const;

  // Returns the URI as a string.
  std::string to_string() const;
  operator std::string() const;

  virtual ~Uri_base() = default;
};

struct Grades_csv : Uri_base {
  Grades_csv() = default;
  explicit Grades_csv(std::smatch const &) {}

  static const std::regex re;
  void format(std::ostream &) const override;
};

struct Users : Uri_base {
  Users() = default;
  explicit Users(std::smatch const &) {}

  static const std::regex re;
  void format(std::ostream &) const override;
};

struct Users_1 : Users {
  std::string name;

  explicit Users_1(std::string const name) : name{std::move(name)} {}
  explicit Users_1(std::smatch const &);

  static const std::regex re;
  void format(std::ostream &) const override;
};

struct Users_1_submissions : Users_1 {
  using Users_1::Users_1;

  static const std::regex re;
  void format(std::ostream &) const override;
};

struct Submissions_1 : Uri_base {
  int submission_id;

  explicit Submissions_1(int submission_id) : submission_id{submission_id} {}
  explicit Submissions_1(std::smatch const &);

  static const std::regex re;
  void format(std::ostream &) const override;
};

struct Submissions_1_files : Submissions_1 {
  using Submissions_1::Submissions_1;

  static const std::regex re;
  void format(std::ostream &) const override;
};

struct Submissions_1_files_2 : Submissions_1_files {
  std::string filename;

  Submissions_1_files_2(int submission_id, std::string filename)
      : Submissions_1_files{submission_id}, filename{std::move(filename)} {}
  explicit Submissions_1_files_2(std::smatch const &);

  static const std::regex re;
  void format(std::ostream &) const override;
};

struct Submissions_1_evals : Submissions_1 {
  using Submissions_1::Submissions_1;

  static const std::regex re;
  void format(std::ostream &) const override;
};

struct Submissions_1_evals_2 : Submissions_1_evals {
  int sequence;

  Submissions_1_evals_2(int assignment_id, int sequence)
      : Submissions_1_evals{assignment_id}, sequence{sequence} {}
  explicit Submissions_1_evals_2(std::smatch const &);

  static const std::regex re;
  void format(std::ostream &) const override;
};

struct Submissions_1_evals_2_self : Submissions_1_evals_2 {
  using Submissions_1_evals_2::Submissions_1_evals_2;

  static const std::regex re;
  void format(std::ostream &) const override;
};

struct Submissions_1_evals_2_grader : Submissions_1_evals_2 {
  using Submissions_1_evals_2::Submissions_1_evals_2;

  static const std::regex re;
  void format(std::ostream &) const override;
};

struct Submissions_hw1 : Uri_base {
  int assignment_number;

  explicit Submissions_hw1(int assignment_number)
      : assignment_number{assignment_number} {}
  explicit Submissions_hw1(std::smatch const &);

  static const std::regex re;
  void format(std::ostream &) const override;
};

struct Whoami : Uri_base {
  Whoami() = default;
  explicit Whoami(std::smatch const &) {}

  static const std::regex re;
  void format(std::ostream &) const override;
};

extern const std::regex grades_csv;
extern const std::regex users;
extern const std::regex users_1;
extern const std::regex users_1_submissions;
extern const std::regex submissions_1;
extern const std::regex submissions_1_files;
extern const std::regex submissions_1_files_2;
extern const std::regex submissions_hw1;
extern const std::regex submissions_1_evals;
extern const std::regex submissions_1_evals_2;
extern const std::regex whoami;

} // end namespace paths

} // namespace api
