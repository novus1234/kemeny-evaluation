/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2010 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ implementation file of votesmanagement
 *   Description: This class manages an election.
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
#include <vector>
#include <time.h>

// project internal
#include "errors.hpp"
#include "tools.hpp"
#include "votesmanagement.hpp"

using namespace std;
using namespace boost;

votesmanagement::votesmanagement()
{
  this->isSynchronized = false;
  this->isDirtysAnalyzed = false;
  this->isSubScoreAnalyzed = false;
  this->majority = 0.0;
  this->hasTies = false;
  this->isMajorityAnalyzed = false;
  this->isMajorityCandidatesVectorSorted = false;
}

votesmanagement::~votesmanagement()
{
}

void votesmanagement::loadSuggestionsFromFile ( string filename )
{
  ifstream source ( filename.data() );
  if ( source == NULL )
  {
    cerr << "ERROR: Could not open source file. Terminating..!\n";
    exit ( ERROR_FILE_NOT_FOUND );
  }
  char c = ' '; // current char read
  string cand; // current candidate
  unsigned long candp = 0; // current position count
  this->suggestions.clear();
  while ( !source.eof() )
  {
    // Read one ranking
    candp = 0;
    CandidateList suggestionAsList;
    string suggestionAsString = "";
    while ( ( c != '\n' ) && ( !source.eof() ) )
    {
      // Read one candidate
      cand = "";
      source.get ( c );
      while ( ( c != '\n' ) && ( c != '>' ) && ( c != '^' ) && ( !source.eof() ) )
      {
        cand += c;
        source.get ( c );
      }
      if ( c == '>' )
        candp++;
      Candidate uc = strtoul ( cand.c_str(), NULL, 0 );
      if ( uc != 0 )
      {
        suggestionAsList.push_back ( uc );
      }
      suggestionAsString += cand;
      if ( c == '>' )
      {
        suggestionAsString += ">";
      }
      if ( c == '^' )
      {
        cerr << "ERROR: Ties are not allowed yet. Terminating..!\n";
        exit ( ERROR_TIES_NOT_ALLOWED );
      }
    }
    c = ' ';
    //direct vote synchronization
    if ( suggestionAsString.size() > 0 )
    {
      preferenceSuggestion newSugg;
      newSugg.asList = suggestionAsList;
      newSugg.asString = suggestionAsString;
      this->suggestions.push_back ( newSugg );
    }
  }
  source.close();
}

void votesmanagement::loadFromFile ( string filename )
{
  this->filenameOfLoadedElection = filename;
  ifstream source ( filename.data() );
  if ( source == NULL )
  {
    cerr << "ERROR: Could not open source file. Terminating..!\n";
    exit ( ERROR_FILE_NOT_FOUND );
  }
  char c = ' '; // current char read
  string cand; // current candidate
  unsigned long candp = 0; // current position count
  this->votes.clear();
  while ( !source.eof() )
  {
    // Read one ranking
    candp = 0;
    CandidatePositionMap voteAsMap;
    CandidateList voteAsList;
    string voteAsString = "";
    CandidateList equals;
    while ( ( c != '\n' ) && ( !source.eof() ) )
    {
      // Read one candidate
      cand = "";
      source.get ( c );
      while ( ( c != '\n' ) && ( c != '>' ) && ( c != '^' ) && ( !source.eof() ) )
      {
        cand += c;
        source.get ( c );
      }
      if ( ( c == '>' ) || c == '\n' )
        candp++;
      if ( c == '^' )
        this->hasTies = true;
      //TODO: update (sub)score functions to work with ties

      Candidate uc = strtoul ( cand.c_str(), NULL, 0 );
      if ( uc != 0 )
      {
        voteAsMap.insert ( CandidatePositionMapData ( uc, candp ) );
        voteAsList.push_back ( uc );
        equals.push_back ( uc );
      }
      if ( ( c == '>' ) || ( c == '\n' ) )
      {
        string append = "";
        // LISTE SORTIEREN
        equals.sort();
        for ( CandidateListIterator i = equals.begin(); i != equals.end(); i++ )
        {
          if ( i == equals.begin() )
          {
            append = cand.c_str();
          }
          else
          {
            append += "^";
            append += cand.c_str();
          }
        }
        if ( c == '>' )
        {
          append = append + ">";
        }
        equals.clear();
        voteAsString += append;
      }
    }
    c = ' ';
    //direct vote synchronization
    if ( voteAsString.size() > 0 )
    {
      Vote newVote;
      newVote.asList = voteAsList;
      newVote.asMap = voteAsMap;
      newVote.asString = voteAsString;
      this->votes.push_back ( newVote );
    }
  }
  source.close();
  //cout << "The election has got " << this->votes.size() << " vote(s)." << endl;
  this->isSynchronized = false;
  // load weights if file exists
  string weightfilename = filename + ".weight";
  if ( FileExists ( weightfilename ) )
  {
    this->loadWeightsFromFile ( weightfilename );
  }
}

void votesmanagement::synchronizeDataTypes()
{
  this->candidateVector.clear();
  this->candidateSet.clear();

  clock_t clock1, clock2;
  clock1 = clock();

  for ( VoteVectorIterator i = this->votes.begin(); i != this->votes.end(); i++ )
  {
    for ( CandidateListIterator x = ( *i ).asList.begin(); x != ( *i ).asList.end(); x++ )
    {
      if ( this->candidateSet.find ( *x ) == this->candidateSet.end() )
      {
        //it's a new one
        this->candidateVector.push_back ( *x );
        this->candidateSet.insert ( *x );
      }
      //cout << "cand:"  << (*x) << "\n";
    }
  }
  std::sort ( this->candidateVector.begin(), this->candidateVector.end() );

  clock2 = clock();
  this->lastSynchonizeDataTypesTime = ( ( double ) ( clock2 - clock1 ) ) / CLOCKS_PER_SEC;

  this->isSynchronized = true;
}

Position votesmanagement::getPosition ( unsigned long voterId, Candidate can )
{
  if ( !this->isSynchronized )
    this->synchronizeDataTypes();
  if ( voterId >= this->getVotesCount() )
  {
    //TODO throw Exception
  }
  else
  {
    CandidatePositionMapIterator pos = this->votes.at ( voterId ).asMap.find ( can );
    if ( pos == this->votes.at ( voterId ).asMap.end() )
    {
      //TODO throw Exception
    }
    return ( *pos ).second;
  }
  // not found =0
  return 0;
}

unsigned long votesmanagement::getVotesCount()
{
  if ( !this->isSynchronized )
    this->synchronizeDataTypes();
  return this->votes.size();
}

unsigned long votesmanagement::getSuggestionsCount()
{
  return this->suggestions.size();
}

