/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2010 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ implementation file of votesplitter
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

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <deque>
#include <iterator>


// boost graph
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/strong_components.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/topological_sort.hpp>

// project internal
#include "errors.hpp"
#include "tools.hpp"
#include "votesmanagement.hpp"
#include "votesplitter.hpp"
#include "votetyps.hpp"
#include "jobtyps.hpp"

votesplitter::votesplitter()
{
  this->isSynchronized = false;
  this->isDirtysAnalyzed = false;
  this->isSubScoreAnalyzed = false;
  this->majority = 0.0;
  this->hasTies = false;
  this->isMajorityAnalyzed = false;
  this->isMajorityCandidatesVectorSorted = false;
  this->isLastMajorityNonDirtySetsVectorSorted = false;
}

votesplitter::~votesplitter()
{
}

int votesplitter::split_instances_RuleMajorityNonDirtySets_exhaustive ( int maxSetSize, int bestChoiceMethod, bool condorcetCandidatesRuleActive, bool nondortyCandidatesRuleActive )
{
  clock_t clock1, clock2;
  clock1 = clock();

  int lastWin = 1;
  int appliedCount = 0;
  while ( lastWin > 0 )
  {
    lastWin = this->split_instances_RuleMajorityNonDirtySets ( maxSetSize, bestChoiceMethod, condorcetCandidatesRuleActive, nondortyCandidatesRuleActive );
    appliedCount++;
  }

  clock2 = clock();
  this->exhaustive_RuleMajorityNonDirtySets_runningtime = ( ( double ) ( clock2 - clock1 ) ) / CLOCKS_PER_SEC;

  return appliedCount;
}

int votesplitter::split_instances_RuleMajorityNonDirtySets ( int maxSetSize, int bestChoiceMethod, bool condorcetCandidatesRuleActive, bool nondortyCandidatesRuleActive )
{
  clock_t clock1, clock2;
  clock1 = clock();

  int minReasonableSetSize;
  if ( nondortyCandidatesRuleActive )
  {
    minReasonableSetSize = 6;
  }
  else
    if ( condorcetCandidatesRuleActive )
    {
      minReasonableSetSize = 5;
    }
    else
    {
      minReasonableSetSize = 2;
    }

  int oldInstancesCount = 0;
  if ( this->instancesCandidateSets.size() < 1 )
  {
    CandidateSetS initialSet;
    initialSet.set = this->candidateSet;
    transferWeights ( this->candidateWeights, initialSet );
    initialSet.instance_prototyp = 0;
    this->instancesCandidateSets.push_back ( initialSet );
  }

  CandidateSetVector oldInstances = this->instancesCandidateSets;
  oldInstancesCount = oldInstances.size();

  this->instancesCandidateSets.clear();
  for ( CandidateSetVectorIterator cs = oldInstances.begin(); cs != oldInstances.end(); cs++ )
  {
    int maxRealSubsetSize = ( int ) ( *cs ).set.size() - 1;
    if ( maxRealSubsetSize > minReasonableSetSize )
    {
      this->split_instance_RuleMajorityNonDirtySets ( min ( maxSetSize, maxRealSubsetSize ), *cs, bestChoiceMethod );
    }
    else
    {
      this->instancesCandidateSets.push_back ( *cs );
    }
  }

  clock2 = clock();
  this->last_RuleMajorityNonDirtySets_runningtime = ( ( double ) ( clock2 - clock1 ) ) / CLOCKS_PER_SEC;

  return this->instancesCandidateSets.size() - oldInstancesCount;
}

void votesplitter::split_instance_RuleMajorityNonDirtySets ( int maxSetSize, const CandidateSetS &instanceToSplitCandidates, int bestChoiceMethod )
{
  CandidateSetSet ndSets;

  for ( CandidateSetIterator c1 = instanceToSplitCandidates.set.begin(); c1 != instanceToSplitCandidates.set.end(); c1++ )
  {
    CandidateSet nonD;
    //cout << "Starting with " << *c1 << "\n";
    nonD.insert ( *c1 );
    bool nonD_changed = true;
    bool c1SetSmallEnough = true;
    while ( nonD_changed && c1SetSmallEnough )
    {
      nonD_changed = false;
      for ( CandidateSetIterator x = instanceToSplitCandidates.set.begin(); x != instanceToSplitCandidates.set.end(); x++ )
      {
        // *x is a candidate in C \ nonD
        if ( nonD.find ( *x ) == nonD.end() )
        {
          CandidatePair c1x, xc1;
          c1x.lower = *c1;
          c1x.upper = *x;
          xc1.lower = *x;
          xc1.upper = *c1;
          bool isXsuccessor = ( this->majorityPairs.find ( c1x ) != this->majorityPairs.end() );
          bool isXpredecessor = ( this->majorityPairs.find ( xc1 ) != this->majorityPairs.end() );
          if ( ( !isXpredecessor ) && ( !isXsuccessor ) )
          {
            nonD.insert ( *x );
            nonD_changed = true;
            break;
          }
          else
            if ( ( isXpredecessor ) && ( isXsuccessor ) )
            {
              cerr << "Something went very wrong!\n";
            }
            else
            {
              for ( CandidateSetIterator nd = nonD.begin(); nd != nonD.end(); nd++ )
              {
                CandidatePair ndx, xnd;
                ndx.lower = *nd;
                ndx.upper = *x;
                xnd.lower = *x;
                xnd.upper = *nd;
                if ( isXsuccessor )
                {
                  if ( this->majorityPairs.find ( ndx ) == this->majorityPairs.end() )
                  {
                    nonD.insert ( *x );
                    if ( nonD.size() > maxSetSize )
                    {
                      c1SetSmallEnough = false;
                    }
                    nonD_changed = true;
                    break;
                  }
                }
                else
                {
                  if ( this->majorityPairs.find ( xnd ) == this->majorityPairs.end() )
                  {
                    nonD.insert ( *x );
                    if ( nonD.size() > maxSetSize )
                    {
                      c1SetSmallEnough = false;
                    }
                    nonD_changed = true;
                    break;
                  }
                }
              }
              if ( nonD_changed )
                break;
            }
        }
      }
    }
    //}
    //cout << "Got the non-Dirty Set: ";
    if ( nonD.size() <= maxSetSize )
    {
      //cout << "Get small Set of Size " << nonD.size() << ":\n";
      //printSet(nonD);

      CandidateSetS nonDS;
      nonDS.set = nonD;
      nonDS.instance_prototyp = *c1;
      ndSets.insert ( nonDS );
      this->lastMajorityNonDirtySetsProposes.push_back ( ( nonDS ) );
    }
  }

  CandidateSetS bestNDSSet;
  updateChoice ( bestNDSSet, bestChoiceMethod, ndSets, instanceToSplitCandidates );

  if ( bestNDSSet.set.size() > 0 )
  {
    CandidateSetS preNDSSet, postNDSSet;
    for ( CandidateSetIterator c = instanceToSplitCandidates.set.begin(); c != instanceToSplitCandidates.set.end(); c++ )
    {
      if ( bestNDSSet.set.find ( *c ) == bestNDSSet.set.end() )
      {
        if ( voteRate ( *c, * ( bestNDSSet.set.begin() ) ) < 0.5 )
        {
          postNDSSet.set.insert ( *c );
        }
        else
        {
          preNDSSet.set.insert ( *c );
        }
      }
    }
    
    if ( preNDSSet.set.size() > 0 )
    {
      transferWeights ( instanceToSplitCandidates, preNDSSet );
      preNDSSet.instance_prototyp = 0;
      this->instancesCandidateSets.push_back ( preNDSSet );
    }
    if ( bestNDSSet.set.size() > 0 )
    {
      transferWeights ( instanceToSplitCandidates, bestNDSSet );
      bestNDSSet.instance_prototyp = 0;
      this->instancesCandidateSets.push_back ( bestNDSSet );
    }
    if ( postNDSSet.set.size() > 0 )
    {
      transferWeights ( instanceToSplitCandidates, postNDSSet );
      postNDSSet.instance_prototyp = 0;
      this->instancesCandidateSets.push_back ( postNDSSet );
    }
  }
  else
  {
    this->instancesCandidateSets.push_back ( instanceToSplitCandidates );
  }

  this->isLastMajorityNonDirtySetsVectorSorted = false;
}

