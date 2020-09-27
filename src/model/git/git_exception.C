#include <git2.h>
#include "git_exception.h"

git_exception::git_exception() {
	auto error = git_error_last();
	message_ = error? error->message: "unknown error";
}

git_exception::git_exception(const char *message) : message_(message) {}
git_exception::git_exception(const std::string &message) : message_(message.c_str()) {}

git_exception::~git_exception() noexcept {}
const char* git_exception::what() const noexcept { return message_; }

void git_exception::clear() { git_error_clear(); }


