#pragma once
#include "git_exception.h"
#include "libgit2_api.h"
#include <git2.h>
#include <string>

class oid: public libgit2_api {
public:
	oid();

	oid(const std::string &hex_string);

	oid(const std::string &hex_string, size_t length);

	oid(const git_oid *g_oid);

	oid(const unsigned char *raw);

	int compare(const oid &rhs) const;

	int compare(const oid &rhs, size_t length) const;

	oid copy() const;

	bool is_zero() const;

	bool operator==(const oid &rhs) const;

	bool operator==(const std::string &rhs) const;

	std::string to_hex_string(size_t n) const;

	std::string to_path_string() const;
	
	git_oid *g_oid();

	const git_oid *g_oid() const;
private:
	friend class repository;
	git_oid g_oid_;

};

