/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2011 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ implementation file of jobmanagement classes
 *   Description: This file contains classes to manage the jobs (tasks) and threads.
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

#include <string>
#include <time.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <boost/thread.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/system/config.hpp>
#include <sys/time.h>

// project internal
#include "jobmanagement.hpp"
#include "jobtyps.hpp"
#include "votesmanagement.hpp"
#include "votesplitter.hpp"
#include "tools.hpp"
#include "kconsens_instance_bykemX.hpp"
#include "kconsens_instance_bycan.hpp"
#include "kconsens_instance_bycand.hpp"
#include "kconsens_instance_approx_pickpermrand.hpp"
#include "kconsens_instance_glpk.hpp"
#include "kconsens_instance_cplex.hpp"
#include "kconsens_instance_gurobi.hpp"
#include "kconsens_instance_charon_hudry.hpp"

using namespace std;
using namespace boost;

mutex fullspeed_core[4];

void InstanceSolver::operator() ()
{
  try
  {
    fullspeed_core[solveParams.coreID].lock();
    double timeoffset = 0.000;

    timeval start, end;
    clock_t clock1, clock2;
    double cpu_time_used;

    gettimeofday ( &start, 0 );
    clock1 = clock();
    stringstream cons;

    votesmanagement vm;
    vm.loadFromFile ( solveParams.instancefilename );

    if ( ( solveParams.modus == 1 ) || ( ( solveParams.modus > 0 ) && ( vm.getCandidatesCount() < solveParams.modus ) ) || ( vm.getCandidatesCount() == 1 ) )
    {
      if ( vm.getCandidatesCount() > 31 )
      {
        kconsens_instance_bycan kinst ( solveParams.outdebug );
        kinst.loadFromFile ( solveParams.instancefilename );

        solveParams.outverbose << "Using dynamic programming <kconsens_instance_bycan> (modus " << solveParams.modus << ")\n";
        kinst.solve ( solveParams.outverbose, solveParams.outtable );
        if ( boost::this_thread::interruption_requested() )
        {
          solveParams.consensus = "";
          solveParams.score = -1;
          return;
        }
        clock2 = clock();
        gettimeofday ( &end, 0 );
        kinst.saveConsensusToStream ( cons );
        solveParams.consensus = cons.str();
        solveParams.score = kinst.getConsensusScore();
      }
      else
      {
        kconsens_instance_bycand kinst ( solveParams.outdebug );
        kinst.loadFromFile ( solveParams.instancefilename );
        solveParams.outverbose << "Using dynamic programming <kconsens_instance_bycand> (modus " << solveParams.modus << " File " << solveParams.instancefilename << ")\n";
        kinst.solve ( solveParams.outverbose, solveParams.outtable );
        if ( boost::this_thread::interruption_requested() )
        {
          solveParams.consensus = "";
          solveParams.score = -1;
          return;
        }
        clock2 = clock();
        gettimeofday ( &end, 0 );
        kinst.saveConsensusToStream ( cons );
        solveParams.consensus = cons.str();
        solveParams.score = kinst.getConsensusScore();
      }
    }
    else
      if ( solveParams.modus > 1 )
      {
        kconsens_instance_bykemX kinst ( solveParams.modus, solveParams.outdebug );
        kinst.loadFromFile ( solveParams.instancefilename );

        solveParams.outverbose << "Using search tree with dirty set size " << solveParams.modus << " <kconsens_instance_bykemX> (modus " << solveParams.modus << ")\n";
        kinst.solve ( solveParams.outverbose, solveParams.outtable );
        if ( boost::this_thread::interruption_requested() )
        {
          solveParams.consensus = "";
          solveParams.score = -1;
          return;
        }
        clock2 = clock();
        gettimeofday ( &end, 0 );
        kinst.saveConsensusToStream ( cons );
        solveParams.consensus = cons.str();
        solveParams.score = kinst.getConsensusScore();
      }
      else
        if ( solveParams.modus == 0 )
        {
          kconsens_instance_approx_pickpermrand kinst ( solveParams.outdebug );
          kinst.loadFromFile ( solveParams.instancefilename );

          solveParams.outverbose << "Using factor-6-approximation algorithm \"pickaperm\" <kconsens_instance_approx_pickpermrand> (modus " << solveParams.modus << ")\n";
          kinst.solve ( solveParams.outverbose, solveParams.outtable );
          if ( boost::this_thread::interruption_requested() )
          {
            solveParams.consensus = "";
            solveParams.score = -1;
            return;
          }
          clock2 = clock();
          gettimeofday ( &end, 0 );
          kinst.saveConsensusToStream ( cons );
          solveParams.consensus = cons.str();
          solveParams.score = kinst.getConsensusScore();
        }
        else
          if ( solveParams.modus == -1 )
          {
            kconsens_instance_glpk kinst ( solveParams.outdebug, solveParams.timelimit, solveParams.timelimit_flag );
            kinst.loadFromFile ( solveParams.instancefilename );

            solveParams.outverbose << "Using glpsol <kconsens_instance_glpk>\n";
            kinst.solve ( solveParams.outverbose, solveParams.outtable );
            if ( boost::this_thread::interruption_requested() )
            {
              solveParams.consensus = "";
              solveParams.score = -1;
              return;
            }
            clock2 = clock();
            gettimeofday ( &end, 0 );
            kinst.saveConsensusToStream ( cons );
            solveParams.consensus = cons.str();
            solveParams.score = kinst.getConsensusScore();
          }
          else
            if ( solveParams.modus == -2 )
            {
              kconsens_instance_cplex kinst ( solveParams.outdebug, solveParams.timelimit, solveParams.timelimit_flag );
              kinst.loadFromFile ( solveParams.instancefilename );

              solveParams.outverbose << "Using cplex <kconsens_instance_cplex>\n";
              kinst.solve ( solveParams.outverbose, solveParams.outtable );
              if ( boost::this_thread::interruption_requested() )
              {
                solveParams.consensus = "";
                solveParams.score = -1;
                return;
              }
              clock2 = clock();
              gettimeofday ( &end, 0 );
              kinst.saveConsensusToStream ( cons );
              solveParams.consensus = cons.str();
              solveParams.score = kinst.getConsensusScore();
            }
            else
              if ( solveParams.modus == -3 )
              {
                kconsens_instance_gurobi kinst ( solveParams.outdebug, solveParams.timelimit, solveParams.timelimit_flag );
                kinst.loadFromFile ( solveParams.instancefilename );

                solveParams.outverbose << "Using gurobi <kconsens_instance_gurobi>\n";
                kinst.solve ( solveParams.outverbose, solveParams.outtable );
                if ( boost::this_thread::interruption_requested() )
                {
                  solveParams.consensus = "";
                  solveParams.score = -1;
                  return;
                }
                clock2 = clock();
                gettimeofday ( &end, 0 );
                kinst.saveConsensusToStream ( cons );
                solveParams.consensus = cons.str();
                solveParams.score = kinst.getConsensusScore();
              }
              else
                if ( solveParams.modus == -4 )
                {
                  kconsens_instance_charon_hudry kinst ( solveParams.outdebug, solveParams.timelimit, solveParams.timelimit_flag );
                  kinst.loadFromFile ( solveParams.instancefilename );

                  solveParams.outverbose << "Using charon_hudry <kconsens_instance_charon_hudry>\n";
                  kinst.solve ( solveParams.outverbose, solveParams.outtable );
                  if ( boost::this_thread::interruption_requested() )
                  {
                    solveParams.consensus = "";
                    solveParams.score = -1;
                    return;
                  }
                  clock2 = clock();
                  gettimeofday ( &end, 0 );
                  kinst.saveConsensusToStream ( cons );
                  solveParams.consensus = cons.str();
                  solveParams.score = kinst.getConsensusScore();
                }
                else
                {

                  solveParams.outverbose << "Modus " << solveParams.modus << " not found.\n";
                  solveParams.consensus = "";
                  solveParams.score = -1;
                  clock2 = clock();
                  gettimeofday ( &end, 0 );
                }

    // correction if we use external programs, then.. taking CPU-time not a good idea
    //cpu_time_used = ( ( double ) ( clock2 - clock1 ) ) / CLOCKS_PER_SEC;
    //if ( solveParams.modus < 0 )
    //{
    cpu_time_used = end.tv_sec - start.tv_sec + ( end.tv_usec - start.tv_usec ) / 1000000.;
    //}

    solveParams.cputime = cpu_time_used + timeoffset;

    solveParams.outverbose << "The kemeny score was " << solveParams.score << " for an optimal consensus: " << solveParams.consensus << ".\n";
    solveParams.outverbose << fixed << setprecision ( 3 ) << "Computation took " << solveParams.cputime << " seconds.\n";
    solveParams.outtable << solveParams.instancefilename << "\t" << solveParams.cputime << "\t" << solveParams.score << "\t" << solveParams.consensus << "\n";

    fullspeed_core[solveParams.coreID].unlock();
  }
  catch ( boost::thread_interrupted& )
  {
    fullspeed_core[solveParams.coreID].unlock();
    cout << "Interrupted, cleaning up Memory.. (Core" << solveParams.coreID << ")\n";
  }
}

