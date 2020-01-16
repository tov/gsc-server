#include "User_suggester.h"
#include "../../../Session.h"

static Wt::WSuggestionPopup::Options const popup_options = {
        "<b>",         // highlightBeginTag
        "</b>",        // highlightEndTag
        '\0',          // listSeparator      (for multiple addresses)
        " ",           // whitespace
        "",            // wordSeparators     (within an address)
        ""             // appendReplacedText (prepare next email address)
};

static Users get_users(Session& session,
                       std::optional<User::Role> role)
{
    auto query = session.find<User>().orderBy("name");
    return role
           ? query.where("role = ?").bind((int) *role)
           : query;
}

User_suggester::User_suggester(Session& session,
                               std::optional<User::Role> role)
        : WSuggestionPopup(popup_options)
{
    Wt::Dbo::Transaction transaction(session);

    for (const auto& user : get_users(session, role))
        addSuggestion(user->name());
}

