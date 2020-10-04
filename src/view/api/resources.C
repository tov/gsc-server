#include "../../Session.h"
#include "../../common/paths.h"
#include "../../common/util.h"
#include "../Http_status.h"
#include "Request_body.h"
#include "Request_handler.h"
#include "Result_array.h"
#include "resources.h"

#include "../../model/Assignment.h"
#include "../../model/Eval_item.h"
#include "../../model/Exam_grade.h"
#include "../../model/File_data.h"
#include "../../model/File_meta.h"
#include "../../model/Partner_request.h"
#include "../../model/Self_eval.h"
#include "../../model/Submission.h"
#include "../../model/auth/User.h"

#include <Wt/Json/Object.h>
#include <Wt/Json/Serializer.h>
#include <Wt/Json/Value.h>

#include <Wt/Utils.h>
#include <Wt/WLocalDateTime.h>

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <map>
#include <regex>
#include <sstream>
#include <string>

namespace api {

namespace resources {

// Helper struct for consuming JSON in the order of our choice.
struct Key_eater {
    J::Object& json;

    template<class F>
    void take(std::string const& key, F f)
    {

        if (auto iter = json.find(key); iter != json.end()) {
            f(iter->second);
            json.erase(iter);
        }
    }
};


template<class T>
std::unique_ptr<T> match(std::string const& path_info)
{
    using uri_type = typename T::uri_type;

    std::smatch sm;

    if (std::regex_match(path_info, sm, uri_type::re)) {
        return std::make_unique<T>(uri_type(sm));
    } else {
        return nullptr;
    }
}

void Resource::not_found()
{
    Http_error{404} << "The named resource does not exist";
}

void Resource::not_supported()
{
    Http_error{405} << "The resource does not support that method";
}

class Grades_csv : public Resource
{
public:
    using uri_type = paths::Grades_csv;

    explicit Grades_csv(uri_type&&) {}

    void load_(Context const&) override;

protected:
    void do_get_(Context const&) override;

private:
    using Exams             = std::vector<int>;
    using Exam_scores       = std::map<int, double>;
    using Assignments       = std::vector<dbo::ptr<Assignment>>;
    using Assignment_scores = std::map<int, double>;
    using Score_map =
            std::map<std::string, std::pair<Exam_scores, Assignment_scores>>;

    Exams exams_;
    Assignments assigns_;
    Score_map scores_;
};

void Grades_csv::load_(const Resource::Context& context)
{
    context.user->check_can_admin();

    auto& dbo = context.session.dbo();

    const auto assigns_q =
            dbo.find<Assignment>().orderBy("number").resultList();
    const auto exams_q =
            dbo.query<int>("SELECT DISTINCT number FROM exam_grade")
                    .orderBy("number")
                    .resultList();

    assigns_.assign(assigns_q.begin(), assigns_q.end());
    exams_.assign(exams_q.begin(), exams_q.end());

    auto users = dbo.find<User>()
                         .where("role = ?")
                         .bind(int(User::Role::Student))
                         .resultList();

    for (auto const& user : users) {
        Exam_scores user_exams;
        Assignment_scores user_assigns;

        for (const auto& assign : assigns_) {
            auto submission =
                    Submission::find_by_assignment_and_user(dbo, assign, user);
            user_assigns[assign->number()] =
                    submission ? submission->grade() : 0;
        }

        for (int exam : exams_) {
            auto exam_grade  = Exam_grade::find_by_user_and_number(user, exam);
            user_exams[exam] = exam_grade ? exam_grade->grade() : 0;
        }

        scores_[user->name()] =
                std::make_pair(std::move(user_assigns), std::move(user_exams));
    }
}

static double lookup_or_0(std::map<int, double> const& map, int key)
{
    auto iter = map.find(key);
    return iter == map.end() ? 0.0 : iter->second;
}

void Grades_csv::do_get_(const Context& context)
{
    std::ostringstream o;

    o << "username";
    for (auto const& assign : assigns_) o << ",hw" << assign->number();
    for (int exam : exams_) o << ",ex" << exam;
    o << "\n";

    for (auto const& row : scores_) {
        o << row.first;

        auto const& user_assigns = row.second.first;
        for (auto const& assign : assigns_)
            o << "," << lookup_or_0(user_assigns, assign->number());

        auto const& user_exams = row.second.second;
        for (int exam : exams_) { o << "," << lookup_or_0(user_exams, exam); }

        o << "\n";
    }

    content_type = "text/csv";
    contents     = Bytes(o.str());
}

class Users : public Resource
{
public:
    using uri_type = paths::Users;

