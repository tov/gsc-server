#pragma once

#include <Wt/Dbo/Dbo.h>

class Assignment;
class Bytes;
//class File_data;
class File_meta;
enum class File_purpose;
class Time_stamp;
class User;
class User_stats;

namespace Wt {

class WDateTime;

namespace Dbo {

// Assignment uses an int (number_) as its id
template<>
struct dbo_traits<Assignment> : public dbo_default_traits {
    using IdType = int;
    static IdType invalidId() { return -1; }
    static const char* surrogateIdField() { return nullptr; }
};

// User_stats uses its user_id foreign key as id.
template<>
struct dbo_traits<User_stats> : public dbo_default_traits {
    using IdType = ptr<User>;
    static IdType invalidId() { return IdType(); }
    static const char* surrogateIdField() { return nullptr; }
};

// File_data uses its file_meta foreign key as id.
/*template<>
struct dbo_traits<File_data> : public dbo_default_traits
{
    using IdType = ptr<File_meta>;
    static IdType invalidId() { return IdType(); }
    static const char* surrogateIdField() { return nullptr; }
};*/

template<>
struct sql_value_traits<Bytes, void> {
    using repr_trait = sql_value_traits<std::vector<unsigned char>, void>;

    static const bool specialized = true;

    static const char* type(SqlConnection* conn, int size);

    static void bind(const Bytes& v, SqlStatement* statement, int column,
                     int size);

    static bool read(Bytes& v, SqlStatement* statement, int column, int size);
};

template<>
struct sql_value_traits<File_purpose, void> {
    using repr_trait = sql_value_traits<int, void>;

    static const bool specialized = true;

    static const char* type(SqlConnection* conn, int size);

    static void bind(const File_purpose& v, SqlStatement* statement, int column,
                     int size);

    static bool read(File_purpose& v, SqlStatement* statement, int column,
                     int size);
};

template<>
struct sql_value_traits<Time_stamp, void> {
    using repr_trait = sql_value_traits<Wt::WDateTime, void>;

    static const bool specialized = true;

    static const char* type(SqlConnection* conn, int size);

    static void bind(const Time_stamp& v, SqlStatement* statement, int column,
                     int size);

    static bool read(Time_stamp& v, SqlStatement* statement, int column,
                     int size);
};

}  // namespace Dbo

}  // namespace Wt