void ResourcesLimiter::operator() ()
{
  xtime endtime;
  xtime_get ( &endtime, boost::TIME_UTC );
  endtime.sec += timelimit;

  xtime nexttime;
  xtime_get ( &nexttime, boost::TIME_UTC );
  nexttime.nsec += 10000;
  thread::sleep ( nexttime );

  bool timeout = true;
  bool ramout = false;
  double freemem;
  while ( nexttime.sec <= endtime.sec )
  {
    nexttime.sec += 1;
    freemem = getFreeRamPerc();
    if ( freemem < 0.05 )
    {
      ramout = true;
      timeout = false;
      break;
    }
    if ( fullspeed_core[coreID].try_lock() )
    {
      fullspeed_core[coreID].unlock();
      timeout = false;
      break;
    }
    thread::sleep ( nexttime );
  }
  if ( timeout || ramout )
  {
    workthread.interrupt();
  }
  workthread.join();
  fullspeed_core[coreID].try_lock();
  fullspeed_core[coreID].unlock();
  if ( timeout )
  {
    cout << "Warning: Cancelled after " << timelimit << " seconds.\n";
  }
  if ( ramout )
  {
    cout << "Warning: Cancelled due to free ram is under " << 5 << "%.\n";
  }
}

void StatusOutput::outputNewContent()
{
  statusString = statusstream.str();
  boost::split ( statusLines, statusString, is_any_of ( "\n" ) );

  while ( statusLines.size() > lastLine + 1 )
  {
    outputstream << "Core[" << coreID << "]: " << statusprefix << statusLines.at ( lastLine ) << "\n";
    lastLine = lastLine + 1;
  }
}

