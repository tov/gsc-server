#include "Evaluation_view.h"
#include "File_list_widget.h"
#include "File_viewer_widget.h"
#include "../model/Assignment.h"
#include "../model/Eval_item.h"
#include "../model/Grader_eval.h"
#include "../model/Self_eval.h"
#include "../model/Session.h"
#include "../model/Submission.h"

#include <Wt/WApplication>
#include <Wt/WBreak>
#include <Wt/WButtonGroup>
#include <Wt/WHBoxLayout>
#include <Wt/WPushButton>
#include <Wt/WRadioButton>
#include <Wt/WSlider>
#include <Wt/WText>
#include <Wt/WTextArea>

#include <iomanip>

struct Evaluation_view::Row_model
{
    dbo::ptr<Eval_item> eval_item;
    dbo::ptr<Self_eval> self_eval;
    dbo::ptr<Grader_eval> grader_eval;
};

class Evaluation_view::Edit_widget : public Wt::WContainerWidget
{
public:
    enum class Mode {
        self_eval,
        self_review,
        grader_eval,
    };

    Edit_widget(Row_model&, Mode,
                Evaluation_view&,
                Session&,
                Wt::WContainerWidget* parent = nullptr);

    static std::unique_ptr<Edit_widget>
    create(Row_model&, Mode, Evaluation_view&, Session&,
           Wt::WContainerWidget* parent = nullptr);

protected:
    Row_model& model_;
    Mode mode_;
    Session& session_;
    Evaluation_view& main_;

    Wt::WContainerWidget* response_;

    virtual double score() const = 0;
    virtual void set_score(double) = 0;
    virtual std::string explanation() const = 0;
    virtual void set_explanation(const std::string&) = 0;
    virtual void reset() = 0;

    void load_();
    void save_();
    void retract_();
};

Evaluation_view::Edit_widget::Edit_widget(
        Row_model& model,
        Mode mode,
        Evaluation_view& main,
        Session& session,
        Wt::WContainerWidget* parent)
        : WContainerWidget(parent),
          model_(model),
          mode_(mode),
          main_(main),
          session_(session)
{
    setStyleClass("edit-widget");

    auto pct = 100 * model.eval_item->relative_value() / main_.total_value_;
    std::ostringstream title;
    title << "<h4>Question " << model.eval_item->sequence();
    title << " <small>(" << std::setprecision(2) << pct << "%)</small>";
    title << "</h4>";
    new Wt::WText(title.str(), this);

    auto prompt = new Wt::WText(model_.eval_item->prompt(), this);
    prompt->setInline(false);

    response_ = new Wt::WContainerWidget(this);

    auto buttons = new Wt::WContainerWidget(this);
    buttons->setStyleClass("buttons");

    if (main_.can_eval_()) {
        switch (mode_) {
            case Mode::self_eval: {
                auto save = new Wt::WPushButton("Save", buttons);
                save->clicked().connect(this, &Edit_widget::save_);
                break;
            }

            case Mode::self_review: {
                auto retract = new Wt::WPushButton("Retract", buttons);
                retract->clicked().connect(this, &Edit_widget::retract_);
                break;
            }

            case Mode::grader_eval:
                break;
        }
    }
}

void Evaluation_view::Edit_widget::load_()
{
    if (model_.self_eval) {
        set_score(model_.self_eval->score());
        set_explanation(model_.self_eval->explanation());
    } else reset();
}

void Evaluation_view::Edit_widget::save_()
{
    if (!main_.can_eval_()) return;

    dbo::Transaction transaction(session_);

    if (!model_.self_eval)
        model_.self_eval
                = session_.add(new Self_eval(model_.eval_item,
                                             main_.submission_));

    auto self_eval = model_.self_eval.modify();
    self_eval->set_score(score());
    self_eval->set_explanation(explanation());

    int next = model_.self_eval->eval_item()->sequence() + 1;
    if (0 < next && next < main_.model_.size())
        main_.go_to((unsigned int) next);
    else
        main_.go_default();
}

void Evaluation_view::Edit_widget::retract_()
{
    if (!main_.can_eval_()) return;

    dbo::Transaction transaction(session_);
    model_.self_eval.remove();
    transaction.commit();

    main_.go_to((unsigned int) model_.eval_item->sequence());
}

class Evaluation_view::Response_edit_widget : public Edit_widget
{
public:
    Response_edit_widget(Row_model&, Mode, Evaluation_view&, Session&,
                         WContainerWidget* parent = nullptr);

protected:
    virtual std::string explanation() const override;
    virtual void set_explanation(const std::string& string) override;
    virtual void reset() override;

