#pragma once

#include <Wt/Dbo/ptr.h>

#include <array>
#include <iostream>
#include <stdexcept>
#include <string>

class File_meta;
class Submission;

struct Access_check_failed : public std::runtime_error {
  using runtime_error::runtime_error;
};

struct Resource_not_found : public std::runtime_error {
  Resource_not_found() : runtime_error("resource not found") {}
};

struct Html_error : public std::runtime_error {
  using runtime_error::runtime_error;

  explicit Html_error(runtime_error const &exn) : runtime_error{exn} {}

  virtual const char *title() const = 0;
  virtual std::ostream &write_body_html(std::ostream &) const = 0;
};

struct Generic_html_error : Html_error {
  explicit Generic_html_error(std::runtime_error const &);

  const char *title() const override;
  std::ostream &write_body_html(std::ostream &) const override;
};

template <class OpenNote, class Fn> void html_try(Fn &&action) {
  try {
    action();
  } catch (Html_error const &exn) {
    auto note = OpenNote(exn.title());
    exn.write_body_html(note);
  } catch (std::runtime_error const &rte) {
    auto exn = Generic_html_error{rte};
    auto note = OpenNote(exn.title());
    exn.write_body_html(note);
  }
}

class No_partner_to_separate : public Html_error {
public:
  No_partner_to_separate(std::string);

  const char *title() const override;
  std::ostream &write_body_html(std::ostream &) const override;

private:
  std::string username_;
};

class Whose_files_are_these : public Html_error {
public:
  using Source_file_vec = std::vector<Wt::Dbo::ptr<File_meta>>;

  Whose_files_are_these(Source_file_vec);

  const char *title() const override;
  std::ostream &write_body_html(std::ostream &) const override;

private:
  Source_file_vec lost_files_;
};

class Move_collision_error : public Html_error {
public:
  using submission_t = Wt::Dbo::ptr<Submission>;
  using filename_t = std::string;

  Move_collision_error(submission_t, filename_t, submission_t, filename_t);

  virtual const char *title() const override;
  std::ostream &write_body_html(std::ostream &) const override;

private:
  std::array<submission_t, 2> submissions_;
  std::array<filename_t, 2> filenames_;
};

class Join_collision_error : public Html_error {
public:
  using submission_t = Wt::Dbo::ptr<Submission>;
  using filenames_t = std::vector<std::string>;

  Join_collision_error(submission_t, submission_t, filenames_t);

  filenames_t const &filenames() const { return filenames_; }

  virtual const char *title() const override;
  std::ostream &write_body_html(std::ostream &) const override;

private:
  std::array<submission_t, 2> submissions_;
  filenames_t filenames_;
};

class File_too_large_error : public Html_error {
public:
  File_too_large_error(std::string filename, int needed, int allowed = -1);

  std::string const &filename() const { return filename_; }
  int needed() const { return needed_; }
  int allowed() const { return allowed_; }

  virtual const char *title() const override;
  std::ostream &write_body_html(std::ostream &) const override;

private:
  std::string filename_;
  int needed_;
  int allowed_;
};

class Would_exceed_quota_error : public Html_error {
public:
  Would_exceed_quota_error(std::string filename, int needed, int available);

  std::string const &filename() const { return filename_; }
  int needed() const { return needed_; }
  int available() const { return available_; }

  virtual const char *title() const override;
  std::ostream &write_body_html(std::ostream &) const override;

private:
  std::string filename_;
  int needed_;
  int available_;
};

class Bad_file_type_error : public Html_error {
public:
  Bad_file_type_error(std::string filename);

  std::string const &filename() const { return filename_; }

  virtual const char *title() const override;
  std::ostream &write_body_html(std::ostream &) const override;

private:
  std::string filename_;
};

class Withdrawn_partner_request_error : public Html_error {
public:
  Withdrawn_partner_request_error();

  virtual const char *title() const override;
  std::ostream &write_body_html(std::ostream &) const override;
};
