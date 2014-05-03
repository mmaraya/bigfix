/**
 *  @file bigfixstats.cpp
 *  @brief Converts BigFix deployment reports into Atlassian Confluence tables
 */

/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Michael Maraya
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <iostream>
#include <string>
#include <vector>
#include "bigfix/bigfixstats.h"

/**
 *  @brief Converts BigFix deployment reports into text for updating Atlassian 
 *         Confluence tables
 *  @param argc number of command-line arguments
 *  @param argv array of command-line arguments
 *  @retval int returns 0 upon successful program completion, non-zero otherwise
 */
int main(int argc, const char * argv[]) {
  // load arguments into vector<string> so we can use std::find
  std::vector<std::string> args;
  std::vector<std::string>::iterator it;
  for (int i = 1; i < argc; ++i) {
    args.push_back(argv[i]);
  }
  // display -h help
  it = find(args.begin(), args.end(), "-h");
  if ((argc == 1) || (it != args.end())) {
    usage();
    return 0;
  }
  // use -i input file
  std::string device_name {};
  it = find(args.begin(), args.end(), "-i");
  if (it != args.end()) {
    if (next(it) != args.end()) {
      device_name = *next(it);
    } else {
      std::cout << bf::kProgramName << ": option -i requires an argument\n";
      usage();
      return 1;
    }
  }
}

/**
 *  @details Display program name, version, and usage
 */
void usage() {
  std::cout << bf::kProgramName << ", version " << bf::kMajorVersion << '.';
  std::cout << bf::kMinorVersion << "\n\n";
  std::cout << "usage: " << bf::kProgramName;
  std::cout << " [-hv] [-i filename]\n";
  std::cout << "-h display usage\n";
  std::cout << "-i filename\n";
  std::cout << std::endl;
}