void StatusOutput::operator() ()
{
  try
  {
    while ( true )
    {
      this->outputNewContent();

      xtime nexttime;
      xtime_get ( &nexttime, boost::TIME_UTC );
      nexttime.nsec += updateIntervalNS;
      thread::sleep ( nexttime );
    }
  }
  catch ( boost::thread_interrupted& )
  {
    this->outputNewContent();
  }
}

long getFirstAlreadyComputedKscore ( string electionFilename, int fromMode, int toMode )
{
  long score;
  for ( int i = fromMode; i <= toMode; i++ )
  {
    string consensusFilename = electionFilename + ".consensus" + toStr ( i ) + ".score";
    ifstream source ( consensusFilename.data() );
    if ( source != NULL )
    {
      source >> score;
      source.close();
      return score;
    }
    source.close();
  }
  return -1;
}

int getFirstAlreadyComputedConsensus ( string electionFilename, int fromMode, int toMode, CandidatePositionMap &consensusMap )
{
  long score;
  for ( int i = fromMode; i <= toMode; i++ )
  {
    string consensusScoreFilename = electionFilename + ".consensus" + toStr ( i ) + ".score";
//     cout << "Looking for solution" << consensusScoreFilename << "\n";
    ifstream source ( consensusScoreFilename.data() );
    if ( source != NULL )
    {
      source >> score;
    }
    source.close();
    if ( score > -1 )
    {
      string consensusFilename = electionFilename + ".consensus" + toStr ( i );
//       cout << "Reading" << consensusFilename << "\n";
      ifstream source ( consensusFilename.data() );
      if ( source != NULL )
      {
        char c = ' ';
        string cand; // current candidate
        unsigned long candp = 0;
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
          Candidate uc = strtoul ( cand.c_str(), NULL, 0 );
          if ( uc != 0 )
          {
//             cout << "Inserting entry for " << uc << " (Position " << candp << ")\n";
            consensusMap.insert ( CandidatePositionMapData ( uc, candp++ ) );
          }
        }
      }
      return 1;
    }
  }
  return -1;
}

