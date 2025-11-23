/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2010 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ header file for data structures.
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

#ifndef KEMTYPS_H
#define KEMTYPS_H

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <list>
#include <boost/bimap.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

// project internal
#include "votetyps.hpp"
#include "EfficientSet.hpp"

using namespace boost::bimaps;
using namespace boost;
using namespace std;

struct CandidateVector2Weighted
{
  CandidateVector candidates;
  unsigned long weight1;
  unsigned long weight2;
};
struct CandidateSetSt
{
  CandidateSet candidates;
};
std::size_t hash_value ( const CandidateSetSt& s );
bool operator== ( const CandidateSetSt& a, const CandidateSetSt& b );
typedef unordered_map<CandidateSetSt, CandidateVector2Weighted, boost::hash<CandidateSetSt> > CandidateSetCandidateVectorWeightedMap;
typedef CandidateSetCandidateVectorWeightedMap::value_type CandidateSetCandidateVectorWeightedMapData;
typedef CandidateSetCandidateVectorWeightedMap::const_iterator CandidateSetCandidateVectorWeightedMapIterator;

typedef vector<CandidateSetSt> CandidateSetStVector;
typedef CandidateSetStVector::iterator CandidateSetStVectorIterator;

//special efficient set structures
typedef EfficientSet<uint32_t> CandidateESet;
typedef vector<CandidateESet> CandidateESetStVector;
typedef CandidateESetStVector::iterator CandidateESetStVectorIterator;
typedef unordered_map<CandidateESet, CandidateVector2Weighted, boost::hash<CandidateESet> > CandidateESetCandidateVectorWeightedMap;
typedef CandidateESetCandidateVectorWeightedMap::value_type CandidateESetCandidateVectorWeightedMapData;
typedef CandidateESetCandidateVectorWeightedMap::const_iterator CandidateESetCandidateVectorWeightedMapIterator;


typedef unordered_multiset<CandidateList>CandidateListMultiset;
typedef CandidateListMultiset::iterator CandidateListMultisetIterator;

std::size_t hash_value ( const Vote& v );
bool operator== ( const Vote& a, const Vote& b );
typedef unordered_multiset<Vote>VoteMultiset;
typedef VoteMultiset::iterator VoteMultisetIterator;

// transformations
void appendCandidateVector2CandidateList ( const CandidateVector &v, CandidateList &l );

void printL ( CandidateCandidateRelationsMap &L );

void printSet ( CandidateSetS &s );

// kconsens_bykemX
struct PermutationScored
{
  CandidateVector permutation;
  unsigned long subScore;
  bool conform;
};

struct DirtySetPermed
{
  CandidateSet candidates;
  long scoreFromSuccessorsMin;
  vector<PermutationScored> permutations;
};

typedef vector<DirtySetPermed> DirtySetsPermed;
typedef DirtySetsPermed::iterator DirtySetsPermedIterator;

#endif
