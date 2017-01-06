#pragma once

#include <Wt/Dbo/Dbo>

class Assignment;
class File_data;
class File_meta;
class User;
class User_stats;

namespace Wt {

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
struct dbo_traits<User_stats> : public dbo_default_traits
{
    using IdType = ptr<User>;
    static IdType invalidId() { return IdType(); }
    static const char* surrogateIdField() { return nullptr; }
};

// File_data uses its file_meta foreign key as id.
template<>
struct dbo_traits<File_data> : public dbo_default_traits
{
    using IdType = ptr<File_meta>;
    static IdType invalidId() { return IdType(); }
    static const char* surrogateIdField() { return nullptr; }
};

}

}
