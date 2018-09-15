#include "Resource.h"
#include "Http_status.h"
#include "Request_body.h"
#include "Request_handler.h"
#include "Result_array.h"
#include "../../Session.h"
#include "../../model/auth/User.h"
#include "../../model/Assignment.h"
#include "../../model/Exam_grade.h"
#include "../../model/File_data.h"
#include "../../model/File_meta.h"
#include "../../model/Submission.h"
#include "../../model/Partner_request.h"

#include <Wt/Json/Value.h>
#include <Wt/Json/Serializer.h>
#include <Wt/Utils.h>
#include <Wt/WLocalDateTime.h>

#include <algorithm>
#include <cstdlib>
#include <map>
#include <regex>
#include <sstream>
#include <string>

namespace dbo = Wt::Dbo;
namespace J = Wt::Json;

namespace api {

namespace Resource {

void Base::denied(int code)
{
    Http_error{403} << "You can't do that (code " << code << ")";
}

void Base::not_found()
{
    Http_error{404} << "The named resource does not exist";
}

void Base::not_supported()
{
    Http_error{405} << "The resource does not support that method";
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

class Grades_csv : public Base
{
public:
    void load(Context const&) override;

protected:
    void do_get_(Context const&) override;

private:
    using Exams             = std::vector<int>;
    using Exam_scores       = std::map<int, double>;
    using Assignments       = std::vector<dbo::ptr<Assignment>>;
    using Assignment_scores = std::map<int, double>;
    using Score_map         = std::map<std::string, std::pair<Exam_scores, Assignment_scores>>;

    Exams exams_;
    Assignments assigns_;
    Score_map scores_;
};

void Grades_csv::load(const Base::Context& context) {
    auto& dbo = context.session;

    const auto assigns_q = dbo.find<Assignment>()
            .orderBy("number")
            .resultList();
    const auto exams_q = dbo.query<int>("SELECT DISTINCT number FROM exam_grades")
            .orderBy("number")
            .resultList();

    assigns_.assign(assigns_q.begin(), assigns_q.end());
    exams_.assign(exams_q.begin(), exams_q.end());

    auto users = dbo.find<User>()
            .where("role = ?").bind(int(User::Role::Student))
            .resultList();

    for (auto const& user : users) {
        Exam_scores user_exams;
        Assignment_scores user_assigns;

        for (const auto& assign : assigns_) {
            auto submission = Submission::find_by_assignment_and_user(dbo, assign, user);
            user_assigns[assign->number()] = submission? submission->grade() : 0;
        }

        for (int exam : exams_) {
            auto exam_grade = Exam_grade::find_by_user_and_number(user, exam);
            user_exams[exam] = exam_grade? exam_grade->grade() : 0;
        }

        scores_[user->name()] = std::make_pair(std::move(user_assigns),
                                               std::move(user_exams));
    }
}

static double lookup_or_0(std::map<int, double> const& map, int key) {
    auto iter = map.find(key);
    return iter == map.end() ? 0.0 : iter->second;
}

void Grades_csv::do_get_(const Context& context)
{
    std::ostringstream o;

    o << "NetId";
    for (auto const& assign : assigns_)
        o << ",hw" << assign->number();
    for (int exam : exams_)
        o << ",ex" << exam;
    o << "\n";

    for (auto const& row : scores_) {
        o << row.first;

        auto const& user_assigns = row.second.first;
        for (auto const& assign : assigns_)
            o << "," << lookup_or_0(user_assigns, assign->number());

        auto const& user_exams   = row.second.second;
        for (int exam : exams_) {
            o << "," << lookup_or_0(user_exams, exam);
        }

        o << "\n";
    }

    std::string buffer1 = o.str();
    std::vector<unsigned char> buffer2(buffer1.begin(), buffer1.end());

    content_type = "text/csv";
    contents = buffer2;
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
    Result_array result;

    try {
        auto json = std::move(body).read_json();
        
        J::Object const& object = json;

        for (auto const& pair : object) {
            if (pair.first == "role") {
                if (!context.user->can_admin())
                    denied(4);

                auto role = destringify<User::Role>(pair.second);
                user_.modify()->set_role(role);
                result.success() << "Role updated to " << stringify(role) << ".";
            }

            else if (pair.first == "password") {
                if (!context.user->can_admin()) {
                    Credentials creds{user_->name(), pair.second};
                    Request_handler::check_password_strength(creds);
                }

                context.session.set_password(user_, pair.second);
                result.success() << "Updated password for user " << user_->name() << ".";
            }

            else if (pair.first == "exam_grades") {
                if (!context.user->can_admin())
                    denied(11);

                Result_array nested;

                J::Array const& exams = pair.second;
                for (J::Object const& exam : exams) {
                    int number   = exam.get("number");
                    int points   = exam.get("points");
                    int possible = exam.get("possible");

                    auto exam_grade = Exam_grade::find_by_user_and_number(user_, number);

                    if (possible == 0) {
                        exam_grade.remove();
                        nested.success() << "Removed exam " << number << " grade for "
                                << user_->name() << ".";
                    } else {
                        exam_grade.modify()->set_points_and_possible(points, possible);
                        nested.success() << "Set exam " << number << " grade for "
                                << user_->name() << " to "
                                << points << " / " << possible << ".";
                    }
                }

                result.add_nested(std::move(nested));
            }

            else if (pair.first == "partner_requests") {
                Result_array nested;

                J::Array const& requests = pair.second;
                for (J::Object const& request : requests) {
                    using S = Partner_request::Status;

                    int hw_number = request.get("assignment_number");
                    std::string username = request.get("user");
                    std::string status_string = request.get("status");

                    auto hw = Assignment::find_by_number(context.session, hw_number);
                    auto other = User::find_by_name(context.session, username);
                    auto status = destringify<S>(status_string);

                    if (!hw)
                        Http_error{403} << "hw" << hw_number << " does not exist.";

                    if (!other)
                        Http_error{403} << "User " << username << " does not exist.";

                    std::string user_and_hw;
                    {
                        std::ostringstream o;
                        o << "user " << other->name() << " for hw" << hw_number;
                        user_and_hw = o.str();
                    }

                    switch (status) {
                        case S::outgoing: {
                            // Check for an incoming request, and if it exists, accept it.
                            auto incoming = Partner_request::find_by_requestor_and_assignment(
                                    context.session, other, hw);

                            if (incoming && incoming->requestee() == user_) {
                                if (incoming->confirm(context.session)) {
                                    nested.success()
                                            << "Requested and confirmed partnership with "
                                            << user_and_hw << ".";
                                    break;
                                }
                            }

                            std::ostringstream reason;
                            auto request_ptr = Partner_request::create(
                                    context.session, user_, other, hw, reason);
                            if (!request_ptr) throw Http_status{403, reason.str()};

                            nested.success()
                                    << "Requested partnership with "
                                    << user_and_hw << ".";
                            break;
                        }

                        case S::incoming:
                            nested.failure() << "You cannot create an incoming partner request.";
                            break;

                        case S::accepted: {
                            auto request_ptr = Partner_request::find_by_requestor_and_requestee(
                                    context.session, other, user_, hw);
                            if (request_ptr) {
                                auto submission = request_ptr.modify()->confirm(context.session);
                                if (submission) {
                                    nested.success()
                                            << "Accepted partner request from "
                                            << user_and_hw << ".";
                                } else {
                                    nested.failure()
                                            << "Could not accept partner request from"
                                            << user_and_hw << ".";
                                }
                            } else {
                                nested.failure()
                                        << "You don’t have an incoming partner request from "
                                        << user_and_hw << ".";
                            }
                            break;
                        }

                        case S::canceled: {
                            auto outgoing = Partner_request::find_by_requestor_and_requestee(
                                    context.session, user_, other, hw);
                            auto incoming = Partner_request::find_by_requestor_and_requestee(
                                    context.session, other, user_, hw);

                            if (incoming && outgoing) {
                                nested.success()
                                        << "Removed incoming partner request from "
                                        << user_and_hw
                                        << " and outgoing partner request to "
                                        << user_and_hw << ".";
                            } else if (incoming) {
                                nested.success()
                                        << "Removed incoming partner request from "
                                        << user_and_hw << ".";
                            } else if (outgoing) {
                                nested.success()
                                        << "Removed outgoing partner request to "
                                        << user_and_hw << ".";
                            } else {
                                nested.failure()
                                        << "No partners requests to remove from/to "
                                        << user_and_hw << ".";
                            }

                            incoming.remove();
                            outgoing.remove();
                        }
                    }
                }

                result.add_nested(std::move(nested));
            }

            else {
                result.failure() << "Unknown key in JSON: ‘" << pair.first << "’.";
            }
        }
    } catch (J::TypeException const& e) {
        throw Http_status{400, "PATCH /users/_1 could not understand request"};
    } catch (J::ParseError const& e) {
        throw Http_status{400, "PATCH /users/_1 could not parse user request as JSON"};
    } catch (std::invalid_argument const& e) {
        throw Http_status{400, e.what()};
    }

    use_json(result);
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
    void do_patch_(Request_body body, Context const &context) override;

private:
    int submission_id_;

    dbo::ptr<Submission> submission_;
};

void Submissions_1::load(const Context& context)
{
    submission_ = load_submission(context, submission_id_);
}

void Submissions_1::do_delete_(const Context& context)
{
    if (!submission_->can_submit(context.user))
        denied(6);

    submission_.remove();
    success();
}

void Submissions_1::do_get_(const Context& context)
{
    use_json(submission_->to_json());
}

void Submissions_1::do_patch_(Request_body body, const Base::Context &context) {
    if (!context.user->can_admin())
        denied(10);

    Result_array result;

    try {
        auto json = std::move(body).read_json();
        J::Object const& object = json;

        for (auto const& pair : object) {
            if (pair.first == "due_date") {
                std::string time_spec = pair.second;
                auto local_time = Wt::WLocalDateTime::fromString(time_spec);

                if (local_time.isValid()) {
                    submission_.modify()->set_due_date(local_time.toUTC());
                    result.success() << "Modified due date to " << local_time.toString() << ".";
                } else {
                    result.failure() << "Could not parse timespec ‘" << time_spec << "’.";
                }
            }

            else if (pair.first == "eval_date") {
                std::string time_spec = pair.second;
                auto local_time = Wt::WLocalDateTime::fromString(time_spec);

                if (local_time.isValid()) {
                    submission_.modify()->set_eval_date(local_time.toUTC());
                    result.success() << "Modified eval date to " << local_time.toString() << ".";
                } else {
                    result.failure() << "Could not parse timespec ‘" << time_spec << "’.";
                }
            }

            else if (pair.first == "bytes_quota") {
                int bytes_quota = pair.second;
                submission_.modify()->set_bytes_quota(bytes_quota);
                result.success() << "Modified quota to " << bytes_quota << ".";
            }

            else if (pair.first == "owner2") {
                if (submission_.modify()->divorce())
                    result.success() << "Divorced succeeded.";
                else
                    result.failure() << "Divorced failed; submission is solo already.";
            }

            else {
                result.failure() << "Unknown key in JSON: ‘" << pair.first << "’.";
            }
        }
    } catch (Wt::Json::ParseError const& e) {
        Http_error{400} << e.what();
    } catch (Wt::Json::TypeException const& e) {
        Http_error{400} << e.what();
    }

    use_json(result);
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
            submission_,
            context.user);

    use_json(file_meta->to_json());
}

class Submissions_hw1 : public Base
{
public:
    Submissions_hw1(int assignment_number)
            : assignment_number_{assignment_number}
    { }

    void load(Context const&) override;

protected:
    void do_get_(Context const& context) override;

private:
    int assignment_number_;

    std::vector<dbo::ptr<Submission>> submissions_;
};

void Submissions_hw1::load(Context const& context)
{
    if (!context.user->can_admin())
        denied(9);

    auto assignment = load_assignment(context, assignment_number_);
    std::copy(assignment->submissions().begin(),
              assignment->submissions().end(),
              std::back_inserter(submissions_));
}

void Submissions_hw1::do_get_(Base::Context const& context)
{
    J::Array result;

    for (const auto& submission : submissions_)
        result.push_back(submission->to_json(true));

    use_json(result);
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

    if (std::regex_match(path_info, sm, Path::submissions_hw1)) {
        int assignment_number = get_number(sm[1]);
        return std::make_unique<Submissions_hw1>(assignment_number);
    }

    if (std::regex_match(path_info, Path::grades_csv)) {
        return std::make_unique<Grades_csv>();
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
