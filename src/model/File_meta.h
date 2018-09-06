#pragma once

#include "specializations.h"

#include <Wt/Dbo/Types.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/WDateTime.h>

namespace dbo = Wt::Dbo;

class File_data;
class Submission;

class File_meta
{
public:
    File_meta() = default;
    File_meta(const std::string& name,
              const dbo::ptr<Submission>& submission,
              int line_count,
              int byte_count);

    const std::string& name() const { return name_; }
    const dbo::weak_ptr<File_data>& file_data() const { return file_data_; }
    int line_count() const { return line_count_; }
    int byte_count() const { return byte_count_; }
    const dbo::ptr<Submission>& submission() const { return submission_; }

    bool is_out_file() const;

    void rename(const std::string&);
    void re_own(const dbo::ptr<Submission>&);

    static dbo::ptr<File_meta>
    upload(const std::string& name,
           const std::string& contents,
           const dbo::ptr<Submission>&);

    static const int max_byte_count;

private:
    std::string              name_;
    Wt::WDateTime            time_stamp_;
    int                      line_count_;
    int                      byte_count_;
    dbo::ptr<Submission>     submission_;
    dbo::weak_ptr<File_data> file_data_;

public:
    template<typename Action>
    void persist(Action& a)
    {
        dbo::field(a, name_, "name");
        dbo::field(a, time_stamp_, "time_stamp");
        dbo::field(a, line_count_, "line_count");
        dbo::field(a, byte_count_, "byte_count");
        dbo::belongsTo(a, submission_, "submission", dbo::OnDeleteCascade);
        dbo::hasOne(a, file_data_, "file_meta");
    }
};

bool operator<(const File_meta&, const File_meta&);

DBO_EXTERN_TEMPLATES(File_meta)
