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
    static std::unique_ptr<Base> create(std::string const& method,
                                        std::string const& path_info);

    virtual void load(dbo::Session&,
                      dbo::ptr<User> const& current_user) = 0;

    void process();

    void send(Wt::Http::Response&) const;

    virtual ~Base() = default;

protected:
    // Override these to handle specific methods.
    virtual void do_delete_();
    virtual void do_get_();
    virtual void do_patch_();
    virtual void do_post_();
    virtual void do_put_();

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
