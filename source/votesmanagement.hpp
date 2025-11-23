/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2010 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ header file of votesmanagement
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

#ifndef VOTESMANAGEMENT_H
#define VOTESMANAGEMENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <sstream>
#include <bits/algorithmfwd.h>

// project internal
#include "votetyps.hpp"

/**
 This class manages a permutation list.
 @author Robert Bredereck <RBredereck@web.de>
*/
class votesmanagement
{
public:
  // const & dest
  votesmanagement();
  ~votesmanagement();

  // minimal k where solution is possible - maybe algorithms can find out something
  unsigned long min_k;

  // important methods
  void loadFromFile ( string filename );

  // suggestions for the consensus
  void loadSuggestionsFromFile ( string filename );
  void testSuggestions ( bool verbose );

  // constrains
  void loadConstraintsFromFile ( string filename );
  void genSuggestionsForConstrains ( string filename );

  // weights
  void loadWeightsFromFile ( string filename );

  // additional information
  unsigned long getVotesCount();
  unsigned long getCandidatesCount();
  unsigned long getDirtyPairsCount();
  unsigned long getSuggestionsCount();

  // Reduction Rule Times
  double getDirtyPairAnalyzeTime();

  // dirty
  void printDirtyPairsBothDirections ( ostream &o );
  void printDirtyPairsBothDirections ( string &filename );
  void printDirtyPairsOneDirection ( ostream &o );
  void printDirtyPairsOneDirection ( string &filename );

  // majority
  double getMajority();
  void analyzeMajority ( double majority );
  void analyzeMajority ( double majority, const CandidateSet &instanceToSplitCandidates );
  double lastAnalyzeMajorityTime;
  CandidateSet majorityInstanceCandidates;
  unsigned long getMajorityPairsCount();
  CandidateSet majRedFront;
  CandidateSet majRedBack;
  void printMajorityPairs ( ostream &o );
  void printMajorityPairs ( string &filename );
  void printNonMajorityPairsBothDirections ( ostream &o );
  void printNonMajorityPairsBothDirections ( string &filename );
  void printNonMajorityPairsOneDirection ( ostream &o );
  void printNonMajorityPairsOneDirection ( string &filename );
  void printMajorityNonDirtyCandidates();
  int getMajorityNonDirtyCandidatesCount();
  void sortMajorityNonDirtyCandidates();
  
  // consensus probability a la Conitzer et al
  float getEmpericalProbability (CandidatePositionMap &consensusMap);
  
  // range
  unsigned long getMaxRange();

  // kt-distance
  double getAverageKTdist();

  // score range
  unsigned long getLowerScoreBound();
  unsigned long getUpperScoreBound();

  // suggestions
  SuggestionVector suggestions;

  // constrains
  CandidatePairSet constraints;

  // election contains ties
  bool hasTies;

  /** This function writes a MathProg source code that solves the kemeny score problem for this instance to a stream.
  @param out Reference to the output stream.
  */
  void putMathProgSourceCode ( ostream &out );

  /** This function writes a Cplex source code that solves the kemeny score problem for this instance to a stream.
   @param out Reference to the output stream.
   */
  void putLpSourceCode ( ostream &out );

  /** This procedure writes the tournament formulation of this instance to solve Kemeny as median order problem (via Charon-Hudry) to a stream.
   @param out Reference to the output stream.
   */
  void putTournamentInput ( ostream &out );
protected:
  // important private vars
  VoteVector votes;
  CandidateVector candidateVector;
  // candidate Weights for candidates a>b>...>c being ordered in this order in every vote
  CandidateWeightMap candidateWeights;
  CandidateSet candidateSet;
  CandidatePairWeightMap subScore;

  // internal data
  // data structures are synchronized?
  bool isSynchronized;
  // data analyzed for subscores
  bool isSubScoreAnalyzed;
  // data analyzed for dirty pairs?
  bool isDirtysAnalyzed;

  // all candidates involved in a dirty pair
  CandidateSet dirtyCandidateSet;
  // all candidates not involved in a dirty pair
  CandidateVector nonDirtyCandidateVector;

  // all dirty pairs assoziated with a (sequentially allocated) unique id [necessary for branching over certain pairs instead of divergation points]
  CandidatePairWeightMap dirtyPairMap;
  // all pairs not dirty and ordered like their ids
  CandidatePairSet pairOrderedAsId;

