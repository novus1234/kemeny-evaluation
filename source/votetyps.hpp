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

#ifndef VOTETYPS_H
#define VOTETYPS_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <list>
#include <boost/bimap.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

using namespace boost::bimaps;
using namespace boost;
using namespace std;

/////////////////////
//   BASE TYPES   ///
/////////////////////
typedef unsigned long Candidate;
typedef unsigned long Position;
typedef unsigned long Weight;

typedef unordered_map<string, string> stringmap;
typedef stringmap::value_type stringpair;

typedef unordered_set<string> stringset;

/////////////////
//   WEIGHT   ///
/////////////////

typedef unordered_map<Candidate, Weight> CandidateWeightMap;
typedef CandidateWeightMap::value_type CandidateWeightMapData;
typedef CandidateWeightMap::const_iterator CandidateWeightMapIterator;

////////////////////////////
//   PAIRS and TRIPLES   ///
////////////////////////////
struct CandidatePair
{
  Candidate lower;
  Candidate upper;
};
std::size_t hash_value ( CandidatePair const& p );
bool operator== ( CandidatePair const& a, CandidatePair const& b );

CandidatePair flipCandidatePair ( CandidatePair &p );
CandidatePair ascSortedCandidatePair ( CandidatePair &p );
CandidatePair ascSortedCandidatePair ( Candidate x, Candidate y );
CandidatePair dscSortedCandidatePair ( CandidatePair &p );
CandidatePair dscSortedCandidatePair ( Candidate x, Candidate y );

struct CandidateTriple
{
  Candidate lower;
  Candidate middle;
  Candidate upper;
};
std::size_t hash_value ( CandidateTriple const& t );
bool operator== ( CandidateTriple const& a, CandidateTriple const& b );

CandidateTriple ascSortedCandidateTriple ( Candidate a, Candidate b, Candidate c );
CandidateTriple dscSortedCandidateTriple ( Candidate a, Candidate b, Candidate c );

//////////////////////////////
//   Vote datastructures   ///
//////////////////////////////
// for complete preference lists (f.e. consensus)
typedef list<Candidate> CandidateList;
typedef CandidateList::iterator CandidateListIterator;

// for partial votes
typedef unordered_map<Candidate, Position> CandidatePositionMap;
typedef CandidatePositionMap::value_type CandidatePositionMapData;
typedef CandidatePositionMap::const_iterator CandidatePositionMapIterator;

// the advatages of both
struct Vote
{
  CandidateList asList;
  CandidatePositionMap asMap;
  string asString;
};

// for the election
typedef vector<Vote>VoteVector;
typedef VoteVector::iterator VoteVectorIterator;

// for an overview of candidates
typedef vector<Candidate>CandidateVector;
typedef CandidateVector::const_iterator CandidateVectorIterator;

typedef unordered_set<Candidate>CandidateSet;
typedef CandidateSet::const_iterator CandidateSetIterator;

// for several consensus suggentions
struct preferenceSuggestion
{
  CandidateList asList;
  string asString;
};
typedef vector<preferenceSuggestion> SuggestionVector;
typedef SuggestionVector::iterator SuggestionVectorIterator;

/////////////////////////
//   reduction rules   //
/////////////////////////
// the set of predecessors (of a non-dirty candidate)
typedef unordered_map<Candidate, CandidateSet>CandidateCandidateSetMap;
typedef CandidateCandidateSetMap::value_type CandidateCandidateSetMapData;
typedef CandidateCandidateSetMap::const_iterator CandidateCandidateSetMapIterator;

// sizes of instances
typedef vector<size_t> InstancesSizes;
typedef InstancesSizes::iterator InstancesSizesIterator;

// reduction quality attributes
struct ReductionQuality
{
  int calls;
  double reductionTime;
  size_t instancesCount;
  size_t maxInstanceSize;
  double avgInstanceSize;
  long double sizeOfDynamicProgrammingTableSum;
  long double sizeOfDynamicProgrammingTableSum_relationToUnreduced;
  InstancesSizes numberOfConflictPairsVector;
  unsigned long numberOfConflictPairsSum;
  double percentageConflictPairsReduced;
  long double sizesOfTrivialSearchTreeSum;
  long double sizesOfTrivialSearchTreeSum_relationToUnreduced;
  string sizesPrint;
  string setsPrint;
  string rulesSequencePrint;
};

////////////////
//   scores   //
////////////////
// assoziate a score to a pair
typedef unordered_map<CandidatePair, unsigned long, boost::hash<CandidatePair> >CandidatePairWeightMap;
typedef CandidatePairWeightMap::value_type CandidatePairWeightMapData;
typedef CandidatePairWeightMap::iterator CandidatePairWeightMapIterator;

// assoziate a score to a triple
typedef unordered_map<CandidateTriple, unsigned long, boost::hash<CandidateTriple> >CandidateTripleWeightMap;
typedef CandidateTripleWeightMap::value_type CandidateTripleWeightMapData;
typedef CandidateTripleWeightMap::iterator CandidateTripleWeightMapIterator;

//////////////////////////////////
//   Extended datastructures   ///
//////////////////////////////////
// sets of predecessors and successors
struct CandidateRelations
{
  CandidateSet lowers;
  CandidateSet uppers;
};

// store the predecessors and successors for each candidate
typedef unordered_map<Candidate, CandidateRelations>CandidateCandidateRelationsMap;
typedef CandidateCandidateRelationsMap::value_type CandidateCandidateRelationsMapData;
typedef CandidateCandidateRelationsMap::iterator CandidateCandidateRelationsMapIterator;

// store pairs and triples
/////////////////////////////
typedef vector<CandidatePair>CandidatePairVector;
typedef CandidatePairVector::iterator CandidatePairVectorIterator;

typedef vector<CandidateTriple>CandidateTripelVector;
typedef CandidateTripelVector::iterator CandidateTripelIterator;

typedef unordered_set<CandidatePair, boost::hash<CandidatePair> >CandidatePairSet;
typedef CandidatePairSet::value_type CandidatePairSetData;
typedef CandidatePairSet::iterator CandidatePairSetIterator;

typedef unordered_set<CandidateTriple, boost::hash<CandidateTriple> >CandidateTripleSet;
typedef CandidateTripleSet::value_type CandidateTripleSetData;
typedef CandidateTripleSet::iterator CandidateTripleSetIterator;

// Sets of Candidate Sets (for data reduction rules)
////////////////////////////////////////////////////
struct CandidateSetS
{
  CandidateSet set;
  Candidate instance_prototyp;
  CandidateWeightMap notEqualOneWeighted;
};
std::size_t hash_value ( CandidateSetS const& cs );
bool operator== ( CandidateSetS const& a, CandidateSetS const& b );

typedef unordered_set<CandidateSetS, boost::hash<CandidateSetS> >CandidateSetSet;
typedef CandidateSetSet::value_type CandidateSetSetData;
typedef CandidateSetSet::iterator CandidateSetSetIterator;

typedef vector<CandidateSetS> CandidateSetVector;
typedef CandidateSetVector::iterator CandidateSetVectorIterator;

#endif
