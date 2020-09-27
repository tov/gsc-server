#pragma once
#include <exception>
#include <string>

class git_exception : public std::exception {
public:
	git_exception();
	git_exception(const char* message);
	git_exception(const std::string &message);

	virtual ~git_exception() noexcept;
	virtual const char *what() const noexcept;

	static void clear();

protected:
	const char *message_;
};
