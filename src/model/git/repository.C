#include "repository.h"
#include "index.h"
#include "commit.h"
#include "oid.h"
#include "tree.h"
#include "git_exception.h"
#include <functional>

repository::repository(git_repository *g_repository) : g_repository_(g_repository) {}

repository::~repository() {
  if (g_repository_){
    git_repository_free(g_repository_);
  }
}

repository repository::init(const std::string &path) {
  repository result(nullptr);
  if (git_repository_init(&result.g_repository_, path.c_str(), false)){
    throw git_exception();
  }
  return result;
}

repository repository::open(const std::string &path) {
  repository result(nullptr);
  if (git_repository_open(&result.g_repository_, path.c_str())){
    throw git_exception();
  }
  return result;
}

index repository::create_index() const {
  index result(nullptr, ownership::user);
  if (git_repository_index(&result.g_index_, g_repository_)){
    throw git_exception();
  }
  return result;
}

oid repository::create_commit(const std::string &update_ref,
                              const signature &author,
                              const signature &committer,
                              const std::string &message_encoding,
                              const std::string &message, const tree &tree,
                              const std::vector<commit> &parents) const {
  oid result;
  const char *update_ref_c = (update_ref == "") ? NULL : update_ref.c_str();
  const char *message_encoding_c =
      message_encoding == "" ? NULL : message_encoding.c_str();
  std::vector<const git_commit*> parents_c;

  for (auto &p : parents) {
    parents_c.push_back(p.g_commit());
  }
  if (git_commit_create_v(result.g_oid(), g_repository_, update_ref_c, author.g_signature(),
                        committer.g_signature(), message_encoding_c, message.c_str(),
                        tree.g_tree(), parents.size(), parents_c.data())){
    throw git_exception();
  }
  return result;
}

tree repository::lookup_tree(const oid &id) const {
  tree result(nullptr, ownership::user);
  if (git_tree_lookup(&result.g_tree_, g_repository_, id.g_oid())) {
    throw git_exception();
  }
  return result;
}

const git_repository *repository::g_repository() const { return g_repository_; }