    explicit Users(uri_type&&) {}

    void load_(Context const&) override;

protected:
    void do_get_(Context const&) override;

private:
    dbo::collection<dbo::ptr<User>> users_;
};

void Users::load_(Context const& context)
{
    context.user->check_can_admin();

    users_ = context.session.find<User>();
}

void Users::do_get_(Context const&)
{
    J::Array result;

    for (auto const& user : users_) result.push_back(user->to_json(true));

    use_json(result);
}

class Users_1 : public Resource
{
public:
    using uri_type = paths::Users_1;

    explicit Users_1(uri_type&& uri)
        : uri_{std::move(uri)}
    {}

    void load_(Context const&) override;

protected:
    void do_get_(Context const&) override;
    void do_delete_(Context const&) override;
    void do_patch_(Request_body body, Context const&) override;

private:
    uri_type uri_;
    dbo::ptr<User> user_;
};

dbo::ptr<User> Resource::load_user(Context const& context,
                                   std::string const& username)
{
    context.user->check_can_view(username);

    auto user = User::find_by_name(context.session, username);
    if (user) return user;

    not_found();
}

Wt::Dbo::ptr<Assignment>
Resource::load_assignment(const Resource::Context& context, int number)
{
    auto result = Assignment::find_by_number(context.session, number);
    if (result) return result;
    not_found();
}

Wt::Dbo::ptr<Submission> Resource::load_submission(Context const& context,
                                                   int submission_id)
{
    auto submission = Submission::find_by_id(context.session, submission_id);
    if (! submission) not_found();
    submission->check_can_view(context.user);
    return submission;
}

Wt::Dbo::ptr<Eval_item>
Resource::load_eval_item(Context const& context,
                         dbo::ptr<Submission> const& as_part_of, int sequence)
{
    auto eval_item =
            as_part_of->assignment()->find_eval_item(context.session, sequence);
    if (! eval_item) not_found();
    return eval_item;
}


Wt::Dbo::ptr<Self_eval>
Resource::load_self_eval(Context const& context,
                         Wt::Dbo::ptr<Submission> const& submission,
                         Wt::Dbo::ptr<Eval_item> const& eval_item)
{
    return Submission::get_self_eval(eval_item, submission,
                                     eval_item->type() ==
                                             Eval_item::Type::Informational);
}

void Users_1::load_(Context const& context)
{
    user_ = load_user(context, uri_.name);
}

void Users_1::do_get_(Context const&)
{
    use_json(user_->to_json());
}

void Users_1::do_delete_(Context const& context)
{
    context.user->check_can_admin();

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
                context.user->check_can_admin();

                User::Role role = destringify(pair.second);
                user_.modify()->set_role(role);
                result.success()
                        << "Role updated to " << stringify(role) << ".";
            }

#ifdef GSC_AUTH_PASSWORD
            else if (pair.first == "password") {
                if (! context.user->can_admin()) {
                    Credentials creds{user_->name(), pair.second};
                    Request_handler::check_password_strength(creds);
                }

                context.session.set_password(user_, pair.second);
                result.success()
                        << "Updated password for user " << user_->name() << ".";
            }
#endif  // GSC_AUTH_PASSWORD

            else if (pair.first == "exam_grades") {
                context.user->check_can_admin();

                Result_array nested;

                J::Array const& exams = pair.second;
                for (J::Object const& exam : exams) {
                    int number   = exam.get("number");
                    int points   = exam.get("points");
                    int possible = exam.get("possible");

                    auto exam_grade =
                            Exam_grade::find_by_user_and_number(user_, number);

                    if (possible == 0) {
                        exam_grade.remove();
                        nested.success()
                                << "Removed exam " << number << " grade for "
                                << user_->name() << ".";
                    } else {
                        exam_grade.modify()->set_points_and_possible(points,
                                                                     possible);
                        nested.success()
                                << "Set exam " << number << " grade for "
                                << user_->name() << " to " << points << " / "
                                << possible << ".";
                    }
                }

                result.add_nested(std::move(nested));
            }

