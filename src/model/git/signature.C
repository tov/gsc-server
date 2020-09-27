#include "signature.h"

signature::signature(const std::string &name, const std::string &email) {
	if (git_signature_now(&g_signature_, name.c_str(), email.c_str())) {
			throw git_exception();
	}
}

const git_signature *signature::g_signature() const { return g_signature_; }
