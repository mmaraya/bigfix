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

#ifndef BIGFIXSTATS_H_
#define BIGFIXSTATS_H_

/**
 *  @brief Model class for BigFix deployment information, following the
 *         model-view-controller software design pattern
 *  @details BigFixStats contains all the operations and attributes associated
 *           with the deployment information for all computer groups
 */
class BigFixStats {
};

/**
 *  @brief BigFix Statistics namespace for library-wide constants
 */
namespace bf {
  
  /** program name */
  const std::string kProgramName {"bfstats"};
  
  /** program major revision number */
  const int kMajorVersion {1};
  
  /** program minor revision number */
  const int kMinorVersion {0};
  
}

/**
 *  @brief Display command-line program usage and options
 */
void usage();

#endif // BIGFIXSTATS_H_