int votesplitter::split_instances_RuleCondorcetSets_exhaustive ( int maxSetSize, int bestChoiceMethod, bool strictBetter )
{
  clock_t clock1, clock2;
  clock1 = clock();

  int lastWin = 1;
  int appliedCount = 0;
  while ( lastWin > 0 )
  {
    lastWin = this->split_instances_RuleCondorcetSets ( maxSetSize, bestChoiceMethod, strictBetter );
    appliedCount++;
  }

  clock2 = clock();
  this->exhaustive_RuleCondorcetSets_runningtime = ( ( double ) ( clock2 - clock1 ) ) / CLOCKS_PER_SEC;

  return appliedCount;
}

int votesplitter::split_instances_RuleCondorcetSets ( int maxSetSize, int bestChoiceMethod, bool strictBetter )
{
  clock_t clock1, clock2;
  clock1 = clock();

  int oldInstancesCount = 0;
  if ( this->instancesCandidateSets.size() < 1 )
  {
    CandidateSetS initialSet;
    initialSet.set = this->candidateSet;
    transferWeights ( this->candidateWeights, initialSet );
    initialSet.instance_prototyp = 0;
    this->instancesCandidateSets.push_back ( initialSet );
  }

  CandidateSetVector oldInstances = this->instancesCandidateSets;
  oldInstancesCount = oldInstances.size();

  this->instancesCandidateSets.clear();
  for ( CandidateSetVectorIterator cs = oldInstances.begin(); cs != oldInstances.end(); cs++ )
  {
    int maxRealSubsetSize = ( int ) ( *cs ).set.size() - 1;
    if ( maxRealSubsetSize > 0 )
    {
      this->split_instance_RuleCondorcetSets ( *cs, min ( maxRealSubsetSize , maxSetSize ), bestChoiceMethod, strictBetter );
    }
    else
    {
      this->instancesCandidateSets.push_back ( *cs );
    }
  }

  clock2 = clock();
  this->last_RuleCondorcetSets_runningtime = ( ( double ) ( clock2 - clock1 ) ) / CLOCKS_PER_SEC;

  return this->instancesCandidateSets.size() - oldInstancesCount;
}

void updateMaxSetSize ( int &maxSetSize, int currSetSize, int originalInstanceSize, int bestChoiceMethod )
{
  if ( bestChoiceMethod == bestChoiceMethod_smallestInstance )
  {
    maxSetSize = currSetSize - 1;
  }
  else
    if ( bestChoiceMethod == bestChoiceMethod_halfOfInstance )
    {
      if ( currSetSize > originalInstanceSize / 2 )
      {
        maxSetSize = currSetSize;
      }
    }
    else
      if ( bestChoiceMethod == bestChoiceMethod_thirdOfInstance )
      {
        if ( currSetSize > originalInstanceSize / 3 )
        {
          maxSetSize = currSetSize;
        }
      }
}

void votesplitter::split_instance_RuleCondorcetSets ( const CandidateSetS &instanceToSplitCandidates, int maxSetSize, int bestChoiceMethod, bool strictBetter )
{
  unsigned long n = this->getVotesCount();
  CandidateSetSet cdFSets;
  CandidateSetSet cdBSets;

  // Doing Concorcet Set Reduction recursively
  cdFSets.clear();
  cdBSets.clear();
  // Fist the Front Sets:
  for ( CandidateSetIterator c1 = instanceToSplitCandidates.set.begin(); c1 != instanceToSplitCandidates.set.end(); c1++ )
  {
    CandidateSet cD;
    // cout << "Starting with " << *c1 << "\n";
    cD.insert ( *c1 );
    bool cD_changed = true;
    bool c1SetSmallEnough = true;
    while ( cD_changed && c1SetSmallEnough )
    {
      cD_changed = false;
      for ( CandidateSetIterator x = instanceToSplitCandidates.set.begin(); x != instanceToSplitCandidates.set.end(); x++ )
      {
        // *x is a candidate in C \ cD
        if ( cD.find ( *x ) == cD.end() )
        {
          bool allInFrontToX = true;
          for ( CandidateSetIterator ci = cD.begin(); ci != cD.end(); ci++ )
          {
            if ( voteRate ( *ci, *x ) <= 0.5 )
            {
              if ( strictBetter )
              {
                allInFrontToX = false;
              }
              else
                if ( voteRate ( *ci, *x ) < 0.5 )
                {
                  allInFrontToX = false;
                }
            }
          }
          if ( !allInFrontToX )
          {
            // So without x, cD cannot be ja condorcet (front) Set
            cD.insert ( *x );
            if ( cD.size() > maxSetSize )
            {
              c1SetSmallEnough = false;
            }
            cD_changed = true;
            break;
          }
        }
      }
    }
    //cout << "Got the condorcet Front Set: ";
    if ( cD.size() <= maxSetSize )
    {
      CandidateSetS cDS;
      cDS.set = cD;
      cDS.instance_prototyp = *c1;
      cdFSets.insert ( cDS );
      updateMaxSetSize ( maxSetSize, cD.size(), instanceToSplitCandidates.set.size(), bestChoiceMethod );
    }
  }
  // Now the Back Sets:
  for ( CandidateSetIterator c1 = instanceToSplitCandidates.set.begin(); c1 != instanceToSplitCandidates.set.end(); c1++ )
  {
    CandidateSet cD;
    //cout << "Starting with " << *c1 << "\n";
    cD.insert ( *c1 );
    bool cD_changed = true;
    bool c1SetSmallEnough = true;
    while ( cD_changed && c1SetSmallEnough )
    {
      cD_changed = false;
      for ( CandidateSetIterator x = instanceToSplitCandidates.set.begin(); x != instanceToSplitCandidates.set.end(); x++ )
      {
        // *x is a candidate in C \ cD
        if ( cD.find ( *x ) == cD.end() )
        {
          bool allInBackOffX = true;
          for ( CandidateSetIterator ci = cD.begin(); ci != cD.end(); ci++ )
          {
            if ( voteRate ( *ci, *x ) <= 0.5 )
            {
              if ( strictBetter )
              {
                allInBackOffX = false;
              }
              else
                if ( voteRate ( *ci, *x ) < 0.5 )
                {
                  allInBackOffX = false;
                }
            }
          }
          if ( !allInBackOffX )
          {
            // So without x, cD cannot be ja condorcet (back) Set
            cD.insert ( *x );
            if ( cD.size() > maxSetSize )
            {
              c1SetSmallEnough = false;
            }
            cD_changed = true;
            break;
          }
        }
      }
    }
    //cout << "Got the condorcet Back Set: ";
    if ( cD.size() <= maxSetSize )
    {
      CandidateSetS cDS;
      cDS.set = cD;
      cDS.instance_prototyp = *c1;
      cout << "foundSomething\n";
      cdBSets.insert ( cDS );
      //updateMaxSetSize ( maxSetSize, cD.size(), instanceToSplitCandidates.set.size(), bestChoiceMethod );
    }
  }

  // find best Condorcet Sets in current loop
  CandidateSetS bestSet;
  bool bestSetIsWinnerSet;

  this->updateChoice ( bestSet, bestSetIsWinnerSet, bestChoiceMethod, cdFSets, cdBSets, instanceToSplitCandidates );

  // manage selected sets
  CandidateSetS restSet;
  addSet1MinusSet2ToSet3 ( instanceToSplitCandidates.set, bestSet.set, restSet.set );

  // building up split sets
  if ( bestSetIsWinnerSet )
  {
    if ( bestSet.set.size() > 0 )
    {
      transferWeights ( instanceToSplitCandidates, bestSet );
      bestSet.instance_prototyp = 0;
      this->instancesCandidateSets.push_back ( bestSet );
    }
  }
  if ( restSet.set.size() > 0 )
  {
    transferWeights ( instanceToSplitCandidates, restSet );
    restSet.instance_prototyp = 0;
    this->instancesCandidateSets.push_back ( restSet );
  }
  if ( !bestSetIsWinnerSet )
  {
    if ( bestSet.set.size() > 0 )
    {
      transferWeights ( instanceToSplitCandidates, bestSet );
      bestSet.instance_prototyp = 0;
      this->instancesCandidateSets.push_back ( bestSet );
    }
  }
}

