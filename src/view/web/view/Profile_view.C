#include "Profile_view.h"
#include "../widget/Api_key_widget.h"
#include "../../../model/auth/User.h"
#include "../../../Session.h"
#include "../../../common/util.h"

#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>

class Profile_view::Call_me : public WCompositeWidget
{
public:
    Call_me(Profile_view& parent);

private:
    Profile_view& parent_;
    WContainerWidget* impl_;

    void view_mode_();
    void edit_mode_();
    void save_edited_value_(WLineEdit*);

    std::string const& value_() const;
};

using Call_me = Profile_view::Call_me;

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

Call_me::Call_me(Profile_view& parent)
        : parent_(parent)
{
    impl_ = setNewImplementation<WContainerWidget>();

    if (value_().empty())
        edit_mode_();
    else
        view_mode_();
}

void Call_me::view_mode_()
{
    impl_->clear();

    impl_->addNew<WText>(value_(), TextFormat::Plain);
    auto edit_button = impl_->addNew<WPushButton>("Edit");

    edit_button->clicked().connect(this, &Call_me::edit_mode_);
}

void Call_me::edit_mode_()
{
    impl_->clear();

    auto line_edit = impl_->addNew<WLineEdit>(value_());
    auto cancel_btn = impl_->addNew<WPushButton>("Cancel");
    auto save_btn = impl_->addNew<WPushButton>("Save");

    cancel_btn->clicked().connect(this, &Call_me::view_mode_);
    save_btn->clicked().connect([=] { save_edited_value_(line_edit); });
}

std::string const& Call_me::value_() const
{
    return parent_.user_->call_me();
}

void Profile_view::Call_me::save_edited_value_(WLineEdit* line_edit)
{
    dbo::Transaction trans(parent_.session_);
    std::string edited_value = line_edit->valueText().toUTF8();
    if (edited_value != value_())
        parent_.user_.modify()->set_call_me(edited_value);
    trans.commit();

    view_mode_();
}
