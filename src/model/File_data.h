#pragma once

#include "specializations.h"
#include <Wt/Dbo/Types.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/WDateTime.h>

#include <iostream>
#include <vector>

namespace dbo = Wt::Dbo;

class File_meta;

class Bytes : public std::vector<unsigned char>
{
public:
    Bytes() = default;
    explicit Bytes(const std::string&);
    Bytes(std::istream&, int size);

    void write(std::ostream&) const;

    explicit operator std::string() const;
};

class File_data
{
public:

    File_data() {}
    File_data(const dbo::ptr<File_meta>& file_meta,
              Bytes const& contents)
            : file_meta_(file_meta),
              contents_(contents) {}

	File_data(const dbo::ptr<File_meta>& file_meta)
			: file_meta_(file_meta) {}

	const dbo::ptr<File_meta>& file_meta() const { return file_meta_; }
	const Bytes& contents() const { return contents_; }
	int write_and_commit();
	int delete_and_commit();
	int populate_contents();
	
private:
	dbo::ptr<File_meta> file_meta_;
    Bytes contents_;
};
