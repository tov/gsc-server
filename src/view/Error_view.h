#pragma once

#include <Wt/WContainerWidget>

class Error_view : public Wt::WContainerWidget
{
public:
    Error_view(const std::string& message);
};