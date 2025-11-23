/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2010 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ implementation file for tool functions
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

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>
#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>

// project internal
#include "tools.hpp"

bool FileExists ( string strFilename )
{
  struct stat stFileInfo;
  bool blnReturn;
  int intStat;

  // Attempt to get the file attributes
  intStat = stat ( strFilename.c_str(), &stFileInfo );
  if ( intStat == 0 )
  {
    // We were able to get the file attributes
    // so the file obviously exists.
    blnReturn = true;
  }
  else
  {
    // We were not able to get the file attributes.
    // This may mean that we don't have permission to
    // access the folder which contains this file. If you
    // need to do that level of checking, lookup the
    // return values of stat which will give you
    // more details on why stat failed.
    blnReturn = false;
  }

  return ( blnReturn );
}

string toStr ( long l )
{
  ostringstream conv;
  if ( ! ( conv << l ) )
    throw BadConversion ( "stringify(long)" );
  return conv.str();
}
string toStr ( unsigned long ul )
{
  ostringstream conv;
  if ( ! ( conv << ul ) )
    throw BadConversion ( "stringify(unsigned long)" );
  return conv.str();
}
string toStr ( int i )
{
  ostringstream conv;
  if ( ! ( conv << i ) )
    throw BadConversion ( "stringify(int)" );
  return conv.str();
}
string toStr ( unsigned int ui )
{
  ostringstream conv;
  if ( ! ( conv << ui ) )
    throw BadConversion ( "stringify(unsigned int)" );
  return conv.str();
}
string toStr ( float f )
{
  ostringstream conv;
  if ( ! ( conv << f ) )
    throw BadConversion ( "stringify(float)" );
  return conv.str();
}
string toStr ( double d )
{
  ostringstream conv;
  if ( ! ( conv << d ) )
    throw BadConversion ( "stringify(double)" );
  return conv.str();
}

void printBranching ( unsigned long branchesCount, int branches[], int branches_max[], long currK )
{
  cout << currK << "=[";
  for ( unsigned long l = 0; l < branchesCount; l++ )
  {
    cout << branches[l] << "/" << branches_max[l] << ",";
  }
  cout << "]\n";
}


void printPairDirty ( bool isDirty, CandidatePair p )
{
  if ( isDirty )
  {
    cout << "(" << p.lower << "," << p.upper << ") is dirty.\n";
  }
  else
  {
    cout << "(" << p.lower << "," << p.upper << ") is not dirty.\n";
  }
}

void printVote ( CandidateList &v, ostream &o )
{
  printCandidateList ( v, o, ">", "\n" );
}

void printCandidateList ( CandidateList &v, ostream &o , string seperator , string postFix )
{
  CandidateListIterator iterator = v.begin();
  while ( iterator != v.end() )
  {
    if ( iterator == v.begin() )
    {
      o << ( *iterator );
    }
    else
    {
      o << seperator << ( *iterator );
    }
    iterator++;
  }
  o << postFix;
}


void printVote ( CandidateList &v )
{
  printVote ( v, cout );
}

void printVote ( CandidateVector &v, ostream &o )
{
  printCandidateVector ( v, o, ">", "\n" );
}

void printCandidateVector ( CandidateVector &v, ostream &o , string seperator , string postFix )
{
  CandidateVectorIterator iterator = v.begin();
  while ( iterator != v.end() )
  {
    if ( iterator == v.begin() )
    {
      o << ( *iterator );
    }
    else
    {
      o << seperator << ( *iterator );
    }
    iterator++;
  }
  o << postFix;
}

void printVote ( CandidateVector &v )
{
  printVote ( v, cout );
}

void printSet ( CandidateSet &s )
{
  printSet ( s, cout );
}

void printSet ( CandidateSet &s, ostream &o )
{
  printCandidateSet ( s, o, ",", "\n" );
}

void printCandidateSet ( CandidateSet &s, ostream &o, string seperator, string postFix )
{
  CandidateSetIterator iterator = s.begin();
  while ( iterator != s.end() )
  {
    if ( iterator == s.begin() )
    {
      o << ( *iterator );
    }
    else
    {
      o << seperator << ( *iterator );
    }
    iterator++;
  }
  o << postFix;
}

void setVoteIteratorBefore ( CandidateList &v, CandidateListIterator &iterator, const unsigned long &searched )
{
  unsigned long debugc = 0;
  while ( ( iterator != v.end() ) && ( ( *iterator ) != searched ) )
  {
    //cout << debugc++ << ": " << (*iterator) << "=?" << searched <<endl;
    iterator++;
  }
}

void setVoteIteratorAfter ( CandidateList &v, CandidateListIterator &iterator, const unsigned long &searched )
{
  setVoteIteratorBefore ( v, iterator, searched );
  iterator++;
}

bool candidateSetsDisjoint ( const CandidateSet &a, const CandidateSet &b )
{
  for ( CandidateSetIterator ai = a.begin(); ai != a.end(); ai++ )
  {
    if ( b.find ( *ai ) != b.end() )
      return false;
  }
  return true;
}

bool isSubset ( const CandidateSet &subset, const CandidateSet &set )
{
  for ( CandidateSetIterator subset_i = subset.begin(); subset_i != subset.end(); subset_i++ )
  {
    if ( set.find ( *subset_i ) == set.end() )
      return false;
  }
  return true;
}

void addSet1MinusSet2ToSet3 ( const CandidateSet &set1, const CandidateSet &set2, CandidateSet &set3 )
{
  for ( CandidateSetIterator c = set1.begin(); c != set1.end(); c++ )
  {
    if ( set2.find ( *c ) == set2.end() )
    {
      set3.insert ( *c );
    }
  }

}


