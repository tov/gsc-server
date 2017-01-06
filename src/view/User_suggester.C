#include "User_suggester.h"
#include "../model/Session.h"

const Wt::WSuggestionPopup::Options User_suggester::options = {
        "<b>",         // highlightBeginTag
        "</b>",        // highlightEndTag
        ',',           // listSeparator      (for multiple addresses)
        " \\n",        // whitespace
        "-., \"@\\n;", // wordSeparators     (within an address)
        ""             // appendReplacedText (prepare next email address)
};

User_suggester::User_suggester(Session& session,
                               User::Role role,
                               Wt::WContainerWidget* parent)
        : WSuggestionPopup(options, parent)
{
    Wt::Dbo::Transaction transaction(session);

    Users users = session.find<User>()
                         .where("role = ?").bind((int) role)
                         .orderBy("name");
    for (const auto& user : users)
        addSuggestion(user->name());
}
