#pragma once

#include "Path.h"
#include "../../Session.h"

#include <Wt/Dbo/Session.h>
#include <Wt/Json/Serializer.h>

#include <istream>
#include <memory>
#include <string>

namespace api {

class Request_body;

namespace Resource {

class Base
{
public:
    struct Context
    {
        Db_session& session;
        dbo::ptr<User> user;
    };

    // Parses the URI into the resource.
    static std::unique_ptr<Base> create(std::string const& method,
                                        std::string const& path_info);

    // Loads the resource on behalf of current_user.
    virtual void load(Context const&) = 0;

    // Processes the request, building the response.
    void process(Wt::Http::Request const& request, Context const&);

    // Sends the response.
    void send(Wt::Http::Response&) const;

    virtual ~Base() = default;

protected:
    // Override these to handle specific methods.
    virtual void do_delete_(Context const&);
    virtual void do_get_(Context const&);
    virtual void do_patch_(Request_body body, Context const&);
    virtual void do_post_(Request_body body, Context const&);
    virtual void do_put_(Request_body body, Context const&);

    // Successful response with no data.
    void success()
    {
        content_type = "application/json";
        contents = "true";
    }

    // Respond with the given JSON.
    template <typename T>
    void use_json(T const& json)
    {
        content_type = "application/json";
        contents = Wt::Json::serialize(json);
    }

    static Wt::Dbo::ptr<User>
    load_user(Context const&, std::string const& username);

    static Wt::Dbo::ptr<Assignment>
    load_assignment(Context const&, int number);

    static Wt::Dbo::ptr<Submission>
    load_submission(Context const&, int submission_id);

    // Give up.
    static void denied [[noreturn]] (int code);
    static void not_found [[noreturn]] ();
    static void not_supported [[noreturn]] ();

    std::string content_type;
    std::string contents;

private:
    static std::unique_ptr<Base> parse_(std::string const& path_info);

    std::string method_;
};

} // end namespace Resource

} // end namespace api
