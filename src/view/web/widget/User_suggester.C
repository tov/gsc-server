#include "../../../Session.h"
#include "../../../common/util.h"
#include "User_suggester.h"

#include <Wt/WStringListModel.h>

#include <memory>

static WSuggestionPopup::Options const popup_options = {
    "<b>",  // highlightBeginTag
    "</b>", // highlightEndTag
    '\0',   // listSeparator      (for multiple addresses)
    " ",    // whitespace
    "",     // wordSeparators     (within an address)
    ""      // appendReplacedText (prepare next email address)
};

static Users get_users(Session &session, optional<User::Role> role) {
  auto query = session.find<User>().orderBy("name");
  return role ? query.where("role = ?").bind((int)*role) : query;
}

static unique_ptr<WStringListModel> get_user_model(Session &session,
                                                   optional<User::Role> role) {
  dbo::Transaction transaction(session);

  auto model = make_unique<WStringListModel>();

  for (auto const &user : get_users(session, role)) {
    model->addString(user->name());
  }

  return model;
}

User_suggester::User_suggester(Session &session, optional<User::Role> role)
    : WSuggestionPopup(popup_options) {
  setModel(get_user_model(session, role));
}
