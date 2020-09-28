#include "specializations.h"
#include "auth/User.h"
#include "Submission.h"
#include "Assignment.h"
#include <git2.h>
#include "File_data.h"
#include "File_meta.h"
#include "../Config.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdio>
#include <cstdlib>
#include <mutex>

#include <filesystem>
#include <Wt/Dbo/Impl.h>
#include <fstream>
#include <iostream>

std::mutex File_data::mtx_;

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
	
	//mtx_.lock();
	const std::string user = file_meta_->submission()->user1()->name();

	const std::string dir_path = CONFIG.gsc_repo + "assignment_" + std::to_string(file_meta_->submission()->assignment_number()) + 
			"/" + user;

	std::filesystem::create_directories(dir_path);
	const std::string file_path = dir_path + '/' + file_meta_->name();

	struct flock fl;
	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 0;
	FILE* fd = fopen(file_path.c_str(), "w+");
	if (fcntl(fileno(fd), F_SETLKW, &fl) == -1) {
		return 0;
	} else {
		//Acquired lock successfully
		struct stat st0;
		fstat(fileno(fd), &st0);
		if (!st0.st_nlink) {
			fclose(fd);
			return this->write_and_commit();
		}
	
		git_libgit2_init();
		git_repository *repo = repo_init_(dir_path);
		fwrite (&contents_[0], sizeof(std::vector<unsigned char>::value_type), contents_.size(), fd);
		fclose(fd);


		repo_add_commit_(repo, dir_path.c_str(), user.c_str(), 0);
		git_repository_free(repo);
		git_libgit2_shutdown();

	}

	return 1;
}

int File_data::delete_and_commit() {

    // Stringify assignment number, user, and data.
    const std::string user = file_meta_->submission()->user1()->name();

	const std::string dir_path = CONFIG.gsc_repo + "assignment_" + std::to_string(file_meta_->submission()->assignment_number()) + 
			"/" + user;

	
	std::filesystem::create_directories(dir_path);
	const std::string file_path = dir_path + '/' + file_meta_->name();

	struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;

	FILE* fd = fopen(file_path.c_str(), "w+");
	if (fcntl(fileno(fd), F_SETLKW, &fl) == -1) {			
		return 0;
	} else {
		struct stat st0;
		fstat(fileno(fd), &st0);
		if (st0.st_nlink) {
			unlink(file_path.c_str());
		}
		fclose(fd);

		git_libgit2_init();
		git_repository *repo = repo_init_(dir_path);

		repo_add_commit_(repo, dir_path.c_str(), user.c_str(), 1);
		git_repository_free(repo);
		git_libgit2_shutdown();


	}
	return 1;
}

int File_data::populate_contents() {

    // Stringify assignment number, user, and data. 
	const std::string user = file_meta_->submission()->user1()->name();
    const std::string dir_path = CONFIG.gsc_repo + "assignment_" + std::to_string(file_meta_->submission()->assignment_number()) +  
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
