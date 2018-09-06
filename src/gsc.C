#include "Application_controller.h"
#include "model/Session.h"
#include "view/REST_endpoint.h"

#include <Wt/Dbo/SqlConnectionPool.h>
#include <Wt/WServer.h>
#include <Wt/WString.h>

#include <cstdlib>


int main(int argc, char** argv)
{
    try {
        Wt::WServer server(argc, argv, WTHTTP_CONFIGURATION);

        Session::configureAuth();

        Wt::WString::setDefaultEncoding(Wt::CharEncoding::UTF8);

        auto env_string = std::getenv("POSTGRES_CONNINFO");
        auto db_string  = env_string? env_string : "dbname=gsc";
        auto pool       = Session::createConnectionPool(db_string);

        REST_endpoint endpoint(*pool);

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
