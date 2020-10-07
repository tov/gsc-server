/*
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>
#include <Wt/WStringUtil.h>

#include "Dictionary.h"
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>

std::wstring RandomWord(Dictionary dictionary) {
  std::ifstream dict;
  if (dictionary == DICT_NL) {
    dict.open((Wt::WApplication::appRoot() + "dict-nl.txt").c_str());
  } else { // english is default
    dict.open((Wt::WApplication::appRoot() + "dict.txt").c_str());
  }

  std::string retval;
  int numwords = 0;
  while (dict) {
    std::getline(dict, retval);
    numwords++;
  }
  dict.clear();
  dict.seekg(0);

  std::srand(std::time(0));
  int selection =
      std::rand() % numwords; // not entirely uniform, but who cares?

  while (selection--) {
    getline(dict, retval);
  }
  std::getline(dict, retval);
  for (unsigned int i = 0; i < retval.size(); ++i)
    if (retval[i] < 'A' || retval[i] > 'Z')
      std::cerr << "word " << retval << " contains illegal data at pos " << i
                << std::endl;

  return Wt::widen(retval);
}
