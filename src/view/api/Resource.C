#include "Resource.h"
#include "Http_status.h"
#include "Request_body.h"
#include "Request_handler.h"
#include "../../Session.h"
#include "../../model/auth/User.h"
#include "../../model/Assignment.h"
#include "../../model/File_data.h"
#include "../../model/File_meta.h"
#include "../../model/Submission.h"

#include <Wt/Json/Value.h>
#include <Wt/Json/Serializer.h>
#include <Wt/Utils.h>

#include <cstdlib>
#include <regex>
#include <sstream>
#include <string>

namespace dbo = Wt::Dbo;
namespace J = Wt::Json;

namespace api {

namespace Resource {

void Base::denied(int code)
{
    std::ostringstream os;
    os << "You can't do that (code " << code << ")";
    throw Http_status{403, os.str()};
}

void Base::not_found()
{
    throw Http_status{404, "The named resource does not exist"};
}

void Base::not_supported()
{
    throw Http_status{405, "The resource does not support that method"};
}

void Base::send(Wt::Http::Response& response) const
{
    if (content_type.empty()) {
        Http_status{500, "No content type"}.respond(response);
    } else {
        response.setMimeType(content_type);
        response.out().write((const char *)contents.data(), contents.size());
    }
}

class Users : public Base
{
public:
    void load(Context const&) override;

protected:
    void do_get_(Context const&) override;

private:
    dbo::collection<dbo::ptr<User>> users_;
};

void Users::load(Context const& context)
{
    if (!context.user->can_admin())
        denied(1);

    users_ = context.session.find<User>();
}

void Users::do_get_(Context const&)
{
    J::Array result;

    for (auto const& user : users_)
        result.push_back(user->to_json(true));

    use_json(result);
}

class Users_1 : public Base
{
public:
    Users_1(std::string username)
            : username_{std::move(username)} {}

    void load(Context const&) override;

protected:
    void do_get_(Context const&) override;
    void do_delete_(Context const&) override;
    void do_patch_(Request_body body, Context const&) override;

private:
    std::string username_;
    dbo::ptr<User> user_;
};

dbo::ptr<User>
Base::load_user(Context const& context, std::string const& username)
{
    if (context.user->name() != username && !context.user->can_admin())
        denied(2);

    auto user = User::find_by_name(context.session, username);
    if (user) return user;

    not_found();
}

Wt::Dbo::ptr<Assignment>
Base::load_assignment(const Base::Context& context, int number)
{
    auto result = Assignment::find_by_number(context.session, number);
    if (result) return result;
    not_found();
}

Wt::Dbo::ptr<Submission>
Base::load_submission(Context const& context, int submission_id)
{
    auto submission = Submission::find_by_id(context.session, submission_id);
    if (!submission) not_found();
    if (!submission->can_view(context.user)) denied(7);
    return submission;
}

void Users_1::load(Context const& context)
{
    user_ = load_user(context, username_);
}

void Users_1::do_get_(Context const&)
{
    use_json(user_->to_json());
}

void Users_1::do_delete_(Context const& context)
{
    if (!context.user->can_admin())
        denied(3);

    user_.remove();
    success();
}

void Users_1::do_patch_(Request_body body, Context const& context)
{
    auto json = std::move(body).read_json();

    if (json.type() != J::Type::Object)
        throw Http_status{400, "PATCH /user/_1 expected a JSON object"};
    J::Object const& object = json;

    for (auto const& pair : object) {
        if (pair.first == "role") {
            if (!context.user->can_admin())
                denied(4);

            try {
                auto role = destringify<User::Role>(pair.second);
                std::cerr << "Modifying user " << user_->name()
                        << " to role " << stringify(role) << "\n";
                user_.modify()->set_role(role);
            } catch (std::invalid_argument const& e) {
                throw Http_status{400, e.what()};
            }
        }

        else if (pair.first == "password") {
            if (!context.user->can_admin()) {
                Credentials creds{user_->name(), pair.second};
                Request_handler::check_password_strength(creds);
            }

            context.session.set_password(user_, pair.second);
        }

        else
            throw Http_status{400, "PATCH got unknown JSON key: " + pair.first};
    }

    success();
}

class Users_1_submissions : public Base
{
public:
    Users_1_submissions(std::string username)
            : username_{std::move(username)} {}

    void load(Context const&) override;

protected:
    void do_get_(Context const& context) override;

private:
    std::string username_;
    dbo::ptr<User> user_;
    std::vector<dbo::ptr<Submission>> submissions_;
};

void Users_1_submissions::load(Context const& context)
{
    user_ = load_user(context, username_);
    submissions_ = user_->submissions();
}

void Users_1_submissions::do_get_(const Base::Context& context)
{
    J::Array result;
    for (const auto& each : submissions_) {
        result.push_back(each->to_json(true));
    }
    use_json(result);
}

class Submissions_1 : public Base
{
public:
    Submissions_1(int submission_id)
            : submission_id_{submission_id}
    { }

    void load(Context const&) override;

protected:
    void do_delete_(Context const& context) override;
    void do_get_(Context const& context) override;

private:
    int submission_id_;

