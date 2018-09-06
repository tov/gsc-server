#pragma once

#include "Path.h"
#include "../../model/Session.h"

#include <Wt/Dbo/Session.h>
#include <Wt/Json/Serializer.h>

#include <memory>
#include <string>

namespace api {

namespace Resource {

class Base
{
public:
    // Parses the URI into the resource.
    static std::unique_ptr<Base> create(std::string const& method,
                                        std::string const& path_info);

    // Loads the resource on behalf of current_user.
    virtual void load(dbo::ptr<User> const& current_user) = 0;

    // Processes the request, building the response.
    void process(Wt::Http::Request const&,
                 dbo::ptr<User> const& current_user);

    // Sends the response.
    void send(Wt::Http::Response&) const;

    virtual ~Base() = default;

protected:
    // Override these to handle specific methods.
    virtual void do_delete_(dbo::ptr<User> const& current_user);
    virtual void do_get_(dbo::ptr<User> const& current_user);
    virtual void do_patch_(Wt::Http::Request const& request,
                           dbo::ptr<User> const& current_user);
    virtual void do_post_(Wt::Http::Request const& request,
                          dbo::ptr<User> const& current_user);
    virtual void do_put_(Wt::Http::Request const& request,
                         dbo::ptr<User> const& current_user);

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

    // Give up.
    static void denied [[noreturn]] ();
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
