/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2010 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ implementation file of kconsens_instance_bykemX
 *   Description: Class that realizes a fixed parameter algorithm for the Kemeny Score problem.
 *                The parameter is the kemeny score.
 *                The algorithm is a search tree algorithm that branches on dirty sets.
 *                Theoretical running time: O(1.51^parameter)
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
#include "kconsens_instance_bykemX.hpp"

using namespace std;
using namespace boost;

kconsens_instance_bykemX::kconsens_instance_bykemX ( int dirtySetSize, stringstream &outdebug ) : kconsens_instance_dirtypairs_based ( outdebug )
{
  this->isSynchronized = false;
  this->isDirtysAnalyzed = false;
  this->isSubScoreAnalyzed = false;
  this->dirtySetSize = dirtySetSize;
}

kconsens_instance_bykemX::~kconsens_instance_bykemX()
{
}

void kconsens_instance_bykemX::solve ( stringstream &outverbose, stringstream &outtable )
{
  if ( !this->isDirtysAnalyzed )
    this->analyzeDirtys();
  if ( !this->isSubScoreAnalyzed )
    this->analyzeSubScore();

  long k = this->min_k;
  unsigned long rule2_k;
  //this->rule1_prepare();
  this->rule2_prepare();

  bool consensusFromRule2 = false;
  unsigned long maxCount;
  rule2_k = this->rule2_check ( consensusFromRule2, maxCount );
  outverbose << "Maximal quantity of a unique vote: " << maxCount << ", score for this vote: " << rule2_k << "\n";
  if ( !consensusFromRule2 )
  {
    //k = max ( k, ( long ) maxCount );

    // initialize L and D
    this->initLD();
    // get the dirty sets

    this->Dsets = this->getDirtySets ( this->dirtySetSize );
    this->setDirtySetSequence ( this->Dsets );
    this->Ll.resize ( this->Dsets.size() + 1 );
    Ll[0] = CandidateCandidateRelationsMap ( this->L );
    this->kl.resize ( this->Dsets.size() + 1 );
    this->kl[0] = k;
    this->branches.resize ( this->Dsets.size() );
    this->branchesMax.resize ( this->Dsets.size() );

    outverbose << "Starting search tree with k=" << k << ".\n";
    this->solvek ( k, outverbose, outtable );
    if ( boost::this_thread::interruption_requested() )
    {
      this->consensScore = -1;
      return;
    }
  }
  else
  {
    outverbose << "Trivial consensus list is the maximum quantity vote.\n";
    k = rule2_k;
    this->consens.reverse();
  }
  this->consens.reverse();
  //this->rule1_reinsert();

  /*outverbose << "The " << this->candidateSet.size() << " ambigious candidates were:\n";
  printSet ( this->candidateSet, outverbose );*/
  this->consensScore = k;
}

