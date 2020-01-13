#include "Profile_view.h"
#include "../widget/Api_key_widget.h"
#include "../../../common/util.h"

#include <Wt/WContainerWidget.h>

Profile_view::Profile_view(dbo::ptr<User> const& user, Session& session)
        : user_(user)
        , session_(session)
{
    impl_ = setNewImplementation<WContainerWidget>();
    impl_->setStyleClass("profile-view");
    impl_->addNew<Api_key_widget>(user_, session_);
}