    dbo::ptr<Submission> submission_;
};

void Submissions_1::load(const api::Resource::Base::Context& context)
{
    submission_ = load_submission(context, submission_id_);
}

void Submissions_1::do_delete_(const api::Resource::Base::Context& context)
{
    if (!submission_->can_submit(context.user))
        denied(6);

    submission_.remove();
    success();
}

void Submissions_1::do_get_(const api::Resource::Base::Context& context)
{
    use_json(submission_->to_json());
}

class Submissions_1_files : public Base
{
public:
    Submissions_1_files(int submission_id)
            : submission_id_{submission_id}
    { }

    void load(Context const&) override;

protected:
    void do_get_(Context const& context) override;

private:
    int submission_id_;
    std::vector<dbo::ptr<File_meta>> file_metas_;
};

void Submissions_1_files::load(Context const& context)
{
    auto submission = load_submission(context, submission_id_);
    auto file_metas = submission->source_files_sorted(true);
    file_metas_.assign(file_metas.begin(), file_metas.end());
}

void Submissions_1_files::do_get_(const Base::Context& context)
{
    J::Array json;
    for (auto const& each : file_metas_)
        json.push_back(each->to_json());
    use_json(json);
}

class Submissions_1_files_2 : public Base
{
public:
    Submissions_1_files_2(int submission_id,
                          std::string filename)
            : submission_id_{submission_id}
            , filename_{std::move(filename)} {}

    void load(Context const&) override;

protected:
    void do_delete_(Context const& context) override;
    void do_get_(Context const& context) override;
    void do_put_(Request_body body, Context const& context) override;

private:
    int submission_id_;
    std::string filename_;

    dbo::ptr<Submission> submission_;
    dbo::ptr<File_meta> file_meta_;
};

void Submissions_1_files_2::load(Context const& context)
{
    submission_ = load_submission(context, submission_id_);
    file_meta_ = submission_->find_file_by_name(filename_);
}

void Submissions_1_files_2::do_delete_(const Base::Context& context)
{
    if (file_meta_)
        file_meta_.remove();
    success();
}

void Submissions_1_files_2::do_get_(const Base::Context& context)
{
    if (!file_meta_) not_found();

    content_type = file_meta_->media_type();
    contents = file_meta_->file_data().lock()->contents();
}

void Submissions_1_files_2::do_put_(
        Request_body body, const Base::Context& context)
{
    if (!submission_->can_submit(context.user))
        denied(8);

    if (!submission_->has_sufficient_space(body.size(), filename_))
        throw Http_status{403, "Upload would exceed quota"};

    auto file_meta = File_meta::upload(
            filename_,
            std::move(body).read_bytes(),
            submission_);

    use_json(file_meta->to_json());
}

std::unique_ptr<Base> Base::create(std::string const& method,
                                   std::string const& path_info)
{
    auto resource = parse_(path_info);
    resource->method_ = method;
    return std::move(resource);
}

std::unique_ptr<Base> Base::parse_(std::string const& path_info)
{
    std::smatch sm;

    auto get_number = [](const auto& sm_i) {
        std::string str(sm_i.first, sm_i.second);
        return std::atoi(str.c_str());
    };

    if (std::regex_match(path_info, Path::users)) {
        return std::make_unique<Users>();
    }

    if (std::regex_match(path_info, sm, Path::users_1)) {
        std::string username(sm[1].first, sm[1].second);
        return std::make_unique<Users_1>(std::move(username));
    }

    if (std::regex_match(path_info, sm, Path::users_1_submissions)) {
        std::string username(sm[1].first, sm[1].second);
        return std::make_unique<Users_1_submissions>(std::move(username));
    }

    if (std::regex_match(path_info, sm, Path::submissions_1)) {
        int submission_id = get_number(sm[1]);
        return std::make_unique<Submissions_1>(submission_id);
    }

    if (std::regex_match(path_info, sm, Path::submissions_1_files)) {
        int submission_id = get_number(sm[1]);
        return std::make_unique<Submissions_1_files>(submission_id);
    }

    if (std::regex_match(path_info, sm, Path::submissions_1_files_2)) {
        int submission_id = get_number(sm[1]);
        std::string filename(sm[2].first, sm[2].second);
        return std::make_unique<Submissions_1_files_2>(
                submission_id, Wt::Utils::urlDecode(filename));
    }

    not_found();
}

void Base::process(Wt::Http::Request const& request,
                   Context const& context)
{
    Request_body body{request};

    if (body.size() > File_meta::max_byte_count)
        throw Http_status{413, "Request exceeds maximum file upload size"};

    if (method_ == "DELETE")
        do_delete_(context);
    else if (method_ == "GET")
        do_get_(context);
    else if (method_ == "PATCH")
        do_patch_(body, context);
    else if (method_ == "POST")
        do_post_(body, context);
    else if (method_ == "PUT")
        do_put_(body, context);
    else not_supported();
}

void Base::do_delete_(Context const&)
{
    not_supported();
}

void Base::do_get_(Context const&)
{
    not_supported();
}

void Base::do_patch_(Request_body, Context const&)
{
    not_supported();
}

void Base::do_post_(Request_body, Context const&)
{
    not_supported();
}

void Base::do_put_(Request_body, Context const&)
{
    not_supported();
}

} // end namespace Resource

} // end namespace api
