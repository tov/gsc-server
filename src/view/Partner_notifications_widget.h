#pragma once

#include "../model/Assignment.h"

#include <Wt/Dbo/ptr>
#include <Wt/WCompositeWidget>
#include <Wt/WContainerWidget>

class Session;
class Partner_request;
class User;

class Partner_notification_widget;

class Partner_pending_widget : public Wt::WContainerWidget
{
public:
    Partner_pending_widget(Partner_notification_widget* main,
                           const Wt::Dbo::ptr<Partner_request>& request,
                           bool inline_buttons = false,
                           bool show_assignment = false);

private:
    Partner_notification_widget* main_;
    Wt::Dbo::ptr<Partner_request> request_;

    void cancel_();
};

class Partner_confirmer_widget : public Wt::WContainerWidget
{
public:
    Partner_confirmer_widget(Partner_notification_widget* main,
                             const Wt::Dbo::ptr<Partner_request>& request,
                             bool inline_buttons = false,
                             bool show_assignment = false);

private:
    Partner_notification_widget* main_;
    Wt::Dbo::ptr<Partner_request> request_;

    void accept_();
    void reject_();
};

class Partner_notification_widget : Wt::WCompositeWidget
{
public:
    Partner_notification_widget(const Wt::Dbo::ptr<User>&,
                                const Wt::Dbo::ptr<Assignment>&,
                                Session& session,
                                Wt::WContainerWidget* parent);

private:
    Wt::Dbo::ptr<User> user_;
    Wt::Dbo::ptr<Assignment> assignment_;
    Session& session_;

    Wt::WContainerWidget* impl_;

    void update_();

    friend class Partner_pending_widget;
    friend class Partner_confirmer_widget;
};
