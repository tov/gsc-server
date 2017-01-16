#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/backend/Postgres>

#include <cstdlib>

namespace dbo = Wt::Dbo;

int main()
{
    auto db_string = std::getenv("POSTGRES_CONNINFO");
    dbo::backend::Postgres connection(db_string ? db_string : "dbname=gsc");
}