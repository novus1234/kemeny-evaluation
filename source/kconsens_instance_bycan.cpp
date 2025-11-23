/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2010 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ implementation file of kconsens_instance_bycan
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

#include <iostream>
#include <fstream>
#include <string>
#include <limits.h>

// project internal
#include "tools.hpp"
#include "kemtyps.hpp"
#include "kconsens_instance_bycan.hpp"

kconsens_instance_bycan::kconsens_instance_bycan ( stringstream &outdebug ) : kconsens_instance ( outdebug )
{
  this->isSynchronized = false;
}

kconsens_instance_bycan::~kconsens_instance_bycan()
{
}

unsigned long kconsens_instance_bycan::subScoreByPushFront ( unsigned long cand, const CandidateVector &cands )
{
  unsigned long tmpResult = 0;
  for ( unsigned long i = 0; i < cands.size(); i++ )
  {
    CandidatePair p;
    p.lower = cand;
    p.upper = cands.at ( i );
    CandidatePairWeightMapIterator pss = this->subScore.find ( p );
    if ( pss != this->subScore.end() )
    {
      tmpResult += ( * ( pss ) ).second;
    }
  }
  return tmpResult;
}

void kconsens_instance_bycan::solve ( stringstream &outverbose, stringstream &outtable )
{
  if ( ! ( this->isSynchronized ) )
    this->synchronizeDataTypes();
  if ( ! ( this->isSubScoreAnalyzed ) )
    this->analyzeSubScore();

  if ( candidateSet.size() < 2 )
  {
    this->consens.clear();
    if ( candidateSet.size() > 0 )
    {
      this->consens.push_back ( this->candidateVector.at ( 0 ) );
    }
    this->consensScore = 0;
    return;
  }
  // init table
  for ( unsigned long i = 0; i < this->candidateVector.size(); i++ )
  {
    CandidateSetSt tmpCs;
    tmpCs.candidates.insert ( this->candidateVector.at ( i ) );

    CandidateVector2Weighted tmpCvw;
    tmpCvw.candidates.push_back ( this->candidateVector.at ( i ) );
    // subscore of this permutation
    tmpCvw.weight1 = 0;
    // first position to search for next cand. in subset
    tmpCvw.weight2 = i + 1;

    this->dynt.insert ( CandidateSetCandidateVectorWeightedMapData ( tmpCs, tmpCvw ) );
    this->lastSubsetLayer.push_back ( tmpCs );
  }
  // fill the table

  for ( unsigned long candcountready = 1; candcountready < this->candidateSet.size(); candcountready++ )
  {
    outverbose << "Computed layer " << candcountready << " of " << this->candidateSet.size() - 1 << ".\n";
    CandidateSetCandidateVectorWeightedMap dyntNew;
    CandidateSetStVector newLayer;
    for ( CandidateSetStVectorIterator subSetI = this->lastSubsetLayer.begin(); subSetI != this->lastSubsetLayer.end(); subSetI++ )
    {
      unsigned long searchpos = 0;
      CandidateSetCandidateVectorWeightedMapIterator sp = this->dynt.find ( *subSetI );
      // comment this dirty trick! (weight2)
      // weight2 contains the correct start position for the iterator to build up unordered pairs correctly
      if ( sp != this->dynt.end() )
        searchpos = ( *sp ).second.weight2;
      for ( unsigned long i = searchpos; i < this->candidateVector.size(); i++ )
      {
        unsigned long w2 = i + 1;
        // build new Subset (bottom-up)
        CandidateSetSt tmpCs; // C'
        tmpCs.candidates = ( *subSetI ).candidates;
        tmpCs.candidates.insert ( this->candidateVector.at ( i ) );

        // rekursive consensus for this subset
        // delete one candidate a get a possible subset
        // who is the best
        CandidateVector2Weighted best;
        best.weight1 = ULONG_MAX;
        if ( boost::this_thread::interruption_requested() )
        {
          this->consensScore = -1;
          return;
        }
        for ( CandidateSetIterator d = tmpCs.candidates.begin(); d != tmpCs.candidates.end(); d++ )
        {
          //CandidateSet currSubSet = tmpSet; //C''
          //currSubSet.erase ( *d );
          CandidateSetSt currSubSetS;
          //currSubSetS.candidates = currSubSet;
          currSubSetS.candidates = tmpCs.candidates; // C'' faster?
          currSubSetS.candidates.erase ( *d );
          CandidateSetCandidateVectorWeightedMapIterator f = this->dynt.find ( currSubSetS );
          if ( f != this->dynt.end() )
          {
            unsigned long subScoreByPush = this->subScoreByPushFront ( *d, ( *f ).second.candidates );
            unsigned long w1 = ( *f ).second.weight1 + subScoreByPush;
            if ( w1 < best.weight1 )
            {
              best.candidates = ( *f ).second.candidates;
              // put d in front of consensus
              best.candidates.insert ( best.candidates.begin(), *d );
              best.weight1 = w1;
              best.weight2 = w2;
            }
          }
          else
          {
            cerr << "error entry in dynt was not define yet\n";
            cout << "ERROR: could not find: ";
            printSet ( currSubSetS.candidates );
          }
        }
        dyntNew.insert ( CandidateSetCandidateVectorWeightedMapData ( tmpCs, best ) );
        //cout << "Inserting:";
        //printSet(tmpCs.candidates);
        newLayer.push_back ( tmpCs );
        //tmpSet.erase(this->candidateVector.at ( i ));
      }
    }
    this->lastSubsetLayer.clear();
    this->lastSubsetLayer = newLayer;
    this->dynt.clear();
    this->dynt = dyntNew;
  }
  CandidateSetSt allCandidates;
  allCandidates.candidates = this->candidateSet;
  CandidateSetCandidateVectorWeightedMapIterator f = this->dynt.find ( allCandidates );
  unsigned long k = ( *f ).second.weight1;
  this->consens.clear();
  appendCandidateVector2CandidateList ( ( *f ).second.candidates, this->consens );
  this->consens.reverse();

  this->consensScore = k;
}
