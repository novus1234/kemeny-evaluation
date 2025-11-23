/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2010 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ header file of kconsens_instance_bycan
 *   Description: Class that realizes a fixed parameter algorithm for the Kemeny Score problem.
 *                The parameter is the number of candidates.
 *                The algorithm uses a dynamic programming.
 *                Theoretical running time: O(2^parameter)
 *                This class is implemented for a huge number of candidates.
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
#ifndef KCONSENS_INSTANCE_CAN_H
#define KCONSENS_INSTANCE_CAN_H

// project internal
#include "kconsens_instance.hpp"

/**
 Class that realizes a fixed parameter algorithm for the Kemeny Score problem.
 The parameter is the number of candidates.
 The algorithm uses a dynamic programming.
 Theoretical running time: O(2^parameter)
 This class is implemented for a huge number of candidates.
 @author Robert Bredereck <RBredereck@web.de>
*/
class kconsens_instance_bycan : public kconsens_instance
{
public:
  // const & dest
  kconsens_instance_bycan ( stringstream &outdebug );
  ~kconsens_instance_bycan();

  void solve ( stringstream &outverbose, stringstream &outtable );
protected:
  // my dynamic programming table
  CandidateSetCandidateVectorWeightedMap dynt;
  CandidateSetStVector lastSubsetLayer;

  unsigned long subScoreByPushFront ( unsigned long cand, const CandidateVector &cands );
};


#endif
