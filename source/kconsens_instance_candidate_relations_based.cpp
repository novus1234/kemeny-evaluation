/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2010 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ implementation file of kconsens_instance_candidate_relations_based
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
#include <iostream>
#include <fstream>
#include <string>

// project internal
#include "tools.hpp"
#include "permutationmanagement.hpp"
#include "kconsens_instance_candidate_relations_based.hpp"

using namespace std;
using namespace boost;

kconsens_instance_candidate_relations_based::kconsens_instance_candidate_relations_based ( stringstream &outdebug ) : kconsens_instance ( outdebug )
{
  this->isSynchronized = false;
  this->isDirtysAnalyzed = false;
  this->isSubScoreAnalyzed = false;
}

kconsens_instance_candidate_relations_based::~kconsens_instance_candidate_relations_based()
{
}

void kconsens_instance_candidate_relations_based::relationsToConsens ( CandidateList &clist, CandidateCandidateRelationsMap &L )
{
  unsigned long canCount = this->candidateSet.size();
  unsigned long candsArray[canCount];
  clist.clear();
  for ( CandidateCandidateRelationsMapIterator li = L.begin(); li != L.end(); li++ )
  {
    if ( ( *li ).second.lowers.size() + ( *li ).second.uppers.size() + 1 != canCount )
    {
      cerr << "L was not completed " << ( *li ).first << ( *li ).second.lowers.size() << ( *li ).second.uppers.size() << canCount << "\n";
    }
    else
    {
      candsArray[ ( *li ).second.lowers.size() ] = ( *li ).first;
    }
  }
  for ( unsigned long i = 0; i < canCount; i++ )
  {
    //printf("Pushing %d\n", candsArray[i]);
    clist.push_back ( candsArray[i] );
  }
  clist.reverse();
}

void kconsens_instance_candidate_relations_based::addLower ( Candidate lowercan, CandidateCandidateRelationsMap &L, Candidate can )
{
  CandidateCandidateRelationsMapIterator candri = L.find ( can );
  if ( candri != L.end() )
  {
    ( *candri ).second.lowers.insert ( lowercan );
  }
  else
  {
    CandidateRelations candrNew;
    candrNew.lowers.insert ( lowercan );
    L.insert ( CandidateCandidateRelationsMapData ( can, candrNew ) );
  }

}

void kconsens_instance_candidate_relations_based::addLowers ( CandidateSet &lowercans, CandidateCandidateRelationsMap &L, Candidate can )
{
  CandidateCandidateRelationsMapIterator candri = L.find ( can );
  if ( candri != L.end() )
  {
    for ( CandidateSetIterator lowercan = lowercans.begin(); lowercan != lowercans.end(); lowercan++ )
      ( *candri ).second.lowers.insert ( *lowercan );
  }
  else
  {
    CandidateRelations candrNew;
    for ( CandidateSetIterator lowercan = lowercans.begin(); lowercan != lowercans.end(); lowercan++ )
      candrNew.lowers.insert ( *lowercan );
    L.insert ( CandidateCandidateRelationsMapData ( can, candrNew ) );
  }

}

void kconsens_instance_candidate_relations_based::addUpper ( Candidate uppercan, CandidateCandidateRelationsMap &L, Candidate can )
{
  CandidateCandidateRelationsMapIterator candri = L.find ( can );
  if ( candri != L.end() )
  {
    ( *candri ).second.uppers.insert ( uppercan );
  }
  else
  {
    CandidateRelations candrNew;
    candrNew.uppers.insert ( uppercan );
    L.insert ( CandidateCandidateRelationsMapData ( can, candrNew ) );
  }

}

void kconsens_instance_candidate_relations_based::addUppers ( CandidateSet &uppercans, CandidateCandidateRelationsMap &L, Candidate can )
{
  CandidateCandidateRelationsMapIterator candri = L.find ( can );
  if ( candri != L.end() )
  {
    for ( CandidateSetIterator uppercan = uppercans.begin(); uppercan != uppercans.end(); uppercan++ )
      ( *candri ).second.uppers.insert ( *uppercan );
  }
  else
  {
    CandidateRelations candrNew;
    for ( CandidateSetIterator uppercan = uppercans.begin(); uppercan != uppercans.end(); uppercan++ )
      candrNew.uppers.insert ( *uppercan );
    L.insert ( CandidateCandidateRelationsMapData ( can, candrNew ) );
  }

}

void kconsens_instance_candidate_relations_based::addInfo ( CandidateVector &v, CandidateCandidateRelationsMap &L )
{
  for ( CandidateVectorIterator x = v.begin(); x != v.end(); x++ )
  {
    CandidateVectorIterator temp = x;
    for ( CandidateVectorIterator y = ++temp; y != v.end(); y++ )
    {
      CandidatePair p;
      p.lower = *x;
      p.upper = *y;
      this->addInfo ( p, L );
    }
  }
}

void kconsens_instance_candidate_relations_based::addInfo ( CandidatePair &p, CandidateCandidateRelationsMap &L )
{
  this->addInfo ( p.lower, p.upper, L );
}

void kconsens_instance_candidate_relations_based::addInfo ( Candidate lower, Candidate upper, CandidateCandidateRelationsMap &L )
{
  //printf("Added Information: %d>%d\n",lower,upper);
  CandidateCandidateRelationsMapIterator ls = L.find ( lower );
  if ( ls != L.end() )
  {
    this->addLowers ( ( *ls ).second.lowers, L, upper );
  }
  this->addLower ( lower, L, upper );
  CandidateCandidateRelationsMapIterator us = L.find ( upper );
  if ( us != L.end() )
  {
    this->addUppers ( ( *us ).second.uppers, L, lower );
  }
  this->addUpper ( upper, L, lower );
  //printL(L);
}

bool kconsens_instance_candidate_relations_based::isConform ( CandidatePair &p, CandidateCandidateRelationsMap &L )
{
  bool tmpConf = true;
  CandidateCandidateRelationsMapIterator ls = L.find ( p.lower );
  CandidateCandidateRelationsMapIterator us = L.find ( p.upper );
  if ( ( ls != L.end() ) && ( us != L.end() ) )
  {
    if ( ( *ls ).second.lowers.find ( p.upper ) != ( *ls ).second.lowers.end() )
    {
      tmpConf = false;
    }
    if ( ( *us ).second.uppers.find ( p.lower ) != ( *us ).second.uppers.end() )
    {
      tmpConf = false;
    }
  }
  else
  {
    //cerr << "L is defect\n";
  }
  //if(!tmpConf)cout << p.lower << ">" << p.upper << " is not conform\n";
  return tmpConf;
}

bool kconsens_instance_candidate_relations_based::isConform ( CandidateVector &v, CandidateCandidateRelationsMap &L )
{
  CandidateCandidateRelationsMapIterator ci[ v.size() ];
  for ( int i = 0; i < v.size() ; i++ )
  {
    ci[i] = L.find ( v.at ( i ) );
  }
  for ( int i = 0; i < v.size() - 1 ; i++ )
  {
    if ( ci[i] != L.end() )
    {
      for ( int j = i + 1; j < v.size(); j++ )
      {
        if ( ci[j] != L.end() )
        {
          if ( ( *ci[i] ).second.lowers.find ( v.at ( j ) ) != ( ( *ci[i] ).second.lowers.end() ) )
          {
            return false;
          }
          if ( ( *ci[j] ).second.uppers.find ( v.at ( i ) ) != ( ( *ci[j] ).second.uppers.end() ) )
          {
            return false;
          }
        }
      }
    }
  }
  return true;
}
