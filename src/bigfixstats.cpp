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
#include <set>
#include <vector>
#include "bigfix/bigfixstats.h"

ComputerGroup::ComputerGroup() {
}

ComputerGroup::ComputerGroup(const std::string name) {
  name_ = name;
}

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
    return static_cast<double>(current_) / target_ * 100;
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

bool operator<(const ComputerGroup& lhs, const ComputerGroup& rhs) {
  return (rhs.name().compare(lhs.name()) > 0);
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
  std::set<ComputerGroup> groups;
  loadTarget(target_file, &groups);
  loadCurrent(current_file, &groups);
  display(groups);
}

/**
 *  @details Load computer groups and target deployment counts
 */
void loadTarget(std::string filename, std::set<ComputerGroup>* groups) {
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
      groups->insert(cg);
    }
    fs.close();
  } else {
    printf("Error: Could not open file %s", filename.c_str());
  }
}

/**
 *  @details Load computer groups and current deployment counts
 */
void loadCurrent(std::string filename, std::set<ComputerGroup>* groups) {
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
          // update computer group
          ComputerGroup cg = ComputerGroup(group);
          cg.set_current(count);
          std::set<ComputerGroup>::iterator it = groups->find(cg);
          if (it != groups->end()) {
            ComputerGroup found = *it;
            cg.set_target(found.target());
            if (cg.name() == "OS") {
              cg.set_current(count + found.current());
            }
            groups->erase(found);
            groups->insert(cg);
          } else {
            if (cg.name() == "MBDA") {
              it = groups->find(ComputerGroup("OS"));
              ComputerGroup found = *it;
              cg.set_name(found.name());
              cg.set_target(found.target());
              groups->erase(found);
              groups->insert(cg);
            }
          }
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
void display(std::set<ComputerGroup> groups) {
  uint32_t currentTotal {0}, targetTotal {0}, percentTotal {0};
  std::string header = "| ", current = "| ", target = "| ", percent = "| ";
  for (auto cg : groups) {
    header += cg.name() + " | ";
    current += std::to_string(cg.current()) + " | ";
    target += std::to_string(cg.target()) + " | ";
    percent += std::to_string(cg.percent()) + " | ";
    currentTotal += cg.current();
    targetTotal += cg.target();
  }
  header += "TOTAL |";
  current += std::to_string(currentTotal) + " |";
  target += std::to_string(targetTotal) + " |";
  if (targetTotal != 0) {
    percentTotal = static_cast<float>(currentTotal) / targetTotal * 100;
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
  printf("usage: %s [-h] -t target -c current \n", bf::kProgramName.c_str());
  printf("-h display usage\n");
  printf("-t filename of the comma-separated computer group targets\n");
  printf("-c filename of the current computer group deployment statistics\n\n");
}
