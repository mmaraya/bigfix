/**
 *  @file bigfixstats.h
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

#ifndef BIGFIX_BIGFIXSTATS_H_
#define BIGFIX_BIGFIXSTATS_H_

#include <set>
#include <string>

/**
 *  @brief Model class for BigFix deployment information for a single computer
 *         group, following the model-view-controller software design pattern
 *  @details ComputerGroup contains all the operations and attributes associated
 *           with the deployment information for a single computer groups
 */
class ComputerGroup {
 private:
  /**
   *  @brief Name of this computer group
   */
  std::string name_;

  /**
   *  @brief Number of computers currently in this computer group
   */
  uint16_t current_ {0};

  /**
   *  @brief Number of computers expected to be in this computer group
   */
  uint16_t target_ {0};

 public:
  /**
   *  @brief Construct an empty computer group
   */
  ComputerGroup();

  /**
   *  @brief Construct a new computer group with the supplied name
   *  @param name name of the computer group
   */
  explicit ComputerGroup(const std::string name);

  /**
   *  @brief Accessor method for the name_ property
   *  @retval std::string Name of this computer group
   */
  std::string name() const;

  /**
   *  @brief Accessor method for the current_ property
   *  @retval uint16_t Number of computers currently in this computer group
   */
  uint16_t current() const;

  /**
   *  @brief Accessor method for the target_ property
   *  @retval uint16_t Number of computers expected to be in this computer group
   */
  uint16_t target() const;

  /**
   *  @brief Accessor method for the deployment percentage computed value
   *  @retval uint8_t Percentage of computers deployed in this computer group
   */
  uint8_t percent() const;

  /**
   *  @brief Mutator method for the name_ property
   *  @param name Name of this computer group
   */
  void set_name(std::string name);

  /**
   *  @brief Mutator method for the current_ property
   *  @param uint16_t Number of computers currently in this computer group
   */
  void set_current(uint16_t current);

  /**
   *  @brief Mutator method for the target_ property
   *  @param uint16_t Number of computers expected to be in this computer group
   */
  void set_target(uint16_t target);
};

/**
 *  @brief Overload the < binary infix comparison operator for ComputerGroup
 */
bool operator<(const ComputerGroup& lhs, const ComputerGroup& rhs);

/**
 *  @brief BigFix Statistics namespace for library-wide constants
 */
namespace bf {
  /** program name */
  const std::string kProgramName {"bfstats"};

  /** program major revision number */
  const uint8_t kMajorVersion {1};

  /** program minor revision number */
  const uint8_t kMinorVersion {0};

  /** text that indicates a line contains our records */
  const std::string kRecord {"<tr>"};

  /** text that indicates the start of a record */
  const std::string kStart {"<td>"};

  /** text that indicates the end of a record */
  const std::string kEnd {"</td>"};

  /** delimiter for deployment targets file */
  const std::string kDelim {","};
}  // namespace bf

/**
 *  @brief Display command-line program usage and options
 */
void usage();

/**
 *  @brief Load target information from file
 *  @param std::string filename input file containing deployment targets
 *  @param std::set<ComputerGroup>& groups vector of computer groups
 */
void loadTarget(std::string filename, std::set<ComputerGroup>* groups);

/**
 *  @brief Load current information from file
 *  @param std::string filename input file containing current status
 *  @param std::set<ComputerGroup> groups vector of computer groups
 */
void loadCurrent(std::string filename, std::set<ComputerGroup>* groups);

/**
 *  @brief Format number into comma-separated groupings
 */
std::string format(uint32_t number);

/**
 *  @brief Display output for pasting into Confluence
 */
void display(std::set<ComputerGroup> groups);

#endif  // BIGFIX_BIGFIXSTATS_H_