    // Allow derived classes to populate this:
    Wt::WContainerWidget* score_holder_;
    // Allow derived classes to hide this:
    Wt::WContainerWidget* explanation_holder_;

private:
    Wt::WTextArea* explanation_;
};

Evaluation_view::Response_edit_widget::Response_edit_widget(
        Row_model& model,
        Mode mode,
        Evaluation_view& main,
        Session& session,
        Wt::WContainerWidget* parent)
        : Edit_widget(model, mode, main, session, parent)
{
    score_holder_ = new Wt::WContainerWidget(response_);
    score_holder_->setStyleClass("score");

    explanation_holder_ = new Wt::WContainerWidget(response_);

    new Wt::WText("<p>Explain, including line references (e.g. L14):</p>",
                  explanation_holder_);

    explanation_ = new Wt::WTextArea(explanation_holder_);
    explanation_->setStyleClass("explanation");
    explanation_->setInline(false);
}

std::string Evaluation_view::Response_edit_widget::explanation() const
{
    return explanation_->text().toUTF8();
}

void
Evaluation_view::Response_edit_widget::set_explanation(const std::string& text)
{
    explanation_->setText(Wt::WString::fromUTF8(text));
}

void Evaluation_view::Response_edit_widget::reset()
{
    explanation_->setText("");
}

class Evaluation_view::Boolean_edit_widget : public Response_edit_widget
{
public:
    Boolean_edit_widget(Row_model&, Mode, Evaluation_view&, Session&,
                        WContainerWidget* parent = nullptr);

protected:
    virtual double score() const override;
    virtual void set_score(double d) override;
    virtual void reset() override;

private:
    Wt::WButtonGroup* no_yes_;
    Wt::WRadioButton* no_;
    Wt::WRadioButton* yes_;

    void toggle_explanation_();
};

Evaluation_view::Boolean_edit_widget::Boolean_edit_widget(
        Row_model& model,
        Mode mode,
        Evaluation_view& main,
        Session& session,
        Wt::WContainerWidget* parent)
        : Response_edit_widget(model, mode, main, session, parent)
{
    no_yes_ = new Wt::WButtonGroup(score_holder_);
    no_yes_->addButton(no_ = new Wt::WRadioButton("No", score_holder_));
    no_yes_->addButton(yes_ = new Wt::WRadioButton("Yes", score_holder_));

    explanation_holder_->hide();
    no_yes_->checkedChanged().connect(this,
                                      &Boolean_edit_widget::toggle_explanation_);

    load_();
}

double Evaluation_view::Boolean_edit_widget::score() const
{
    return no_yes_->checkedButton() == yes_ ? 1.0 : 0.0;
}

void Evaluation_view::Boolean_edit_widget::set_score(double d)
{
    no_yes_->setCheckedButton(d > 0 ? yes_ : no_);
    toggle_explanation_();
}

void Evaluation_view::Boolean_edit_widget::reset()
{
    Response_edit_widget::reset();
    no_yes_->setCheckedButton(nullptr);
}

void Evaluation_view::Boolean_edit_widget::toggle_explanation_()
{
    if (no_yes_->checkedButton() == yes_)
        explanation_holder_->show();
    else
        explanation_holder_->hide();
}

class Evaluation_view::Scale_edit_widget
        : public Evaluation_view::Response_edit_widget
{
public:
    Scale_edit_widget(Row_model&, Mode, Evaluation_view&, Session&,
                      WContainerWidget* parent = nullptr);

protected:
    virtual double score() const override;
    virtual void set_score(double d) override;
    virtual void reset() override;

private:
    Wt::WSlider* slider_;
    Wt::WText* number_;

    void update_number_();
};

Evaluation_view::Scale_edit_widget::Scale_edit_widget(
        Row_model& model,
        Mode mode,
        Evaluation_view& main,
        Session& session,
        Wt::WContainerWidget* parent)
        : Response_edit_widget(model, mode, main, session, parent)
{
    slider_ = new Wt::WSlider(score_holder_);
    slider_->resize(200, 50);
    slider_->setTickPosition(Wt::WSlider::TicksAbove);
    slider_->setTickInterval(20);
    slider_->setMinimum(0);
    slider_->setMaximum(100);

    new Wt::WBreak(score_holder_);

    number_ = new Wt::WText(score_holder_);

    slider_->valueChanged().connect(this, &Scale_edit_widget::update_number_);

    load_();
}

double Evaluation_view::Scale_edit_widget::score() const
{
    return slider_->value() / 100.0;
}

void Evaluation_view::Scale_edit_widget::set_score(double d)
{
    slider_->setValue(int(100 * d));
    update_number_();
}

