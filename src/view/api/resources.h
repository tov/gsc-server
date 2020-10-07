#pragma once

#include "../../Session.h"
#include "../../common/paths.h"
#include "../../model/File_data.h"
#include "../Resource_base.h"

#include <Wt/Dbo/Session.h>
#include <Wt/Json/Serializer.h>

#include <istream>
#include <memory>
#include <string>
#include <vector>

class Eval_item;
class Self_eval;

namespace api {

class Request_body;

namespace resources {

class Resource {
public:
  struct Context {
    Db_session &session;
    dbo::ptr<User> user;
  };

  // Parses the URI into the resource.
  static std::unique_ptr<Resource> create(std::string const &method,
                                          std::string const &path_info);

  // Processes the request, building the response.
  void process(Wt::Http::Request const &, Resource_response &, Context const &);

  virtual ~Resource() = default;

protected:
  // Loads the resource on behalf of current_user.
  virtual void load_(Context const &) = 0;

  // Override these to handle specific methods.
  virtual void do_delete_(Context const &);
  virtual void do_get_(Context const &);
  virtual void do_patch_(Request_body body, Context const &);
  virtual void do_post_(Request_body body, Context const &);
  virtual void do_put_(Request_body body, Context const &);

  std::string const &method() const { return method_; }

  // Successful response with no data.
  void success() {
    content_type = "application/json";
    contents = Bytes("true");
  }

  // Respond with the given JSON.
  template <typename T> void use_json(T const &json) {
    content_type = "application/json";
    contents = Bytes(Wt::Json::serialize(json));
  }

  static Wt::Dbo::ptr<User> load_user(Context const &,
                                      std::string const &username);

  static Wt::Dbo::ptr<Assignment> load_assignment(Context const &, int number);

  static Wt::Dbo::ptr<Submission> load_submission(Context const &,
                                                  int submission_id);

  static Wt::Dbo::ptr<Eval_item>
  load_eval_item(Context const &, Wt::Dbo::ptr<Submission> const &,
                 int sequence);

  static Wt::Dbo::ptr<Self_eval>
  load_self_eval(Context const &, Wt::Dbo::ptr<Submission> const &,
                 Wt::Dbo::ptr<Eval_item> const &);

  // Give up.
  static void not_found [[noreturn]] ();
  static void not_supported [[noreturn]] ();

  std::string content_type;
  std::vector<unsigned char> contents;

private:
  static std::unique_ptr<Resource> dispatch_(std::string path_info);

  std::string method_;
};

} // end namespace resources

} // end namespace api
