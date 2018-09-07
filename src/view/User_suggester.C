#include "User_suggester.h"
#include "../Session.h"

const Wt::WSuggestionPopup::Options User_suggester::options = {
        "<b>",         // highlightBeginTag
        "</b>",        // highlightEndTag
        ',',           // listSeparator      (for multiple addresses)
        " ",           // whitespace
        "-., \"@;",    // wordSeparators     (within an address)
        ""             // appendReplacedText (prepare next email address)
};

User_suggester::User_suggester(Session& session, User::Role role)
        : WSuggestionPopup(options)
{
    Wt::Dbo::Transaction transaction(session);

    Users users = session.find<User>()
                         .where("role = ?").bind((int) role)
                         .orderBy("name");
    for (const auto& user : users)
        addSuggestion(user->name());
}

User_suggester::User_suggester(Session& session)
        : WSuggestionPopup(options)
{
    Wt::Dbo::Transaction transaction(session);

    Users users = session.find<User>()
                         .orderBy("name");
    for (const auto& user : users)
        addSuggestion(user->name());
}
