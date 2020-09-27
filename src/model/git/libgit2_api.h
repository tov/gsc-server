#pragma once
#include <git2.h>
#include <iostream>
#include <tuple>

class libgit2_api {
public: 
	libgit2_api();

	~libgit2_api();

	std::tuple<int,int,int> version() const;
};

