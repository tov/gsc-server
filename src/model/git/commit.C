#include "commit.h"
#include "tree.h"
#include "git_exception.h"
#include "oid.h"
#include "ownership.h"
#include "signature.h"
#include "repository.h"

commit::commit() : g_commit_(nullptr), owner_(ownership::libgit2) {}

commit::commit(git_commit *g_commit, ownership owner)
    : g_commit_(g_commit), owner_(owner) {}

commit::~commit() {
  if (g_commit_ && owner_ == ownership::user)
    git_commit_free(g_commit_);
}

signature commit::author() const {
  auto ret = git_commit_author(g_commit_);
  return signature(ret->name, ret->email);
}

std::string commit::body() const {
  auto ret = git_commit_body(g_commit_);
  if (ret)
    return std::string(ret);
  else
    return "";
}

signature commit::committer() const {
  auto ret = git_commit_committer(g_commit_);
  return signature(ret->name, ret->email);
}

oid commit::id() const { return oid(git_commit_id(g_commit_)); }

std::string commit::message() const {
  auto ret = git_commit_message(g_commit_);
  if (ret)
    return std::string(ret);
  else
    return "";
}

std::string commit::message_raw() const {
  auto ret = git_commit_message_raw(g_commit_);
  if (ret)
    return std::string(ret);
  else
    return "";
}

commit commit::ancestor(unsigned long n) const {
  commit result(nullptr, ownership::user);
  if (git_commit_nth_gen_ancestor(&result.g_commit_, g_commit_, n))
    throw git_exception();
  return result;
}

commit commit::parent(const unsigned int n) const {
  commit result(nullptr, ownership::user);
  if (git_commit_parent(&result.g_commit_, g_commit_, n))
    throw git_exception();
  return result;
}

oid commit::parent_id(unsigned int n) const {
  return oid(git_commit_parent_id(g_commit_, n));
}

unsigned int commit::parent_count() const {
  return git_commit_parentcount(g_commit_);
}

tree commit::tree_to() const {
  tree result(nullptr, ownership::user);
  if (git_commit_tree(&result.g_tree_, g_commit_))
    throw git_exception();
  return result;
}

oid commit::tree_id() const { return oid(git_commit_tree_id(g_commit_)); }

repository commit::owner() const {
  return repository(git_commit_owner(g_commit_));
}

const git_commit *commit::g_commit() const { return g_commit_; }

