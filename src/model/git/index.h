#pragma once
#include <git2.h>
#include <functional>
#include <string>
#include "libgit2_api.h"
#include "oid.h"
#include "ownership.h"
#include "tree.h"

class repository;

class index : public libgit2_api {
public:
	index();

	index(git_index *g_index, ownership owner = ownership::libgit2);

	~index();

	void add_entries_that_match(const std::vector<std::string> &pathspec);

	void add_entry_by_path(const std::string &path);

	const oid checksum();

	void clear();

	size_t size() const;

	bool has_conflicts() const;

	std::string path() const;

	void read_tree(const tree &tree);

	void write();

	oid write_tree();

	class repository owner() const;

	const git_index* g_index() const;
private:
	friend class repository;
	git_index *g_index_;
	ownership owner_;
};
