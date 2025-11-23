/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2010 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ header file for tool functions
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

#ifndef TOOLS_H
#define TOOLS_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <stdexcept>
#include <sys/sysinfo.h>
#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>

// project internal
#include "votetyps.hpp"
#include "EfficientSet.hpp"

using namespace std;

typedef boost::minstd_rand base_generator_type;

string longToString ( long l );

int d2i ( double d );

unsigned int getRandomUint ( unsigned int range_min, unsigned int range_max, base_generator_type &generator );


class BadConversion : public runtime_error
{
public:
  BadConversion ( const string& s )
      : runtime_error ( s )
  { }
};

template <class T>
bool fromString(T& t, const std::string& s, std::ios_base& (*f)(std::ios_base&))
{
  std::istringstream iss(s);
  return !(iss >> f >> t).fail();
}

bool FileExists ( string strFilename );

string toStr ( long l );
string toStr ( unsigned long ul );
string toStr ( int i );
string toStr ( unsigned int ul );
string toStr ( float f );
string toStr ( double d );

void printBranching ( unsigned long branchesCount, int branches[], int branches_max[], long currK );

void printPairDirty ( bool isDirty, CandidatePair p );

void printCandidateList ( CandidateList &v, ostream &o , string seperator, string postFix );
void printVote ( CandidateList &v, ostream &o );
void printVote ( CandidateList &v );
void printCandidateVector ( CandidateVector &v, ostream &o , string seperator, string postFix );
void printVote ( CandidateVector &v, ostream &o );
void printVote ( CandidateVector &v );

void printCandidateSet ( CandidateSet &s, ostream &o , string seperator, string postFix );
void printSet ( CandidateSet &s );
void printSet ( CandidateSet &s, ostream &o );

void setVoteIteratorBefore ( CandidateList &v, CandidateListIterator &iterator, const unsigned long &searched );

void setVoteIteratorAfter ( CandidateList &v, CandidateListIterator &iterator, const unsigned long &searched );

bool candidateSetsDisjoint ( const CandidateSet &a, const CandidateSet &b );
bool isSubset ( const CandidateSet &subset, const CandidateSet &set );
void addSet1MinusSet2ToSet3 ( const CandidateSet &set1, const CandidateSet &set2, CandidateSet &set3 );

void transferWeights ( const CandidateSetS &source, CandidateSetS &subsetOfSource );
void transferWeights ( const CandidateWeightMap &source, CandidateSetS &subsetOfSource );

Weight getWeight ( const CandidateWeightMap &map, Candidate cand , Weight defaultval );

float getPairwiseProbabilityWrtRefereceVector ( CandidateList &ranking, CandidatePositionMap &referenceMap );
float getPairwiseProbability ( CandidateVector &ranking );

uint32_t getMemberOfSingleTon_fast ( const EfficientSet<uint32_t, uint32_t> &M );
uint32_t getMemberOfSingleTon_fast ( const EfficientSet<uint32_t, uint64_t> &M );
int numberOfElements_fast ( const EfficientSet<uint32_t, uint64_t> &M );

unsigned long getTotalRam();
unsigned long getFreeRam();
double getFreeRamPerc();

#endif
