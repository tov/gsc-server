#pragma once

#include "../common/stringify.h"
#include "specializations.h"

#include <Wt/Dbo/Types.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/WDateTime.h>

#include <string>

namespace dbo = Wt::Dbo;

class File_data;

class User;

class Submission;

enum class File_purpose {
  source,
  test,
  config,
  resource,
  log,
  forbidden,
};

template <> struct Enum<File_purpose> {
  static char const *show(File_purpose);
  static File_purpose read(const char *);
};

class File_meta {
public:
  File_meta() = default;
  File_meta(const std::string &name, const std::string &media_type,
            File_purpose purpose, const dbo::ptr<User> &uploader,
            int line_count, int byte_count,
            const dbo::ptr<Submission> &submission);

  const std::string &name() const { return name_; }

  std::string const &media_type() const { return media_type_; }

  File_purpose const &purpose() const { return purpose_; }

  const dbo::weak_ptr<File_data> &file_data() const { return file_data_; }

  int line_count() const { return line_count_; }

  int byte_count() const { return byte_count_; }

  const dbo::ptr<Submission> &submission() const { return submission_; }

  const dbo::ptr<User> &uploader() const { return uploader_; }

  void move(const dbo::ptr<Submission> &, const std::string &,
            bool overwrite = false);

  void set_media_type(const std::string &media_type);

  void reclassify();
  void reclassify(File_purpose);

  bool is_line_numbered() const;

  bool is_automatically_deletable() const;

  static Wt::Dbo::ptr<File_meta> upload(const std::string &name,
                                        const Bytes &contents,
                                        const dbo::ptr<Submission> &,
                                        const dbo::ptr<User> &uploader);

  static const int max_byte_count;

  std::string rest_uri() const;
  Wt::Json::Object to_json(bool brief = false) const;

  template <class Less_string = std::less<std::string>> struct Less_by_name {
    Less_by_name() = default;

    explicit Less_by_name(Less_string less_string)
        : less_string(std::move(less_string)) {}

    size_t operator()(Wt::Dbo::ptr<File_meta> const &a,
                      Wt::Dbo::ptr<File_meta> const &b) const {
      return less_string(a->name(), b->name());
    }

    Less_string less_string;
  };

private:
  std::string name_;
  Wt::WDateTime time_stamp_;
  int line_count_;
  int byte_count_;
  dbo::ptr<Submission> submission_;
  std::string media_type_;
  File_purpose purpose_;
  dbo::weak_ptr<File_data> file_data_;
  dbo::ptr<User> uploader_;

public:
  template <typename Action> void persist(Action &a) {
    dbo::field(a, name_, "name");
    dbo::field(a, media_type_, "media_type");
    dbo::field(a, purpose_, "purpose");
    dbo::field(a, time_stamp_, "time_stamp");
    dbo::field(a, line_count_, "line_count");
    dbo::field(a, byte_count_, "byte_count");
    dbo::belongsTo(a, submission_, "submission", dbo::OnDeleteCascade);
    dbo::hasOne(a, file_data_, "file_meta");
    dbo::belongsTo(a, uploader_, "uploader", dbo::OnDeleteSetNull);
  }
};

bool operator<(const File_meta &, const File_meta &);

DBO_EXTERN_TEMPLATES(File_meta)
