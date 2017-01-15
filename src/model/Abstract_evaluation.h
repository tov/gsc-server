#pragma once

#include <string>

struct Abstract_evaluation
{
    double score() const = 0;
    void set_score(double) = 0;

    const std::string& explanation() const = 0;
    void set_explanation(const std::string&) = 0;

    virtual ~Abstract_evaluation() {}
};