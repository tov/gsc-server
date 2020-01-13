#include "Profile_view.h"
#include "../../Session.h"
#include "../../model/auth/User.h"
#include "../../common/util.h"

#include <Wt/WBreak.h>
#include <Wt/WPushButton.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>

Profile_view::Profile_view(dbo::ptr<User> const& user, Session& session)
        : user_(user)
        , session_(session)
{
    impl_ = setNewImplementation<WContainerWidget>();
    load_();
}

void Profile_view::load_() {
    impl_->clear();

    auto api_key = session_.get_api_key(user_);

    impl_->addNew<WText>("API KEY: ");
    impl_->addNew<WText>(api_key);
    impl_->addNew<WBreak>();

    auto button = impl_->addNew<WPushButton>("reset");
    button->clicked().connect([=] () { regenerate_(); });
}

void Profile_view::regenerate_()
{
    session_.set_api_key(user_);
    load_();
}
