#pragma once

#include <Wt/Dbo/ptr.h>
#include <Wt/WCompositeWidget.h>
#include <Wt/WContainerWidget.h>

class Session;
class Partner_request;
class Submission;
class User;

class Partner_notification_widget;

class Partner_requestor_widget : public Wt::WContainerWidget
{
public:
    explicit Partner_requestor_widget(Partner_notification_widget*);

private:
    Partner_notification_widget* main_;
    Wt::WLineEdit* edit_;

    void submit_();
    void error_();
};

class Partner_pending_widget : public Wt::WContainerWidget
{
public:
    Partner_pending_widget(Partner_notification_widget* main,
                           const Wt::Dbo::ptr<Partner_request>& request,
                           bool inline_buttons = false);

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
                             bool inline_buttons = false);

private:
    Partner_notification_widget* main_;
    Wt::Dbo::ptr<Partner_request> request_;

    void accept_();
    void reject_();
};


class Partner_notification_widget : public Wt::WCompositeWidget
{
public:
    Partner_notification_widget(
                const Wt::Dbo::ptr<User>&,
                const Wt::Dbo::ptr<Submission>&,
                Session& session);

private:
    Wt::Dbo::ptr<User> user_;
    Wt::Dbo::ptr<Submission> submission_;
    Session& session_;

    Wt::WContainerWidget* impl_;

    void update_();

    friend class Partner_requestor_widget;
    friend class Partner_pending_widget;
    friend class Partner_confirmer_widget;
};