long getAlreadyComputedSubscore ( string electionFilename )
{
  long score;
  string consensusFilename = electionFilename + ".scoreToSuccessors";
  ifstream source ( consensusFilename.data() );
  if ( source != NULL )
  {
    source >> score;
    source.close();
    return score;
  }
  source.close();
  return -1;
}

void InstanceInformation::operator() ()
{
  fullspeed_core[coreID].lock();

  votesmanagement vm;

  outverbose << "Analysing the election in [" << instancefilename << "]\n";
  vm.loadFromFile ( instancefilename );
  vm.analyzeMajority ( majorityRatio );

  unsigned long n = vm.getVotesCount();
  unsigned long m = vm.getCandidatesCount();
  unsigned long dp = vm.getDirtyPairsCount();
  unsigned long pairs = m * ( m - 1 ) / 2;
  unsigned long pairs_red = pairs - dp;
  double pairs_red_time = vm.getDirtyPairAnalyzeTime();
  double dpp = ( double ) dp * 2 / ( m * ( m - 1 ) );
  unsigned long ndp23 = vm.getMajorityPairsCount();
  double ndp23p = ( double ) ndp23 * 2 / ( m * ( m - 1 ) );
  unsigned long lowScore = vm.getLowerScoreBound();
  unsigned long uppScore = vm.getUpperScoreBound();
  unsigned long maxrange = vm.getMaxRange();
  double avgKtDist = vm.getAverageKTdist();
  // TODO Error message, when we get two different scores (both unequal to -1)
  long kscoreComputedBySearchtree = getFirstAlreadyComputedKscore ( instancefilename, 2, 10 );
  long kscoreComputedByDynprog = getFirstAlreadyComputedKscore ( instancefilename, 1, 1 );
  long kscoreComputedByGlpk = getFirstAlreadyComputedKscore ( instancefilename, -1, -1 );
  long kscoreComputedByCplex = getFirstAlreadyComputedKscore ( instancefilename, -2, -2 );
  long kscoreComputedByGurobi = getFirstAlreadyComputedKscore ( instancefilename, -3, -3 );
  CandidatePositionMap computedConsensusMap;
  getFirstAlreadyComputedConsensus(instancefilename, -3, 2, computedConsensusMap);
  float probability = vm.getEmpericalProbability( computedConsensusMap );
  long maxScore = max ( kscoreComputedByGlpk, max ( ( kscoreComputedByGurobi, kscoreComputedByCplex ), max ( kscoreComputedBySearchtree, kscoreComputedByDynprog ) ) );
  double kDIVm = ( double ) maxScore / ( double ) m;
  if ( kDIVm < 0. )
    kDIVm = -1.;
  int majCC = vm.getMajorityNonDirtyCandidatesCount();
  /*if ( vm.hasTies ) TODO: Reimplement if ties are supported
  {
      outverbose << "This election has ties! This is also respected in the scores.\n";
      outtable << "1\t";
  }
  else
  {
      outverbose << "This election does not have ties.\n";
      outtable << "0\t";
  }*/
  outverbose << "Number of votes: " << n << "\n";
  outtable << n << "\t";
  outverbose << "Number of candidates: " << m << "\n";
  outtable << m << "\t";
  outverbose << fixed << setprecision ( 2 ) << "Number of dirty pairs: " << dp << " (" << dpp*100. << "%)\n";
  outtable << fixed << dp << "\t" << dpp*100. << "\t";
  outverbose << fixed << setprecision ( 2 ) << "Number of " << majorityRatio*100. << "% majority pairs: " << ndp23 << " (" << ndp23p*100. << "%)\n";
  outtable << fixed << majorityRatio*100. << "\t" << ndp23 << "\t" << ndp23p*100. << "\t";
  outverbose << "The kemeny score is between " << lowScore << " and " << uppScore << "\n";
  outtable << lowScore << "\t" << uppScore << "\t";
  outverbose << "Maximum range of a candidate: " << maxrange << "\n";
  outtable << maxrange << "\t";
  outverbose << fixed << setprecision ( 2 ) << "Avarage kt-dist of two votes: " << avgKtDist << "\n";
  outtable << fixed << avgKtDist << "\t";
  outverbose << "Kemeny Score of with search tree solved instance: " << kscoreComputedBySearchtree << "\n";
  outtable << kscoreComputedBySearchtree << "\t";
  outverbose << "Kemeny Score of with dynamic programming solved instance: " << kscoreComputedByDynprog << "\n";
  outtable << kscoreComputedByDynprog << "\t";
  outverbose << "Kemeny Score of with GLPK solved instance: " << kscoreComputedByGlpk << "\n";
  outtable << kscoreComputedByGlpk << "\t";
  outverbose << "Kemeny Score of with CPLEX solved instance: " << kscoreComputedByCplex << "\n";
  outtable << kscoreComputedByCplex << "\t";
  outverbose << "Kemeny Score of with GUROBI solved instance: " << kscoreComputedByGurobi << "\n";
  outtable << kscoreComputedByGurobi << "\t";
  outverbose << fixed << setprecision ( 2 ) << "Kemeny Score / #candidates: " << kDIVm << "\n";
  outtable << fixed << kDIVm << "\t";
  outverbose << "There are " << majCC << " Majority non-dirty Candidates (ranked according to the 3/4 majority).\n";
  outtable << majCC << "\t";
  outverbose << "The average consensus probability is " << probability << "%.\n";
  outtable << probability << "\t";
  
  fullspeed_core[coreID].unlock();
}