void votesmanagement::testSuggestions ( bool verbose )
{
  for ( SuggestionVectorIterator i = this->suggestions.begin(); i != this->suggestions.end(); i++ )
  {
    unsigned long suggScore = this->kscore ( ( *i ).asList );
    if ( verbose )
    {
      cout << "Suggestion " << ( *i ).asString << " has a score of " << suggScore << ".\n";
    }
    else
      printf ( "%lu\n", suggScore );
    {
    }
  }
}

unsigned long votesmanagement::getCandidatesCount()
{
  if ( !this->isSynchronized )
    this->synchronizeDataTypes();
  return this->candidateSet.size();
}

unsigned long votesmanagement::getDirtyPairsCount()
{
  if ( !this->isSynchronized )
    this->synchronizeDataTypes();
  if ( !this->isSubScoreAnalyzed )
    this->analyzeSubScore();
  if ( !this->isDirtysAnalyzed )
    this->analyzeDirtys();
  return this->dirtyPairMap.size();
}

/*
returns 0 if upper>lower
returns 1 if lower>upper
else returns (-1)*(lower not found+2*uppernotfound)
*/
int votesmanagement::lowerThenUpper ( CandidatePositionMap &v, CandidatePositionMapIterator &foundl, CandidatePositionMapIterator &foundu )
{
  int notfound = 0;
  if ( foundl == v.end() )
    notfound--;
  if ( foundu == v.end() )
    notfound = notfound - 2;
  if ( notfound == 0 )
  {
    unsigned long lowerposition = ( *foundl ).second;
    unsigned long upperposition = ( *foundu ).second;
    return ( lowerposition > upperposition );
  }
  return notfound;
}

int votesmanagement::lowerThenUpper ( unsigned long voterID, CandidatePair &p )
{
  CandidateList &l = this->votes.at ( voterID ).asList;
  return lowerThenUpper ( l, p );

  //CandidatePositionMap &m = this->votes.at( voterID ).asMap;
  //return lowerThenUpper( m, p );
}

int votesmanagement::lowerThenUpper ( CandidatePositionMap &v, CandidatePair &p )
{
  CandidatePositionMapIterator foundl = v.find ( p.lower );
  CandidatePositionMapIterator foundu = v.find ( p.upper );
  return lowerThenUpper ( v, foundl, foundu );
}


int votesmanagement::lowerThenUpper ( CandidateList &v, const CandidatePair &p )
{
  int notfound = -3;
  unsigned long lowerposition;
  unsigned long upperposition;
  unsigned long pos = 0;
  for ( CandidateListIterator x = v.begin(); x != v.end(); x++ )
  {
    if ( ( *x ) == p.lower )
    {
      lowerposition = pos;
      notfound++;
      if ( notfound == 0 )
        break;
    }
    if ( ( *x ) == p.upper )
    {
      upperposition = pos;
      notfound = notfound + 2;
      if ( notfound == 0 )
        break;
    }
    pos++;
  }
  if ( notfound == 0 )
  {
    return ( lowerposition > upperposition );
  }
  return notfound;
}

void votesmanagement::analyzeSubScore()
{
  this->subScore.clear();
  this->min_k = 0;

  if ( !this->isSynchronized )
    this->synchronizeDataTypes();

  clock_t clock1, clock2;
  clock1 = clock();

  // using ordered data structur here for sequential (and ordered) access.
  unsigned long pid = 0;
  for ( CandidateVectorIterator x = this->candidateVector.begin(); x != this->candidateVector.end(); x++ )
  {
    CandidateVectorIterator temp = x;
    for ( CandidateVectorIterator y = ++temp; y != this->candidateVector.end(); y++ )
    {
      CandidatePair p = ascSortedCandidatePair ( *x, *y );

      unsigned long ltu = 0;
      unsigned long utl = 0;
      for ( unsigned long i = 0; i < this->votes.size(); i++ )
      {
        int status = this->lowerThenUpper ( i, p );
        if ( status )
        {
          ltu++;
        }
        else
        {
          utl++;
        }
      }
      CandidatePair unsortedP;
      unsortedP.lower = p.upper;
      unsortedP.upper = p.lower;

      // respect the candidate weights, assume weight=1 as default
      Weight xweight = 1;
      Weight yweight = 1;
      CandidateWeightMapIterator xw = this->candidateWeights.find ( *x );
      CandidateWeightMapIterator yw = this->candidateWeights.find ( *y );
      if ( xw != this->candidateWeights.end() )
      {
        xweight = ( *xw ).second;
      }
      if ( yw != this->candidateWeights.end() )
      {
        yweight = ( *yw ).second;
      }
      Weight scoreWeight = xweight * yweight;

      this->subScore.insert ( CandidatePairWeightMapData ( unsortedP, ltu * scoreWeight ) );
      //cout << "subscore for " << unsortedP.lower << ","<< unsortedP.upper << " = " << ltu << endl;
      this->subScore.insert ( CandidatePairWeightMapData ( p, utl * scoreWeight ) );
      //cout << "subscore for " << p.lower << "," << p.upper << " = " << utl << endl;
      this->min_k += min ( ltu, utl );
    }
  }

  clock2 = clock();
  this->lastAnalyzeSubScoreTime = ( ( double ) ( clock2 - clock1 ) ) / CLOCKS_PER_SEC;

  this->isSubScoreAnalyzed = true;
}

unsigned long votesmanagement::kscore ( CandidateList &consensus )
{
  unsigned long tmpResult = 0;
  for ( CandidateListIterator c1 = consensus.begin(); c1 != consensus.end(); c1++ )
  {
    CandidateListIterator c2i = c1;
    c2i++;
    for ( CandidateListIterator c2 = c2i; c2 != consensus.end(); c2++ )
    {
      CandidatePair p;
      p.lower = *c1;
      p.upper = *c2;
      tmpResult += this->subscore ( p.upper, p.lower );
      //printf("subscore +with (%d,%d) -> %d\n", p.upper, p.lower, tmpResult);
    }
  }
  return tmpResult;
}

unsigned long votesmanagement::subscore ( CandidatePair &p )
{
  CandidatePairWeightMapIterator sf = this->subScore.find ( p );
  if ( sf != this->subScore.end() )
  {
    return ( *sf ).second;
  }
  return 0;
}

unsigned long votesmanagement::subscore ( Candidate lower, Candidate upper )
{
  CandidatePair p;
  p.lower = lower;
  p.upper = upper;
  return subscore ( p );
}


bool votesmanagement::isDirty ( unsigned long x, unsigned long y )
{
  this->isDirty ( ascSortedCandidatePair ( x, y ) );
}

bool votesmanagement::isDirty ( CandidatePair p )
{
  if ( !this->isDirtysAnalyzed )
    this->analyzeDirtys();
  return ( ( this->dirtyPairMap.find ( p ) ) != ( this->dirtyPairMap.end() ) );
};