int votesplitter::split_instance_RuleCondorcetComponents ()
{
  adjacency_list <> G(this->candidateSet.size());
 
  for ( CandidateSetIterator x = this->candidateSet.begin(); x != this->candidateSet.end(); x++ )
  {
    for ( CandidateSetIterator y = this->candidateSet.begin(); y != this->candidateSet.end(); y++ )
    {
      if ( *x != *y )
      {
        double xyScoreRate = voteRate ( *x, *y );
	if (xyScoreRate > 1./2.)
	{
	  add_edge((*x)-1,(*y)-1,G);
	}
      }
    }
  }
    vector<int> component(num_vertices(G)), discover_time(num_vertices(G));
    vector<default_color_type> color(num_vertices(G));
    typedef graph_traits<adjacency_list<vecS, vecS, directedS> >::vertex_descriptor Vertex;
    vector<Vertex> root(num_vertices(G));
    int numComponents = strong_components(G, make_iterator_property_map(component.begin(), get(vertex_index, G)), 
                              root_map(make_iterator_property_map(root.begin(), get(vertex_index, G))).
                              color_map(make_iterator_property_map(color.begin(), get(vertex_index, G))).
                              discover_time_map(make_iterator_property_map(discover_time.begin(), get(vertex_index, G))));
    
//     // Print the results.
//     cout << "Total number of components: " << numComponents << std::endl;
//     for ( CandidateSetIterator x = this->candidateSet.begin(); x != this->candidateSet.end(); x++ )
//     {
//       std::cout << "Vertex " << *x <<" is in component " << component[(*x)-1] << std::endl;
//     }

    // Perform a topological sort.
    typedef adjacency_list< vecS, vecS, directedS > compGraph;
    compGraph compG(numComponents);
    pair<graph_traits<adjacency_list<vecS, vecS, directedS> >::edge_iterator, graph_traits<adjacency_list<vecS, vecS, directedS> >::edge_iterator> edgeIteratorRange = edges(G);
    for(graph_traits<adjacency_list<vecS, vecS, directedS> >::edge_iterator edgeIterator = edgeIteratorRange.first; edgeIterator != edgeIteratorRange.second; ++edgeIterator)
    {
      if (component[source(*edgeIterator, G)] != component[target(*edgeIterator, G)])
      {
        add_edge(component[source(*edgeIterator, G)], component[target(*edgeIterator, G)],compG);
      }
    }

    std::deque<int> topo_order;
    boost::topological_sort(compG, std::front_inserter(topo_order));
    
//     // Print the results.
//     cout << "A topological ordering: ";
//     for(std::deque<int>::const_iterator i = topo_order.begin(); i != topo_order.end(); ++i)
//     {
//         cout << "comp" << *i << ",";
//     }
//     cout << "\n";

    
    // build up candidate sets
    vector<CandidateSetS> compCandidatesSets(numComponents);
    for ( CandidateSetIterator x = this->candidateSet.begin(); x != this->candidateSet.end(); x++ )
    {
      int compXid=component[(*x)-1];
      compCandidatesSets[compXid].set.insert(*x);
      transferWeights ( this->candidateWeights, compCandidatesSets[compXid] );
      compCandidatesSets[compXid].instance_prototyp = 0;
    }
    
    for(std::deque<int>::const_iterator i = topo_order.begin(); i != topo_order.end(); ++i)
    {
        this->instancesCandidateSets.push_back(compCandidatesSets.at(*i));
    }
    
  return 1; 
}

bool votesplitter::updateChoice ( CandidateSetS &bestSet, int bestChoiceMethod, CandidateSetSet &setsToChoice, const CandidateSetS &instanceToSplitCandidates )
{
  // TODO: Evtl, implement weights for the instanceToSplitCandidates.set, sinnvoll?
  bool updatedBestSet = false;
  int instanceSizeDivisor;
  switch ( bestChoiceMethod )
  {
  case bestChoiceMethod_smallestInstance:
    instanceSizeDivisor = instanceToSplitCandidates.set.size();
    break;
  case bestChoiceMethod_halfOfInstance:
    instanceSizeDivisor = 2;
    break;
  case bestChoiceMethod_thirdOfInstance:
    instanceSizeDivisor = 3;
    break;
  case bestChoiceMethod_biggestInstance:
    instanceSizeDivisor = 1;
    break;
  default:
    instanceSizeDivisor = instanceToSplitCandidates.set.size();
    break;
  }

  for ( CandidateSetSetIterator cds = setsToChoice.begin(); cds != setsToChoice.end(); cds++ )
  {
    this->lastCondorcetFrontSetsProposes.push_back ( ( *cds ) );
    long destBest = ( long ) bestSet.set.size() - ( ( long ) instanceToSplitCandidates.set.size() / ( long ) instanceSizeDivisor );
    long destCds = ( long ) ( *cds ).set.size() - ( ( long ) instanceToSplitCandidates.set.size() / ( long ) instanceSizeDivisor );
    if ( bestSet.set.size() == 0 || ( destCds*destCds < destBest*destBest ) )
    {
      bestSet = ( *cds );
      updatedBestSet = true;
    }
  }
  return updatedBestSet;
}

bool votesplitter::updateChoice ( CandidateSetS &bestSet, bool &bestIsFromSet1, int bestChoiceMethod, CandidateSetSet &setsToChoice1, CandidateSetSet &setsToChoice2, const CandidateSetS &instanceToSplitCandidates )
{
  bool set1Update = this->updateChoice ( bestSet, bestChoiceMethod, setsToChoice1, instanceToSplitCandidates );
  bool set2Update = this->updateChoice ( bestSet, bestChoiceMethod, setsToChoice2, instanceToSplitCandidates );
  bestIsFromSet1 = set1Update && !set2Update;
  return set1Update || set2Update;
}

int votesplitter::extract_instances_RuleWeightedCandidateSet_exhaustive ( int maxSetSize, int bestChoiceMethod )
{
  clock_t clock1, clock2;
  clock1 = clock();

  int lastWin = 1;
  int appliedCount = 0;
  while ( lastWin > 0 )
  {
    lastWin = this->extract_instances_RuleWeightedCandidateSet ( maxSetSize, bestChoiceMethod );
    appliedCount++;
  }

  clock2 = clock();
  this->exhaustive_RuleWeightedCandidateSet_runningtime = ( ( double ) ( clock2 - clock1 ) ) / CLOCKS_PER_SEC;

  return appliedCount;
}

