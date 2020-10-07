#include "Time_stamp.h"

#include <Wt/Dbo/WtSqlTraits.h>

using my_trait = Wt::Dbo::sql_value_traits<Time_stamp, void>;

void Time_stamp::touch() { value_ = Wt::WDateTime::currentDateTime(); }

Time_stamp::Time_stamp() { touch(); }

Time_stamp::Time_stamp(std::nullptr_t) {}

char const *my_trait::type(SqlConnection *conn, int size) {
  return repr_trait::type(conn, size);
}

void my_trait::bind(const Time_stamp &v, SqlStatement *statement, int column,
                    int size) {
  repr_trait::bind(v.value_, statement, column, size);
}

bool my_trait::read(Time_stamp &v, SqlStatement *statement, int column,
                    int size) {
  return repr_trait::read(v.value_, statement, column, size);
}