void votesmanagement::analyzeDirtys()
{
  if ( !this->isSynchronized )
    this->synchronizeDataTypes();
  if ( !this->isSubScoreAnalyzed )
    this->analyzeSubScore();

  this->dirtyPairMap.clear();
  unsigned long pid = 0;

  clock_t clock1, clock2;
  clock1 = clock();

  for ( CandidateVectorIterator x = this->candidateVector.begin(); x != this->candidateVector.end(); x++ )
  {
    CandidateVectorIterator temp = x;
    for ( CandidateVectorIterator y = ++temp; y != this->candidateVector.end(); y++ )
    {
      CandidatePair p = ascSortedCandidatePair ( *x, *y );
      unsigned long subScoreOrderedAsID = ( * ( this->subScore.find ( p ) ) ).second;
      CandidatePair unsortedP;
      unsortedP.lower = p.upper;
      unsortedP.upper = p.lower;
      unsigned long subScoreOrderedNotAsID = ( * ( this->subScore.find ( unsortedP ) ) ).second;
      if ( ( subScoreOrderedAsID > 0 ) && ( subScoreOrderedNotAsID > 0 ) )
      {
        this->dirtyPairMap.insert ( CandidatePairWeightMapData ( p, pid++ ) );
        this->dirtyCandidateSet.insert ( p.lower );
        this->dirtyCandidateSet.insert ( p.upper );
      }
      else
      {
        if ( lowerThenUpper ( 0, p ) )
          this->pairOrderedAsId.insert ( p );
      }
    }
  }

  clock2 = clock();
  this->lastAnalyzeDirtysTime = ( ( double ) ( clock2 - clock1 ) ) / CLOCKS_PER_SEC;

  this->isDirtysAnalyzed = true;
}

bool votesmanagement::isInvolved ( Candidate can, CandidateList &list )
{
  for ( CandidateListIterator i = list.begin(); i != list.end(); i++ )
  {
    if ( *i == can )
      return true;
  }
  return false;
}

bool votesmanagement::isInvolved ( Candidate can, CandidatePositionMap &map )
{
  return ( map.find ( can ) != map.end() );
}

bool votesmanagement::isInvolved ( CandidatePair &p, CandidateList &list )
{
  bool lowIn = false;
  bool uppIn = false;
  for ( CandidateListIterator i = list.begin(); i != list.end(); i++ )
  {
    if ( *i == p.lower )
    {
      lowIn = true;
      if ( uppIn )
        return true;
    }
    else
      if ( *i == p.upper )
      {
        uppIn = true;
        if ( lowIn )
          return true;
      }
  }
  return false;
}

bool votesmanagement::isInvolved ( CandidatePair &p, CandidatePositionMap &map )
{
  return ( ( map.find ( p.lower ) != map.end() ) && ( map.find ( p.upper ) != map.end() ) );
}

bool votesmanagement::agree ( CandidatePair &p, CandidateList &list )
{
  bool upperWasFound = false;
  for ( CandidateListIterator i = list.begin(); i != list.end(); i++ )
  {
    if ( *i == p.lower )
    {
      if ( upperWasFound )
        return false;
    }
    else
      if ( *i == p.upper )
      {
        upperWasFound = true;
      }
  }
  return true;
}

bool votesmanagement::agree ( CandidatePairSet &Ps, CandidateList &list )
{
  for ( CandidatePairSetIterator i = Ps.begin(); i != Ps.end(); i++ )
  {
    CandidatePair p = *i;
    if ( ! ( this->agree ( p, list ) ) )
      return false;
  }
  return true;
}

bool votesmanagement::notlastpos ( vector<int> relpos )
{
  int m = relpos.size();
  //cout << "lll" << m << endl;
  for ( int i = 0; i < m; i++ )
  {
    int lpos = ( m - i - 1 );
    int apos = relpos.at ( i );
    //cout << i << ":" << apos << "<" << lpos << "?\n";
    if ( apos < lpos )
    {
      return true;
    }
  }
  return false;
}

void votesmanagement::nextpos ( vector<int> &relpos )
{
  int m = relpos.size();
  relpos[m-1] = 1;
  for ( int i = m - 1; i > 0; i-- )
  {
    if ( relpos[i] > ( m - i - 1 ) )
    {
      relpos[i] = 0;
      relpos[i-1] = relpos[i-1] + 1;
    }
  }
}

void votesmanagement::genSuggestionsForConstrains ( string filename )
{
  ofstream sugg ( filename.data() );
  if ( sugg == NULL )
  {
    cerr << "Cannot create suggestions file.\n";
    exit ( ERROR_FILE_NOT_FOUND );
  }
  int m = this->getCandidatesCount();
  vector<int> relpos;
  vector<int> perm;
  vector<int> positions;
  for ( int i = 0; i < m; i++ )
  {
    relpos.push_back ( 0 );
    perm.push_back ( 0 );
    positions.push_back ( i );
  }
  while ( notlastpos ( relpos ) )
  {
    vector<int> permPos = positions;
    for ( int i = 1; i < m + 1; i++ )
    {
      int posOFi = permPos.at ( relpos[i-1] );
      //cout << posOFi << "/" << perm.size() << "\n";
      perm[posOFi] = i;
      //cout << relpos[i-1] << "/" << permPos.size() << "\n";
      permPos.erase ( permPos.begin() + relpos[i-1] );
    }
    CandidateList permAsList;
    for ( int i = 0; i < m; i++ )
    {
      permAsList.push_back ( perm[i] );
    }
    if ( this->agree ( this->constraints, permAsList ) )
      printVote ( permAsList, sugg );
    nextpos ( relpos );
  }
  sugg.close();
}

void votesmanagement::loadWeightsFromFile ( string filename )
{
  ifstream source ( filename.data() );
  if ( source == NULL )
  {
    cerr << "ERROR: Could not open source file. Terminating..!\n";
    exit ( ERROR_FILE_NOT_FOUND );
  }
  this->candidateWeights.clear();
  string candname;
  string candweight;
  char nextChar;
  long fpos = 0;
  while ( !source.eof() )
  {
    candname = "";
    nextChar = ' ';
    while ( nextChar == ' ' )
    {
      fpos = source.tellg();
      source >> candweight;
      //if(dict.gcount()<1)break;
      source.get ( nextChar );
      source.unget();
      if ( nextChar == ' ' )
        candname = candname + candweight;
      if ( fpos == source.tellg() )
        break;
    }
    if ( fpos == source.tellg() )
      break;
    this->candidateWeights.insert ( CandidateWeightMapData ( strtoul ( candname.data(), NULL, 0 ), strtoul ( candweight.data(), NULL, 0 ) ) );
    cout << "Cand:" << candname << " Weight: " << candweight << endl;
  }
  source.close();
}