int votesplitter::extract_instances_RuleWeightedCandidateSet ( int maxSetSize, int bestChoiceMethod )
{
  clock_t clock1, clock2;
  clock1 = clock();

  int oldInstancesCount = 0;
  if ( this->instancesCandidateSets.size() < 1 )
  {
    CandidateSetS initialSet;
    initialSet.set = this->candidateSet;
    transferWeights ( this->candidateWeights, initialSet );
    initialSet.instance_prototyp = 0;
    this->instancesCandidateSets.push_back ( initialSet );
  }

  CandidateSetVector oldInstances = this->instancesCandidateSets;
  oldInstancesCount = oldInstances.size();

  this->instancesCandidateSets.clear();
  for ( CandidateSetVectorIterator cs = oldInstances.begin(); cs != oldInstances.end(); cs++ )
  {
    int maxRealSubsetSize = ( int ) ( *cs ).set.size() - 1;
    if ( maxRealSubsetSize > 0 )
    {
      this->extract_instance_RuleWeightedCandidateSet ( *cs, min ( maxRealSubsetSize , maxSetSize ), bestChoiceMethod );
    }
    else
    {
      this->instancesCandidateSets.push_back ( *cs );
    }
  }

  clock2 = clock();
  this->last_RuleWeightedCandidateSet_runningtime = ( ( double ) ( clock2 - clock1 ) ) / CLOCKS_PER_SEC;

  return this->instancesCandidateSets.size() - oldInstancesCount;
}

void votesplitter::extract_instance_RuleWeightedCandidateSet ( const CandidateSetS &instanceToSplitCandidates, int maxSetSize, int bestChoiceMethod )
{
  CandidateSetSet wdSets;

  for ( CandidateSetIterator c1 = instanceToSplitCandidates.set.begin(); c1 != instanceToSplitCandidates.set.end(); c1++ )
  {
    CandidateSet wD;
    CandidateSetIterator c1t = c1;
    wD.insert ( *c1 );
    for ( CandidateSetIterator c2 = ++c1t; c2 != instanceToSplitCandidates.set.end(); c2++ )
    {
      wD.insert ( *c2 );
      //cout << "Starting with " << *c1 << "and" << *c2 << "\n";
      bool wD_changed = true;
      while ( wD_changed )
      {
        wD_changed = false;
        bool rateAllEqual = true;
        for ( CandidateSetIterator y = instanceToSplitCandidates.set.begin(); y != instanceToSplitCandidates.set.end(); y++ )
        {
          // *y is a candidate in C \ wD
          if ( wD.find ( *y ) == wD.end() )
          {
            double c1rate = voteRate ( *c1, *y );
            for ( CandidateSetIterator ci = wD.begin(); ci != wD.end(); ci++ )
            {
              // if the rate of y with every candidate in wD has to be the same
              // otherwise we try to add y to the Set of potential weighted Candidates
              if ( voteRate ( *ci, *y ) != c1rate )
              {
                rateAllEqual = false;
                break;
              }
            }
          }
          if ( !rateAllEqual )
          {
            // So without x, cD cannot be ja condorcet (front) Set
            wD.insert ( *y );
            wD_changed = true;
            break;
          }
        }
      }
    }
    //cout << "Got the weighted Set: ";
    if ( ( wD.size() > 1 ) && ( wD.size() <= maxSetSize ) )
    {
      CandidateSetS wDS;
      wDS.set = wD;
      wDS.instance_prototyp = *c1;
      wdSets.insert ( wDS );
    }
  }

  CandidateSetS bestWeightedSet;
  updateChoice ( bestWeightedSet, bestChoiceMethod, wdSets, instanceToSplitCandidates );

  /*for ( CandidateSetSetIterator wds = wdSets.begin(); wds != wdSets.end(); wds++ )
  {
      this->weightedSets.push_back ( ( *wds ) );
      if ( bestWeightedSet.set.size() == 0 || ( abs ( ( long ) ( bestWeightedSet.set.size() *3 - this->condorcetSetsCount() ) ) > abs ( ( long ) ( ( *wds ).set.size() *3 - this->condorcetSetsCount() ) ) ) )
      {
          bestWeightedSet = ( *wds );
      }
  }*/

  CandidateSetS weightedInstanceSet = instanceToSplitCandidates;
  Weight weightSum = 0;
  Candidate prototyp = 0;
  for ( CandidateSetIterator c = bestWeightedSet.set.begin(); c != bestWeightedSet.set.end(); c++ )
  {
    if ( c != bestWeightedSet.set.begin() )
    {
      weightedInstanceSet.set.erase ( *c );
    }
    else
    {
      prototyp = *c;
    }
    Weight cWeight = getWeight ( weightedInstanceSet.notEqualOneWeighted, *c, 1 );
    weightSum = weightSum + cWeight;
    weightedInstanceSet.notEqualOneWeighted.erase ( *c );
  }
  if ( prototyp != 0 )
  {
    weightedInstanceSet.notEqualOneWeighted.insert ( CandidateWeightMapData ( prototyp, weightSum ) );
  }

  transferWeights ( instanceToSplitCandidates, bestWeightedSet );
  bestWeightedSet.instance_prototyp = prototyp;

  this->instancesCandidateSets.push_back ( weightedInstanceSet );
  if ( bestWeightedSet.set.size() > 0 )
  {
    this->instancesCandidateSets.push_back ( bestWeightedSet );
  }
}

int votesplitter::split_instances_RuleCondorcet_exhaustive ( bool strictBetter )
{
  clock_t clock1, clock2;
  clock1 = clock();

  int lastWin = 1;
  int appliedCount = 0;
  while ( lastWin > 0 )
  {
    lastWin = this->split_instances_RuleCondorcet ( strictBetter );
    appliedCount++;
  }

  clock2 = clock();
  this->exhaustive_RuleCondorcet_runningtime = ( ( double ) ( clock2 - clock1 ) ) / CLOCKS_PER_SEC;

  return appliedCount;
}

int votesplitter::split_instances_RuleCondorcet ( bool strictBetter )
{
  clock_t clock1, clock2;
  clock1 = clock();

  if ( this->instancesCandidateSets.size() < 1 )
  {
    CandidateSetS initialSet;
    initialSet.set = this->candidateSet;
    transferWeights ( this->candidateWeights, initialSet );
    initialSet.instance_prototyp = 0;
    this->instancesCandidateSets.push_back ( initialSet );
  }

  CandidateSetVector oldInstances = this->instancesCandidateSets;
  int oldInstancesCount = oldInstances.size();

  this->instancesCandidateSets.clear();
  for ( CandidateSetVectorIterator cs = oldInstances.begin(); cs != oldInstances.end(); cs++ )
  {
    int maxRealSubsetSize = ( int ) ( *cs ).set.size() - 1;
    if ( maxRealSubsetSize > 0 )
    {
      this->split_instance_RuleCondorcet ( strictBetter, *cs );
    }
    else
    {
      this->instancesCandidateSets.push_back ( *cs );
    }
  }

  clock2 = clock();
  this->last_RuleCondorcet_runningtime = ( ( double ) ( clock2 - clock1 ) ) / CLOCKS_PER_SEC;

  return this->instancesCandidateSets.size() - oldInstancesCount;
}

bool votesplitter::split_instance_RuleCondorcet ( bool strictBetter, const CandidateSetS &instanceToSplitCandidates )
{
  long condorcetCandidate = this->find_CondorcetCandidate ( strictBetter, instanceToSplitCandidates );

  if ( condorcetCandidate == 0 )
  {
    this->instancesCandidateSets.push_back ( instanceToSplitCandidates );
    return false;
  }
  else
  {
    // condorcet winner or looser?
    bool isCondorcetLooser = false;
    if ( condorcetCandidate < 0 )
    {
      isCondorcetLooser = true;
      condorcetCandidate = condorcetCandidate * ( -1 );
    }

    // condorcet candidate set..
    CandidateSetS condorcetCandidateSet;
    condorcetCandidateSet.set.insert ( condorcetCandidate );
    condorcetCandidateSet.instance_prototyp = 0;
    transferWeights ( instanceToSplitCandidates, condorcetCandidateSet );

    // reduced instance set..
    CandidateSetS reducedInstanceSet = instanceToSplitCandidates;
    reducedInstanceSet.set.erase ( condorcetCandidate );
    if ( reducedInstanceSet.instance_prototyp == condorcetCandidate )
    {
      reducedInstanceSet.instance_prototyp = 0;
    }
    transferWeights ( instanceToSplitCandidates, reducedInstanceSet );

    if ( !isCondorcetLooser )
    {
      this->instancesCandidateSets.push_back ( condorcetCandidateSet );
    }
    if ( reducedInstanceSet.set.size() > 0 )
    {
      this->instancesCandidateSets.push_back ( reducedInstanceSet );
    }
    if ( isCondorcetLooser )
    {
      this->instancesCandidateSets.push_back ( condorcetCandidateSet );
    }
  }

  return true;
}

