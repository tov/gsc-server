#pragma once

#include <Wt/Dbo/Dbo>
#include <string>

namespace dbo = Wt::Dbo;

class User;

class Exam_grade
{
public:
    Exam_grade() {}
    Exam_grade(const dbo::ptr<User>& user, int sequence);

    const dbo::ptr<User>& user() const { return user_; }
    int sequence() const { return sequence_; }
    int points() const { return points_; }
    int possible() const { return possible_; }

    void set_points_and_possible(int points, int possible);

    std::string pct_string() const;

    static dbo::collection<dbo::ptr<Exam_grade>>
    find_by_user(const dbo::ptr<User>&);

    static dbo::ptr<Exam_grade>
    get_by_user_and_sequence(const dbo::ptr<User>&, int sequence);

private:
    dbo::ptr<User> user_;
    int sequence_;
    int points_;
    int possible_;

public:
    template<typename Action>
    void persist(Action& a)
    {
        dbo::belongsTo(a, user_, "user", dbo::OnDeleteCascade);
        dbo::field(a, sequence_, "sequence");
        dbo::field(a, points_, "points");
        dbo::field(a, possible_, "possible");
    }
};

DBO_EXTERN_TEMPLATES(Exam_grade);
