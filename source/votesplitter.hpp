/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2010 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ header file of votesplitter
 *   Description: Analyses an election file.
 *              - various data reduction rules are implemented
 *              - if there are independent subsets of candidates split election into independent parts
 *              - solve independent parts (if possible in parallel)
 *              - joins independent consensus and sum up kscore to build total solution
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

#ifndef VOTESPLITTER_H
#define VOTESPLITTER_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

// project internal
#include "votetyps.hpp"
#include "votesmanagement.hpp"

const int bestChoiceMethod_smallestInstance = 0;
const int bestChoiceMethod_thirdOfInstance = 3;
const int bestChoiceMethod_halfOfInstance = 2;
const int bestChoiceMethod_biggestInstance = 1000;

class votesplitter : public votesmanagement
{
public:
  votesplitter();
  ~votesplitter();

  // condorcet weak and strict Rules
  int split_instances_RuleCondorcet_exhaustive ( bool strictBetter );
  int split_instances_RuleCondorcet ( bool strictBetter );
  bool split_instance_RuleCondorcet ( bool strictBetter, const CandidateSetS &instanceToSplitCandidates );
  long find_CondorcetCandidate ( bool strictBetter, const CandidateSetS &instanceToSplitCandidates );
  double last_RuleCondorcet_runningtime;
  double exhaustive_RuleCondorcet_runningtime;

  // condorcet components
  int split_instance_RuleCondorcetComponents();
  
  // condorcet Sets
  int split_instances_RuleCondorcetSets_exhaustive ( int maxSetSize, int bestChoiceMethod, bool strictBetter );
  int split_instances_RuleCondorcetSets ( int maxSetSize, int bestChoiceMethod, bool strictBetter );
  void split_instance_RuleCondorcetSets ( const CandidateSetS &instanceToSplitCandidates, int maxSetSize, int bestChoiceMethod, bool strictBetter );
  double last_RuleCondorcetSets_runningtime;
  double exhaustive_RuleCondorcetSets_runningtime;
  int condorcetSetsCount();
  void printCondorcetSets();
  void printCondorcetSetsSizes();

  // weighted Candidate Set
  int extract_instances_RuleWeightedCandidateSet_exhaustive ( int maxSetSize, int bestChoiceMethod );
  int extract_instances_RuleWeightedCandidateSet ( int maxSetSize, int bestChoiceMethod );
  void extract_instance_RuleWeightedCandidateSet ( const CandidateSetS &instanceToSplitCandidates, int maxSetSize, int bestChoiceMethod );
  double last_RuleWeightedCandidateSet_runningtime;
  double exhaustive_RuleWeightedCandidateSet_runningtime;
  int weightedSetsCount();
  void printWeightedSets();
  void printWeightedSetsSizes();

  // majorityNonDirty Candidates
  int split_instances_RuleMajorityNonDirtyCandidates_exhaustive ( bool condorcetCandidatesRuleActive );
  int split_instances_RuleMajorityNonDirtyCandidates ( bool condorcetCandidatesRuleActive );
  void split_instance_RuleMajorityNonDirtyCandidates ( const CandidateSetS &instanceToSplitCandidates );
  double last_RuleMajorityNonDirtyCandidates_runningtime;
  double exhaustive_RuleMajorityNonDirtyCandidates_runningtime;

  // majorityNonDirtySets
  int split_instances_RuleMajorityNonDirtySets_exhaustive ( int maxSetSize, int bestChoiceMethod, bool condorcetCandidatesRuleActive, bool nondortyCandidatesRuleActive );
  int split_instances_RuleMajorityNonDirtySets ( int maxSetSize, int bestChoiceMethod, bool condorcetCandidatesRuleActive, bool nondortyCandidatesRuleActive );
  void split_instance_RuleMajorityNonDirtySets ( int maxSetSize, const CandidateSetS &instanceToSplitCandidates, int bestChoiceMethod );
  double last_RuleMajorityNonDirtySets_runningtime;
  double exhaustive_RuleMajorityNonDirtySets_runningtime;
  void sort_majorityNonDirtySets();
  int majorityNonDirtySetsCount();
  void printMajorityNonDirtySets();
  void printMajorityNonDirtySetsSizes();