long votesplitter::find_CondorcetCandidate ( bool strictBetter, const CandidateSetS &instanceToSplitCandidates )
{
  for ( CandidateSetIterator x = instanceToSplitCandidates.set.begin(); x != instanceToSplitCandidates.set.end(); x++ )
  {
    //bool xWasCondorcetWinner = ( this->majRedFront.find ( *x ) != this->majRedFront.end() );
    //bool xWasCondorcetLooser = ( this->majRedBack.find ( *x ) != this->majRedBack.end() );
    bool xIsNewCondorcetLooser = true;
    bool xIsNewCondorcetWinner = true;
    //cout << "\nCandidate " << *x << ": ";
    //if ( ! ( xWasCondorcetLooser ) && ! ( xWasCondorcetWinner ) )
    //{
    for ( CandidateSetIterator y = instanceToSplitCandidates.set.begin(); y != instanceToSplitCandidates.set.end(); y++ )
    {
      if ( *x != *y )
      {
        bool yWasF = ( this->majRedFront.find ( *y ) != this->majRedFront.end() );
        bool yWasB = ( this->majRedBack.find ( *y ) != this->majRedBack.end() );
        double xyScoreRate = voteRate ( *x, *y );
        double yxScoreRate = voteRate ( *y, *x );
        //cout << xyScoreRate << ",";
        //cout << yxScoreRate << " ";
        if ( ( strictBetter && ( xyScoreRate  >= 0.5 ) ) || ( ( xyScoreRate  > 0.5 ) ) )
        {
          if ( ! ( yWasB ) )
          {
            xIsNewCondorcetLooser = false;
          }
        }
        if ( ( strictBetter && ( yxScoreRate  >= 0.5 ) ) || ( ( yxScoreRate  > 0.5 ) ) )
        {
          if ( ! ( yWasF ) )
          {
            xIsNewCondorcetWinner = false;
          }
        }
      }
    }
    if ( xIsNewCondorcetWinner )
      return *x;
    if ( xIsNewCondorcetLooser )
      return *x * ( -1 );
  }
  //}
  return 0;
}

int votesplitter::split_instances_RuleMajorityNonDirtyCandidates_exhaustive ( bool condorcetCandidatesRuleActive )
{
  clock_t clock1, clock2;
  clock1 = clock();

  int lastWin = 1;
  int appliedCount = 0;
  while ( lastWin > 0 )
  {
    lastWin = this->split_instances_RuleMajorityNonDirtyCandidates ( condorcetCandidatesRuleActive );
    appliedCount++;
  }

  clock2 = clock();
  this->exhaustive_RuleMajorityNonDirtyCandidates_runningtime = ( ( double ) ( clock2 - clock1 ) ) / CLOCKS_PER_SEC;

  return appliedCount;
}

int votesplitter::split_instances_RuleMajorityNonDirtyCandidates ( bool condorcetCandidatesRuleActive )
{
  clock_t clock1, clock2;
  clock1 = clock();

  int minReasonableSetSize;
  if ( condorcetCandidatesRuleActive )
  {
    minReasonableSetSize = 5;
  }
  else
  {
    minReasonableSetSize = 2;
  }

  int oldInstancesCount = 0;
  if ( this->instancesCandidateSets.size() < 1 )
  {
    CandidateSetS initialSet;
    initialSet.set = this->candidateSet;
    transferWeights ( this->candidateWeights, initialSet );
    initialSet.instance_prototyp = 0;
    this->instancesCandidateSets.push_back ( initialSet );
  }

  CandidateSetVector oldInstances = this->instancesCandidateSets;
  oldInstancesCount = oldInstances.size();

  this->instancesCandidateSets.clear();
  for ( CandidateSetVectorIterator cs = oldInstances.begin(); cs != oldInstances.end(); cs++ )
  {
    int maxRealSubsetSize = ( int ) ( *cs ).set.size() - 1;
    if ( maxRealSubsetSize >= minReasonableSetSize )
    {
      this->split_instance_RuleMajorityNonDirtyCandidates ( *cs );
    }
    else
    {
      this->instancesCandidateSets.push_back ( *cs );
    }
  }

  clock2 = clock();
  this->last_RuleMajorityNonDirtyCandidates_runningtime = ( ( double ) ( clock2 - clock1 ) ) / CLOCKS_PER_SEC;

  return this->instancesCandidateSets.size() - oldInstancesCount;
}

void votesplitter::split_instance_RuleMajorityNonDirtyCandidates ( const CandidateSetS &instanceToSplitCandidates )
{
  if ( ( !this->isMajorityAnalyzed ) || ( this->majorityInstanceCandidates != instanceToSplitCandidates.set ) || ( this->getMajority() < 3. / 4. ) )
  {
    this->analyzeMajority ( 3. / 4., instanceToSplitCandidates.set );
  }
  if ( !this->isMajorityCandidatesVectorSorted )
  {
    this->sortMajorityNonDirtyCandidates();
  }

  int instancedCount = this->majorityCandidates.size() * 2 + 1;
  CandidateSet instances[instancedCount];
  for ( int i = 1; i < instancedCount; i = i + 2 )
  {
    instances[i].insert ( this->majorityCandidates.at ( ( i - 1 ) / 2 ) );
  }
  for ( CandidateSetIterator c = instanceToSplitCandidates.set.begin(); c != instanceToSplitCandidates.set.end(); c++ )
  {
    // Is candidate majority dirty?
    if ( this->majorityDirtyCandidates.find ( *c ) != this->majorityDirtyCandidates.end() )
    {
      // Find position in splitted instance
      int firstNonDirtySuccessor = 0;
      CandidatePair compPair;
      compPair.upper = *c;
      for ( int i = 0; i < this->majorityCandidates.size(); i++ )
      {
        compPair.lower = this->majorityCandidates.at ( i );
        if ( this->majorityPairs.find ( compPair ) != this->majorityPairs.end() )
        {
          firstNonDirtySuccessor = i + 1;
        }
        else
        {
          break;
        }
      }
      instances[2*firstNonDirtySuccessor].insert ( *c );
    }
  }
  for ( int i = 0; i < instancedCount; i++ )
  {
    if ( instances[i].size() > 0 )
    {
      CandidateSetS newSet;
      newSet.set = instances[i];
      newSet.instance_prototyp = 0;
      this->instancesCandidateSets.push_back ( newSet );
    }
  }

  bool isSplitted = true;
}

void votesplitter::printCondorcetSets()
{
  printf ( "\nFront: " );
  for ( CandidateSetVectorIterator cs = this->lastCondorcetFrontSetsProposes.begin(); cs != this->lastCondorcetFrontSetsProposes.end(); cs++ )
  {
    if ( cs != this->lastCondorcetFrontSetsProposes.begin() )
    {
      printf ( ",{" );
    }
    else
    {
      printf ( "{" );
    }
    printCandidateSet ( ( *cs ).set, cout, ",", "}" );
  }
  printf ( "\nBack: " );
  for ( CandidateSetVectorIterator cs = this->lastCondorcetBackSetsProposes.begin(); cs != this->lastCondorcetBackSetsProposes.end(); cs++ )
  {
    if ( cs != this->lastCondorcetBackSetsProposes.begin() )
    {
      printf ( ",{" );
    }
    else
    {
      printf ( "{" );
    }
    printCandidateSet ( ( *cs ).set, cout, ",", "}" );
  }
  printf ( "\n" );
}

