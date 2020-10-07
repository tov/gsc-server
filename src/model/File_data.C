#include "File_data.h"
#include "File_meta.h"
#include "specializations.h"

#include <Wt/Dbo/Impl.h>

DBO_INSTANTIATE_TEMPLATES(File_data)

Bytes::Bytes(std::string const &data) {
  reserve(data.size());
  assign(data.begin(), data.end());
}

Bytes::Bytes(std::istream &is, int size) {
  reserve(size);
  std::copy(std::istreambuf_iterator<char>(is), {}, std::back_inserter(*this));
}

void Bytes::write(std::ostream &os) const {
  os.write((char const *)data(), size());
}

Bytes::operator std::string() const {
  std::string result;
  result.reserve(size());
  result.assign(begin(), end());
  return result;
}

namespace Wt {
namespace Dbo {

const char *sql_value_traits<Bytes, void>::type(SqlConnection *conn, int size) {
  return repr_trait::type(conn, size);
}

void sql_value_traits<Bytes, void>::bind(const Bytes &v,
                                         SqlStatement *statement, int column,
                                         int size) {
  repr_trait::bind(v, statement, column, size);
}

bool sql_value_traits<Bytes, void>::read(Bytes &v, SqlStatement *statement,
                                         int column, int size) {
  return repr_trait::read(v, statement, column, size);
}

} // end namespace Dbo
} // end namespace Wt