void Evaluation_view::Scale_edit_widget::reset()
{
    Response_edit_widget::reset();
    slider_->setValue(50);
}

void Evaluation_view::Scale_edit_widget::update_number_()
{
    number_->setText(slider_->valueText() + "%");
}

class Evaluation_view::Informational_edit_widget
        : public Evaluation_view::Edit_widget
{
public:
    Informational_edit_widget(Row_model&, Mode, Evaluation_view&, Session&,
                              WContainerWidget* parent = nullptr);

protected:
    virtual double score() const override;
    virtual void set_score(double d) override;
    virtual std::string explanation() const override;
    virtual void set_explanation(const std::string& string) override;
    virtual void reset() override;
};

Evaluation_view::Informational_edit_widget::Informational_edit_widget(
        Row_model& model,
        Mode mode,
        Evaluation_view& main,
        Session& session,
        Wt::WContainerWidget* parent)
        : Edit_widget(model, mode, main, session, parent)
{
    load_();
}

double Evaluation_view::Informational_edit_widget::score() const
{
    return 1;
}

void Evaluation_view::Informational_edit_widget::set_score(double)
{
    // no-op
}

std::string Evaluation_view::Informational_edit_widget::explanation() const
{
    return "";
}

void
Evaluation_view::Informational_edit_widget::set_explanation(const std::string&)
{
    // no-op
}

void Evaluation_view::Informational_edit_widget::reset()
{
    // no-op
}

std::unique_ptr<Evaluation_view::Edit_widget>
Evaluation_view::Edit_widget::create(Row_model& model,
                                     Mode mode,
                                     Evaluation_view& main,
                                     Session& session,
                                     Wt::WContainerWidget* parent)
{
    switch (model.eval_item->type()) {
        case Eval_item::Type::Boolean:
            return std::make_unique<Boolean_edit_widget>(model, mode, main,
                                                         session, parent);
        case Eval_item::Type::Scale:
            return std::make_unique<Scale_edit_widget>(model, mode, main,
                                                       session, parent);
        case Eval_item::Type::Informational:
            return std::make_unique<Informational_edit_widget>(model, mode, main,
                                                               session, parent);
    }
}

Evaluation_view::Evaluation_view(const dbo::ptr<Submission>& submission,
                                 Session& session,
                                 Wt::WContainerWidget* parent)
        : WContainerWidget(parent),
          submission_(submission),
          session_(session)
{
    load_();

    setStyleClass("evaluation-view");

    auto hbox = new Wt::WHBoxLayout();
    setLayout(hbox);

    auto viewer_ = new File_viewer_widget(submission_, session_);
    hbox->addWidget(viewer_);

    right_column_ = new Wt::WContainerWidget;
    hbox->addWidget(right_column_, 1);
    right_column_->setStyleClass("right-column");
}

void Evaluation_view::load_()
{
    dbo::Transaction transaction(session_);

    total_value_ = 0;

    for (const auto& eval_item : submission_->assignment()->eval_items()) {
        total_value_ += eval_item->relative_value();
        auto sequence = eval_item->sequence();
        while (model_.size() <= sequence) model_.emplace_back();
        model_[sequence].eval_item = eval_item;
    }

    for (const auto& self_eval : submission_->self_evals()) {
        auto sequence = self_eval->eval_item()->sequence();
        model_[sequence].self_eval = self_eval;
        model_[sequence].grader_eval = self_eval->grader_eval();
    }
}

void Evaluation_view::go_to(unsigned int index)
{
    std::ostringstream path;
    path << "/hw/" << submission_->assignment()->number()
         << "/eval/" << index;
    Wt::WApplication::instance()->setInternalPath(path.str());

    rows_.clear();
    rows_.push_back(Edit_widget::create(model_.at(index),
                                        Edit_widget::Mode::self_eval,
                                        *this, session_, right_column_));
}

void Evaluation_view::go_default()
{
    for (auto& row : model_) {
        if (row.eval_item && !row.self_eval) {
            go_to((unsigned int) row.eval_item->sequence());
            return;
        }
    }

    std::ostringstream path;
    path << "/hw/" << submission_->assignment()->number() << "/eval";
    Wt::WApplication::instance()->setInternalPath(path.str());

    rows_.clear();
    for (auto& row : model_)
        if (row.eval_item)
            rows_.push_back(Edit_widget::create(row,
                                                Edit_widget::Mode::self_review,
                                                *this, session_, right_column_));
}

bool Evaluation_view::can_eval_()
{
    return submission_->can_eval(session_.user());
}