void votesplitter::printMajorityNonDirtySets()
{
  if ( ( !this->isMajorityAnalyzed ) || ( this->getMajority() < 3. / 4. ) )
    this->analyzeMajority ( 3. / 4. );
  if ( !this->isMajorityCandidatesVectorSorted )
    this->sortMajorityNonDirtyCandidates();
  this->sort_majorityNonDirtySets();
  for ( CandidateSetVectorIterator cs = this->lastMajorityNonDirtySetsProposes.begin(); cs != this->lastMajorityNonDirtySetsProposes.end(); cs++ )
  {
    if ( cs != this->lastMajorityNonDirtySetsProposes.begin() )
    {
      printf ( ",{" );
    }
    else
    {
      printf ( "{" );
    }
    printCandidateSet ( ( *cs ).set, cout, ",", "}" );
  }
  printf ( "\n" );
}

void votesplitter::printWeightedSets()
{
  for ( CandidateSetVectorIterator ws = this->weightedSets.begin(); ws != this->weightedSets.end(); ws++ )
  {
    if ( ws != this->weightedSets.begin() )
    {
      printf ( ",{" );
    }
    else
    {
      printf ( "{" );
    }
    printCandidateSet ( ( *ws ).set, cout, ",", "}" );
  }
  printf ( "\n" );
}

int votesplitter::majorityNonDirtySetsCount()
{
  return this->lastMajorityNonDirtySetsProposes.size();
}

int votesplitter::weightedSetsCount()
{
  return this->weightedSets.size();
}

int votesplitter::condorcetSetsCount()
{
  return this->lastCondorcetFrontSetsProposes.size() + this->lastCondorcetBackSetsProposes.size();
}

void votesplitter::printMajorityNonDirtySetsSizes()
{
  for ( CandidateSetVectorIterator cs = this->lastMajorityNonDirtySetsProposes.begin(); cs != this->lastMajorityNonDirtySetsProposes.end(); cs++ )
  {
    if ( cs != this->lastMajorityNonDirtySetsProposes.begin() )
    {
      printf ( ", %zu", ( *cs ).set.size() );
    }
    else
    {
      printf ( "%zu", ( *cs ).set.size() );
    }
  }
  printf ( "\n" );
}

void votesplitter::printWeightedSetsSizes()
{
  for ( CandidateSetVectorIterator ws = this->weightedSets.begin(); ws != this->weightedSets.end(); ws++ )
  {
    if ( ws != this->weightedSets.begin() )
    {
      printf ( ", %zu", ( *ws ).set.size() );
    }
    else
    {
      printf ( "%zu", ( *ws ).set.size() );
    }
  }
  printf ( "\n" );
}

void votesplitter::printCondorcetSetsSizes()
{
  printf ( "\nFront: " );
  for ( CandidateSetVectorIterator cs = this->lastCondorcetFrontSetsProposes.begin(); cs != this->lastCondorcetFrontSetsProposes.end(); cs++ )
  {
    if ( cs != this->lastCondorcetFrontSetsProposes.begin() )
    {
      printf ( ", %zu", ( *cs ).set.size() );
    }
    else
    {
      printf ( "%zu", ( *cs ).set.size() );
    }
  }
  printf ( "\nBack: " );
  for ( CandidateSetVectorIterator cs = this->lastCondorcetBackSetsProposes.begin(); cs != this->lastCondorcetBackSetsProposes.end(); cs++ )
  {
    if ( cs != this->lastCondorcetBackSetsProposes.begin() )
    {
      printf ( ", %zu", ( *cs ).set.size() );
    }
    else
    {
      printf ( "%zu", ( *cs ).set.size() );
    }
  }
  printf ( "\n" );
}

void votesplitter::clearSplits()
{
  this->instancesCandidateSets.clear();
}

string votesplitter::getPrintInstancesCandidateSets()
{
  stringstream text;
  for ( int i = 0; i < this->instancesCandidateSets.size(); i++ )
  {
    if ( i > 0 )
    {
      if ( instancesCandidateSets[i].instance_prototyp != 0 )
      {
        text << " <<[" << instancesCandidateSets[i].instance_prototyp << "]<< ";
      }
      else
      {
        text << " > ";
      }

    }
    if ( instancesCandidateSets[i].set.size() > 1 )
    {
      text << "{";
    }
    for ( CandidateSetIterator c = this->instancesCandidateSets[i].set.begin(); c != this->instancesCandidateSets[i].set.end(); c++ )
    {
      if ( c != this->instancesCandidateSets[i].set.begin() )
      {
        text << ",";
      }
      Weight cWeight = getWeight ( instancesCandidateSets[i].notEqualOneWeighted, *c, 1 );
      if ( cWeight > 1 )
      {
        text << *c << "x" << cWeight;
      }
      else
      {
        text << *c;
      }
    }
    if ( instancesCandidateSets[i].set.size() > 1 )
    {
      text << "}";
    }
  }
  return text.str();
}

string votesplitter::getPrintInstancesCandidateSetSizes()
{
  stringstream text;
  text << "(";
  bool printed = false;
  int lastSingleCandidatesCount = 0;
  for ( int i = 0; i < this->instancesCandidateSets.size(); i++ )
  {
    if ( i == ( this->instancesCandidateSets.size() - 1 ) || this->instancesCandidateSets[i].set.size() > 1 )
    {
      if ( this->instancesCandidateSets[i].set.size() == 1 )
        lastSingleCandidatesCount++;
      if ( lastSingleCandidatesCount > 0 )
      {
        if ( printed )
          text << ")>(";
        text << "S" << lastSingleCandidatesCount;
        lastSingleCandidatesCount = 0;
        printed = true;
      }
      if ( this->instancesCandidateSets[i].set.size() > 1 )
      {
        if ( printed )
        {
          if ( instancesCandidateSets[i].instance_prototyp != 0 )
          {
            text << "<<";
          }
          else
          {
            text << ")>(";
          }
        }
        text << this->instancesCandidateSets[i].set.size();
        printed = true;
      }
    }
    else
    {
      if ( this->instancesCandidateSets[i].set.size() == 1 )
        lastSingleCandidatesCount++;
    }
  }
  text << ")";
  return text.str();
}

long votesplitter::subscoreToSuccessors ( int instanceCandidateSetId )
{
  long sc = 0;
  int instanceCandidateSetId_max = this->instancesCandidateSets.size();
  if ( instanceCandidateSetId < instanceCandidateSetId_max )
  {
    CandidateSetS &instanceCandidates = this->instancesCandidateSets[instanceCandidateSetId];
    for ( int i = instanceCandidateSetId + 1; i < instanceCandidateSetId_max; i++ )
    {
      CandidateSetS &otherCandidates = this->instancesCandidateSets[i];
      for ( CandidateSetIterator x = instanceCandidates.set.begin(); x != instanceCandidates.set.end(); x++ )
      {
        for ( CandidateSetIterator y = otherCandidates.set.begin(); y != otherCandidates.set.end(); y++ )
        {
          CandidatePair yx;
          yx.lower = *y;
          yx.upper = *x;
          //cout << *x << "," << *y << "\n";
          sc = sc + this->subscore ( yx );
        }
      }
    }
  }
  return sc;
}

void votesplitter::writeInstanceToFile ( int instanceCandidateSetId, string newFilename )
{
  //TODO write weight files if WEIGHTEDSET REDUCTION IS REIMPLEMENTED

  CandidateSetS &instanceCandidates = this->instancesCandidateSets[instanceCandidateSetId];
 
  ofstream ifile ( newFilename.data() );
  if ( ifile == NULL )
  {
    cerr << "Cannot create instance file.\n";
    exit ( ERROR_FILE_NOT_FOUND );
  }
  // write votes
  for ( VoteVectorIterator vote = this->votes.begin(); vote != this->votes.end(); vote++ )
  {
    bool wroteCandidateForThisVote = false;
    for ( CandidateListIterator candidate = ( *vote ).asList.begin(); candidate != ( *vote ).asList.end(); candidate++ )
    {
      if ( instanceCandidates.set.find ( *candidate ) != instanceCandidates.set.end() )
      {
        if ( wroteCandidateForThisVote )
          ifile << ">";
        ifile << *candidate;
        wroteCandidateForThisVote = true;
      }
    }
    ifile << "\n";
  }
  ifile.close();
}

