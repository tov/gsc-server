#include "Bytes.h"

Bytes::Bytes(std::string const& data)
{
    data_.reserve(data.size());
    data_.assign(data.begin(), data.end());
}

Bytes::Bytes(std::istream& is, int size)
{
    data_.reserve(size);
    std::copy(std::istreambuf_iterator<char>(is), {},
              std::back_inserter(data_));
}

void Bytes::write(std::ostream& os) const
{
    os.write(data(), size());
}

Bytes::operator std::string() const
{
    std::string result;
    result.reserve(size());
    result.assign(begin(), end());
    return result;
}

namespace Wt::Dbo {

char const*
sql_value_traits<Bytes, void>::type(SqlConnection *conn, int size)
{
    return repr_trait::type(conn, size);
}

void
sql_value_traits<Bytes, void>::bind(
        const Bytes& v,
        SqlStatement* statement, int column, int size)
{
    repr_trait::bind(v.data_, statement, column, size);
}

bool
sql_value_traits<Bytes, void>::read(
        Bytes& v,
        SqlStatement* statement, int column, int size)
{
    return repr_trait::read(v.data_, statement, column, size);
}

} // end namespace Wt::Dbo

