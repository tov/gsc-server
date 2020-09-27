#include "specializations.h"
#include "Submission.h"
#include "Assignment.h"
#include "git/repository.h"
#include "git/index.h"
#include "File_data.h"
#include "File_meta.h"
#include "../Config.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdio>
#include <cstdlib>

#include <filesystem>
#include <Wt/Dbo/Impl.h>
#include <fstream>

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

	// Stringify assignment number, user, and data.
	const std::string dir_path = CONFIG.gsc_repo + "assignment_" + std::to_string(file_meta_->submission()->assignment_number()) + "/" + file_meta_->submission()->assignment()->name();
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
		// Init the path
		repository repo = repository::init(dir_path);

		//Write to file;
		fwrite (&contents_[0], sizeof(std::vector<unsigned char>::value_type), contents_.size(), fd);
		
		// Stage 
		auto index = repo.create_index();
		index.add_entries_that_match({});
		index.write();
		auto tree_oid = index.write_tree();

		//Prep Signatures
		auto author = signature("test", "test@email.com");
		auto committer = signature("test", "test@email.com");

		// Get parent if unborn HEAD
		commit parent(nullptr, ownership::user);
		if (!git_repository_head_unborn((git_repository*)repo.g_repository())) {
			oid parent_id;
			commit parent;
			git_reference_name_to_id(parent_id.g_oid(), (git_repository*)repo.g_repository(), "HEAD");
			git_commit *parent_commit = (git_commit*)parent.g_commit();
			git_commit_lookup(&parent_commit, (git_repository*)repo.g_repository(), (const git_oid*)parent_id.g_oid());
		}
		//Commit
		auto commit_oid = repo.create_commit("HEAD", author, committer, "utf-8", "Commit",
											 repo.lookup_tree(tree_oid), {parent});
	}

	fclose(fd);
	return 1;
}

int File_data::delete_and_commit() {

    // Stringify assignment number, user, and data.
    const std::string dir_path = CONFIG.gsc_repo + "assignment_" + std::to_string(file_meta_->submission()->assignment_number()) + "/" + file_meta_->submission()->assignment()->name();
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
	}
	return 1;
}

int File_data::populate_contents() {

    // Stringify assignment number, user, and data.
    const std::string dir_path = CONFIG.gsc_repo + "assignment_" + std::to_string(file_meta_->submission()->assignment_number()) + "/" + file_meta_->submission()->assignment()->name();
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
