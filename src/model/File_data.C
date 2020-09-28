#include "specializations.h"
#include "auth/User.h"
#include "Submission.h"
#include "Assignment.h"
#include <git2.h>
#include "File_data.h"
#include "File_meta.h"
#include "../Config.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <mutex>

#include <filesystem>
#include <Wt/Dbo/Impl.h>
#include <fstream>
#include <iostream>

std::unordered_map<std::string, std::mutex*> File_data::mtx_map_;
std::mutex File_data::access_global_;

Bytes::Bytes(std::string const& data)
{
    reserve(data.size());
    assign(data.begin(), data.end());
}

Bytes::Bytes(std::istream& is, int size)
{
    reserve(size);
    std::copy(std::istreambuf_iterator<char>(is), {},
              std::back_inserter(*this));
}

void Bytes::write(std::ostream& os) const
{
    os.write((char const*)data(), size());
}

Bytes::operator std::string() const
{
    std::string result;
    result.reserve(size());
    result.assign(begin(), end());
    return result;
}

int File_data::write_and_commit() {
	
	const std::string user = file_meta_->submission()->user1()->name();

	const std::string dir_path = CONFIG.gsc_repo + "assignment_" + std::to_string(file_meta_->submission()->assignment_number()) + 
			"/" + user;


	std::filesystem::create_directories(dir_path);
	const std::string file_path = dir_path + '/' + file_meta_->name();
	
	std::mutex* repo_mtx = get_mutex_(dir_path);
	std::lock_guard<std::mutex> lock(*repo_mtx);
	
	std::ofstream file(file_path, std::ios::out);

	git_libgit2_init();
	git_repository *repo = repo_init_(dir_path);

	for (const auto &b : contents_) {
		file << b;
	}
	file.close();

	repo_add_commit_(repo, dir_path.c_str(), user.c_str(), 0);
	git_repository_free(repo);
	git_libgit2_shutdown();

	return 1;
}

int File_data::delete_and_commit() {

    // Stringify assignment number, user, and data.
    const std::string user = file_meta_->submission()->user1()->name();
	
	const std::string dir_path = CONFIG.gsc_repo + "assignment_" + std::to_string(file_meta_->submission()->assignment_number()) + 
			"/" + user;

	
	std::filesystem::create_directories(dir_path);
	const std::string file_path = dir_path + '/' + file_meta_->name();
	
	std::mutex *repo_mtx = get_mutex_(dir_path);
    std::lock_guard<std::mutex> lock(*repo_mtx);
	
	git_libgit2_init();
	git_repository *repo = repo_init_(dir_path);
	
	std::remove(file_path.c_str());
	
	repo_add_commit_(repo, dir_path.c_str(), user.c_str(), 1);
	git_repository_free(repo);
	git_libgit2_shutdown();

	return 1;
}

int File_data::populate_contents() {

    // Stringify assignment number, user, and data. 
	const std::string user = file_meta_->submission()->user1()->name();
    const std::string dir_path = CONFIG.gsc_repo + "assignment_" + 
			std::to_string(file_meta_->submission()->assignment_number()) +  
			"/" + user;

	const std::string file_path = dir_path + '/' + file_meta_->name();

	std::ifstream file(file_path, std::ifstream::in);
	if (file.fail()) {
		return 0;
	}
	std::streampos fileSize;
	file.seekg(0, std::ios::end);
	fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	contents_.reserve(fileSize);
	contents_.insert(contents_.begin(),
					 std::istream_iterator<unsigned char>(file),
					 {});
	return 1;
}

git_repository* File_data::repo_init_(std::string repo_path) {

	const char* path = repo_path.c_str();
	git_repository* repo;
	if (!git_repository_open_ext(&repo, path, 0, NULL)) {
		return repo;
	}
	
	git_repository_init(&repo, path, 0);
	return repo;
}

bool File_data::repo_add_commit_(git_repository *repo, 
				const char* repo_path, const char* committer, int update) {

	git_oid commit_oid, tree_oid;
	git_tree *tree;
	git_index *index;
	git_object *parent = NULL;
	git_reference *ref = NULL;
	git_signature *signature;
	git_strarray array;
	array.strings = (char**) &repo_path;
	array.count = 1;

	int error;

	error = git_revparse_ext(&parent, &ref, repo, "HEAD");
	if (error == GIT_ENOTFOUND) {
		error = 0;
	} else if (error) {
		const git_error *err = git_error_last();
		if (err) std::cout << "Error " << err->klass << " " << err->message << std::endl;
		return false;

	} else {
		std::cout << "Head is found. Creating second commit!\n\n" << std::endl;
	}
	
	if (git_repository_index(&index, repo)) {
		return false;
	}

	if (!update) {

		if (git_index_add_all(index, NULL, 0, NULL, NULL)){ 
			return false;
		}
	} else {
		
		if (git_index_update_all(index, NULL, NULL, NULL)) {
			return false;
		}
	}

	if (git_index_write(index)) {
		return false;
	}

	if (git_index_write_tree(&tree_oid, index)) {
		return false;
	}

	if (git_tree_lookup(&tree, repo, &tree_oid)) {
		return false;
	}

	if (git_signature_now(&signature, committer, committer)) {
		return false;
	}

	if (git_commit_create_v(
							&commit_oid,
							repo,
							"HEAD",
							signature,
							signature,
							NULL,
							"Message",
							tree,
							parent ? 1 : 0, parent)) {

		return false;
	}

	git_index_free(index);
	git_signature_free(signature);
	git_tree_free(tree);

	return true;
}	

std::mutex* File_data::get_mutex_(std::string repo_path) {

	const std::lock_guard<std::mutex> lock(access_global_);
	
	if (mtx_map_.count(repo_path)){
		return mtx_map_[repo_path];
	}
	
	mtx_map_[repo_path] = new std::mutex();

	return mtx_map_[repo_path];

}	
