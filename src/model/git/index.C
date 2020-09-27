#include "index.h"
#include "strarray.h"
#include "oid.h"
#include "tree.h"
#include "ownership.h"
#include "repository.h"
#include <functional>

index::index(): g_index_(nullptr), owner_(ownership::user) {
	git_index_new(&g_index_);
}

index::index(git_index *g_index, ownership owner)
		: g_index_(g_index), owner_(owner) {}

index::~index() {
	if (g_index_ && owner_ == ownership::user) {
		git_index_free(g_index_);
	}
}

void index::add_entries_that_match(const std::vector<std::string> &pathspec) {

		auto pathspec_c = strarray(pathspec).g_strarray();
		if (git_index_add_all(g_index_, pathspec_c, 0, NULL, NULL)) {
			throw git_exception();
		}
}

void index::add_entry_by_path(const std::string &path) {
	if (git_index_add_bypath(g_index_, path.c_str())) {
		throw git_exception();
	}
}

const oid index::checksum() { 
	return oid(git_index_checksum(g_index_)); 
}

void index::clear() {
	if (git_index_clear(g_index_)) {
		throw git_exception();
	}
}

size_t index::size() const {
	return git_index_entrycount(g_index_);
}

bool index::has_conflicts() const {
	return git_index_has_conflicts(g_index_);
}

std::string index::path() const {
	auto ret = git_index_path(g_index_);
	if (ret)
		return std::string(ret);
	else
		return "";
}

void index::read_tree(const tree &tree) {
  if (git_index_read_tree(g_index_, tree.g_tree())) {
    throw git_exception();
  }
}

oid index::write_tree() {
  oid result;
  if (git_index_write_tree(result.g_oid(), g_index_)) {
    throw git_exception();
  }
  return result;
}

repository index::owner() const {
   	return repository(git_index_owner(g_index_)); 
}

void index::write() {
  if (git_index_write(g_index_))
    throw git_exception();
}

const git_index *index::g_index() const { 
	return g_index_; 
}
