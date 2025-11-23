/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2010 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ header file of permutationmanagement
 *   Description: This class manages a permutation list.
 *
 *   Kconsens is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Kconsens is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Kconsens.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PERMUTATIONMANAGEMENT_H
#define PERMUTATIONMANAGEMENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <sstream>

// project internal
#include "votetyps.hpp"

/**
 This class manages a permutation list.
 @author Robert Bredereck <RBredereck@web.de>
*/
class permutationmanagement
{
public:
  // const & dest
  permutationmanagement ( CandidateVector &element_numbers );
  ~permutationmanagement();

  // set to first permutation
  void setFirst();
  // set to next permutation, true if it was not the last one
  bool setNext();
  // true, if current permutation ist last one
  bool isLast();
  // true if setNext was called on last permutation
  bool lastAgain;
  // get current permutation as candidate vector
  CandidateVector getAsVector();
  void updateVector ( CandidateVector &permAsVector );
private:
  // size of permutation
  int m;
  // elements numbers
  vector<Candidate> elements;
  // relative positions of the elements
  vector<int> relpos;
  // absolute positions of the elements
  vector<int> positions;

  bool markLast;
  bool notlastpos();
  void nextpos();
};

#endif
