#include "Application_controller.h"
#include "model/Session.h"

#include <Wt/Dbo/SqlConnectionPool>
#include <Wt/WServer>
#include <Wt/WString>

#include <cstdlib>

int main(int argc, char** argv)
{
    try {
        Wt::WServer server(argc, argv, WTHTTP_CONFIGURATION);

        Session::configureAuth();

        Wt::WString::setDefaultEncoding(Wt::UTF8);

        auto db_string = std::getenv("POSTGRES_CONNINFO");
        std::unique_ptr<Wt::Dbo::SqlConnectionPool> pool(
                Session::createConnectionPool(db_string ? db_string :
                                              "dbname=gsc"));

        server.addEntryPoint(Wt::Application,
                             boost::bind(&Application_controller::create,
                                         pool.get(),
                                         _1),
                             "/gsc");
        // Why doesn't std::bind work here?

        server.run();
    } catch (Wt::WServer::Exception& e) {
        std::cerr << e.what() << std::endl;
    } catch (std::exception& e) {
        std::cerr << "exception: " << e.what() << std::endl;
    }
}
