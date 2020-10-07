#pragma once

#include <Wt/Dbo/ptr.h>

#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

class User;

class Submission;

namespace Wt {
class WContainerWidget;

class WString;

class WText;

class WWidget;
} // namespace Wt

class PageTitle {
public:
  static PageTitle user_home(Wt::Dbo::ptr<User> const &);
  static PageTitle user_profile(Wt::Dbo::ptr<User> const &);
  static PageTitle user_hwN(Wt::Dbo::ptr<Submission> const &,
                            Wt::Dbo::ptr<User> const &current_user);
  static PageTitle user_hwN_eval(Wt::Dbo::ptr<Submission> const &,
                                 Wt::Dbo::ptr<User> const &current_user);

  std::string get_text() const;
  std::unique_ptr<Wt::WWidget> build_widget() const;

protected:
  void plain(std::string_view text);
  void link(std::string_view text, std::string_view link);

private:
  struct Chunk_;

  std::vector<Chunk_> chunks_;
};

struct PageTitle::Chunk_ {
  std::string text;
  std::string link; // empty means plain text

  explicit Chunk_(std::string_view text, std::string_view link = "");
};

// std::ostringstream                    oss_;
// std::unique_ptr<Wt::WContainerWidget> widget_;
//
// Wt::WText* text_(std::string const& text);
// Wt::WText* link_(std::string const& link,
//                 std::string const& text);

// namespace
//{
//
// std::ostream& insert_into(std::ostream& out)
//{
//    return out;
//}
//
// template<class T, class... Args>
// void insert_into(std::ostream& out, T&& first, Args&& ... rest)
//{
//    insert_into(out << std::forward<T>(first),
//                std::forward<Args>(rest)...);
//}
//
// template<class... Args>
// std::string concat_anything(Args&& ... args)
//{
//    std::ostringstream out;
//    insert_into(out, std::forward<Args>(args)...);
//    return out.str();
//}
//
//} // end inline namespace

// template<class T1, class... Ts>
// Wt::WText* PageTitle::text(T1&& first, Ts&& ... rest)
//{
//    return text(concat_anything(std::forward<T1>(first),
//                                std::forward<Ts>(rest) ...));
//}

// template<class T1, class... Ts>
// Wt::WText* PageTitle::link(std::string const& link, T1&& first, Ts&& ...
// rest)
//{
//    return link(link,
//                concat_anything(std::forward<T1>(first),
//                                std::forward<Ts>(rest)...));
//}
//
// template<class T1, class... Ts>
// Wt::WText* text(T1&&, Ts&& ...);

// template<class T1, class... Ts>
// Wt::WText* link(std::string const& link,
//                T1&&, Ts&& ...);