void votesmanagement::loadConstraintsFromFile ( string filename )
{
  ifstream source ( filename.data() );
  if ( source == NULL )
  {
    cerr << "ERROR: Could not open source file. Terminating..!\n";
    exit ( ERROR_FILE_NOT_FOUND );
  }
  char c = ' '; // current char read
  string cand; // current candidate
  unsigned long candp = 0; // current position count
  this->suggestions.clear();
  while ( !source.eof() )
  {
    // Read one ranking
    candp = 0;
    CandidateList constraintAsList;
    string constraintAsString = "";
    while ( ( c != '\n' ) && ( !source.eof() ) )
    {
      // Read one candidate
      cand = "";
      source.get ( c );
      while ( ( c != '\n' ) && ( c != '>' ) && ( c != '^' ) && ( !source.eof() ) )
      {
        cand += c;
        source.get ( c );
      }
      if ( c == '>' )
        candp++;
      Candidate uc = strtoul ( cand.c_str(), NULL, 0 );
      if ( uc != 0 )
      {
        constraintAsList.push_back ( uc );
      }
      constraintAsString += cand;
      if ( c == '>' )
      {
        constraintAsString += ">";
      }
      if ( c == '^' )
      {
        cerr << "ERROR: Ties are not allowed yet. Terminating..!\n";
        exit ( ERROR_TIES_NOT_ALLOWED );
      }
    }
    c = ' ';
    //direct vote synchronization
    if ( constraintAsString.size() > 0 )
    {
      for ( CandidateListIterator a = constraintAsList.begin(); a != constraintAsList.end(); a++ )
      {
        CandidateListIterator ap1 = a;
        for ( CandidateListIterator b = ++ap1; b != constraintAsList.end(); b++ )
        {
          CandidatePair p;
          p.lower = *a;
          p.upper = *b;
          this->constraints.insert ( p );
        }
      }
    }
  }
  source.close();
}

double votesmanagement::getMajority()
{

  return this->majority;
}

void votesmanagement::analyzeMajority ( double majority )
{
  analyzeMajority ( majority, this->candidateSet );
}

void votesmanagement::analyzeMajority ( double majority, const CandidateSet &instanceToSplitCandidates )
{
  if ( !this->isSynchronized )
    this->synchronizeDataTypes();
  if ( !this->isSubScoreAnalyzed )
    this->analyzeSubScore();
  if ( !this->isDirtysAnalyzed )
    this->analyzeDirtys();
  this->majority = majority;
  this->majorityInstanceCandidates = instanceToSplitCandidates;
  this->majorityPairs.clear();
  this->majorityCandidates.clear();

  clock_t clock1, clock2;
  clock1 = clock();

  unsigned long n = this->getVotesCount();
  for ( CandidateSetIterator x = instanceToSplitCandidates.begin(); x != instanceToSplitCandidates.end(); x++ )
  {
    CandidateSetIterator temp = x;
    for ( CandidateSetIterator y = ++temp; y != instanceToSplitCandidates.end(); y++ )
    {
      CandidatePair xy, yx;
      xy.lower = *x;
      xy.upper = *y;
      yx.lower = *y;
      yx.upper = *x;
      double xyScoreRate = voteRate ( *x, *y );
      double yxScoreRate = voteRate ( *y, *x );
      if ( xyScoreRate >= majority )
      {
        //this->majorityPairs.insert(yx);
        //cout << xyScoreRate << "\n";
        this->majorityPairs.insert ( xy );
      }
      else
        if ( yxScoreRate >= majority )
        {
          //cout << yxScoreRate << "\n";
          this->majorityPairs.insert ( yx );
          //this->majorityPairs.insert(xy);
        }
        else
        {
          this->nonMajorityPairs.insert ( xy );
          this->nonMajorityPairs.insert ( yx );
          if ( this->majorityDirtyCandidates.find ( *x ) == this->majorityDirtyCandidates.end() )
          {
            this->majorityDirtyCandidates.insert ( *x );
            //printf ( "%d is majority dirty because of %d, score is %f\n", *x, *y, xyScoreRate);
          }
          if ( this->majorityDirtyCandidates.find ( *y ) == this->majorityDirtyCandidates.end() )
          {
            this->majorityDirtyCandidates.insert ( *y );
            //printf ( "%d is majority dirty because of %d, score is %f\n", *y, *x, yxScoreRate );
          }
        }
    }
  }
  for ( CandidateSetIterator x = instanceToSplitCandidates.begin(); x != instanceToSplitCandidates.end(); x++ )
  {
    if ( this->majorityDirtyCandidates.find ( *x ) == this->majorityDirtyCandidates.end() )
    {
      this->majorityCandidates.push_back ( *x );
    }
  }

  clock2 = clock();
  this->lastAnalyzeMajorityTime = ( ( double ) ( clock2 - clock1 ) ) / CLOCKS_PER_SEC;

  this->isMajorityAnalyzed = true;
  this->isMajorityCandidatesVectorSorted = false;
}

void votesmanagement::printMajorityNonDirtyCandidates()
{
  printCandidateVector ( this->majorityCandidates, cout, ",", "\n" );
}

bool comperator_candidatesByRelationsSet::operator() ( const Candidate &a, const Candidate &b )
{
  CandidatePair ab;
  ab.lower = a;
  ab.upper = b;
  bool pairListed = ( this->relationsSet.find ( ab ) != this->relationsSet.end() );
  //cout << a << ">" << b << "? " << pairListed << "\n";
  return pairListed;
}

void votesmanagement::sortMajorityNonDirtyCandidates()
{
  if ( !this->isMajorityAnalyzed )
    this->analyzeMajority ( this->getMajority() );
  comperator_candidatesByRelationsSet mNDCc;
  for ( CandidateVectorIterator x = this->majorityCandidates.begin(); x != this->majorityCandidates.end(); x++ )
  {
    CandidateVectorIterator temp = x;
    bool xismajorityorderedtoall = true;
    for ( CandidateVectorIterator y = ++temp; y != this->majorityCandidates.end(); y++ )
    {
      CandidatePair xy, yx;
      xy.lower = *x;
      xy.upper = *y;
      yx.lower = *y;
      yx.upper = *x;
      if ( this->majorityPairs.find ( xy ) != this->majorityPairs.end() )
      {
        //cout << "inserting: " << *x << ">" << *y << "\n";
        mNDCc.relationsSet.insert ( xy );
      }
      else
      {
        mNDCc.relationsSet.insert ( yx );
      }
    }
  }
  std::sort ( this->majorityCandidates.begin(), this->majorityCandidates.end(), mNDCc );
  this->isMajorityCandidatesVectorSorted = true;
}

int votesmanagement::getMajorityNonDirtyCandidatesCount()
{
  return ( this->majorityCandidates.size() );
}

unsigned long votesmanagement::getMajorityPairsCount()
{
  return this->majorityPairs.size();
}

