#include "commit.h"
#include "tree.h"
#include "oid.h"
#include "ownership.h"
#include "repository.h"
#include <functional>

tree::tree() : g_tree_(nullptr), owner_(ownership::libgit2) {}

tree::tree(git_tree *g_tree, ownership owner) : g_tree_(g_tree), owner_(owner) {}

tree::~tree() {
  if (g_tree_ && owner_ == ownership::user)
    git_tree_free(g_tree_);
}

oid tree::id() const { return oid(git_tree_id(g_tree_)); }

git_tree *tree::g_tree() { return g_tree_; }

const git_tree *tree::g_tree() const { return g_tree_; }

