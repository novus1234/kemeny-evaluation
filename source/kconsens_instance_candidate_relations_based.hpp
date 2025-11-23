/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2010 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ header file of kconsens_instance_candidate_relations_based
 *   Description: Definition of the class of algorithms that use additional data structures
 *                that store and process information about (pairwise) candidate relations.
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

#ifndef KCONSENS_INSTANCE_CANDIDATE_RELATIONS_BASED_H
#define KCONSENS_INSTANCE_CANDIDATE_RELATIONS_BASED_H

// project internal
#include "kconsens_instance.hpp"
#include "kemtyps.hpp"

/**
 Essential data structures and functions for algorithms processing (pairwise) candidate relations.
 @author Robert Bredereck <RBredereck@web.de>
*/
class kconsens_instance_candidate_relations_based : public kconsens_instance
{
public:
  // const & dest
  kconsens_instance_candidate_relations_based ( stringstream &outdebug );
  ~kconsens_instance_candidate_relations_based();

  /// solve this problem instance (virtual)
  virtual void solve ( stringstream &outverbose, stringstream &outtable ) = 0;

protected:
  /** This data structure store information about (pairwise) candidate relations.
      weight 0: lower>upper, weight 1: upper>lower **/
  CandidateCandidateRelationsMap L;

  /// create consensus from full information object L
  void relationsToConsens ( CandidateList &clist, CandidateCandidateRelationsMap &L );
  
  // operations for managing L
  void addLower ( Candidate lowercan, CandidateCandidateRelationsMap &L, Candidate can );
  void addLowers ( CandidateSet &lowercans, CandidateCandidateRelationsMap &L, Candidate can );
  void addUpper ( Candidate uppercan, CandidateCandidateRelationsMap &L, Candidate can );
  void addUppers ( CandidateSet &uppercans, CandidateCandidateRelationsMap &L, Candidate can );

  // add an information to L
  void addInfo ( CandidateVector &v, CandidateCandidateRelationsMap &L );
  void addInfo ( CandidatePair &p, CandidateCandidateRelationsMap &L );
  void addInfo ( Candidate lower, Candidate upper, CandidateCandidateRelationsMap &L );

  // check confimity with L
  bool isConform ( CandidateVector &v, CandidateCandidateRelationsMap &L );
  bool isConform ( CandidatePair &p, CandidateCandidateRelationsMap &L );
};

#endif