void votesmanagement::printMajorityPairs ( ostream &o )
{
  for ( CandidatePairSetIterator i = this->majorityPairs.begin(); i != this->majorityPairs.end(); i++ )
  {
    o << ( *i ).lower << ">" << ( *i ).upper << "\n";
  }
}

void votesmanagement::printMajorityPairs ( string &filename )
{
  ofstream majf ( filename.data() );
  if ( majf == NULL )
  {
    cerr << "Cannot create majoritys file.\n";
    exit ( ERROR_FILE_NOT_FOUND );
  }
  printMajorityPairs ( majf );
  majf.close();
}

void votesmanagement::printNonMajorityPairsBothDirections ( ostream &o )
{
  for ( CandidatePairSetIterator i = this->nonMajorityPairs.begin(); i != this->nonMajorityPairs.end(); i++ )
  {
    o << ( *i ).lower << ">" << ( *i ).upper << "\n";
    o << ( *i ).upper << ">" << ( *i ).lower << "\n";
  }
}

void votesmanagement::printNonMajorityPairsBothDirections ( string &filename )
{
  ofstream majf ( filename.data() );
  if ( majf == NULL )
  {
    cerr << "Cannot create majoritys file.\n";
    exit ( ERROR_FILE_NOT_FOUND );
  }
  printNonMajorityPairsBothDirections ( majf );
  majf.close();
}

void votesmanagement::printNonMajorityPairsOneDirection ( ostream &o )
{
  for ( CandidatePairSetIterator i = this->nonMajorityPairs.begin(); i != this->nonMajorityPairs.end(); i++ )
  {
    o << ( *i ).lower << ">" << ( *i ).upper << "\n";
  }
}

void votesmanagement::printNonMajorityPairsOneDirection ( string &filename )
{
  ofstream majf ( filename.data() );
  if ( majf == NULL )
  {
    cerr << "Cannot create majoritys file.\n";
    exit ( ERROR_FILE_NOT_FOUND );
  }
  printNonMajorityPairsOneDirection ( majf );
  majf.close();
}

void votesmanagement::printDirtyPairsBothDirections ( ostream &o )
{
  if ( !this->isSynchronized )
    this->synchronizeDataTypes();
  if ( !this->isSubScoreAnalyzed )
    this->analyzeSubScore();
  if ( !this->isDirtysAnalyzed )
    this->analyzeDirtys();
  for ( CandidatePairWeightMapIterator i = this->dirtyPairMap.begin(); i != this->dirtyPairMap.end(); i++ )
  {
    o << ( *i ).first.lower << ">" << ( *i ).first.upper << "\n";
    o << ( *i ).first.upper << ">" << ( *i ).first.lower << "\n";
  }
}

void votesmanagement::printDirtyPairsBothDirections ( string &filename )
{
  ofstream dirt ( filename.data() );
  if ( dirt == NULL )
  {
    cerr << "Cannot create majoritys file.\n";
    exit ( ERROR_FILE_NOT_FOUND );
  }
  printDirtyPairsBothDirections ( dirt );
  dirt.close();
}

void votesmanagement::printDirtyPairsOneDirection ( ostream &o )
{
  if ( !this->isSynchronized )
    this->synchronizeDataTypes();
  if ( !this->isSubScoreAnalyzed )
    this->analyzeSubScore();
  if ( !this->isDirtysAnalyzed )
    this->analyzeDirtys();
  for ( CandidatePairWeightMapIterator i = this->dirtyPairMap.begin(); i != this->dirtyPairMap.end(); i++ )
  {
    o << ( *i ).first.lower << ">" << ( *i ).first.upper << "\n";
  }
}

void votesmanagement::printDirtyPairsOneDirection ( string &filename )
{
  ofstream dirt ( filename.data() );
  if ( dirt == NULL )
  {
    cerr << "Cannot create majoritys file.\n";
    exit ( ERROR_FILE_NOT_FOUND );
  }
  printDirtyPairsOneDirection ( dirt );
  dirt.close();
}

unsigned long votesmanagement::getRange ( Candidate can )
{
  unsigned long maxPos = this->getPosition ( 0, can );
  unsigned long minPos = this->getPosition ( 0, can );
  for ( VoteVectorIterator v = this->votes.begin(); v != this->votes.end(); v++ )
  {
    CandidatePositionMapIterator cinv = ( *v ).asMap.find ( can );
    if ( cinv != ( *v ).asMap.end() )
    {
      unsigned long cinvPos = ( *cinv ).second;
      if ( cinvPos < minPos )
        minPos = cinvPos;
      if ( cinvPos > maxPos )
        maxPos = cinvPos;
    }
  }
  return maxPos -minPos + 1;
}

unsigned long votesmanagement::getMaxRange()
{
  unsigned long maxRange = 0;
  for ( CandidateVectorIterator c = this->candidateVector.begin(); c != this->candidateVector.end(); c++ )
  {
    unsigned long cRange = this->getRange ( *c );
    if ( cRange > maxRange )
      maxRange = cRange;
    //printf("Range of Candidate %d was: %d\n",*c,cRange);
  }
  return maxRange;
}

unsigned long votesmanagement::ktdist ( int votea, int voteb )
{
  unsigned long retDist = 0;
  if ( this->hasTies )
  {
    for ( CandidateVectorIterator x = this->candidateVector.begin(); x != this->candidateVector.end(); x++ )
    {
      CandidateVectorIterator temp = x;
      for ( CandidateVectorIterator y = ++temp; y != this->candidateVector.end(); y++ )
      {
        CandidatePositionMapIterator axPos = this->votes.at ( votea ).asMap.find ( *x );
        CandidatePositionMapIterator ayPos = this->votes.at ( votea ).asMap.find ( *y );
        CandidatePositionMapIterator bxPos = this->votes.at ( voteb ).asMap.find ( *x );
        CandidatePositionMapIterator byPos = this->votes.at ( voteb ).asMap.find ( *y );
        if ( ( ( *axPos ).second > ( *ayPos ).second ) && ( ( *bxPos ).second < ( *byPos ).second ) )
          retDist = retDist + 2;
        if ( ( ( *axPos ).second < ( *ayPos ).second ) && ( ( *bxPos ).second > ( *byPos ).second ) )
          retDist = retDist + 2;
        if ( ( ( *axPos ).second == ( *ayPos ).second ) && ( ( *bxPos ).second != ( *byPos ).second ) )
          retDist = retDist + 1;
        if ( ( ( *axPos ).second != ( *ayPos ).second ) && ( ( *bxPos ).second == ( *byPos ).second ) )
          retDist = retDist + 1;
      }
    }
  }
  else
  {
    for ( CandidateVectorIterator x = this->candidateVector.begin(); x != this->candidateVector.end(); x++ )
    {
      CandidateVectorIterator temp = x;
      for ( CandidateVectorIterator y = ++temp; y != this->candidateVector.end(); y++ )
      {
        CandidatePositionMapIterator axPos = this->votes.at ( votea ).asMap.find ( *x );
        CandidatePositionMapIterator ayPos = this->votes.at ( votea ).asMap.find ( *y );
        CandidatePositionMapIterator bxPos = this->votes.at ( voteb ).asMap.find ( *x );
        CandidatePositionMapIterator byPos = this->votes.at ( voteb ).asMap.find ( *y );
        if ( ( ( *axPos ).second > ( *ayPos ).second ) && ( ( *bxPos ).second < ( *byPos ).second ) )
          retDist++;
        if ( ( ( *axPos ).second < ( *ayPos ).second ) && ( ( *bxPos ).second > ( *byPos ).second ) )
          retDist++;
      }
    }
  }
  return retDist;
}

