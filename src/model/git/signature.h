#pragma once
#include "git_exception.h"
#include "libgit2_api.h"
#include <git2.h>
#include <string>

class signature : public libgit2_api {
public:
	signature(const std::string &name, const std::string &email);
	 const git_signature *g_signature() const;
private:
	friend class repository;
	git_signature *g_signature_;
};

