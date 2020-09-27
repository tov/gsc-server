#include "libgit2_api.h"
#include "git_exception.h"

libgit2_api::libgit2_api() {
	git_libgit2_init();
}

libgit2_api::~libgit2_api() {
	git_libgit2_shutdown();
}

std::tuple<int,int,int> libgit2_api::version() const {
	int major, minor, rev;
	if (git_libgit2_version(&major, &minor, &rev)) {
		throw git_exception();
	}
	return std::tuple<int, int, int>{major, minor, rev};
}