void InstanceSplitter::operator() ()
{
  fullspeed_core[coreID].lock();
  double timeoffset = 0.000;

  clock_t clock1, clock2;
  double cpu_time_used;

  clock1 = clock();

  string timefile = instancefilename + ".time";
  string scorefile = instancefilename + ".score";

  votesplitter vs;
  vs.loadFromFile ( instancefilename );
  vs.analyzeMajority ( 3. / 4. );
  outverbose << "Combinated appliance of all Rules:\n";
  vs.clearSplits();

  vs.heuristikalSplitTotal ( extrMaxsize, bestChoiceMethod, keepAllSolutionsWithReductions, ruleNonDirtyCandidates, ruleCondorcetCandidates, ruleNonDirtySets, ruleCondorcetSets, ruleCondorcetComponents );
  ReductionQuality heuristikalQuality = getReductionQuality ( vs, vs.last_heuristikal_runningtime, 1 );
  printReductionQuality ( heuristikalQuality, "combined Rules", outverbose );

  vs.writeInstancesToFiles ( instancefilename, subinstancesFilenames );


  clock2 = clock();
  cpu_time_used = ( ( double ) ( clock2 - clock1 ) ) / CLOCKS_PER_SEC;

  ofstream tfile ( timefile.data() );
  tfile << ( cpu_time_used + timeoffset ) << endl;
  tfile.close();

  double timeused = cpu_time_used + timeoffset;
  outverbose << fixed << setprecision ( 3 ) << "Ruduction took " << timeused << " seconds.\n";
  outtable << instancefilename << "\t" << timeused << "\t" << vs.getPrintInstancesCandidateSetSizes() << "\t";
  printReductionQuality ( heuristikalQuality, outtable, sepText );

  fullspeed_core[coreID].unlock();
}

void solveInstance ( SolveParams &solveParams )
{
  boost::thread thread_statusoutput;
  if ( solveParams.verbose_flag )
  {
    thread_statusoutput = thread ( StatusOutput ( solveParams.outverbose, cout, 100000, solveParams.coreID, solveParams.statusprefix ) );
  }
  boost::thread thread_s = thread ( InstanceSolver ( solveParams ) );
  if ( solveParams.timelimit_flag )
  {
    boost::thread thread_t = thread ( ResourcesLimiter ( solveParams.timelimit, thread_s, solveParams.coreID ) );
    thread_t.join();
  }
  else
  {
    thread_s.join();
  }
  thread_statusoutput.interrupt();
  thread_statusoutput.join();
  cout << "";
}

