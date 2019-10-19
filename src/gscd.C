#include "Application_controller.h"
#include "Session.h"
#include "common/env_var.h"
#include "view/api/Endpoint.h"

#include <Wt/Dbo/SqlConnectionPool.h>
#include <Wt/WLocale.h>
#include <Wt/WServer.h>
#include <Wt/WString.h>

int main(int argc, char** argv)
{
    try {
        Wt::WString::setDefaultEncoding(Wt::CharEncoding::UTF8);

        auto db_string  = get_env_var("POSTGRES_CONNINFO", "dbname=gsc");
        auto pool       = Db_session::createConnectionPool(db_string);
        Db_session::configure_auth();
        // Maybe we don't need to do this every time:
//        Db_session(*pool).initialize_db();

        api::Endpoint endpoint(*pool);
        Wt::WServer server(argc, argv);

        server.addResource(&endpoint, "/api");
        server.addEntryPoint(Wt::EntryPointType::Application,
                             [&](const Wt::WEnvironment& env) {
                                 return Application_controller::create(*pool, env);
                             },
                             "/gsc");

        server.run();
    } catch (Wt::WServer::Exception& e) {
        std::cerr << e.what() << std::endl;
    } catch (std::exception& e) {
        std::cerr << "exception: " << e.what() << std::endl;
    }
}
