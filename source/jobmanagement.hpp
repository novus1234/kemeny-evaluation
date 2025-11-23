/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2011 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ header file of jobmanagement classes
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

#ifndef JOBMANAGEMENT_H
#define JOBMANAGEMENT_H

#include <string>
#include <time.h>
#include <iostream>
#include <bitset>
#include <fstream>
#include <boost/thread.hpp>
#include <boost/thread/xtime.hpp>

// project internal
#include "jobtyps.hpp"
#include "votetyps.hpp"

using namespace std;
using namespace boost;

class InstanceSolver
{
public:
    explicit InstanceSolver ( SolveParams &solveParams_ ) :
            solveParams ( solveParams_ )
    {}
    void operator() ();
private:
    SolveParams &solveParams;
};

class InstanceSplitter
{
public:
    explicit InstanceSplitter ( string const& instanceFilename_, vector<string> &subinstancesFilenames_, int extrMaxsize_, int bestChoiceMethod_, bool keepAllSolutionsWithReductions_, bool ruleNonDirtyCandidates_, bool ruleCondorcetCandidates_, bool ruleNonDirtySets_, bool ruleCondorcetSets_, bool ruleCondorcetComponents_, int coreID_, stringstream &outverbose_, stringstream &outtable_, stringstream &outdebug_ ) :
            instancefilename ( instanceFilename_ ),
            subinstancesFilenames ( subinstancesFilenames_),
            coreID ( coreID_ ),
            outverbose ( outverbose_ ),
            outtable ( outtable_ ),
            outdebug ( outdebug_ ),
            extrMaxsize ( extrMaxsize_ ),
            bestChoiceMethod ( bestChoiceMethod_ ),
            keepAllSolutionsWithReductions ( keepAllSolutionsWithReductions_ ),
            ruleNonDirtyCandidates (ruleNonDirtyCandidates_),
            ruleCondorcetCandidates (ruleCondorcetCandidates_),
            ruleNonDirtySets (ruleNonDirtySets_),
            ruleCondorcetSets(ruleCondorcetSets_),
            ruleCondorcetComponents(ruleCondorcetComponents_)
    {}
    void operator() ();
private:
    string const& instancefilename;
    vector<string> &subinstancesFilenames;
    int coreID;
    stringstream &outverbose;
    stringstream &outtable;
    stringstream &outdebug;
    int extrMaxsize;
    int bestChoiceMethod;
    bool keepAllSolutionsWithReductions;
    bool ruleNonDirtyCandidates;
    bool ruleCondorcetCandidates;
    bool ruleNonDirtySets;
    bool ruleCondorcetSets;
    bool ruleCondorcetComponents;
};

class InstanceInformation
{
public:
    explicit InstanceInformation ( string const& instanceFilename_, double majorityRatio_, int coreID_, stringstream &outverbose_, stringstream &outtable_, stringstream &outdebug_ ) :
            instancefilename ( instanceFilename_ ),
            majorityRatio ( majorityRatio_ ),
            coreID ( coreID_ ),
            outverbose ( outverbose_ ),
            outtable ( outtable_ ),
            outdebug ( outdebug_ )
    {}
    void operator() ();
private:
    string const& instancefilename;
    int coreID;
    double majorityRatio;
    stringstream &outverbose;
    stringstream &outtable;
    stringstream &outdebug;
};

class ResourcesLimiter
{
public:
    explicit ResourcesLimiter ( long const& timelimit_, thread &workthead_, int coreID_ ) :
            timelimit ( timelimit_ ),
            workthread ( workthead_ ),
            coreID ( coreID_ )
    {}
    void operator() ();
private:
    long const& timelimit;
    thread &workthread;
    int coreID;
};

class StatusOutput
{
public:
    explicit StatusOutput ( stringstream &statusstream_, ostream &outputstream_, int updateIntervalNS_, int coreID_, string &statusprefix_ ) :
            statusstream ( statusstream_ ),
            outputstream ( outputstream_ ),
            updateIntervalNS ( updateIntervalNS_ ),
            coreID ( coreID_ ),
            statusprefix ( statusprefix_)
    {
        lastLine = 0;
    }
    void operator() ();
    void outputNewContent();
private:
    stringstream &statusstream;
    ostream &outputstream;
    int updateIntervalNS;
    int coreID;
    string &statusprefix;

    string statusString;
    vector<string> statusLines;
    int lastLine;
};

/// Reads already computed kemeny score for an instancefile.
long getFirstAlreadyComputedKscore ( string electionFilename, int fromMode, int toMode );

/// Reads already computed kemeny consensus for an instancefile.
int getFirstAlreadyComputedConsensus ( string electionFilename, int fromMode, int toMode, CandidatePositionMap &consensusMap );

/// Reads computed kemeny score from an glpsol output file.
long getGlpsolKscore ( string glpsolOutputFilename );

/// Reads computed kemeny score from an cplex output file.
long getCplexKscore ( string cplexOutputFilename );

/// Reads already computed subscore to the successor instances for an instancefile.
long getAlreadyComputedSubscore ( string electionFilename );

/// Creates a new thread that solves the instance
void solveInstance ( SolveParams &solveParams );

/// Creates a new thread that analyzes the attributes of the instance
void analyzeInstance ( string &electionFilename, double &majorityRatio, int coreID, long &timelimit, bool &timelimit_flag, bool verbose_flag, stringstream &outverbose, stringstream &outtable, stringstream &outdebug );

/// Creates a new thread that applies the data reduction rules on the instance
void splitInstance ( string &electionFilename, vector<string> &subinstancesFilenames, int extrMaxsize, int bestChoiceMethod, bool keepAllSolutionsWithReductions, int coreID, bool ruleNonDirtyCandidates, bool ruleCondorcetCandidates, bool ruleNonDirtySets, bool ruleCondorcetSets, bool ruleCondorcetComponents, long &timelimit, bool &timelimit_flag, bool verbose_flag, stringstream &outverbose, stringstream &outtable, stringstream &outdebug );

/// Solves the subinstances.
void solveSplittedInstances ( int &solveModus, string &electionFilename, vector<string> &subinstancesFilenames, int coreID, long &timelimit, bool &timelimit_flag, bool verbose_flag, stringstream &outverbose, stringstream &outtable, stringstream &outdebug, double &cputime, string &consensus, long &score, string &statusprefix );

void deleteSplittedInstances ( string &electionFilename, vector<string> &subinstancesFilenames );

void writeResultFiles ( string &consensusFilename, string &scoreFilename, string &timeFilename, string &verboseLogFilename, string &tableLogFilename, string &debuglogFilename, string &consensus, long &score, double &cputime, stringstream &outverbose, stringstream &outtable, stringstream &outdebug);

#endif
