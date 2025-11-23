/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2010 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ header file of kconsens_instance_advancedsearchtree
 *   Description: The search tree algorithms that use dirty triples and sets need additional data structures.
 *                They are defined here.
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

#ifndef KCONSENS_INSTANCE_ADVANCEDSEARCHTREE_H
#define KCONSENS_INSTANCE_ADVANCEDSEARCHTREE_H

// project internal
#include "kconsens_instance.hpp"
#include "kemtyps.hpp"

/**
 Essential data structures and functions for search tree algorithms.
 @author Robert Bredereck <RBredereck@web.de>
*/
class kconsens_instance_advancedsearchtree : public kconsens_instance
{
public:
    // const & dest
    kconsens_instance_advancedsearchtree( stringstream &outdebug );
    ~kconsens_instance_advancedsearchtree();

    /// solve this problem instance (virtual)
    virtual void solve ( stringstream &outverbose, stringstream &outtable ) = 0;

protected:
    // solve for a specified kemeny score "k"
    virtual bool solvek ( long &k, stringstream &outverbose, stringstream &outtable ) = 0;

    // information structure (see algorithm)
    // weight 0: lower>upper, weight 1: upper>lower
    CandidateCandidateRelationsMap L;
    // (still) dirty pairs (see algorithm)
    CandidatePairSet D;

    // gets all dirty X sets with size <= X
    DirtySetsPermed getDirtySets ( int maxSize );
    void setDirtySetSequence ( DirtySetsPermed &dsp );

    // operations for managing L
    void addLower ( Candidate lowercan, CandidateCandidateRelationsMap &L, Candidate can );
    void addLowers ( CandidateSet &lowercans, CandidateCandidateRelationsMap &L, Candidate can );
    void addUpper ( Candidate uppercan, CandidateCandidateRelationsMap &L, Candidate can );
    void addUppers ( CandidateSet &uppercans, CandidateCandidateRelationsMap &L, Candidate can );

    // add an information to L
    void addInfo ( CandidateVector &v, CandidateCandidateRelationsMap &L );
    void addInfo ( CandidatePair &p, CandidateCandidateRelationsMap &L );
    void addInfo ( Candidate lower, Candidate upper, CandidateCandidateRelationsMap &L );

    // check dirty attribute
    bool isInSet ( CandidatePairSet &D, CandidatePair p );

    // check confimity with L
    bool isConform ( CandidateVector &v, CandidateCandidateRelationsMap &L );
    bool isConform ( CandidatePair &p, CandidateCandidateRelationsMap &L );

    // create consensus from full information object L
    void relationsToConsens ( CandidateList &clist, CandidateCandidateRelationsMap &L );

    // init L and D
    void initLD();

    // rule 1: assoziates a non-ambiguous candidate with the set of more-prefered candidates
    CandidateCandidateSetMap nonDirtyInjection;
    // rule 2: analyze multiple votes
    VoteMultiset uniqueVotes;

    // find candidates that aren't members of any pair save their relative order and remove them from candidatesvector
    void rule1_prepare();
    // count the (unique) votes
    void rule2_prepare();
    // reinsert the removes (non-dirty) candidate in their relative order
    void rule1_reinsert();
    // if there is a vote max-times, check if it is a solution
    // returns kemeny score (k) if consensusFount==true and saves consensus in this->consens, else it returns a lower bound for k
    unsigned long rule2_check ( bool &consensusFound, unsigned long &maxCountReturn );
};

#endif
