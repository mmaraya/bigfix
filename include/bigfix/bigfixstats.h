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

#include <string>
#include <vector>

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
  uint32_t current_ {0};

  /**
   *  @brief Number of computers expected to be in this computer group
   */
  uint32_t target_ {0};

  /**
   *  @brief Format a number into comma-separated groupings
   *  @param number 32-bit unsigned integer to format
   *  @retval std::string comma-separated thousands
   */
  std::string format(uint32_t number) const;

  /**
   *  @brief Return the length of widest display element for this record
   *  @retval uint8_t widest display element for this record
   */
  uint8_t widest() const;

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
   *  @brief Return formatted version of the computer group name
   *  @retval output display formatted version of the computer group name
   */
  std::string formatted_name() const;

  /**
   *  @brief Accessor method for the current_ property
   *  @retval Number of computers currently in this computer group
   */
  uint32_t current() const;

  /**
   *  @brief Return formatted version of the current_ property
   *  @retval output display formatted version of the number of computers 
   *          currently in this computer group
   */
  std::string formatted_current() const;

  /**
   *  @brief Accessor method for the target_ property
   *  @retval Number of computers expected to be in this computer group
   */
  uint32_t target() const;

  /**
   *  @brief Return formatted version of the target_ property
   *  @retval output display formatted version of the number of computers
   *          expected to be in this computer group
   */
  std::string formatted_target() const;

  /**
   *  @brief Accessor method for the deployment percentage computed value
   *  @retval uint8_t Percentage of computers deployed in this computer group
   */
  uint8_t percent() const;

  /**
   *  @brief Return formatted version of the deployment percentage computed
   *         value
   *  @retval output display formatted version of the Percentage of computers 
   *          deployed in this computer group
   */
  std::string formatted_percent() const;

  /**
   *  @brief Mutator method for the name_ property
   *  @param name Name of this computer group
   */
  void set_name(std::string name);

  /**
   *  @brief Mutator method for the current_ property
   *  @param current Number of computers currently in this computer group
   */
  void set_current(uint32_t current);

  /**
   *  @brief Mutator method for the target_ property
   *  @param target Number of computers expected to be in this computer group
   */
  void set_target(uint32_t target);
};

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
 *  @param filename input file containing deployment targets
 *  @param groups collection of computer groups
 */
void loadTarget(std::string filename, std::vector<ComputerGroup>* groups);

/**
 *  @brief Load current information from file
 *  @param filename input file containing current status
 *  @param raw collection of computer groups with raw deployment counts
 *  @param final collection of computer groups with finalized counts
 */
void loadCurrent(std::string filename, std::map<std::string, uint32_t>* raw,
                 std::vector<ComputerGroup>* final);

/**
 *  @brief Display output for pasting into Confluence
 *  @param date date of raw deployment counts
 *  @param raw collection of raw computer group deployment counts
 *  @param final collection of computer groups with finalized counts
 */
void display(std::string date, std::map<std::string, uint32_t>* raw,
             std::vector<ComputerGroup>* final);

#endif  // BIGFIX_BIGFIXSTATS_H_