void transferWeights ( const CandidateSetS &source, CandidateSetS &subsetOfSource )
{
  transferWeights ( source.notEqualOneWeighted, subsetOfSource );
}

void transferWeights ( const CandidateWeightMap &source, CandidateSetS &subsetOfSource )
{
  for ( CandidateWeightMapIterator cw = source.begin(); cw != source.end(); cw++ )
  {
    if ( subsetOfSource.set.find ( ( *cw ).first ) != subsetOfSource.set.end() )
    {
      subsetOfSource.notEqualOneWeighted.insert ( *cw );
    }
  }
}

Weight getWeight ( const CandidateWeightMap &map, Candidate cand , Weight defaultval )
{
  CandidateWeightMapIterator candWeight = map.find ( cand );
  if ( candWeight == map.end() )
  {
    return defaultval;
  }
  else
  {
    return ( *candWeight ).second;
  }
}

uint32_t getMemberOfSingleTon_fast ( const EfficientSet<uint32_t, uint32_t> &M )
{
  register unsigned int r = ( M.rep & b32[0] ) != 0;
  /*for ( int i = 4; i > 0; i-- )
  {
      r |= ( ( rep & b[i] ) != 0 ) << i;
  }*/ // unrolled:
  r |= ( ( M.rep & b32[4] ) != 0 ) << 4;
  r |= ( ( M.rep & b32[3] ) != 0 ) << 3;
  r |= ( ( M.rep & b32[2] ) != 0 ) << 2;
  r |= ( ( M.rep & b32[1] ) != 0 ) << 1;
  r |= ( ( M.rep & b32[0] ) != 0 ) << 0;
  return r;
}

uint32_t getMemberOfSingleTon_fast ( const EfficientSet<uint32_t, uint64_t> &M )
{
  register unsigned int r = ( M.rep & b64[0] ) != 0;
  /*for ( int i = 4; i > 0; i-- )
  {
      r |= ( ( rep & b64[i] ) != 0 ) << i;
  }*/ // unrolled:
  r |= ( ( M.rep & b64[5] ) != 0 ) << 5;
  r |= ( ( M.rep & b64[4] ) != 0 ) << 4;
  r |= ( ( M.rep & b64[3] ) != 0 ) << 3;
  r |= ( ( M.rep & b64[2] ) != 0 ) << 2;
  r |= ( ( M.rep & b64[1] ) != 0 ) << 1;
  r |= ( ( M.rep & b64[0] ) != 0 ) << 0;
  return r;
}

int numberOfElements_fast ( const EfficientSet<uint32_t, uint64_t> &M )
{
  unsigned char * p = ( unsigned char * ) & M.rep;
  return BitsSetTable256[p[0]] +
         BitsSetTable256[p[1]] +
         BitsSetTable256[p[2]] +
         BitsSetTable256[p[3]];
}

string longToString ( long &l )
{
  ostringstream conv;
  if ( ! ( conv << l ) )
    throw BadConversion ( "stringify(long)" );
  return conv.str();
}

int d2i ( double d )
{
  return d < 0 ? d - .5 : d + .5;
}

unsigned int getRandomUint ( unsigned int range_min, unsigned int range_max, base_generator_type &generator )
{
  boost::uniform_int<> cands ( ( int ) range_min, ( int ) range_max );
  boost::variate_generator<base_generator_type&, boost::uniform_int<> >
  gen ( generator, cands );
  return gen();
}

float getPairwiseProbabilityWrtRefereceVector ( CandidateList &ranking, CandidatePositionMap &referenceMap )
{
  long dirtySum = 0;
  for ( CandidateListIterator x = ranking.begin(); x != ranking.end(); x++ )
  {
    CandidateListIterator temp = x;
    for ( CandidateListIterator y = ++temp; y != ranking.end(); y++ )
    {
      CandidatePositionMapIterator xMapEntry = referenceMap.find(*x);
      if ( xMapEntry == referenceMap.end())
      {
        cout << "Couldnt find entry for " << *x << "\n";
      }
      CandidatePositionMapIterator yMapEntry = referenceMap.find(*y);
      if ( yMapEntry == referenceMap.end())
      {
        cout << "Couldnt find entry for " << *y << "\n";
      }
      if ( (*xMapEntry).second > (*yMapEntry).second)
      {
        dirtySum++;
      }
    }
  }
  return 1. - ( ( float ) dirtySum / ( ( float ) ranking.size() * ( ( float ) ranking.size() - 1. ) / 2. ) );
}

float getPairwiseProbability ( CandidateVector &ranking )
{
  CandidatePositionMap referenceMap;
  CandidateList rankingList;
  Candidate c=1;
  for ( CandidateVectorIterator x = ranking.begin(); x != ranking.end(); x++ )
  {
    rankingList.push_back((*x));
    referenceMap.insert( CandidatePositionMapData ( c, c-1 ) );
    c++;
  }
  return getPairwiseProbabilityWrtRefereceVector( rankingList, referenceMap );
}

unsigned long getTotalRam()
{
  struct sysinfo si;
  sysinfo ( &si );
  return si.totalram;
}

unsigned long getFreeRam()
{
  struct sysinfo si;
  sysinfo ( &si );
  return si.freeram;
}

double getFreeRamPerc()
{
  struct sysinfo si;
  sysinfo ( &si );
  return ( double ) si.freeram / ( double ) si.totalram * 100.;
}
