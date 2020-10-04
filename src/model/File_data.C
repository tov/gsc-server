#include "../Config.h"
#include "Assignment.h"
#include "File_data.h"
#include "File_meta.h"
#include "Submission.h"
#include "auth/User.h"
#include "specializations.h"
#include <git2.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <Wt/Dbo/Impl.h>
#include <cstdio>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unistd.h>

namespace fs = std::filesystem;

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
    os.write((char const*) data(), size());
}

Bytes::operator std::string() const
{
    std::string result;
    result.reserve(size());
    result.assign(begin(), end());
    return result;
}

// return 0 on success
int File_data::write_and_commit()
{

    const std::string user = file_meta_->submission()->user1()->name();
    const fs::path dir_path =
            fs::path(CONFIG.gsc_repo) /
            fs::path(std::to_string(
                    file_meta_->submission()->assignment_number())) /
            fs::path(user);
    const fs::path file_path = dir_path / fs::path(file_meta_->name());
    const fs::path lock_path = dir_path / fs::path(CONFIG.git_lockfile);

    fs::create_directories(dir_path);

    struct flock fl;
    fl.l_type   = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start  = 0;
    fl.l_len    = 0;

    FILE* lock_fd = fopen(lock_path.c_str(), "w+");
    if (! lock_fd) { return 1; }

    if (fcntl(fileno(lock_fd), F_SETLKW, &fl) == -1) { return 1; }


    std::ofstream file(file_path.c_str(), std::ios::out);
    if (file.fail()) { return 1; }
    git_libgit2_init();
    git_repository* repo = repo_init_(dir_path);
    int error            = 0;

    if (! repo) {
        error = 1;
        git_libgit2_shutdown();
        return error;
    }

    for (const auto& b : contents_) { file << b; }

    file.close();
    if (repo_add_commit_(repo, dir_path.c_str(), user.c_str(), 0)) {
        error = 1;
    }

    git_repository_free(repo);
    git_libgit2_shutdown();
    fclose(lock_fd);
    return error;
}

// return 0 on success
int File_data::delete_and_commit()
{

    // Stringify assignment number, user, and data.
    const std::string user = file_meta_->submission()->user1()->name();
    const fs::path dir_path =
            fs::path(CONFIG.gsc_repo) /
            fs::path(std::to_string(
                    file_meta_->submission()->assignment_number())) /
            fs::path(user);
    const fs::path file_path = dir_path / fs::path(file_meta_->name());
    const fs::path lock_path = dir_path / fs::path(CONFIG.git_lockfile);

    fs::create_directories(dir_path);

    struct flock fl;
    fl.l_type   = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start  = 0;
    fl.l_len    = 0;

    FILE* lock_fd = fopen(lock_path.c_str(), "w+");
    if (lock_fd == NULL) { return 1; }

    if (fcntl(fileno(lock_fd), F_SETLKW, &fl) == -1) { return 1; }

    git_libgit2_init();
    git_repository* repo;
    int error = 0;

    if (! (repo = repo_init_(dir_path))) {
        error = 1;
        git_libgit2_shutdown();
        fclose(lock_fd);
        return error;
    }

    if (std::remove(file_path.c_str())) {
        error = 1;
        git_repository_free(repo);
        git_libgit2_shutdown();
        fclose(lock_fd);
        return error;
    }

    if (repo_add_commit_(repo, dir_path.c_str(), user.c_str(), 1)) {
        error = 1;
    }

    git_repository_free(repo);
    git_libgit2_shutdown();
    fclose(lock_fd);

    return error;
}

// Returns true on success
bool File_data::populate_contents()
{

    // Stringify assignment number, user, and data.
    const std::string user = file_meta_->submission()->user1()->name();
    const fs::path dir_path =
            fs::path(CONFIG.gsc_repo) /
            fs::path(std::to_string(
                    file_meta_->submission()->assignment_number())) /
            fs::path(user);
    const fs::path file_path = dir_path / fs::path(file_meta_->name());

    std::ifstream file(file_path.c_str(), std::ifstream::in);
    if (file.fail()) { return false; }

    std::streampos fileSize;
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    contents_.reserve(fileSize);
    contents_.insert(contents_.begin(),
                     std::istream_iterator<unsigned char>(file), {});
    return true;
}

git_repository* File_data::repo_init_(std::string repo_path)
{

    const char* path = repo_path.c_str();
    git_repository* repo;
    if (! git_repository_open_ext(&repo, path, 0, NULL)) { return repo; }

    if (! git_repository_init(&repo, path, 0)) { return repo; }
    return NULL;
}

// returns 0 on success
int File_data::repo_add_commit_(git_repository* repo, const char* repo_path,
                                const char* committer, int update)
{

    git_oid commit_oid, tree_oid;
    git_tree* tree;
    git_index* index;
    git_object* parent = NULL;
    git_reference* ref = NULL;
    git_signature* signature;
    git_strarray array;
    array.strings = (char**) &repo_path;
    array.count   = 1;

    int error;

    error = git_revparse_ext(&parent, &ref, repo, "HEAD");
    if (error == GIT_ENOTFOUND) {
        error = 1;
    } else if (error) {
        error = 1;
        return error;
    }

    error = 0;

    if (git_repository_index(&index, repo)) {
        error = 1;
        return error;
    }

    if (! update) {

        if (git_index_add_all(index, NULL, 0, NULL, NULL)) {
            error = 1;
            git_index_free(index);
            return error;
        }
    } else {

        if (git_index_update_all(index, NULL, NULL, NULL)) {
            error = 1;
            git_index_free(index);
            return error;
        }
    }

    if (git_index_write(index)) {
        error = 1;
        git_index_free(index);
        return error;
    }

    if (git_index_write_tree(&tree_oid, index)) {
        error = 1;
        git_index_free(index);
        return error;
    }

    if (git_tree_lookup(&tree, repo, &tree_oid)) {
        error = 1;
        git_index_free(index);
        return error;
    }

    if (git_signature_now(&signature, committer, committer)) {
        error = 1;
        git_index_free(index);
        git_tree_free(tree);
        return error;
    }

    if (git_commit_create_v(&commit_oid, repo, "HEAD", signature, signature,
                            NULL, "Message", tree, parent ? 1 : 0, parent)) {

        error = 1;
    }

    git_index_free(index);
    git_signature_free(signature);
    git_tree_free(tree);

    return error;
}
