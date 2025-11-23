/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2010 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ header file of kconsens_instance_bykemX
 *   Description: Class that realizes a fixed parameter algorithm for the Kemeny Score problem.
 *                The parameter is the kemeny score.
 *                The algorithm is a search tree algorithm that branches on dirty sets.
 *                Theoretical running time: O(1.51^parameter)
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

#ifndef KCONSENS_INSTANCE_KEMX_H
#define KCONSENS_INSTANCE_KEMX_H

// project internal
#include "kconsens_instance_dirtypairs_based.hpp"
#include "permutationmanagement.hpp"
#include "kemtyps.hpp"

/**
 Class that realizes a fixed parameter algorithm for the Kemeny Score problem.
 The parameter is the kemeny score.
 The algorithm is a search tree algorithm that branches on dirty sets.
 Theoretical running time: O(1.51^parameter)
 @author Robert Bredereck <RBredereck@web.de>
*/
class kconsens_instance_bykemX : public kconsens_instance_dirtypairs_based
{
public:
  // const & dest
  kconsens_instance_bykemX ( int dirtySetSize, stringstream &outdebug );
  ~kconsens_instance_bykemX();

  // solve this problem instance
  void solve ( stringstream &outverbose, stringstream &outtable );
protected:
  // solve for a specified kemeny score "k"
  bool solvek ( long &k, stringstream &outverbose, stringstream &outtable );

  int dirtySetSize;
  DirtySetsPermed Dsets;

  vector<int> branches;
  vector<int> branchesMax;
  vector<CandidateCandidateRelationsMap> Ll;
  vector<long> kl;
};

#endif