void analyzeInstance ( string &electionFilename, double &majorityRatio, int coreID, long &timelimit, bool &timelimit_flag, bool verbose_flag, stringstream &outverbose, stringstream &outtable, stringstream &outdebug )
{
  string prefix = "[analyzing instance]: ";
  boost::thread thread_statusoutput;
  if ( verbose_flag )
  {
    thread_statusoutput = thread ( StatusOutput ( outverbose, cout, 100000, coreID, prefix ) );
  }
  boost::thread thread_i = thread ( InstanceInformation ( electionFilename, majorityRatio, coreID, outverbose, outtable, outdebug ) );
  if ( timelimit_flag )
  {
    boost::thread thread_t = thread ( ResourcesLimiter ( timelimit, thread_i, coreID ) );
    thread_t.join();
  }
  else
  {
    thread_i.join();
  }
  thread_statusoutput.interrupt();
  thread_statusoutput.join();
  cout << "";
}

void splitInstance ( string &electionFilename, vector<string> &subinstancesFilenames, int extrMaxsize, int bestChoiceMethod, bool keepAllSolutionsWithReductions, int coreID, bool ruleNonDirtyCandidates, bool ruleCondorcetCandidates, bool ruleNonDirtySets, bool ruleCondorcetSets, bool ruleCondorcetComponents, long &timelimit, bool &timelimit_flag, bool verbose_flag, stringstream &outverbose, stringstream &outtable, stringstream &outdebug )
{
  string prefix = "[reducing instance]: ";
  boost::thread thread_statusoutput;
  if ( verbose_flag )
  {
    thread_statusoutput = thread ( StatusOutput ( outverbose, cout, 100000, coreID, prefix ) );
  }
  boost::thread thread_s = thread ( InstanceSplitter ( electionFilename, subinstancesFilenames, extrMaxsize, bestChoiceMethod, keepAllSolutionsWithReductions, ruleNonDirtyCandidates, ruleCondorcetCandidates, ruleNonDirtySets, ruleCondorcetSets, ruleCondorcetComponents, coreID, outverbose, outtable, outdebug ) );
  if ( timelimit_flag )
  {
    boost::thread thread_t = thread ( ResourcesLimiter ( timelimit, thread_s, coreID ) );
    thread_t.join();
  }
  else
  {
    thread_s.join();
  }
  thread_statusoutput.interrupt();
  thread_statusoutput.join();
  cout << "";
}