double votesmanagement::getAverageKTdist()
{
  double retAvgDist = 0.;
  int n = this->votes.size();
  double dn = n * ( n - 1 ) / 2.;
  for ( int a = 0; a < n; a++ )
  {
    for ( int b = a + 1; b < n; b++ )
    {
      double newDist = this->ktdist ( a, b );
      retAvgDist = retAvgDist +  newDist / dn;
      //printf("ktdist(%d,%d)=%f",a,b,newDist);
    }
  }
  return retAvgDist;
}

unsigned long votesmanagement::getLowerScoreBound()
{
  unsigned long retScore = 0;
  for ( CandidateVectorIterator x = this->candidateVector.begin(); x != this->candidateVector.end(); x++ )
  {
    CandidateVectorIterator temp = x;
    for ( CandidateVectorIterator y = ++temp; y != this->candidateVector.end(); y++ )
    {
      CandidatePair xy, yx;
      xy.lower = *x;
      xy.upper = *y;
      yx.lower = *y;
      yx.upper = *x;
      unsigned long xyScore = this->subscore ( xy );
      unsigned long yxScore = this->subscore ( yx );
      retScore += min ( xyScore, yxScore );
      /*if(retScore==1)
      {
       printf("%d %d\n",*x,*y);
      }*/
    }
  }
  return retScore;
}

unsigned long votesmanagement::getUpperScoreBound()
{
  unsigned long retScore = 0;
  for ( CandidateVectorIterator x = this->candidateVector.begin(); x != this->candidateVector.end(); x++ )
  {
    CandidateVectorIterator temp = x;
    for ( CandidateVectorIterator y = ++temp; y != this->candidateVector.end(); y++ )
    {
      CandidatePair xy, yx;
      xy.lower = *x;
      xy.upper = *y;
      yx.lower = *y;
      yx.upper = *x;
      unsigned long xyScore = this->subscore ( xy );
      unsigned long yxScore = this->subscore ( yx );
      retScore += max ( xyScore, yxScore );
    }
  }
  return retScore;
}

double votesmanagement::voteRate ( Candidate lower, Candidate upper )
{
  unsigned long n = this->getVotesCount();
  CandidatePair loUp;
  loUp.lower = lower;
  loUp.upper = upper;
  unsigned long loUpScore = this->subscore ( loUp );
  return ( double ) loUpScore / ( double ) n;
}

double votesmanagement::getDirtyPairAnalyzeTime()
{
  return ( this->lastAnalyzeSubScoreTime + this->lastAnalyzeDirtysTime );
}

long anon_and_map_sourcefile ( string sourcefile, stringmap &dmap, stringmap &amap, string dictfile, string anonfile )
{
  ifstream source ( sourcefile.data() );
  if ( source == NULL )
  {
    cerr << "Cannot open source file.\n";
    return -1;
  }
  ofstream dict ( dictfile.data() );
  if ( dict == NULL )
  {
    cerr << "Cannot create dictionary file.\n";
    return -2;
  }
  ofstream anon ( anonfile.data() );
  if ( anon == NULL )
  {
    cerr << "Cannot create anonymized election file.\n";
    return -3;
  }
  char c = ' '; // current char read
  string cand; // current candidate
  long anon_counter = 1; // counte to create anonymired names
  string cand_anon; // current candidate anonymized
  stringmap::const_iterator iter;
  while ( !source.eof() )
  {
    // Read one ranking
    while ( ( c != '\n' ) && ( !source.eof() ) )
    {
      // Read one candidate
      cand = "";
      source.get ( c );
      while ( ( c != '>' ) && ( c != '^' ) && ( c != '\n' ) && ( !source.eof() ) )
      {
        cand += c;
        source.get ( c );
      }
      iter = amap.find ( cand );
      if ( iter == amap.end() )
      {
        if ( cand.length() > 0 )
        {
          cand_anon = toStr ( anon_counter++ );
          amap.insert ( stringpair ( cand, cand_anon ) );
          dmap.insert ( stringpair ( cand_anon, cand ) );
          dict << cand << " " << cand_anon << endl;
        }
      }
      iter = amap.find ( cand );
      if ( iter != amap.end() )
      {
        anon << ( *iter ).second;
      }
      if ( ( c != '\n' ) && ( !source.eof() ) )
      {
        anon.put ( c );
        c = ' ';
      }
    }
    if ( ( !source.eof() ) )
    {
      anon.put ( c );
      c = ' ';
    }
  }
  source.close();
  dict.close();
  anon.close();
  string weightfilename = sourcefile + ".weight";
  string anonweightfilename = anonfile + ".weight";
  bool readCand = false; // else we read the weight
  if ( FileExists ( weightfilename ) )
  {
    ifstream source ( weightfilename.data() );
    if ( source == NULL )
    {
      cerr << "Cannot open weight source file.\n";
      return -1;
    }
    ofstream anon ( anonweightfilename.data() );
    if ( anon == NULL )
    {
      cerr << "Cannot create anonymized weight file.\n";
      return -3;
    }
    while ( !source.eof() )
    {
      source >> cand;
      readCand = !readCand;
      if ( readCand )
      {
        iter = amap.find ( cand );
        if ( iter != amap.end() )
        {
          anon << ( *iter ).second; // writing anonymized candidate name
        }
      }
      else
      {
        anon << " " << cand << endl; // writing weight
      }
    }
  }
  source.close();
  anon.close();
}

