#include "../../common/util.h"
#include "Username_validator.h"

#include "Wt/WString.h"

namespace {
namespace re {

WString const NETID = "[a-z]{3}[0-9]{3,4}";
WString const ANY = ".*";

} // namespace re
} // namespace

Username_validator::Username_validator(bool strict)
    : WRegExpValidator(strict ? re::NETID : re::ANY) {}

string Username_validator::inputFilter() const { return "[a-zA-Z0-9]"; }