void solveSplittedInstances ( int &solveModus, string &electionFilename, vector<string> &subinstancesFilenames, int coreID, long &timelimit, bool &timelimit_flag, bool verbose_flag, stringstream &outverbose, stringstream &outtable, stringstream &outdebug, double &cputime, string &consensus, long &score, string &statusprefix )
{
  cputime = 0.;
  consensus = "";
  score = 0;

  bool wasCanceled = false;
  for ( int i = 0; i < subinstancesFilenames.size(); i++ )
  {
    stringstream i_outverbose;
    stringstream i_outtable;
    stringstream i_outdebug;

    string subinstanceFilename = subinstancesFilenames.at ( i );
    long splitScoreOffset = getAlreadyComputedSubscore ( subinstanceFilename );;

    stringmap amap;
    stringmap dmap;
    stringset commons;
    write_clean ( subinstanceFilename, subinstanceFilename + ".clean" );
    analyze_commons ( subinstanceFilename + ".clean", commons );
    write_commons ( subinstanceFilename + ".clean", subinstanceFilename + ".complete", commons );
    anon_and_map_sourcefile ( subinstanceFilename + ".complete", dmap, amap, subinstanceFilename + ".dict", subinstanceFilename + ".anoncomplete" );
    subinstanceFilename = subinstanceFilename + ".anoncomplete";

    double i_cputime;
    string i_consensus;
    long i_score;
    string i_prefix = statusprefix + " [subinstance " + toStr ( i ) + " of " + toStr ( subinstancesFilenames.size() - 1 ) + "]: ";

    SolveParams solveParams = SolveParams ( solveModus, subinstanceFilename, 0, timelimit, timelimit_flag, verbose_flag, i_outverbose, i_outtable, i_outdebug, i_cputime, i_consensus, i_score, i_prefix );
    solveInstance ( solveParams );

    string di_consens;
    deanon_preferenceString ( i_consensus, dmap, di_consens );

    cputime += i_cputime;
    if ( i_score > -1 )
    {
      if ( i > 0 )
      {
        consensus += ">" + di_consens;
      }
      else
      {
        consensus += di_consens;
      }
      score += i_score + splitScoreOffset;
    }
    else
    {
      score = -1;
      consensus = "";
      wasCanceled = true;
      break;
    }

    outverbose << i_outverbose.str();
    //outtable << i_outtable.str();
    outdebug << i_outdebug.str();

    /*boost::filesystem::remove( subinstanceFilename + ".scoreToSuccessors" );
    boost::filesystem::remove( subinstanceFilename + ".replaceCandidate" );
    boost::filesystem::remove( subinstanceFilename + ".weight" );
    boost::filesystem::remove( subinstanceFilename );*/
  }
  stringstream o_outverbose;
  stringstream o_outtable;
  stringstream o_outdebug;

  if ( !wasCanceled )
  {
    o_outverbose << "The overall kemeny score was " << score << " for an optimal consensus: " << consensus << ".\n";
    o_outverbose << fixed << setprecision ( 3 ) << "Computation took " << cputime << " seconds.\n";
    o_outtable << cputime << "\t" << score << "\t" << consensus;
  }
  else
  {
    o_outverbose << "Computation was canceled. No overall result available.\n";
    o_outverbose << fixed << setprecision ( 3 ) << "Computation took " << cputime << " seconds.\n";
    o_outtable << cputime << "\t" << score << "\t" << consensus;
  }

  boost::thread thread_statusoutput;
  if ( verbose_flag )
  {
    thread_statusoutput = thread ( StatusOutput ( o_outverbose, cout, 100000, coreID, statusprefix ) );
  }
  thread_statusoutput.interrupt();
  thread_statusoutput.join();

  outverbose << o_outverbose.str();
  outtable << o_outtable.str();
  outdebug << o_outdebug.str();

  cout << "";
}

void deleteSplittedInstances ( string &electionFilename, vector<string> &subinstancesFilenames )
{
  for ( int i = 0; i < subinstancesFilenames.size(); i++ )
  {

    string subinstanceFilename = subinstancesFilenames.at ( i );

    boost::filesystem::remove ( subinstanceFilename + ".scoreToSuccessors" );
    boost::filesystem::remove ( subinstanceFilename + ".replaceCandidate" );
    boost::filesystem::remove ( subinstanceFilename + ".weight" );
    boost::filesystem::remove ( subinstanceFilename );

    boost::filesystem::remove ( subinstanceFilename + ".anoncomplete" );
    boost::filesystem::remove ( subinstanceFilename + ".clean" );
    boost::filesystem::remove ( subinstanceFilename + ".complete" );
    boost::filesystem::remove ( subinstanceFilename + ".dict" );
  }
}

void writeResultFiles ( string& consensusFilename, string& scoreFilename, string& timeFilename, string& verboseLogFilename, string& tableLogFilename, string& debuglogFilename, string &consensus, long &score, double &cputime, stringstream &outverbose, stringstream &outtable, stringstream &outdebug )
{
  ofstream consensusFile ( consensusFilename.data() );
  consensusFile << consensus << "\n";
  consensusFile.close();

  ofstream scoreFile ( scoreFilename.data() );
  scoreFile << score << "\n";
  scoreFile.close();

  ofstream timeFile ( timeFilename.data() );
  timeFile << cputime << "\n";
  timeFile.close();

  ofstream verboseFile ( verboseLogFilename.data() );
  verboseFile << outverbose.str();
  verboseFile.close();

  ofstream tableFile ( tableLogFilename.data() );
  tableFile << outtable.str();
  tableFile.close();

  ofstream debugFile ( debuglogFilename.data() );
  debugFile << outdebug.str();
  debugFile.close();
}
