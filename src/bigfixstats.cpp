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
  std::string ret;
  if (name_ == "OS") {
    ret = name_ + "*" + std::string(this->widest() - name_.length() - 1, ' ');
  } else {
    ret = name_ + std::string(this->widest() - name_.length(), ' ');
  }
  return ret;
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
  std::map<std::string, uint32_t> raw;
  std::vector<ComputerGroup> final;
  loadTarget(target_file, &final);
  loadCurrent(current_file, &raw, &final);
  display(current_file, &raw, &final);
}

/**
 *  @details Load computer groups and target deployment counts
 */
void loadTarget(std::string filename, std::vector<ComputerGroup>* final) {
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
      final->push_back(cg);
    }
    fs.close();
  } else {
    printf("Error: Could not open file %s", filename.c_str());
  }
}

/**
 *  @details Load current deployment counts
 */
void loadCurrent(std::string filename, std::map<std::string, uint32_t>* raw,
                 std::vector<ComputerGroup>* final) {
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
          raw->emplace(group, count);
          // read next computer group
          start = line.find(bf::kStart, start + bf::kStart.length());
        }
      }
    }
    fs.close();
    // update computer group collection
    std::map<std::string, uint32_t>::iterator it;
    for (auto &cg : *final) {
      it = raw->find(cg.name());
      if (it != raw->end()) {
        cg.set_current(it->second);
        // add MBDA current deployment stats to OS
        if (cg.name() == "OS") {
          it = raw->find("MBDA");
          cg.set_current(cg.current() + it->second);
        }
      }
    }
  } else {
    printf("Error: Could not open file %s", filename.c_str());
  }
}

/**
 *  @details Display computer group, current, target and percentage
 */
void display(std::string filename, std::map<std::string, uint32_t>* raw,
             std::vector<ComputerGroup>* final) {
  // extract date from filename
  size_t begin = filename.length() - bf::kExt.length() - bf::kDate.length();
  std::string date = filename.substr(begin, bf::kDate.length());
  // store raw results
  std::string raw_display[2] {"||   Date   || ", "|  " + date + "  | "};
  // compute raw totals
  uint32_t raw_total {0};
  for (auto cg : *raw) {
    raw_total += cg.second;
    if (cg.first != "CBS" && cg.first != "HCHB") {
      raw_display[0] += cg.first + " || ";
      raw_display[1] += std::to_string(cg.second) + " | ";
    }
  }
  printf("%s\n%s\n\n", raw_display[0].c_str(), raw_display[1].c_str());
  // compute final totals
  uint32_t current_total {0}, target_total {0};
  for (auto cg : *final) {
    current_total += cg.current();
    target_total += cg.target();
  }
  ComputerGroup total = ComputerGroup("TOTAL");
  total.set_current(current_total);
  total.set_target(target_total);
  final->push_back(total);
  // populate rows
  std::string header = "|| Nodes       || ";
  std::string current = "| *Current*    | ";
  std::string target = "| *Target*     | ";
  std::string percent = "| *% Comp*     | ";
  // display results
  for (auto cg : *final) {
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