            else if (pair.first == "partner_requests") {
                Result_array nested;

                J::Array const& requests = pair.second;
                for (J::Object const& request : requests) {
                    using S = Partner_request::Status;

                    int hw_number        = request.get("assignment_number");
                    std::string username = request.get("user");
                    std::string status_string = request.get("status");

                    auto hw    = Assignment::find_by_number(context.session,
                                                         hw_number);
                    auto other = User::find_by_name(context.session, username);
                    S status   = destringify(status_string);

                    if (! hw)
                        Http_error{403} << "hw" << hw_number
                                        << " does not exist.";

                    if (! other)
                        Http_error{403} << "User " << username
                                        << " does not exist.";

                    std::string user_and_hw;
                    {
                        std::ostringstream o;
                        o << "user " << other->name() << " for hw" << hw_number;
                        user_and_hw = o.str();
                    }

                    switch (status) {
                    case S::outgoing: {
                        // Check for an incoming request, and if it exists, accept it.
                        auto incoming = Partner_request::
                                find_by_requestor_and_assignment(
                                        context.session, other, hw);

                        if (incoming && incoming->requestee() == user_) {
                            if (incoming->confirm(context.session)) {
                                nested.success() << "Requested and confirmed "
                                                    "partnership with "
                                                 << user_and_hw << ".";
                                break;
                            }
                        }

                        std::ostringstream reason;
                        auto request_ptr = Partner_request::create(
                                context.session, user_, other, hw, reason);
                        if (! request_ptr) throw Http_status{403, reason.str()};

                        nested.success() << "Requested partnership with "
                                         << user_and_hw << ".";
                        break;
                    }

                    case S::incoming:
                        nested.failure() << "You cannot create an incoming "
                                            "partner request.";
                        break;

                    case S::accepted: {
                        auto request_ptr = Partner_request::
                                find_by_requestor_and_requestee(
                                        context.session, other, user_, hw);
                        if (request_ptr) {
                            auto submission = request_ptr.modify()->confirm(
                                    context.session);
                            if (submission) {
                                nested.success()
                                        << "Accepted partner request from "
                                        << user_and_hw << ".";
                            } else {
                                nested.failure() << "Could not accept partner "
                                                    "request from"
                                                 << user_and_hw << ".";
                            }
                        } else {
                            nested.failure() << "You don’t have an incoming "
                                                "partner request from "
                                             << user_and_hw << ".";
                        }
                        break;
                    }

                    case S::canceled: {
                        auto outgoing = Partner_request::
                                find_by_requestor_and_requestee(
                                        context.session, user_, other, hw);
                        auto incoming = Partner_request::
                                find_by_requestor_and_requestee(
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
                result.failure()
                        << "Unknown key in JSON: ‘" << pair.first << "’.";
            }
        }
    } catch (J::TypeException const& e) {
        throw Http_status{400, "PATCH /users/_1 could not understand request"};
    } catch (J::ParseError const& e) {
        throw Http_status{
                400, "PATCH /users/_1 could not parse user request as JSON"};
    } catch (std::invalid_argument const& e) {
        throw Http_status{400, e.what()};
    }

    use_json(result);
}

class Users_1_submissions : public Resource
{
public:
    using uri_type = paths::Users_1_submissions;

    explicit Users_1_submissions(uri_type&& uri)
        : uri_{std::move(uri)}
    {}

    void load_(Context const&) override;

protected:
    void do_get_(Context const& context) override;

private:
    uri_type uri_;

    dbo::ptr<User> user_;
    std::vector<dbo::ptr<Submission>> submissions_;
};

void Users_1_submissions::load_(Context const& context)
{
    user_        = load_user(context, uri_.name);
    submissions_ = user_->submissions();
}

void Users_1_submissions::do_get_(const Resource::Context& context)
{
    J::Array result;
    for (const auto& each : submissions_) {
        result.push_back(each->to_json(true));
    }
    use_json(result);
}

class Submissions_1 : public Resource
{
public:
    using uri_type = paths::Submissions_1;

    explicit Submissions_1(uri_type&& uri)
        : uri_{std::move(uri)}
    {}

    void load_(Context const&) override;

protected:
    void do_delete_(Context const& context) override;
    void do_get_(Context const& context) override;
    void do_patch_(Request_body body, Context const& context) override;

private:
    uri_type uri_;

    dbo::ptr<Submission> submission_;
};

void Submissions_1::load_(const Context& context)
{
    submission_ = load_submission(context, uri_.submission_id);
}

void Submissions_1::do_delete_(const Context& context)
{
    submission_->check_can_submit(context.user);

    submission_.remove();
    success();
}

void Submissions_1::do_get_(const Context& context)
{
    use_json(submission_->to_json());
}

void Submissions_1::do_patch_(Request_body body,
                              const Resource::Context& context)
{
    context.user->check_can_admin();

    Result_array result;

    try {
        auto json               = std::move(body).read_json();
        J::Object const& object = json;

        for (auto const& pair : object) {
            if (pair.first == "due_date") {
                std::string time_spec = pair.second;

                if (auto res = submission_.modify()->set_due_date(time_spec);
                    res.has_value()) {
                    result.success() << "Modified due date to "
                                     << res->toString() << " UTC";
                } else {
                    result.failure()
                            << "Could not parse timespec ‘" << time_spec << "’";
                }
            }

            else if (pair.first == "eval_date") {
                std::string time_spec = pair.second;

                if (auto res = submission_.modify()->set_eval_date(time_spec);
                    res.has_value()) {
                    result.success() << "Modified eval date to "
                                     << res->toString() << " UTC";
                } else {
                    result.failure()
                            << "Could not parse timespec ‘" << time_spec << "’";
                }
            }

            else if (pair.first == "bytes_quota") {
                int bytes_quota = pair.second;
                submission_.modify()->set_bytes_quota(bytes_quota);
                result.success() << "Modified quota to " << bytes_quota << ".";
            }

            else if (pair.first == "owner2") {
                try {
                    auto guard = dbo::Transaction(context.session);
                    submission_.modify()->divorce();
                    result.success() << "Departnering succeeded.";
                } catch (runtime_error const& exn) {
                    result.failure() << exn.what();
                }
            }

            else {
                result.failure()
                        << "Unknown key in JSON: ‘" << pair.first << "’.";
            }
        }
    } catch (Wt::Json::ParseError const& e) {
        Http_error{400} << e.what();
    } catch (Wt::Json::TypeException const& e) {
        Http_error{400} << e.what();
    }

    use_json(result);
}

class Submissions_1_files : public Resource
{
public:
    using uri_type = paths::Submissions_1_files;

    explicit Submissions_1_files(uri_type&& uri)
        : uri_{std::move(uri)}
    {}

    void load_(Context const&) override;

protected:
    void do_get_(Context const& context) override;

private:
    uri_type uri_;
    std::vector<dbo::ptr<File_meta>> file_metas_;
};

void Submissions_1_files::load_(Context const& context)
{
    auto submission = load_submission(context, uri_.submission_id);
    auto file_metas = submission->source_files_sorted();
    file_metas_.assign(file_metas.begin(), file_metas.end());
}

void Submissions_1_files::do_get_(const Resource::Context& context)
{
    J::Array json;
    for (auto const& each : file_metas_) json.push_back(each->to_json());
    use_json(json);
}

class Submissions_1_files_2 : public Resource
{
public:
    using uri_type = paths::Submissions_1_files_2;

    explicit Submissions_1_files_2(uri_type&& uri)
        : uri_{std::move(uri)}
    {}

    void load_(Context const&) override;

protected:
    void do_delete_(Context const& context) override;
    void do_get_(Context const& context) override;
    void do_patch_(Request_body body, Context const& context) override;
    void do_put_(Request_body body, Context const& context) override;

private:
    uri_type uri_;

    dbo::ptr<Submission> submission_;
    dbo::ptr<File_meta> file_meta_;
};

void Submissions_1_files_2::load_(Context const& context)
{
    submission_ = load_submission(context, uri_.submission_id);
    file_meta_  = submission_->find_file_by_name(uri_.filename);
}

void Submissions_1_files_2::do_delete_(const Resource::Context& context)
{
    if (file_meta_) file_meta_.remove();
    success();
}

void Submissions_1_files_2::do_get_(const Resource::Context& context)
{
    if (! file_meta_) not_found();

    content_type = file_meta_->media_type();
    std::unique_ptr<File_data> file_data =
            std::make_unique<File_data>(file_meta_);
    if (file_data->populate_contents()) { contents = file_data->contents(); }
}

void Submissions_1_files_2::do_patch_(Request_body body, Context const& context)
{
    if (! file_meta_) not_found();

    try {
        J::Object json = move(body).read_json();

        dbo::ptr<Submission> owner = submission_;
        optional<string> opt_name;
        optional<string> opt_media_type;
        optional<File_purpose> opt_purpose;

        auto overwrite = false;

        Key_eater processor{json};

        processor.take("overwrite", [&](bool value) { overwrite = value; });

        processor.take("name",
                       [&](std::string const& value) { opt_name = value; });

        processor.take("assignment_number", [&](int value) {
            if (submission_->assignment_number() != value) {
                owner = Submission::find_by_assignment_number_and_user(
                        context.session, value, context.user);
                owner->check_can_submit(context.user);
            }
        });

        processor.take("media_type", [&](std::string const& value) {
            opt_media_type = value;
        });

        processor.take("purpose", [&](std::string const& value) {
            opt_purpose = destringify(value);
        });

        if (! json.empty()) {
            Result_array result;
            for (auto const& pair : json)
                result.failure()
                        << "Unknown key in JSON: ‘" << pair.first << "’.";
            return use_json(result);
        }

        auto file_meta_m = file_meta_.modify();

        if (opt_media_type) file_meta_m->set_media_type(*opt_media_type);

        if (opt_name || owner != submission_) {
            string name = opt_name ? *opt_name : file_meta_m->name();
            file_meta_m->move(owner, name, overwrite);
        }

        if (opt_purpose) file_meta_m->reclassify(*opt_purpose);
        else if (opt_name || opt_media_type)
            file_meta_m->reclassify();

    } catch (J::TypeException const& e) {
        throw Http_status{
                400,
                "PATCH /submissions/_1/files/_2 could not understand request"};
    } catch (J::ParseError const& e) {
        throw Http_status{400, "PATCH /submissions/_1/files/_2 could not parse "
                               "user request as JSON"};
    } catch (std::invalid_argument const& e) {
        throw Http_status{400, e.what()};
    }

    use_json(file_meta_->to_json(true));
}

void Submissions_1_files_2::do_put_(Request_body body,
                                    const Resource::Context& context)
{
    submission_->check_can_submit(context.user);

    if (! submission_->has_sufficient_space(body.size(), uri_.filename))
        throw Http_status{413, "Upload would exceed quota"};

    auto file_meta =
            File_meta::upload(uri_.filename, std::move(body).read_bytes(),
                              submission_, context.user);

    use_json(file_meta->to_json());
}

class Submissions_1_evals : public Resource
{
public:
    using uri_type = paths::Submissions_1_evals;

    explicit Submissions_1_evals(uri_type&& uri)
        : uri_{std::move(uri)}
    {}

    void load_(Context const&) override;

protected:
    void do_get_(Context const& context) override;

private:
    uri_type uri_;

    dbo::ptr<Submission> submission_;
    std::vector<dbo::ptr<Eval_item>> eval_items_;
};

void Submissions_1_evals::load_(Context const& context)
{
    submission_ = load_submission(context, uri_.submission_id);
    eval_items_ = submission_->assignment()->eval_item_vec();
}

void Submissions_1_evals::do_get_(const Resource::Context& context)
{
    J::Array json;

    for (auto const& each : eval_items_)
        if (each)
            json.push_back(each->to_json(submission_, context.user, true));

    use_json(json);
}

class Submissions_1_evals_2 : public Resource
{
public:
    using uri_type = paths::Submissions_1_evals_2;

    explicit Submissions_1_evals_2(uri_type&& uri)
        : uri_{std::move(uri)}
    {}

    void load_(Context const&) override;

protected:
    void do_get_(Context const& context) override;

private:
    uri_type uri_;

    dbo::ptr<Submission> submission_;
    dbo::ptr<Eval_item> eval_item_;
};

void Submissions_1_evals_2::load_(Context const& context)
{
    submission_ = load_submission(context, uri_.submission_id);
    eval_item_  = load_eval_item(context, submission_, uri_.sequence);
}

void Submissions_1_evals_2::do_get_(const Context& context)
{
    use_json(eval_item_->to_json(submission_, context.user));
}

class Submissions_1_evals_2_self : public Resource
{
public:
    using uri_type = paths::Submissions_1_evals_2_self;

    explicit Submissions_1_evals_2_self(uri_type&& uri)
        : uri_{std::move(uri)}
    {}

    void load_(Context const&) override;

protected:
    void do_delete_(Context const& context) override;
    void do_get_(Context const& context) override;
    void do_put_(Request_body body, Context const& context) override;

private:
    uri_type uri_;

    dbo::ptr<Submission> submission_;
    dbo::ptr<Eval_item> eval_item_;
    dbo::ptr<Self_eval> self_eval_;
};

void Submissions_1_evals_2_self::load_(Context const& context)
{
    submission_ = load_submission(context, uri_.submission_id);
    eval_item_  = load_eval_item(context, submission_, uri_.sequence);
    self_eval_  = load_self_eval(context, submission_, eval_item_);
}

void Submissions_1_evals_2_self::do_get_(Resource::Context const& context)
{
    if (! self_eval_) not_found();

    use_json(self_eval_->to_json({context.user}));
}

void Submissions_1_evals_2_self::do_delete_(Resource::Context const& context)
{
    submission_->check_can_eval(context.user);

    self_eval_.remove();
    success();
}

void Submissions_1_evals_2_self::do_put_(Request_body body,
                                         Resource::Context const& context)
{
    submission_->check_can_eval(context.user);

    try {
        auto json               = std::move(body).read_json();
        J::Object const& object = json;

        std::string explanation = object.get("explanation");
        double score            = object.get("score");

        if (! self_eval_)
            self_eval_ =
                    Submission::get_self_eval(eval_item_, submission_, true);

        auto modifiable = self_eval_.modify();
        modifiable->set_explanation(explanation);
        modifiable->set_score(score);

    } catch (J::TypeException const& e) {
        throw Http_status{400, "PUT /submissions/_1/evals/_2/self could not "
                               "understand request"};
    } catch (J::ParseError const& e) {
        throw Http_status{400, "PUT /submissions/_1/evals/_2/self could not "
                               "parse user request as JSON"};
    }

    use_json(self_eval_->to_json({context.user}));
}

class Submissions_1_evals_2_grader : public Resource
{
public:
    using uri_type = paths::Submissions_1_evals_2_grader;

    explicit Submissions_1_evals_2_grader(uri_type&& uri)
        : uri_{std::move(uri)}
    {}

    void load_(Context const&) override;

protected:
    void do_delete_(Context const& context) override;
    void do_get_(Context const& context) override;
    void do_put_(Request_body body, Context const& context) override;


private:
    uri_type uri_;

    dbo::ptr<Submission> submission_;
    dbo::ptr<Eval_item> eval_item_;
    dbo::ptr<Self_eval> self_eval_;
    dbo::ptr<Grader_eval> grader_eval_;
};

void Submissions_1_evals_2_grader::load_(Context const& context)
{
    submission_ = load_submission(context, uri_.submission_id);
    eval_item_  = load_eval_item(context, submission_, uri_.sequence);
    self_eval_  = load_self_eval(context, submission_, eval_item_);

    if (! self_eval_)
        Http_error{403}
                << "Cannot create grader eval if self eval doesn't exist.";

    grader_eval_ = self_eval_->grader_eval();
}

void Submissions_1_evals_2_grader::do_get_(Resource::Context const& context)
{
    if (! grader_eval_) not_found();
    grader_eval_->check_can_view(context.user);

    use_json(grader_eval_->to_json({context.user}));
}

void Submissions_1_evals_2_grader::do_delete_(Resource::Context const& context)
{
    context.user->check_can_grade();

    grader_eval_.remove();
    success();
}

void Submissions_1_evals_2_grader::do_put_(Request_body body,
                                           Resource::Context const& context)
{
    context.user->check_can_grade();

    try {
        auto json               = std::move(body).read_json();
        J::Object const& object = json;

        std::string explanation    = object.get("explanation");
        double score               = object.get("score");
        std::string status_str     = object.get("status");
        Grader_eval::Status status = destringify(status_str);

        if (! grader_eval_)
            grader_eval_ =
                    Submission::get_grader_eval(self_eval_, context.user);

        auto modifiable = grader_eval_.modify();
        modifiable->set_explanation(explanation);
        modifiable->set_grader(context.user);
        modifiable->set_score(score);
        modifiable->set_status(status);

    } catch (J::TypeException const& e) {
        throw Http_status{400, "PUT /submissions/_1/evals/_2/self could not "
                               "understand request"};
    } catch (J::ParseError const& e) {
        throw Http_status{400, "PUT /submissions/_1/evals/_2/self could not "
                               "parse user request as JSON"};
    } catch (std::invalid_argument const& e) {
        throw Http_status{400, e.what()};
    }

    use_json(grader_eval_->to_json({context.user}));
}

class Submissions_hw1 : public Resource
{
public:
    using uri_type = paths::Submissions_hw1;

    explicit Submissions_hw1(uri_type&& uri)
        : uri_{std::move(uri)}
    {}

    void load_(Context const&) override;

protected:
    void do_get_(Context const& context) override;

private:
    uri_type uri_;

    std::vector<dbo::ptr<Submission>> submissions_;
};

void Submissions_hw1::load_(Context const& context)
{
    context.user->check_can_admin();

    auto assignment = load_assignment(context, uri_.assignment_number);
    std::copy(assignment->submissions().begin(),
              assignment->submissions().end(),
              std::back_inserter(submissions_));
}

void Submissions_hw1::do_get_(Resource::Context const& context)
{
    J::Array result;

    for (const auto& submission : submissions_)
        result.push_back(submission->to_json(true));

    use_json(result);
}

std::unique_ptr<Resource> Resource::create(std::string const& method,
                                           std::string const& path_info)
{
    auto resource     = dispatch_(path_info);
    resource->method_ = method;
    return resource;
}

class Whoami : public Resource
{
public:
    using uri_type = paths::Whoami;

    explicit Whoami(uri_type&&) {}

    void load_(Context const&) override;

protected:
    void do_get_(Context const&) override;

private:
    dbo::ptr<User> user_;
};

void Whoami::load_(const Resource::Context& context)
{
    user_ = context.user;
}

void Whoami::do_get_(const Resource::Context&)
{
    content_type = "text/plain";
    if (user_) contents = Bytes(user_->name());
}

#define dispatch_to(T)                                                         \
    do {                                                                       \
        auto resource = match<T>(path_info);                                   \
        if (resource) return resource;                                         \
    } while (false)

std::unique_ptr<Resource> Resource::dispatch_(std::string path_info)
{
    std::smatch sm;

    dispatch_to(Grades_csv);
    dispatch_to(Users);
    dispatch_to(Users_1);
    dispatch_to(Users_1_submissions);
    dispatch_to(Submissions_1);
    dispatch_to(Submissions_1_files);
    dispatch_to(Submissions_1_files_2);
    dispatch_to(Submissions_1_evals);
    dispatch_to(Submissions_1_evals_2);
    dispatch_to(Submissions_1_evals_2_self);
    dispatch_to(Submissions_1_evals_2_grader);
    dispatch_to(Submissions_hw1);
    dispatch_to(Whoami);

    not_found();
}

void Resource::process(Wt::Http::Request const& request,
                       Resource_response& response, Context const& context)
try {
    load_(context);

    Request_body body{request};

    if (body.size() > File_meta::max_byte_count)
        throw Http_status{413, "Request exceeds maximum file upload size"};

    if (method_ == "DELETE") do_delete_(context);
    else if (method_ == "GET")
        do_get_(context);
    else if (method_ == "PATCH")
        do_patch_(body, context);
    else if (method_ == "POST")
        do_post_(body, context);
    else if (method_ == "PUT")
        do_put_(body, context);
    else
        not_supported();

    response.content_type = move(content_type);
    response.contents     = move(contents);

} catch (Resource_not_found const&) {
    not_found();
} catch (Access_check_failed const& e) {
    throw Http_status{403, e.what()};
}

void Resource::do_delete_(Context const&)
{
    not_supported();
}

void Resource::do_get_(Context const&)
{
    not_supported();
}

void Resource::do_patch_(Request_body, Context const&)
{
    not_supported();
}

void Resource::do_post_(Request_body, Context const&)
{
    not_supported();
}

void Resource::do_put_(Request_body, Context const&)
{
    not_supported();
}

}  // end namespace resources

}  // end namespace api