  // heuristikal splitting
  void heuristikalSplitTotal ( int maxSetSize, int bestChoiceMethod, bool strictBetter, bool ruleNonDirtyCandidates, bool ruleCondorcetCandidates, bool ruleNonDirtySets, bool ruleCondorcetSets, bool ruleCondorcetComponents );
  double last_heuristikal_runningtime;

  // splitting Instances Tool Function
  void clearSplits();
  long subscoreToSuccessors ( int instanceCandidateSetId );
  void writeInstanceToFile ( int instanceCandidateSetId, string newFilename );

  /** Decrepaced function that writes down all subinstances to Files.
  @param originalFilename Filename of the original Instance.
                          Subinstances will be named is [originalFilname].REDUCED.split[subinstance number]*/
  void writeInstancesToFiles ( string originalFilename );

  /** Function that writes down all subinstances to Files.
  @param originalFilename Filename of the original Instance.
                          Subinstances will be named is [originalFilname].REDUCED.split[subinstance number]
  @param subinstances     Filenames of the subinstances.*/
  void writeInstancesToFiles ( string originalFilename, vector<string> &subinstances );

  // common attributes
  string getPrintInstancesCandidateSets();
  string getPrintInstancesCandidateSetSizes();
  size_t getInstancesCount();
  size_t getInstancesMaxSize();
  double getInstancesAvgSize();
  // attributes for dynamic programming
  long double getInstances2ToTheCandidateSetSizesSum();
  long double getInstances2ToTheCandidateSetSizesSum_relationToUnreduced();
  // attributes for search trees
  InstancesSizes getInstancesNumberOfConflictPairsVector();
  unsigned long getInstancesNumberOfConflictPairsSum ( InstancesSizes &numberOfConflictPairsVector );
  double getInstancesPercentageConflictPairsReduced ( InstancesSizes &numberOfConflictPairsVector );
  long double getInstancesSizesOfTrivialSearchTreeSum ( InstancesSizes &numberOfConflictPairsVector );
  long double getInstancesSizesOfTrivialSearchTreeSum_relationToUnreduced ( InstancesSizes &numberOfConflictPairsVector );
  string getRulesSequenceprint();

protected:
  // majorityNonDirtySets
  bool isLastMajorityNonDirtySetsVectorSorted;
  CandidateSetVector lastMajorityNonDirtySetsProposes;

  // condorcet Sets
  CandidateSetVector lastCondorcetFrontSetsProposes;
  CandidateSetVector lastCondorcetBackSetsProposes;

  // weighted Candidates
  CandidateSetVector weightedSets;

  // splitting Rules
  CandidateSetVector instancesCandidateSets;

private:
  stringstream rulesSequence;

  // tool function for splits
  // updates bestSet to the "best" splitting set reffering to bestChoiceMethod
  // returns true, if bestSet was updated
  // if bestSet has Size == 0, and setsToChoice has Size > 0, updateChoice will always find a better (notempty) set
  bool updateChoice ( CandidateSetS &bestSet, int bestChoiceMethod, CandidateSetSet &setsToChoice, const CandidateSetS &instanceToSplitCandidates );
  bool updateChoice ( CandidateSetS &bestSet, bool &bestIsFromSet1, int bestChoiceMethod, CandidateSetSet &setsToChoice1, CandidateSetSet &setsToChoice2, const CandidateSetS &instanceToSplitCandidates );
};

/*
  This object can be used to sort a CandidateSet Vector so that the order is consistent with the raltions set.
  We only have to use this as comparator object with the std::sort function.
*/
class comperator_candidateSetByRelationsSet
{
public:
  bool operator() ( const CandidateSetS &a, const CandidateSetS &b );
  CandidatePairSet relationsSet;
};

ReductionQuality getReductionQuality ( votesplitter &vs, double reductionTime, int calls );
void printReductionQuality ( ReductionQuality &rq, const string &rulename );
void printReductionQuality ( ReductionQuality &rq, const string &rulename, ostream &out );
void printReductionQuality ( ReductionQuality &rq );
void printReductionQuality ( ReductionQuality &rq, ostream &out, const string &cellSeperator );


#endif
