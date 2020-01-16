#include "User_selector.h"
#include "User_suggester.h"
#include "../../../Session.h"
#include "../../../common/util.h"

#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>

#include <sstream>

User_selector::User_selector(Session& session,
                             optional<User::Role> role)
        : session_(session)
{
    auto impl = setNewImplementation<WContainerWidget>();

    edit_ = impl->addNew<WLineEdit>();
    edit_->setPlaceholderText("username");
    edit_->setStyleClass("username");

    auto popup = impl->addNew<User_suggester>(session, role);
    popup->forEdit(edit_);
}

string User_selector::cleaned_text() const
{
    return clean_text(text());
}

dbo::ptr<User> User_selector::selected_user() const
{
    Wt::Dbo::Transaction transaction(session_);
    return User::find_by_name(session_, cleaned_text());
}

string clean_text(Wt::WString const& text)
{
    ostringstream os;

    for (char c : text.toUTF8()) {
        if (isalnum(c) || c == '-' || c == '_') {
            os << c;
        }
    }

    return os.str();
}

