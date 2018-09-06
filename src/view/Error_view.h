#pragma once

#include <Wt/WContainerWidget.h>

class Error_view : public Wt::WContainerWidget
{
public:
    Error_view(const std::string& message);
};