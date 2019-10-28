#include "User_suggester.h"
#include "../../Session.h"

static Wt::WSuggestionPopup::Options const popup_options = {
        "<b>",         // highlightBeginTag
        "</b>",        // highlightEndTag
        '\0',          // listSeparator      (for multiple addresses)
        " ",           // whitespace
        "",            // wordSeparators     (within an address)
        ""             // appendReplacedText (prepare next email address)
};

User_suggester::User_suggester(Session& session, User::Role role)
        : WSuggestionPopup(popup_options)
{
    Wt::Dbo::Transaction transaction(session);

    Users users = session.find<User>()
                         .where("role = ?").bind((int) role)
                         .orderBy("name");
    for (const auto& user : users)
        addSuggestion(user->name());
}

User_suggester::User_suggester(Session& session)
        : WSuggestionPopup(popup_options)
{
    Wt::Dbo::Transaction transaction(session);

    Users users = session.find<User>()
                         .orderBy("name");
    for (const auto& user : users)
        addSuggestion(user->name());
}
