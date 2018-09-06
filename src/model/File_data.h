#pragma once

#include "specializations.h"

#include <Wt/Dbo/Types.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/WDateTime.h>

namespace dbo = Wt::Dbo;

class File_meta;

class File_data
{
public:
    File_data() {}
    File_data(const dbo::ptr<File_meta>& file_meta,
              const std::string& contents)
            : file_meta_(file_meta),
              contents_(contents) {}

    const dbo::ptr<File_meta>& file_meta() const { return file_meta_; }
    const std::string& contents() const { return contents_; }

private:
    dbo::ptr<File_meta> file_meta_;
    std::string         contents_;

public:
    template<typename Action>
    void persist(Action& a)
    {
        dbo::id(a, file_meta_, "file_meta", dbo::OnDeleteCascade);
        dbo::field(a, contents_, "contents");
    }
};

DBO_EXTERN_TEMPLATES(File_data);