void votesplitter::writeInstancesToFiles ( string originalFilename )
{
  vector<string> tempFilenames;
  this->writeInstancesToFiles ( originalFilename, tempFilenames );
}

void votesplitter::writeInstancesToFiles ( string originalFilename, vector< string >& subinstances )
{
  for ( int i = 0; i < this->instancesCandidateSets.size(); i++ )
  {
    string instanceFilename = originalFilename + ".split" + toStr ( i + 1 );
    string scoreFilename = originalFilename + ".split" + toStr ( i + 1 ) + ".scoreToSuccessors";
    string replaceFilename = originalFilename + ".split" + toStr ( i + 1 ) + ".replaceCandidate";

    this->writeInstanceToFile ( i, instanceFilename );
    subinstances.push_back ( instanceFilename );

    ofstream sifile ( scoreFilename.data() );
    ofstream rifile ( replaceFilename.data() );
    if ( this->instancesCandidateSets[i].instance_prototyp == 0 )
    {
      sifile << this->subscoreToSuccessors ( i ) << "\n";
      rifile << "0\n";
    }
    else
    {
      rifile << this->instancesCandidateSets[i].instance_prototyp << "\n";
      sifile << "0\n";
    }
    sifile.close();
    rifile.close();
  }

}

size_t votesplitter::getInstancesCount()
{
  return this->instancesCandidateSets.size();
}

size_t votesplitter::getInstancesMaxSize()
{
  size_t maxsize = 0;
  for ( int i = 0; i < this->instancesCandidateSets.size(); i++ )
  {
    if ( this->instancesCandidateSets[i].set.size() > maxsize )
    {
      maxsize = this->instancesCandidateSets[i].set.size();
    }
  }
  return maxsize;
}

double votesplitter::getInstancesAvgSize()
{
  double avgs = 0.;
  for ( int i = 0; i < this->instancesCandidateSets.size(); i++ )
  {
    avgs = avgs + ( double ) this->instancesCandidateSets[i].set.size() / ( double ) this->instancesCandidateSets.size();
  }
  return avgs;
}

long double votesplitter::getInstances2ToTheCandidateSetSizesSum()
{
  long double toTheSizesSum = 0.;
  for ( int i = 0; i < this->instancesCandidateSets.size(); i++ )
  {
    toTheSizesSum = toTheSizesSum + pow ( ( long double ) 2, ( long double ) this->instancesCandidateSets[i].set.size() );
  }
  return toTheSizesSum;
}

long double votesplitter::getInstances2ToTheCandidateSetSizesSum_relationToUnreduced()
{
  long double reduced = 0.;
  for ( int i = 0; i < this->instancesCandidateSets.size(); i++ )
  {
    reduced = reduced + pow ( 2, ( long double ) this->instancesCandidateSets[i].set.size() - ( long double ) this->candidateSet.size() ) * ( long double ) 100;
  }
  return reduced;
}

InstancesSizes votesplitter::getInstancesNumberOfConflictPairsVector()
{
  InstancesSizes numberOfConflictPairsPerInstance;
  for ( int i = 0; i < this->instancesCandidateSets.size(); i++ )
  {
    unsigned long conflPairs = 0;
    for ( CandidateSetIterator x = this->instancesCandidateSets[i].set.begin(); x != this->instancesCandidateSets[i].set.end(); x++ )
    {
      CandidateSetIterator xTemp = x;
      for ( CandidateSetIterator y = ++xTemp; y != this->instancesCandidateSets[i].set.end(); y++ )
      {
        if ( this->isDirty ( *x, *y ) )
        {
          conflPairs++;
        }
      }
    }
    numberOfConflictPairsPerInstance.push_back ( conflPairs );
  }
  return numberOfConflictPairsPerInstance;
}

unsigned long votesplitter::getInstancesNumberOfConflictPairsSum ( InstancesSizes &numberOfConflictPairsVector )
{
  unsigned long conflPairs = 0;
  for ( InstancesSizesIterator i = numberOfConflictPairsVector.begin(); i != numberOfConflictPairsVector.end(); i++ )
  {
    conflPairs = conflPairs + *i;
  }
  return conflPairs;
}

double votesplitter::getInstancesPercentageConflictPairsReduced ( InstancesSizes &numberOfConflictPairsVector )
{
  unsigned long conflPairs = 0;
  for ( InstancesSizesIterator i = numberOfConflictPairsVector.begin(); i != numberOfConflictPairsVector.end(); i++ )
  {
    conflPairs = conflPairs + *i;
  }
  unsigned long originalConflPairs = this->getDirtyPairsCount();
  unsigned long reducedConflictPairs =  originalConflPairs - conflPairs;
  return ( double ) reducedConflictPairs * 100. / ( double ) originalConflPairs;
}

long double votesplitter::getInstancesSizesOfTrivialSearchTreeSum ( InstancesSizes &numberOfConflictPairsVector )
{
  long double toTheSizesSum = 0.;
  for ( InstancesSizesIterator i = numberOfConflictPairsVector.begin(); i != numberOfConflictPairsVector.end(); i++ )
  {
    toTheSizesSum = toTheSizesSum + pow ( ( long double ) 2, ( long double ) * i );
  }
  return toTheSizesSum;
}

long double votesplitter::getInstancesSizesOfTrivialSearchTreeSum_relationToUnreduced ( InstancesSizes &numberOfConflictPairsVector )
{
  long double conflPairs = this->getDirtyPairsCount();
  long double toTheSizesSum = 0.;
  for ( InstancesSizesIterator i = numberOfConflictPairsVector.begin(); i != numberOfConflictPairsVector.end(); i++ )
  {
    toTheSizesSum = toTheSizesSum + pow ( ( long double ) 2, ( long double ) * i - conflPairs ) * ( long double ) 100;
  }
  return toTheSizesSum;
}

void votesplitter::sort_majorityNonDirtySets()
{
  if ( !this->isMajorityAnalyzed )
    cerr << "No majarity was anazyzed.\n";

  // sorting majorityNonDirty Sets according to the majority pairs
  comperator_candidateSetByRelationsSet mNDCSc;
  mNDCSc.relationsSet = this->majorityPairs;
  std::sort ( this->lastMajorityNonDirtySetsProposes.begin(), this->lastMajorityNonDirtySetsProposes.end(), mNDCSc );
  this->isLastMajorityNonDirtySetsVectorSorted = true;
}

bool comperator_candidateSetByRelationsSet::operator() ( const CandidateSetS &a, const CandidateSetS &b )
{
  CandidatePair ab;
  // since the majority ralation is equivalent in one nondirty Set to all other candidates,
  // we can check the relation by the comparision of one candidate of each set
  ab.lower = * ( a.set.begin() );
  ab.upper = * ( b.set.begin() );
  bool pairListed = ( this->relationsSet.find ( ab ) != this->relationsSet.end() );
  //cout << a << ">" << b << "? " << pairListed << "\n";
  return pairListed;
}