long analyze_commons ( string sourcefile, stringset &commons )
{
//      printf("Analysing common candidates\n");
  stringset cand_set;

  ifstream source ( sourcefile.data() );
  if ( source == NULL )
  {
    cerr << "Cannot open source file.\n";
    return -1;
  }
  char c = ' '; // current char read
  string cand; // current candidate
  stringset::const_iterator iter;
  bool firstvote = true;
  while ( !source.eof() )
  {
    // Read one ranking
    while ( ( c != '\n' ) && ( !source.eof() ) )
    {
      // Read one candidate
      cand = "";
      source.get ( c );
      while ( ( c != '>' ) && ( c != '^' ) && ( c != '\n' ) && ( !source.eof() ) )
      {
        cand += c;
        source.get ( c );
      }
      if ( firstvote )
      {
        iter = commons.find ( cand );
        if ( iter == commons.end() )
        {
          commons.insert ( cand );
        }
      }
      else
      {
        iter = cand_set.find ( cand );
        if ( iter == cand_set.end() )
        {
          cand_set.insert ( cand );
        }
      }
      if ( ( c != '\n' ) && ( !source.eof() ) )
      {
        c = ' ';
      }
    }
    if ( ( !source.eof() ) )
    {
      // kill all candidates that are not in current vote
      stringset del_set;
      if ( !firstvote )
      {
        for ( stringset::const_iterator iter = commons.begin(); iter != commons.end(); iter++ )
        {
          // is candidate in current vote?
          if ( cand_set.find ( *iter ) == cand_set.end() )
          {
            del_set.insert ( *iter );
          }
        }
        cand_set.clear();
        for ( stringset::const_iterator iter = del_set.begin(); iter != del_set.end(); iter++ )
        {
          commons.erase ( *iter );
        }
      }
      firstvote = false;
      c = ' ';
    }
  }
  source.close();
}

long write_commons ( string sourcefile, string cutfile, stringset &commons )
{
//      printf("Writing common candidates\n");
  ifstream source ( sourcefile.data() );
  if ( source == NULL )
  {
    cerr << "Cannot open source file.\n";
    return -1;
  }
  ofstream cut ( cutfile.data() );
  if ( cut == NULL )
  {
    cerr << "Cannot create cut election file.\n";
    source.close();
    return -2;
  }
  char c = ' '; // current char read
  string cand; // current candidate
  stringset::const_iterator iter;
  while ( !source.eof() )
  {
    // Read one ranking
    bool firstcand = true;
    char sepchar = '|';
    while ( ( c != '\n' ) && ( !source.eof() ) )
    {
      // Read one candidate
      cand = "";
      source.get ( c );
      while ( ( c != '>' ) && ( c != '^' ) && ( c != '\n' ) && ( !source.eof() ) )
      {
        cand += c;
        source.get ( c );
      }
      iter = commons.find ( cand );
      if ( iter != commons.end() )
      {
        if ( !firstcand )
          cut << sepchar;
        sepchar = '|';
        cut << cand;
        firstcand = false;
        // this is to provide correct relations, when cutting candidates ( a=b>c=d --> a>d when cutting b and c )
        if ( ( c != '\n' ) && ( !source.eof() ) )
        {
          if ( sepchar != '>' )
            sepchar = c;
          c = ' ';
        }
      }
      else
      {
        if ( ( c != '\n' ) && ( !source.eof() ) )
        {
          if ( sepchar != '>' )
            sepchar = c;
          c = ' ';
        }
      }

    }
    if ( ( !source.eof() ) )
    {
      cut.put ( c );
      c = ' ';
    }
  }
  source.close();
  cut.close();
  string weightfilename = sourcefile + ".weight";
  string cutweightfilename = cutfile + ".weight";
  bool readCand = false; // else we read the weight
  bool dontWriteNextWeight = false;
  if ( FileExists ( weightfilename ) )
  {
    ifstream source ( weightfilename.data() );
    if ( source == NULL )
    {
      cerr << "Cannot open weight source file.\n";
      return -1;
    }
    ofstream cut ( cutweightfilename.data() );
    if ( cut == NULL )
    {
      cerr << "Cannot create cutted weight file.\n";
      return -3;
    }
    while ( !source.eof() )
    {
      source >> cand;
      readCand = !readCand;
      if ( readCand )
      {
        iter = commons.find ( cand );
        if ( iter != commons.end() )
        {
          cut << cand; // writing anonymized candidate name
          dontWriteNextWeight = false;
        }
        else
        {
          dontWriteNextWeight = true;
        }
      }
      else
      {
        if ( !dontWriteNextWeight )
        {
          cut << " " << cand << endl; // writing weight
        }
      }
    }
  }
  source.close();
  cut.close();
}


long write_clean ( string sourcefile, string cleanfile )
{
  ifstream source ( sourcefile.data() );
  if ( source == NULL )
  {
    cerr << "Cannot open source file.\n";
    return -1;
  }
  ofstream clean ( cleanfile.data() );
  if ( clean == NULL )
  {
    cerr << "Cannot create clean election file.\n";
    source.close();
    return -2;
  }
  char c = ' '; // current char read
  string cand; // current candidate
  stringset cand_set;
  stringset::const_iterator iter;
  while ( !source.eof() )
  {
    // Read one ranking
    while ( ( c != '\n' ) && ( !source.eof() ) )
    {
      // Read one candidate
      cand = "";
      source.get ( c );
      while ( ( c != '>' ) && ( c != '^' ) && ( c != '\n' ) && ( !source.eof() ) )
      {
        cand += c;
        source.get ( c );
      }
      iter = cand_set.find ( cand );
      if ( iter == cand_set.end() )
      {
        clean << cand;
        if ( ( c != '\n' ) && ( !source.eof() ) )
        {
          clean.put ( c );
          c = ' ';
        }
        cand_set.insert ( cand );
      }

    }
    if ( ( !source.eof() ) )
    {
      clean.put ( c );
      c = ' ';
      cand_set.clear();
    }
  }
  source.close();
  clean.close();
}

