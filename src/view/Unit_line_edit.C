#include "Unit_line_edit.h"

#include <iomanip>
#include <sstream>

Unit_line_edit::Unit_line_edit(Wt::WContainerWidget* parent)
        : WCompositeWidget(parent)
{
    setImplementation(edit_ = new Wt::WLineEdit);
    edit_->setEmptyText("[0.0, 1.0]");
    edit_->keyWentUp().connect(std::bind([=]() { changed().emit(); }));
}

double Unit_line_edit::value() const
{
    std::istringstream input(edit_->text().toUTF8());
    double numeric;

    if (input >> numeric && 0 <= numeric && numeric <= 1)
        return numeric;
    else return -1;
}

void Unit_line_edit::set_value(double numeric)
{
    std::ostringstream fmt;
    if (0 <= numeric && numeric <= 1) {
        fmt.setf(std::ios::fixed);
        fmt << std::setprecision(2) << numeric;
    }
    edit_->setText(fmt.str());
}