  // important internal methods
  void synchronizeDataTypes();
  double lastSynchonizeDataTypesTime;
  void analyzeSubScore();
  double lastAnalyzeSubScoreTime;

  // analyze all pairs if they are dirty
  void analyzeDirtys();
  double lastAnalyzeDirtysTime;

  // dirty based funtions
  // do the candidates "x" and "y" form a dirty pair?
  bool isDirty ( unsigned long x, unsigned long y );
  // is the pair "p" a dirty one?
  bool isDirty ( CandidatePair p );

  /** Compares the relative positions of a candidate pair with a vote.
  @param voteID Index of considered vote.
  @param p Ordered candidate pair.
  @return Returns 0 if p.upper>p.lower in the considered vote, returns 1 if p.lower>p.upper in considered vote, and else it returns -1 if p.lower was not found, -2 if p.upper was not found and -3 if both candidates were not found in the considered vote.
  **/
  int lowerThenUpper ( unsigned long voterID, CandidatePair &p );

  // compare positions
  // decrapaced
  int lowerThenUpper ( CandidatePositionMap &v, CandidatePair &p );
  int lowerThenUpper ( CandidateList &v, const CandidatePair &p );
  int lowerThenUpper ( CandidatePositionMap &v, CandidatePositionMapIterator &foundl, CandidatePositionMapIterator &foundu );
  int lowerThenUpper ( CandidateList &v, CandidateListIterator &foundl, CandidateListIterator &foundu );

  // candidate check functions
  bool isInvolved ( Candidate can, CandidateList &list );
  bool isInvolved ( Candidate can, CandidatePositionMap &map );
  bool isInvolved ( CandidatePair &p, CandidateList &list );
  bool isInvolved ( CandidatePair &p, CandidatePositionMap &map );

  // consistence checking
  bool agree ( CandidatePair &p, CandidateList &list );
  bool agree ( CandidatePair &p, CandidatePositionMap &map );
  bool agree ( CandidatePair &p, Vote &v );

  bool agree ( CandidatePairSet &Ps, CandidateList &list );
  bool agree ( CandidatePairSet &Ps, CandidatePositionMap &map );
  bool agree ( CandidatePairSet &Ps, Vote &v );

  // majorities
  bool isMajorityAnalyzed;
  double majority;
  CandidatePairSet majorityPairs;
  CandidatePairSet nonMajorityPairs;
  CandidateSet majorityDirtyCandidates;
  CandidateVector majorityCandidates;
  bool isMajorityCandidatesVectorSorted;

  double voteRate ( Candidate lower, Candidate upper );

  // max range
  unsigned long getRange ( Candidate can );

  // kt distance
  unsigned long ktdist ( int votea, int voteb );

  // ranking functions
  Position getPosition ( unsigned long voterId, Candidate can );

  // compute kemeny score for a consensus suggestion
  unsigned long subscore ( CandidatePair &p );
  unsigned long subscore ( Candidate lower, Candidate upper );
  unsigned long kscore ( CandidateList &consensus );

  string filenameOfLoadedElection;
private:
  void nextpos ( vector<int> &relpos );
  bool notlastpos ( vector<int> relpos );
};

/**
 This class can be used to sort a Candidate Vector so that the order is consistent with the raltions set.
 We only have to use this as comparator object with the std::sort function.
 @author Robert Bredereck <RBredereck@web.de>
*/
class comperator_candidatesByRelationsSet
{
public:
  bool operator() ( const Candidate &a, const Candidate &b );
  CandidatePairSet relationsSet;
};


/**
 This function converts an election file to an anonymized electionfile with integer values as candidate names.
 @author Robert Bredereck <RBredereck@web.de>
*/
long anon_and_map_sourcefile ( string sourcefile, stringmap &dmap, stringmap &amap, string dictfile, string anonfile );

int deanon_preferenceString ( string sourcestring, stringmap &amap, string &deststring );

/**
 This function builds up the set of candidates which belong to each vote.
 @author Robert Bredereck <RBredereck@web.de>
*/
long analyze_commons ( string sourcefile, stringset &commons );

/**
 Writes a new electionfile and filters out candidates which do not belong to each vote.
 @author Robert Bredereck <RBredereck@web.de>
*/
long write_commons ( string sourcefile, string cutfile, stringset &commons );

long write_clean ( string sourcefile, string cleanfile );
#endif
