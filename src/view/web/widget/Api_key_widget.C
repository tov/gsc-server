#include "Api_key_widget.h"
#include "../../../common/util.h"
#include "../../../Session.h"
#include "../Confirmation_dialog.h"

#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>

namespace templates {

WString const
        widget  = WString::tr("gsc.template.api-key-widget"),
        button  = WString::tr("gsc.message.api-key.generate-button"),
        confirm = WString::tr("gsc.message.api-key.confirm-regenerate");

}

Api_key_widget::Api_key_widget(dbo::ptr<User> const& user, Session& session)
        : user_(user)
        , session_(session)
{
    impl_ = setNewImplementation<WTemplate>(templates::widget);

    auto button = impl_->bindNew<WPushButton>("button", templates::button);
    button->clicked().connect(this, &Api_key_widget::confirm_regenerate_);

    load_();
}

void Api_key_widget::load_() {
    auto api_key = session_.get_api_key(user_);
    impl_->bindString("key", api_key);
}

void Api_key_widget::confirm_regenerate_()
{
    Confirmation_dialog::create(templates::confirm)
            .accepted().connect(this, &Api_key_widget::regenerate_);
}

void Api_key_widget::regenerate_()
{
    session_.set_api_key(user_);
    load_();
}

