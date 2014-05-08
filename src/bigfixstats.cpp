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

#include <algorithm>
#include <fstream>  // NOLINT
#include <string>
#include <vector>
#include "bigfix/bigfixstats.h"

std::string ComputerGroup::name() const {
  return name_;
}

uint16_t ComputerGroup::current() const {
  return current_;
}

uint16_t ComputerGroup::target() const {
  return target_;
}

uint8_t ComputerGroup::percent() const {
  if (target_ != 0) {
    return current_ / target_ * 100;
  } else {
    return 0;
  }
}

void ComputerGroup::set_name(std::string name) {
  name_ = name;
}

void ComputerGroup::set_current(uint16_t current) {
  current_ = current;
}

void ComputerGroup::set_target(uint16_t target) {
  target_ = target;
}

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
  it = std::find(args.begin(), args.end(), "-h");
  if ((argc == 1) || (it != args.end())) {
    usage();
    return 0;
  }
  // use -i input file
  std::string infile {};
  it = std::find(args.begin(), args.end(), "-i");
  if (it != args.end()) {
    if (next(it) != args.end()) {
      infile = *next(it);
    } else {
      printf("%s: option -i requires an argument\n", bf::kProgramName.c_str());
      usage();
      return 1;
    }
  }
  std::vector<ComputerGroup> groups;
  loadCurrent(infile, &groups);
  display(&groups);
}

/**
 *  @details Load computer groups and current deployment counts
 */
void loadCurrent(std::string filename, std::vector<ComputerGroup>* groups) {
  std::ifstream fs(filename);
  if (fs.is_open()) {
    std::string line {};
    while (std::getline(fs, line)) {
      if (line.compare(0, bf::kRecord.length(), bf::kRecord) == 0) {
        // read records
        std::size_t start = line.find(bf::kStart, 0), end {0};
        while (start != std::string::npos) {
          uint16_t count {0};
          std::string group {}, current {};
          // read computer group
          end = line.find(bf::kEnd, start);
          if (end != std::string::npos) {
            start += bf::kStart.length();
            group = line.substr(start, end - start);
          }
          // read computer count
          start = line.find(bf::kStart, start + bf::kStart.length());
          end = line.find(bf::kEnd, start);
          if (end != std::string::npos) {
            start += bf::kStart.length();
            current = line.substr(start, end - start);
            count = std::stoi(current);
          }
          // create new computer group
          ComputerGroup cg = ComputerGroup();
          cg.set_name(group);
          cg.set_current(count);
          groups->push_back(cg);
          // read next computer group
          start = line.find(bf::kStart, start + bf::kStart.length());
        }
      }
    }
    fs.close();
  } else {
    printf("Error: Could not open file %s", filename.c_str());
  }
}

/**
 *  @details Display computer group, current, target and percentage
 */
void display(std::vector<ComputerGroup>* groups) {
  uint16_t currentTotal {0}, targetTotal {0}, percentTotal {0};
  std::string header = "| ", current = "| ", target = "| ", percent = "| ";
  for (auto cg : *groups) {
    header += cg.name() + " | ";
    current += std::to_string(cg.current()) + " | ";
    target += std::to_string(cg.target()) + " | ";
    percent += std::to_string(cg.percent()) + " | ";
    currentTotal += cg.current();
    targetTotal += cg.target();
  }
  header += " TOTAL |";
  current += std::to_string(currentTotal) + " |";
  target += std::to_string(targetTotal) + " |";
  if (targetTotal != 0) {
    percentTotal = static_cast<uint16_t>(currentTotal / targetTotal * 100);
  } else {
    percentTotal = 0;
  }
  percent += std::to_string(percentTotal) + " |";
  printf("%s\n%s\n%s\n%s\n", header.c_str(), current.c_str(), target.c_str(),
         percent.c_str());
}

/**
 *  @details Display program name, version, and usage
 */
void usage() {
  printf("%s, version %u.%u\n\n", bf::kProgramName.c_str(), bf::kMajorVersion,
         bf::kMinorVersion);
  printf("usage: %s [-h] [-i filename]\n", bf::kProgramName.c_str());
  printf("-h display usage\n");
  printf("-i filename\n\n");
}
