#include "../../../Session.h"
#include "../../../common/util.h"
#include "../Confirmation_dialog.h"
#include "Api_key_widget.h"
#include "Glyph_button.h"

#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>

namespace templates {

WString const widget = WString::tr("gsc.template.api-key-widget"),
              confirm = WString::tr("gsc.message.api-key.confirm-regenerate");

}

Api_key_widget::Api_key_widget(dbo::ptr<User> const &user, Session &session)
    : user_(user), session_(session) {
  impl_ = setNewImplementation<WTemplate>(templates::widget);

  auto regen_button =
      impl_->bindNew<Glyph_button>("regenerate-button", "alert", "Regenerate");
  regen_button->setStyleClass("btn btn-danger");
  regen_button->clicked().connect([=] { confirm_regenerate_(); });

  load_();
}

void Api_key_widget::load_() {
  auto api_key = session_.get_api_key(user_);
  impl_->bindString("key", api_key);
}

void Api_key_widget::copy_() const {
  WApplication::instance()->doJavaScript("GSC.selectCopyId('api-key-input')");
}

void Api_key_widget::confirm_regenerate_() {
  Confirmation_dialog::create(templates::confirm)
      .accepted()
      .connect(this, &Api_key_widget::regenerate_);
}

void Api_key_widget::regenerate_() {
  session_.set_api_key(user_);
  load_();
}
