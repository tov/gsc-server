#include "commit.h"
#include "index.h"
#include "libgit2_api.h"
#include "oid.h"
#include <git2.h>
#include <string>
#include <utility>
#include <vector>

class repository : public libgit2_api{
public:
	// Create a git repo
	repository(git_repository *repo = nullptr);

	// Free a git repo
	~repository();

	// git init
	static repository init(const std::string &path);

	// Open a git repo
	static repository open(const std::string &path);

	// Construct in-memory index object
	class index create_index() const;

	// Create commit
	oid create_commit(const std::string &update_ref, const signature &author,
                    const signature &committer,
                    const std::string &message_encoding,
                    const std::string &message, const tree &tree,
                    const std::vector<commit> &parents) const;

	tree lookup_tree(const oid &id) const;

	const git_repository *g_repository() const;

private:
	friend class index;
	git_repository *g_repository_;
};

