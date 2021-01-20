#include <Wt/WTable.h>

struct Fancy_table : public Wt::WTable
{
    Fancy_table()
    { setStyleClass("fancy"); }
};