void votesplitter::heuristikalSplitTotal ( int maxSetSize, int bestChoiceMethod, bool strictBetter, bool ruleNonDirtyCandidates, bool ruleCondorcetCandidates, bool ruleNonDirtySets, bool ruleCondorcetSets, bool ruleCondorcetComponents )
{
  clock_t clock1, clock2;
  clock1 = clock();

  this->clearSplits();
  this->rulesSequence.clear();
  
  if(ruleCondorcetComponents)
  {
    int lastReductionWasSuccessfull;
    lastReductionWasSuccessfull = this->split_instance_RuleCondorcetComponents();
    this->rulesSequence << lastReductionWasSuccessfull << "*compC,";
  }
  else
  {
  CandidateSetS tmp;
  tmp.set = this->candidateSet;
  tmp.instance_prototyp = 0;
  this->instancesCandidateSets.push_back ( tmp );
  int lastReductionWasSuccessfull;
  bool jumpBack = true;
  while ( jumpBack )
  {
    jumpBack = false;
    if ( ruleNonDirtyCandidates )
    {
      lastReductionWasSuccessfull = this->split_instances_RuleMajorityNonDirtyCandidates_exhaustive ( ruleCondorcetCandidates );
      this->rulesSequence << lastReductionWasSuccessfull << "*nC,";
    }
    if ( jumpBack == false )
    {
      if ( ruleCondorcetCandidates )
      {
        lastReductionWasSuccessfull = this->split_instances_RuleCondorcet ( strictBetter );
        this->rulesSequence << lastReductionWasSuccessfull << "*cC,";
        if ( lastReductionWasSuccessfull > 0 )
          jumpBack = true;
      }
      if ( jumpBack == false )
      {
        if ( ruleNonDirtySets )
        {
          lastReductionWasSuccessfull = split_instances_RuleMajorityNonDirtySets ( maxSetSize, bestChoiceMethod, ruleCondorcetCandidates, ruleNonDirtyCandidates );
          this->rulesSequence << lastReductionWasSuccessfull << "*nS,";
          if ( lastReductionWasSuccessfull > 0 )
            jumpBack = true;
        }
        if ( jumpBack == false )
        {
          if ( ruleCondorcetSets )
          {
            lastReductionWasSuccessfull = split_instances_RuleCondorcetSets ( maxSetSize, bestChoiceMethod, strictBetter );
            this->rulesSequence << lastReductionWasSuccessfull << "*cS,";
            if ( lastReductionWasSuccessfull > 0 )
              jumpBack = true;
          }
          // TODO reactivate weighted sets rule when fitting instances available and tested successfully
          //if ( jumpBack = false )
          //{
          //    lastReductionWasSuccessfull = this->extract_instances_RuleWeightedCandidateSet ( maxSetSize, bestChoiceMethod );
          //    this->rulesSequence << lastReductionWasSuccessfull << "xWeightedSets,";
          //    if ( ( ruleNonDirtyCandidates || ruleCondorcetCandidates || ruleNonDirtySets || ruleCondorcetSets ) && lastReductionWasSuccessfull > 0 ) jumpBack = true;
          //}
        }
      }
    }
  }
  }
  clock2 = clock();
  this->last_heuristikal_runningtime = ( ( double ) ( clock2 - clock1 ) ) / CLOCKS_PER_SEC;
}

string votesplitter::getRulesSequenceprint()
{
  return this->rulesSequence.str();
}

ReductionQuality getReductionQuality ( votesplitter &vs, double reductionTime, int calls )
{
  ReductionQuality r;
  r.calls = calls;
  r.reductionTime = reductionTime;
  r.instancesCount = vs.getInstancesCount();
  r.maxInstanceSize = vs.getInstancesMaxSize();
  r.avgInstanceSize = vs.getInstancesAvgSize();
  r.sizeOfDynamicProgrammingTableSum = vs.getInstances2ToTheCandidateSetSizesSum();
  r.sizeOfDynamicProgrammingTableSum_relationToUnreduced = vs.getInstances2ToTheCandidateSetSizesSum_relationToUnreduced();
  r.numberOfConflictPairsVector = vs.getInstancesNumberOfConflictPairsVector();
  r.numberOfConflictPairsSum = vs.getInstancesNumberOfConflictPairsSum ( r.numberOfConflictPairsVector );
  r.percentageConflictPairsReduced = vs.getInstancesPercentageConflictPairsReduced ( r.numberOfConflictPairsVector );
  r.sizesOfTrivialSearchTreeSum = vs.getInstancesSizesOfTrivialSearchTreeSum ( r.numberOfConflictPairsVector );
  r.sizesOfTrivialSearchTreeSum_relationToUnreduced = vs.getInstancesSizesOfTrivialSearchTreeSum_relationToUnreduced ( r.numberOfConflictPairsVector );
  r.sizesPrint = vs.getPrintInstancesCandidateSetSizes();
  r.setsPrint = vs.getPrintInstancesCandidateSets();
  r.rulesSequencePrint = vs.getRulesSequenceprint();
  return r;
}

void printReductionQuality ( ReductionQuality &rq, const string &rulename )
{
  printReductionQuality ( rq, rulename, cout );
}

void printReductionQuality ( ReductionQuality &rq, const string &rulename, ostream &out )
{
  /*if ( rq.calls == 1 )
  {
      out << fixed << rulename << ": One single run took " << rq.reductionTime << " seconds.\n";
  }
  else
  {
      out << fixed << rulename << ": Exhaustive appliance (" << rq.calls << " runs) splitted the instance in " << rq.reductionTime << " seconds.\n";
  }*/
  out << rulename << ": Rules calling sequence: " << rq.rulesSequencePrint << "\n";
  out << rulename << ": Number of instances: " << rq.instancesCount << "\n";
  out << rulename << ": Maximal size of the instances: " << rq.maxInstanceSize << "\n";
  out << fixed << setprecision ( 2 ) << rulename << ": Average size of the instances: " << rq.avgInstanceSize << "\n";
  //out << fixed << setprecision ( 2 ) << rulename << ": Size of dynamic programming table: " << rq.sizeOfDynamicProgrammingTableSum << "\n";
  out << fixed << setprecision ( 2 ) << rulename << ": Size of dynamic programming table relation to unreduced instance: " << rq.sizeOfDynamicProgrammingTableSum_relationToUnreduced << "%\n";
  out << rulename << ": Total number of conflict pairs: " << rq.numberOfConflictPairsSum << "\n";
  out << fixed << setprecision ( 2 ) << rulename << ": Percentage of conflict pairs reduced: " << rq.percentageConflictPairsReduced << "%\n";
  //out << fixed << setprecision ( 2 ) << rulename << ": Size of the trivial search tree: " << rq.sizesOfTrivialSearchTreeSum << "\n";
  out << fixed << setprecision ( 2 ) << rulename << ": Size of the trivial search tree relation to unreduced instance: " << rq.sizesOfTrivialSearchTreeSum_relationToUnreduced << "%\n";
  out << rulename << ": Candidate sets sizes of the instances are: " << rq.sizesPrint << "\n";
  out << rulename << ": Candidate sets of the instances are: " << rq.setsPrint << "\n";
}

void printReductionQuality ( ReductionQuality &rq )
{
  printReductionQuality ( rq, cout, sepText );
}

void printReductionQuality ( ReductionQuality &rq, ostream &out, const string &cellSeperator )
{
  //out << fixed << rq.calls << cellSeperator;
  //out << fixed << rq.reductionTime << cellSeperator;
  out << fixed << rq.instancesCount << cellSeperator;
  out << fixed << rq.maxInstanceSize << cellSeperator;
  out << fixed << rq.avgInstanceSize << cellSeperator;
  //out << fixed << rq.sizeOfDynamicProgrammingTableSum << cellSeperator;
  out << fixed << rq.sizeOfDynamicProgrammingTableSum_relationToUnreduced << cellSeperator;
  out << fixed << rq.numberOfConflictPairsSum << cellSeperator;
  out << fixed << rq.percentageConflictPairsReduced << cellSeperator;
  //out << fixed << rq.sizesOfTrivialSearchTreeSum << cellSeperator;
  out << fixed << rq.sizesOfTrivialSearchTreeSum_relationToUnreduced << cellSeperator;
  out << rq.rulesSequencePrint << cellSeperator;
}