bool kconsens_instance_bykemX::solvek ( long &k, stringstream &outverbose, stringstream &outtable )
{
  bool solved = false;
  while ( ! ( solved ) )
  {
    for ( int i = 0; i < Dsets.size(); i++ )
    {
      branches[i] = -1;
      branchesMax[i] = Dsets[i].permutations.size() - 1;
    }
    branches[0] = 0;
    // filter inconform (with non-dirty pairs informations) pairs out
    for ( int j = 0; j < Dsets[0].permutations.size(); j++ )
    {
      bool conf = ( this->isConform ( Dsets[0].permutations[j].permutation, Ll[0] ) );
      Dsets[0].permutations[j].conform = conf;
      // do not start with inconform permutaion in first level
      // this would cause an error in kl reset
      if ( ( !conf ) && ( branches[0] == j ) )
        branches[0]++;
    }
    bool lastBranch = false;
    int branchLevel = 0;
    int oldBranchLevel = -1;
    bool nextBranchInLevel = false;
    do
    {
      //printf("branchLevel=%d/%d\n",branchLevel,Dsets.size());
      //printL(Ll[branchLevel+1]);
      //printBranching(Dsets.size(),branches,branchesMax,kl[branchLevel+1]);
      if ( branchLevel == 0 )
      {
        Ll[1] = this->L;
        kl[1] = k;
        // filter inconform (with non-dirty pairs informations) pairs out
      }
      else
      {
        // if something changed in lower level
        if ( oldBranchLevel != branchLevel )
        {
          Ll[branchLevel+1] = Ll[branchLevel];
          kl[branchLevel+1] = kl[branchLevel];
        }
      }

      oldBranchLevel = branchLevel;
      bool branchOk = this->isConform ( Dsets[branchLevel].permutations[branches[branchLevel]].permutation, Ll[branchLevel+1] );
      if ( branchOk )
      {
        // subscores
        kl[branchLevel+1] = kl[branchLevel+1] - Dsets[branchLevel].permutations[branches[branchLevel]].subScore;

        // Done: check Dsets[branchLevel].scoreFromSuccessorsMin or Dsets[branchLevel + 1].scoreFromSuccessorsMin
        if ( kl[branchLevel+1] - Dsets[branchLevel].scoreFromSuccessorsMin < 0 )
        {
          //printf("k was reduces from %d to %d in level %d/%d\n",k,kl[branchLevel+1],branchLevel,);
          branches[branchLevel] = branchesMax[branchLevel];
          nextBranchInLevel = true;
        }
        else
        {
          //printf("current branch ok\n");
          this->addInfo ( Dsets[branchLevel].permutations[branches[branchLevel]].permutation, Ll[branchLevel+1] );
          //printf("Added Info about: ");
          //printVote(Dsets[branchLevel].permutations[branches[branchLevel]].permutation);
          if ( branchLevel < ( Dsets.size() - 1 ) )
          {
            //printf("next level\n");
            branchLevel = branchLevel + 1;
            nextBranchInLevel = true;
            // filter inconform (with already fixed pairs informations) pairs out
            if ( Dsets[branchLevel].candidates.size() == this->dirtySetSize )
            {
              for ( int j = 0; j < Dsets[branchLevel].permutations.size(); j++ )
              {
                if ( this->isConform ( Dsets[branchLevel].permutations[j].permutation, Ll[branchLevel] ) )
                {
                  Dsets[branchLevel].permutations[j].conform = true;
                }
                else
                {
                  Dsets[branchLevel].permutations[j].conform = false;
                  // start with conform
                  if ( branches[branchLevel] == j - 1 )
                    branches[branchLevel]++;
                }
              }
            }
            else
            {
              long minScore = -1;
              int bestPermId = -1;
              for ( int j = 0; j < Dsets[branchLevel].permutations.size(); j++ )
              {
                //Dsets[branchLevel].permutations[j].conform = false;
                if ( this->isConform ( Dsets[branchLevel].permutations[j].permutation, Ll[branchLevel] ) )
                {
                  if ( ( minScore > Dsets[branchLevel].permutations[j].subScore ) || ( minScore == -1 ) )
                  {
                    bestPermId = j;
                    minScore = Dsets[branchLevel].permutations[j].subScore;
                  }
                }
              }
              Dsets[branchLevel].permutations[bestPermId].conform = true;
              branches[branchLevel] = bestPermId - 1;
            }

          }
          else
          {
            //printL(Ll[branchLevel+1]);
            nextBranchInLevel = false;
            this->relationsToConsens ( this->consens, Ll[branchLevel+1] );

            //this->consens.reverse();
            //printVote(this->consens);
            //this->consens.reverse();

            lastBranch = true;
            return true;
            //nextBranchInLevel=true;
          }
        }
      }
      else
      {
        //printf("This was not conform: ");
        //printVote(Dsets[branchLevel].permutations[branches[branchLevel]].permutation);
        //kl[branchLevel+1] = kl[branchLevel];
        if ( Dsets[branchLevel].candidates.size() == this->dirtySetSize )
        {
          nextBranchInLevel = true;
          // TODO CHECK!!
          //kl[branchLevel+1] = kl[branchLevel];
        }
        else
        {
          //printf("beep");
          nextBranchInLevel = true;
          //lastBranch = true;
        }
        //if(branchOk)printf("k was reduces from %d to %d in level %d\n",k,kl[branchLevel+1],branchLevel);
      }

      while ( nextBranchInLevel )
      {
        if ( branches[branchLevel] < branchesMax[branchLevel] )
        {
          branches[branchLevel]++;
          nextBranchInLevel = !Dsets[branchLevel].permutations[branches[branchLevel]].conform;
        }
        else
        {
          if ( branchLevel == 0 )
          {
            lastBranch = true;
            nextBranchInLevel = false;
          }
          else
          {
            if ( boost::this_thread::interruption_requested() )
              return false;
            branches[branchLevel] = -1;
            branchLevel = branchLevel - 1;
            nextBranchInLevel = true;
          }
        }
      }
    }
    while ( !lastBranch );
    //printf("stop7\n");
    k++;
    outverbose << "Setting k=" << k << "\n";
  }
  return solved;
}
