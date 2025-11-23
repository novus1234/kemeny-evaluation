/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2010 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ implementation file of kconsens_instance_dirtypairs_based
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
#include <iostream>
#include <fstream>
#include <string>

// project internal
#include "tools.hpp"
#include "permutationmanagement.hpp"
#include "kconsens_instance_dirtypairs_based.hpp"

using namespace std;
using namespace boost;

kconsens_instance_dirtypairs_based::kconsens_instance_dirtypairs_based ( stringstream &outdebug ) : kconsens_instance_candidate_relations_based ( outdebug )
{
  this->isSynchronized = false;
  this->isDirtysAnalyzed = false;
  this->isSubScoreAnalyzed = false;
}

kconsens_instance_dirtypairs_based::~kconsens_instance_dirtypairs_based()
{
}

void kconsens_instance_dirtypairs_based::rule1_prepare()
{
  this->nonDirtyCandidateVector.clear();
  CandidateVector tmp = this->candidateVector;
  for ( CandidateVectorIterator x = this->candidateVector.begin(); x != this->candidateVector.end(); x++ )
  {
    CandidateSetIterator isDirty = this->dirtyCandidateSet.find ( *x );
    if ( isDirty == this->dirtyCandidateSet.end() )
    {
      //candidate not dirty
      this->nonDirtyCandidateVector.push_back ( *x );
      //scan all precandidates
      CandidateSet precands;
      for ( CandidatePositionMapIterator pr = this->votes.at ( 1 ).asMap.begin(); ( pr != this->votes.at ( 1 ).asMap.end() ) && ( pr->first != *x ); pr++ )
      {
        precands.insert ( pr->first );
      }
      this->nonDirtyInjection.insert ( CandidateCandidateSetMapData ( *x, precands ) );
      this->candidateSet.erase ( *x );
    }
  }
  this->candidateVector.clear();
  for ( CandidateVectorIterator x = tmp.begin(); x != tmp.end(); x++ )
  {
    if ( this->candidateSet.find ( *x ) != this->candidateSet.end() )
      this->candidateVector.push_back ( *x );
  }
}

void kconsens_instance_dirtypairs_based::rule1_reinsert()
{
  for ( CandidateVectorIterator x = this->nonDirtyCandidateVector.begin(); x != this->nonDirtyCandidateVector.end(); x++ )
  {
    CandidateCandidateSetMapIterator xf = this->nonDirtyInjection.find ( *x );
    if ( xf != this->nonDirtyInjection.end() )
    {
      CandidateSet prcands = ( *xf ).second;
      for ( CandidateListIterator cx = this->consens.begin(); cx != this->consens.end(); cx++ )
      {
        if ( prcands.find ( *cx ) != prcands.end() )
        {
          prcands.erase ( *cx );
        }
        if ( prcands.size() == 0 )
        {
          this->consens.insert ( cx, ( *xf ).first );
          break;
        }
      }
    }
    else
    {
      cerr << "rule1_prepare was inconsistent!\n";
    }
  }
}

void kconsens_instance_dirtypairs_based::rule2_prepare()
{
  this->uniqueVotes.clear();
  for ( VoteVectorIterator v = this->votes.begin(); v != this->votes.end(); v++ )
  {
    this->uniqueVotes.insert ( *v );
  }
}

unsigned long kconsens_instance_dirtypairs_based::rule2_check ( bool &consensusFound, unsigned long &maxCountReturn )
{
  consensusFound = false;
  unsigned long maxCount = 0;
  unsigned long currCount = 0;
  unsigned long kscore = 0;
  CandidateList consensusSuggestion;
  for ( VoteMultisetIterator u = this->uniqueVotes.begin(); u != this->uniqueVotes.end(); u++ )
  {
    currCount = this->uniqueVotes.count ( *u );
    if ( currCount > maxCount )
    {
      maxCount = currCount;
      consensusSuggestion = ( *u ).asList;
    }
  }
  if ( maxCount > 0 )
  {
    kscore = this->kscore ( consensusSuggestion );
    if ( kscore <= maxCount )
    {
      this->consens = consensusSuggestion;
      maxCountReturn = maxCount;
      consensusFound = true;
      //printf("Rule2: maximal# of an unique vote: %d, kscore for this vote: %d\n",maxCount,kscore);
      return kscore;
    }
  }
  maxCountReturn = maxCount;
  return kscore;
}

void kconsens_instance_dirtypairs_based::initLD()
{
  if ( !this->isDirtysAnalyzed )
    this->analyzeDirtys();
  if ( !this->isSubScoreAnalyzed )
    this->analyzeSubScore();
  this->D.clear();
  /*for ( CandidatePairWeightMapIterator dpw = this->dirtyPairMap.begin(); dpw != this->dirtyPairMap.end(); dpw++ )
  {
   this->D.insert ( ( *dpw ).first );
  }*/
  this->L.clear();
  for ( CandidateVectorIterator a = this->candidateVector.begin(); a != this->candidateVector.end(); a++ )
  {
    CandidateVectorIterator binit = a;
    binit++;
    for ( CandidateVectorIterator b = binit; b != this->candidateVector.end(); b++ )
    {
      CandidatePair p = ascSortedCandidatePair ( *a, *b );
      if ( ! ( this->isDirty ( p ) ) )
      {
        if ( this->getPosition ( 0, p.lower ) < this->getPosition ( 0, p.upper ) )
        {
          this->addInfo ( p.lower, p.upper, this->L );
        }
        else
        {
          this->addInfo ( p.upper, p.lower, this->L );
        }

      }
      else
      {
        this->D.insert ( p );
      }
    }
  }
}

