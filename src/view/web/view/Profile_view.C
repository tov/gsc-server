#include "Profile_view.h"
#include "../widget/Api_key_widget.h"
#include "../../../model/auth/User.h"
#include "../../../common/util.h"

#include <Wt/WTemplate.h>

Profile_view::Profile_view(dbo::ptr<User> const& user, Session& session)
        : user_(user)
        , session_(session)
{
    impl_ = setNewImplementation<WTemplate>(tr("gsc.template.profile-table"));

    impl_->setStyleClass("profile-view");
    impl_->bindString("login-name", user->name());
    impl_->bindString("role", user->role_string());
    impl_->bindNew<Api_key_widget>("api-key", user_, session_);
}
