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
#include <cmath>
#include <fstream>  // NOLINT
#include <map>
#include <string>
#include <vector>
#include "bigfix/bigfixstats.h"

ComputerGroup::ComputerGroup() {
}

ComputerGroup::ComputerGroup(const std::string name) {
  name_ = name;
}

uint8_t ComputerGroup::widest() const {
  uint8_t top {0};
  uint8_t name = name_.length();
  uint8_t current = format(current_).length();
  uint8_t target = format(target_).length();
  uint8_t percent = format(this->percent()).length() + 2;
  std::vector<uint8_t> vector = {name, current, target, percent};
  for (auto it : vector) {
    if (it > top) {
      top = it;
    }
  }
  return top;
}

std::string ComputerGroup::name() const {
  return name_;
}

std::string ComputerGroup::formatted_name() const {
  return name_ + std::string(this->widest() - name_.length(), ' ');
}

uint32_t ComputerGroup::current() const {
  return current_;
}

std::string ComputerGroup::formatted_current() const {
  std::string output = format(current_);
  return output + std::string(this->widest() - output.length() + 1, ' ');
}

uint32_t ComputerGroup::target() const {
  return target_;
}

std::string ComputerGroup::formatted_target() const {
  std::string output = format(target_);
  return output + std::string(this->widest() - output.length() + 1, ' ');
}

uint8_t ComputerGroup::percent() const {
  if (target_ != 0) {
    return round(static_cast<double>(current_) / target_ * 100);
  } else {
    return 0;
  }
}

std::string ComputerGroup::formatted_percent() const {
  std::string output = "*" + std::to_string(this->percent()) + "*";
  return output + std::string(this->widest() - output.length() + 1, ' ');
}

void ComputerGroup::set_name(std::string name) {
  name_ = name;
}

void ComputerGroup::set_current(uint32_t current) {
  current_ = current;
}

void ComputerGroup::set_target(uint32_t target) {
  target_ = target;
}

/**
 *  @details format the supplied number into comma-separated groupings since
 *           there apparently is no portable way of doing this
 */
std::string ComputerGroup::format(uint32_t number) const {
  std::string output = std::to_string(number);
  if (output.length() > 3) {
    for (int i = static_cast<int>(output.length() - 3); i > 0; i -= 3) {
      output.insert(i, ",");
    }
  }
  return output;
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
  // use -c current file
  std::string current_file {};
  it = std::find(args.begin(), args.end(), "-c");
  if (it != args.end()) {
    if (next(it) != args.end()) {
      current_file = *next(it);
    } else {
      printf("%s: option -c requires an argument\n", bf::kProgramName.c_str());
      usage();
      return 1;
    }
  }
  // use -t target file
  std::string target_file {};
  it = std::find(args.begin(), args.end(), "-t");
  if (it != args.end()) {
    if (next(it) != args.end()) {
      target_file = *next(it);
    } else {
      printf("%s: option -t requires an argument\n", bf::kProgramName.c_str());
      usage();
      return 1;
    }
  }
  std::vector<ComputerGroup> groups;
  loadTarget(target_file, groups);
  loadCurrent(current_file, groups);
  display(groups);
}

/**
 *  @details Load computer groups and target deployment counts
 */
void loadTarget(std::string filename, std::vector<ComputerGroup>& groups) {
  std::ifstream fs(filename);
  if (fs.is_open()) {
    std::string line {};
    while (std::getline(fs, line)) {
      std::string group {};
      uint16_t target {0};
      std::size_t delim = line.find(bf::kDelim, 0);
      // read computer group and target
      group = line.substr(0, delim);
      target = std::stoi(line.substr(delim + 1, line.length()));
      // create new computer group
      ComputerGroup cg = ComputerGroup(group);
      cg.set_target(target);
      groups.push_back(cg);
    }
    fs.close();
  } else {
    printf("Error: Could not open file %s", filename.c_str());
  }
}

/**
 *  @details Load current deployment counts
 */
void loadCurrent(std::string filename, std::vector<ComputerGroup>& groups) {
  std::map<std::string, uint32_t> current;
  std::ifstream fs(filename);
  if (fs.is_open()) {
    std::string line {};
    while (std::getline(fs, line)) {
      if (line.compare(0, bf::kRecord.length(), bf::kRecord) == 0) {
        // read records
        std::size_t start = line.find(bf::kStart, 0), end {0};
        while (start != std::string::npos) {
          std::string group {};
          uint32_t count {0};
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
            count = std::stoi(line.substr(start, end - start));
          }
          // populate collection
          current[group] = count;
          // read next computer group
          start = line.find(bf::kStart, start + bf::kStart.length());
        }
      }
    }
    fs.close();
    // update computer group collection
    std::map<std::string, uint32_t>::iterator it;
    for (auto &cg : groups) {
      it = current.find(cg.name());
      uint32_t count = current.find(cg.name())->second;
      cg.set_current(count);
      // do replacements for MBDA and OS
      if (cg.name() == "OS") {
        cg.set_name("OS*");
        cg.set_current(count + cg.current());
      }
      if (cg.name() == "MBDA") {
        cg.set_target(count);
      }
    }
  } else {
    printf("Error: Could not open file %s", filename.c_str());
  }
}

/**
 *  @details Display computer group, current, target and percentage
 */
void display(std::vector<ComputerGroup>& groups) {
  uint32_t currentTotal {0}, targetTotal {0};
  // compute totals
  for (auto cg : groups) {
    currentTotal += cg.current();
    targetTotal += cg.target();
  }
  ComputerGroup total = ComputerGroup("TOTAL");
  total.set_current(currentTotal);
  total.set_target(targetTotal);
  groups.push_back(total);
  // populate rows
  std::string header = "|| Nodes       || ";
  std::string current = "| *Current*    | ";
  std::string target = "| *Target*     | ";
  std::string percent = "| *% Comp*     | ";
  // display results
  for (auto cg : groups) {
    header += cg.formatted_name() + " || ";
    current += cg.formatted_current() + " | ";
    target += cg.formatted_target() + " | ";
    percent += cg.formatted_percent() + " | ";
  }
  printf("%s\n%s\n%s\n%s\n", header.c_str(), current.c_str(), target.c_str(),
         percent.c_str());
}

/**
 *  @details Display program name, version, and usage
 */
void usage() {
  printf("%s, version %u.%u\n\n", bf::kProgramName.c_str(), bf::kMajorVersion,
         bf::kMinorVersion);
  printf("usage: %s [-h] -t target -c current \n", bf::kProgramName.c_str());
  printf("-h display usage\n");
  printf("-t filename of the comma-separated computer group targets\n");
  printf("-c filename of the current computer group deployment statistics\n\n");
}