bool kconsens_instance_dirtypairs_based::isInSet ( CandidatePairSet &D, CandidatePair p )
{
  return ( D.end() != D.find ( p ) );
}

bool sortBySize ( DirtySetPermed a, DirtySetPermed b )
{
  return ( a.candidates.size() > b.candidates.size() );
}

bool sortBySubScore ( PermutationScored a, PermutationScored b )
{
  return ( a.subScore < b.subScore );
}

DirtySetsPermed kconsens_instance_dirtypairs_based::getDirtySets ( int maxSize )
{
  DirtySetsPermed dirtySets;
  CandidatePairSet tD = this->D;
  while ( tD.size() > 0 )
  {
    DirtySetPermed newDirtySetPermed;
    CandidateSet newDirtySet;
    CandidateVector newDirtyVector;
    for ( CandidatePairSetIterator xy = tD.begin(); xy != tD.end(); xy++ )
    {
      //printf("Performing dirty pair: (%d,%d)",(*xy).lower,(*xy).upper);
      // Case 1: no candidate yet
      bool lowerInD = ( newDirtySet.find ( ( *xy ).lower ) != newDirtySet.end() );
      bool upperInD = ( newDirtySet.find ( ( *xy ).upper ) != newDirtySet.end() );
      if ( newDirtySet.size() == 0 )
      {
        newDirtySet.insert ( ( *xy ).lower );
        newDirtyVector.push_back ( ( *xy ).lower );
        newDirtySet.insert ( ( *xy ).upper );
        newDirtyVector.push_back ( ( *xy ).upper );
        // Case 2: at least two candidate in set
        if ( newDirtySet.size() == maxSize )
          break;
      }
      else
        if ( lowerInD && ! ( upperInD ) )
        {
          newDirtySet.insert ( ( *xy ).upper );
          newDirtyVector.push_back ( ( *xy ).upper );
          if ( newDirtySet.size() == maxSize )
            break;
        }
        else
          if ( ! ( lowerInD ) && upperInD )
          {
            newDirtySet.insert ( ( *xy ).lower );
            newDirtyVector.push_back ( ( *xy ).lower );
            if ( newDirtySet.size() == maxSize )
              break;
          }
    }
    //printf("New dirty Set found:");
    //printVote(newDirtyVector);

    // get all permutations of the current set
    CandidateVector perm;
    for ( permutationmanagement pm = permutationmanagement ( newDirtyVector );!pm.lastAgain;pm.setNext() )
    {
      PermutationScored permScored;
      pm.updateVector ( perm );
      //printVote(perm);
      permScored.permutation = perm;
      unsigned long pscore = 0;
      for ( CandidateVectorIterator x = perm.begin(); x != perm.end(); x++ )
      {
        CandidateVectorIterator temp = x;
        for ( CandidateVectorIterator y = ++temp; y != perm.end(); y++ )
        {
          CandidatePair p;
          p.lower = *x;
          p.upper = *y;
          // add score, if is still in iD
          if ( tD.find ( ascSortedCandidatePair ( *x, *y ) ) != tD.end() )
            pscore += this->subscore ( *y, *x );
        }
      }
      permScored.subScore = pscore;
      //printf("Score: %d Permutation:",pscore);
      //printVote(perm);
      permScored.conform = true;
      newDirtySetPermed.permutations.push_back ( permScored );
    }
    // sort (asc) the permutations vector by subScore
    sort ( newDirtySetPermed.permutations.begin(), newDirtySetPermed.permutations.end(), sortBySubScore );

    // remove all pairs of current dirty set
    for ( CandidateVectorIterator x = newDirtyVector.begin(); x != newDirtyVector.end(); x++ )
    {
      CandidateVectorIterator temp = x;
      for ( CandidateVectorIterator y = ++temp; y != newDirtyVector.end(); y++ )
      {
        CandidatePair p = ascSortedCandidatePair ( *x, *y );
        //printf("Removing (%d,%d)\n..", p.lower, p.upper);
        tD.erase ( p );
      }
    }

    newDirtySetPermed.candidates = newDirtySet;
    dirtySets.push_back ( newDirtySetPermed );
  }
  // sort (dec) the vector by CandidateSet.size()
  sort ( dirtySets.begin(), dirtySets.end(), sortBySize );

  for ( DirtySetsPermedIterator di = dirtySets.begin(); di != dirtySets.end(); di++ )
  {
    ( *di ).scoreFromSuccessorsMin = 0;
    DirtySetsPermedIterator dinext = di;
    for ( DirtySetsPermedIterator dj = ++dinext; dj != dirtySets.end(); dj++ )
    {
      ( *di ).scoreFromSuccessorsMin += ( *dj ).permutations.at ( 0 ).subScore;
    }
  }

  return dirtySets;
}

void kconsens_instance_dirtypairs_based::setDirtySetSequence ( DirtySetsPermed &dsp )
{
  ostringstream conv;
  conv << "(";
  for ( DirtySetsPermedIterator i = dsp.begin(); i != dsp.end(); i++ )
  {
    if ( i != dsp.begin() )
      conv << ",";
    conv << ( *i ).candidates.size();
  }
  conv << ")";
  this->dirtySetSequence = conv.str();
}
