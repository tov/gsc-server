#include "Application_controller.h"
#include "Session.h"
#include "common/env_var.h"
#include "view/api/Resource.h"
#include "view/files/Resource.h"

#include <Wt/Dbo/SqlConnectionPool.h>
#include <Wt/WServer.h>
#include <Wt/WString.h>

int main(int argc, char **argv) {
  try {
    Wt::WString::setDefaultEncoding(Wt::CharEncoding::UTF8);

    auto db_string = get_env_var("POSTGRES_CONNINFO", "dbname=gsc");
    auto pool = Db_session::createConnectionPool(db_string);
    Db_session::configure_auth();

    Wt::WServer server(argc, argv);
    api::Resource rest_api(*pool);
    files::Resource files(*pool);

    server.addEntryPoint(
        Wt::EntryPointType::Application,
        [&](const Wt::WEnvironment &env) {
          return Application_controller::create(*pool, env);
        },
        "/gsc");
    server.addResource(&rest_api, "/api");
    server.addResource(&files, "/gsc/files");

    server.run();
  } catch (Wt::WServer::Exception &e) {
    std::cerr << e.what() << std::endl;
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << std::endl;
  }
}
