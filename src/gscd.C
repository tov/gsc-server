#include "Application_controller.h"
#include "Session.h"
#include "view/api/Endpoint.h"

#include <Wt/Dbo/SqlConnectionPool.h>
#include <Wt/WLocale.h>
#include <Wt/WServer.h>
#include <Wt/WString.h>

#include <cstdlib>

int main(int argc, char** argv)
{
    try {
        Wt::WString::setDefaultEncoding(Wt::CharEncoding::UTF8);

        auto env_string = std::getenv("POSTGRES_CONNINFO");
        auto db_string  = env_string? env_string : "dbname=gsc";
        auto pool       = Db_session::createConnectionPool(db_string);
        Db_session::initialize_db(*pool);
        Db_session::configure_auth();

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
