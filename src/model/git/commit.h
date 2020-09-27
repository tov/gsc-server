#pragma once
#include "git_exception.h"
#include "libgit2_api.h"
#include "oid.h"
#include "ownership.h"
#include "signature.h"
#include "tree.h"
#include <git2.h>
#include <string>

class commit: public libgit2_api {
public:
	commit();
	
	commit(git_commit *g_commit, ownership owner = ownership::libgit2);

	~commit();

	signature author() const;

	std::string body() const;

	signature committer() const;

	oid id() const;

	std::string message() const;

	std::string message_raw() const;

	commit ancestor(unsigned long n) const;

	commit parent(unsigned int n) const;

	oid parent_id(unsigned int n) const;

	unsigned int parent_count() const;

	tree tree_to() const;

	oid tree_id() const;

	class repository owner() const;

	const git_commit *g_commit() const;

private:
	friend class repository;
	git_commit *g_commit_;
	ownership owner_;
};
