#include "PageTitle.h"

#include "../../Navigate.h"
#include "../../common/util.h"
#include "../../model/Assignment.h"
#include "../../model/Submission.h"
#include "../../model/auth/User.h"

#include <Wt/Dbo/ptr.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>

string PageTitle::get_text() const {
  ostringstream o;
  o << "gsc: ";
  for (auto const &chunk : chunks_) {
    o << chunk.text;
  }
  return o.str();
}

unique_ptr<WWidget> PageTitle::build_widget() const {
  auto w = make_unique<WContainerWidget>();
  w->setInline(true);

  for (auto chunkp = begin(chunks_), limit = end(chunks_); chunkp != limit;
       ++chunkp) {
    auto span = w->addNew<WText>(chunkp->text);

    if (!chunkp->link.empty() && chunkp + 1 != limit) {
      span->clicked().connect(Navigate(chunkp->link));
      span->setStyleClass("header-nav-link");
    } else {
      span->setStyleClass("header-nav-current");
    }
  }

  return w;
}

PageTitle PageTitle::user_home(const dbo::ptr<User> &user) {
  PageTitle result;
  result.link("~" + user->name(), user->hw_url());
  return result;
}

PageTitle PageTitle::user_profile(Wt::Dbo::ptr<User> const &user) {
  PageTitle result = user_home(user);
  result.plain(" › ");
  result.link("Profile", user->profile_url());
  return result;
}

PageTitle PageTitle::user_hwN(const dbo::ptr<Submission> &submission,
                              const dbo::ptr<User> &current_user) {
  PageTitle result =
      user_home(current_user == submission->user2() ? submission->user2()
                                                    : submission->user1());
  result.plain(" › ");
  result.link(submission->assignment()->slug_string(),
              submission->url_for_user(current_user));
  return result;
}

PageTitle PageTitle::user_hwN_eval(const dbo::ptr<Submission> &submission,
                                   const dbo::ptr<User> &current_user) {
  PageTitle result = user_hwN(submission, current_user);
  result.plain(" › ");
  result.link("Evaluation", submission->url_for_user(current_user, true));
  return result;
}

void PageTitle::plain(std::string_view text) { chunks_.emplace_back(text); }

void PageTitle::link(std::string_view text, std::string_view link) {
  chunks_.emplace_back(text, link);
}

PageTitle::Chunk_::Chunk_(std::string_view text, std::string_view link)
    : text(text), link(link) {}