void votesmanagement::putMathProgSourceCode ( ostream& out )
{
  this->analyzeSubScore();
  out << "#\n";
  out << "# Kemeny Score Problem\n";
  out << "#\n";
  out << "# This finds the optimal kemeny score for the given instance.\n";

  out << "/* sets */\n";
  out << "set CANDS;\n";

  out << "\n/* parameters */\n";
  out << "param Costs{i in CANDS, j in CANDS};\n";

  out << "\n/* descision variables */\n";
  // Use binary for solution guarantee
  //out << "var x {i in CANDS, j in CANDS} binary >= 0;\n";
  //out << "var x {i in CANDS, j in CANDS} >= 0;\n";
  out << "var x {i in CANDS, j in CANDS} binary;\n";

  out << "\n/* objective function */\n";
  out << "minimize z: sum{i in CANDS, j in CANDS} Costs[i,j] * x[i,j];\n";

  out << "\n/* constrains */\n";
  out << "/* pairs */\n";
  int constNumber = 1;
  //out << "pairs{i in CANDS, j in CANDS } : x[i,j] + x[j,i] == 1;\n";
  for ( int i = 1; i <= this->candidateVector.size(); i++ )
  {
    for ( int j = i + 1; j <= this->candidateVector.size(); j++ )
    {
      out << "pair" << constNumber++ << ": x[" << i << "," << j << "] + x[" << j << "," << i << "] >= 1;\n";
    }
  }
  out << "/* triples */\n";
  //out << "triple{a in CANDS, b in CANDS, c in CANDS} : x[a,b] + x[b,c] + x[c,a] >= 1;\n";
  for ( int a = 1; a <= this->candidateVector.size(); a++ )
  {
    for ( int b = 1; b <= this->candidateVector.size(); b++ )
    {
      for ( int c = 1; c <= this->candidateVector.size(); c++ )
      {
        if ( ! ( ( a == b ) || ( a == c ) || ( b == c ) ) )
        {
          out << "triple" << constNumber++ << ": x[" << a << "," << b << "] + x[" << b << "," << c << "] + x[" << c << "," << a << "] >= 1;\n";
        }
      }
    }
  }

  out << "\n/* data section */\n";
  out << "data;\n";

  out << "\n/* sets data */\n";
  out << "set CANDS :=";
  for ( int i = 0; i < this->candidateVector.size(); i++ )
  {
    out << " " << ( i + 1 );
  }
  out << ";\n";

  out << "\n/* parameters data */\n";
  out << "param Costs:";
  for ( int i = 0; i < this->candidateVector.size(); i++ )
  {
    out << "\t" << ( i + 1 );
  }
  out << ":=\n";
  for ( int i = 0; i < this->candidateVector.size(); i++ )
  {
    out << "" << ( i + 1 );
    for ( int j = 0; j < this->candidateVector.size(); j++ )
    {
      out << "\t" << this->subscore ( this->candidateVector.at ( i ), this->candidateVector.at ( j ) );
      if ( ( i == this->candidateVector.size() - 1 ) && ( j == this->candidateVector.size() - 1 ) )
      {
        out << ";";
      }
    }
    out << "\n";
  }
  out << "\nend;\n";
}

void votesmanagement::putLpSourceCode ( ostream& out )
{
  this->analyzeSubScore();
  out << "\n";
  out << "\\* Kemeny Score Problem*\\ \n";
  out << "\n";
  out << "\\* This finds the optimal kemeny score for the given instance.*\\\n";

  int linepaircounter = 0;

  out << "Minimize\n";
  out << " z:";
  for ( int a = 1; a <= this->candidateVector.size(); a++ )
  {
    for ( int b = 1; b <= this->candidateVector.size(); b++ )
    {
      if ( a != b )
      {
        long unsigned int scoreab = this->subscore ( this->candidateVector.at ( a - 1 ), this->candidateVector.at ( b - 1 ) );
        if ( scoreab > 1 )
        {
          out << " + " << scoreab << " x(" << a << "," << b << ")";
        }
        else
          if ( scoreab > 0 )
          {
            out << " + " << "x(" << a << "," << b << ")";
          }
        linepaircounter++;
        if ( linepaircounter > 5 )
        {
          out << "\n";
          linepaircounter = 0;
        }
      }
    }
  }

  out << "\nSubject To\n";
  int constNumber = 1;
  for ( int i = 1; i <= this->candidateVector.size(); i++ )
  {
    for ( int j = i + 1; j <= this->candidateVector.size(); j++ )
    {
      out << " pair" << constNumber++ << ": + x(" << i << "," << j << ") + x(" << j << "," << i << ") >= 1\n";
    }
  }
  for ( int a = 1; a <= this->candidateVector.size(); a++ )
  {
    for ( int b = 1; b <= this->candidateVector.size(); b++ )
    {
      for ( int c = 1; c <= this->candidateVector.size(); c++ )
      {
        if ( ! ( ( a == b ) || ( a == c ) || ( b == c ) ) )
        {
          out << " triple" << constNumber++ << ": + x(" << a << "," << b << ") + x(" << b << "," << c << ") + x(" << c << "," << a << ") >= 1\n";
        }
      }
    }
  }


  out << "\nBounds\n";
  for ( int a = 1; a <= this->candidateVector.size(); a++ )
  {
    for ( int b = 1; b <= this->candidateVector.size(); b++ )
    {
      if ( a != b )
      {
        out << " 0 <= x(" << a << "," << b << ") <= 1\n";
      }
    }
  }

  out << "\nGenerals\n";
  for ( int a = 1; a <= this->candidateVector.size(); a++ )
  {
    for ( int b = 1; b <= this->candidateVector.size(); b++ )
    {
      if ( a != b )
      {
        out << " x(" << a << "," << b << ")\n";
      }
    }
  }

  out << "\nEnd\n";
}

void votesmanagement::putTournamentInput ( ostream& out )
{
 this->analyzeSubScore();
 out << this->getCandidatesCount();
 out << "\n";
 
  for ( int i = 0; i < this->candidateVector.size(); i++ )
  {
    for ( int j = 0; j <= i; j++ )
    {
      long int diff_ij=0;
      if ( i != j )
      {
        long unsigned int scoreij = this->subscore ( this->candidateVector.at ( i ), this->candidateVector.at ( j ) );
        long unsigned int scoreji = this->subscore ( this->candidateVector.at ( j ), this->candidateVector.at ( i ) );
        diff_ij = scoreij - scoreji;
      }
      out << "   " << diff_ij;
    }
    out << "\n";
  }
 
}

int deanon_preferenceString ( string sourcestring, stringmap &amap, string &deststring )
{
  stringstream anon;
  anon << sourcestring;
  anon.seekg ( 0 );
  stringstream dest;

  char c = ' '; // current char read
  string cand; // current candidate
  string cand_deanon; // current candidate anonymized
  stringmap::const_iterator iter;
  while ( !anon.eof() )
  {
    // Read one ranking
    while ( ( c != '\n' ) && ( !anon.eof() ) )
    {
      // Read one candidate
      cand = "";
      anon.get ( c );
      while ( ( c != '>' ) && ( c != '^' ) && ( c != '\n' ) && ( !anon.eof() ) )
      {
        cand += c;
        anon.get ( c );
      }
      iter = amap.find ( cand );
      if ( iter != amap.end() )
      {
        cand_deanon = ( *iter ).second;
        dest << cand_deanon;
      }
      else
        if ( cand.length() > 0 )
        {
          cerr << "Cannot find dictionary entry for \"" + cand + "\"\n";
          return -1;
        }
      if ( ( c != '\n' ) && ( !anon.eof() ) )
      {
        dest.put ( c );
        c = ' ';
      }
    }
    if ( ( !anon.eof() ) )
    {
      dest.put ( c );
      c = ' ';
    }
  }
  deststring = dest.str();
}

float votesmanagement::getEmpericalProbability (CandidatePositionMap &consensusMap)
{
  if (consensusMap.size()==0)
  {
    return -1.;
  }
  float probSum = 0.;
  for( VoteVectorIterator v = votes.begin(); v != votes.end(); v++)
  {
    probSum += getPairwiseProbabilityWrtRefereceVector( (*v).asList, consensusMap );
  }
  return ( probSum / getVotesCount() * 100.);
}
