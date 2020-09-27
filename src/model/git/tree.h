#pragma once
#include "libgit2_api.h"
#include "ownership.h"
#include <functional>
#include <git2.h>
#include <memory>
#include <string>
#include <utility>

class repository;
class commit;

class tree: public libgit2_api {
public:
	tree();

	tree(git_tree *g_tree, ownership owner = ownership::libgit2);

	~tree();

	oid id() const;

	git_tree *g_tree();

	const git_tree *g_tree() const;
private:
	friend class commit;
	friend class repository;
	git_tree *g_tree_;
	ownership owner_;
};
