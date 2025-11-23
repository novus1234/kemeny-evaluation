/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2011 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ implementation file for the program (startup methods).
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

// gcc and libraries
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <getopt.h>

// project internal
#include "tools.hpp"
#include "kemtyps.hpp"
#include "errors.hpp"
#include "votesplitter.hpp"
#include "jobmanagement.hpp"

using namespace std;
using namespace boost;

stringstream kcsoreComputation_verbose;
stringstream kcsoreComputation_table;
stringstream kcsoreComputation_debuglog;
stringstream electionAttributes_verbose;
stringstream electionAttributes_table;
stringstream electionAttributes_debuglog;
stringstream reductionResults_verbose;
stringstream reductionResults_table;
stringstream reductionResults_debuglog;


static int verbose_flag = false;

void printUsage()
{
  printf ( "Usage of kconsens:\n" );
  printf ( "  --election-file [filname] (-e [filname]):\n" );
  printf ( "    Filename of the election.\n" );
  printf ( "  --print-information (-i):\n" );
  printf ( "    Prints some information about an election.\n" );
  printf ( "  --solve-with-modus [solvemodus] (-s [solvemodus]):\n" );
  printf ( "    Computes the optimal Kemeny score and a consensus list.\n" );
  printf ( "    Solvemodus X means:\n" );
  printf ( "      X=-3 - Use gurobi ILP-solver\n" );
  printf ( "      X=-2 - Use CPLEX ILP-solver\n" );
  printf ( "      X=-1 - Use GLPK ILP-solver\n" );
  printf ( "      X=0 - Take best vote. NO OPTIMAL SOLUTION\n" );
  printf ( "      X=1 - Dynamic Programming\n" );
  printf ( "      X>1 - Search Tree Algorithm with dirty set size X\n" );
  printf ( "  --partitionate-with-datareductions (-p):\n" );
  printf ( "    Partitionates instance into subinstances with data reductions rules.\n" );
  printf ( "    This may highly improve the running-time.\n" );
  printf ( "  --verbose:\n" );
  printf ( "    This option will produce a human readable output instead of tables.\n" );
  printf ( "    This also includes some real-time status information about the computation process.\n" );
}

void updateBestChoiceMethod ( int &bestChoiceMethod, string &bestChoiceMethodString, ostream &out )
{
  if ( bestChoiceMethodString == "smallestInstance" )
  {
    bestChoiceMethod = bestChoiceMethod_smallestInstance;
    out << "Using method: " << bestChoiceMethodString << "\n";
  }
  else
  {
    if ( bestChoiceMethodString == "halfOfInstance" )
    {
      bestChoiceMethod = bestChoiceMethod_halfOfInstance;
      out << "Using method: " << bestChoiceMethodString << "\n";
    }
    else
    {
      if ( bestChoiceMethodString == "thirdOfInstance" )
      {
        bestChoiceMethod = bestChoiceMethod_thirdOfInstance;
        out << "Using method: " << bestChoiceMethodString << "\n";
      }
      else
      {
        if ( bestChoiceMethodString == "biggestInstance" )
        {
          bestChoiceMethod = bestChoiceMethod_biggestInstance;
          out << "Using method: " << bestChoiceMethodString << "\n";
        }
        else
        {
          bestChoiceMethod = bestChoiceMethod_smallestInstance;
          out << "Do not know method \"" << bestChoiceMethodString << "\". Using default method: smallestInstance\n";
        }
      }
    }
  }
}

void writeDeanonymizedResultfiles ( string &consensus, long score, double cputime, string &cFilename, string &sFilename, string &tFilename, stringmap &dmap )
{
  string dconsens;
  ofstream cF ( cFilename.data() );
  deanon_preferenceString ( consensus, dmap, dconsens );
  cF << dconsens;
  cF.close();
  ofstream cS ( sFilename.data() );
  cS << score;
  cS.close();
  ofstream cT ( tFilename.data() );
  cT << cputime;
  cT.close();
}

int main ( int argc, char** argv )
{
  bool printHelp_flag = false;
  bool printInformation_flag = false;
  bool electionFilename_flag = false;
  bool suggestionFilename_flag = false;
  bool constraintFilename_flag = false;
  bool testSuggestions_flag = false;
  bool glpkVerification_flag = false;
  bool partitionInstance_flag = false;
  bool fastDatareduction_flag = false;
  bool solveInstance_flag = false;
  bool generateSuggestions_flag = false;
  bool majorityFilename_flag = false;
  bool nonMajorityFilename_flag = false;
  bool majorityRatio_flag = false;
  bool majorityAnalyze_flag = false;
  bool dirtyFilename_flag = false;
  bool dirtyAnalyze_flag = false;
  bool timelimit_flag = false;
  bool noarguments = true;

  string electionFilename;
  string consensusFilename;
  string scoreFilename;
  string timeFilename;
  string verboseLogFilename;
  string tableLogFilename;
  string debugLogFilename;
  string dconsensFilename;
  string dscoreFilename;
  string dtimeFilename;
  
  string constraintFilename;
  string suggestionFilename;
  string nonMajorityFilename;
  string majorityFilename;
  string dirtyFilename;
  double majorityRatio = 3. / 4.;
  int extrMaxsize = -1;
  int solveModus = -1;
  bool keepAllSolutionsWithReductions = false;
  string bestChoiceMethodString;
  int bestChoiceMethod = bestChoiceMethod_smallestInstance;
  long timelimit = 0;

  string modiAsString;
  bitset<4> rulesOnOff;
  bool ruleCondorcetComponents = true;
  bool ruleNonDirtyCandidates = false;
  bool ruleCondorcetCandidates = false;
  bool ruleNonDirtySets = false;
  bool ruleCondorcetSets = false;

  // Map: original candidate name - unique integer index
  stringmap amap;
  // Map: unique integer index - original candidate name
  stringmap dmap;
  // The set of candidates which are part of each vote
  stringset commons;

  int c;

  while ( 1 )
  {

    static struct option long_options[] =
    {
      {"verbose",                          no_argument,       &verbose_flag, 1},
      {"brief",                            no_argument,       &verbose_flag, 0},
      {"keep-all-solutions",               no_argument,       0, 'k'},
      {"election-file",                    required_argument, 0, 'e'},
      {"constraints-for-suggestions",      required_argument, 0, 'c'},
      {"list-of-suggestions",              required_argument, 0, 'l'},
      {"nondirty-majority-ratio",          required_argument, 0, 'm'},
      {"nondirty-majority-file",           required_argument, 0, 'f'},
      {"non-majority-file",                required_argument, 0, 'n'},
      {"extract-instances-maxsize",        required_argument, 0, 'x'},
      {"dirty-file",                       required_argument, 0, 'd'},
      {"print-information",                no_argument,       0, 'i'},
      {"partitionate-with-datareductions", no_argument,       0, 'p'},
      {"set-choice-method",                required_argument, 0, '~'},
      {"rules-selection-mode",             required_argument, 0, 'r'},
      {"GLPK-verification",                no_argument,       0, 'g'},
      {"analyze-list-of-suggestions",      no_argument,       0, 'a'},
      {"solve-with-modus",                 required_argument, 0, 's'},
      {"timelimit",                        required_argument, 0, 't'},
      {"help",                             no_argument,       0, 'h'},
      {"help",                             no_argument,       0, '?'},
      {0, 0, 0, 0}
    };
    int option_index = 0;
    c = getopt_long ( argc, argv, "ke:c:l:m:q:n:x:d:ipfr:gas:t:h?", long_options, &option_index );

    if ( c == -1 )
      break;

    switch ( c )
    {
    case 0:
      if ( long_options[option_index].flag != 0 )
        break;
      printf ( "option %s", long_options[option_index].name );
      if ( optarg )
        printf ( " with arg %s", optarg );
      printf ( "\n" );
      break;
    case 'k':
      noarguments = false;
      keepAllSolutionsWithReductions = true;
      break;
    case 'e':
      noarguments = false;
      electionFilename_flag = true;
      electionFilename = string ( optarg );
      break;
    case 'c':
      noarguments = false;
      constraintFilename_flag = true;
      constraintFilename = string ( optarg );
      generateSuggestions_flag = true;
      break;
    case 'l':
      noarguments = false;
      suggestionFilename_flag = true;
      suggestionFilename = string ( optarg );
      break;
    case 'm':
      noarguments = false;
      majorityRatio_flag = true;
      majorityRatio = atof ( optarg );
      break;
    case 'f':
      noarguments = false;
      dirtyAnalyze_flag = true;
      majorityAnalyze_flag = true;
      majorityFilename_flag = true;
      majorityFilename = string ( optarg );
      break;
    case 'n':
      noarguments = false;
      majorityAnalyze_flag = true;
      nonMajorityFilename_flag = true;
      nonMajorityFilename = string ( optarg );
      break;
    case 'x':
      noarguments = false;
      extrMaxsize = atoi ( optarg );
      break;
    case 'd':
      noarguments = false;
      dirtyAnalyze_flag = true;
      dirtyFilename_flag = true;
      dirtyFilename = string ( optarg );
      break;
    case 'i':
      noarguments = false;
      printInformation_flag = true;
      majorityAnalyze_flag = true;
      dirtyAnalyze_flag = true;
      break;
    case 'p':
      noarguments = false;
      majorityAnalyze_flag = true;
      dirtyAnalyze_flag = true;
      partitionInstance_flag = true;
      break;
    case '~':
      bestChoiceMethodString = string ( optarg );
      break;
    case 'r':
      noarguments = false;

      modiAsString = string ( optarg );
      rulesOnOff = bitset<4> ( modiAsString );

      ruleNonDirtyCandidates = rulesOnOff.test ( 0 );
      ruleCondorcetCandidates = rulesOnOff.test ( 1 );
      ruleNonDirtySets = rulesOnOff.test ( 2 );
      ruleCondorcetSets = rulesOnOff.test ( 3 );

      if ( !rulesOnOff.any() )
      {
        //cerr << "Sorry, but if you want to reduce/partitionate your instance with data reduction rules, then you should turn on at least one Rule!\n";
        //exit ( -2 );
	ruleCondorcetComponents = true;
      } else
      {
	ruleCondorcetComponents = false;
      }
      

      break;
    case 'g':
      noarguments = false;
      glpkVerification_flag = true;
      break;
    case 'a':
      noarguments = false;
      testSuggestions_flag = true;
      break;
    case 's':
      noarguments = false;
      solveInstance_flag = true;
      solveModus = atoi ( optarg );
      break;
    case 't':
      noarguments = false;
      timelimit_flag = true;
      timelimit = atol ( optarg );
      break;
    case '?':
    case 'h':
      printUsage();
      break;
    default:
      abort ();
    }
  }

  if ( optind < argc )
  {
    printf ( "unknwon options: " );
    while ( optind < argc )
      printf ( "%s ", argv[optind++] );
    putchar ( '\n' );
    putchar ( '\n' );
    printUsage();
  }

  if ( noarguments )
  {
    printUsage();
  }

  if ( partitionInstance_flag )
  {
    updateBestChoiceMethod ( bestChoiceMethod, bestChoiceMethodString, reductionResults_debuglog );
  }

  votesplitter vm;

  if ( electionFilename_flag )
  {
    write_clean ( electionFilename, electionFilename + ".clean" );
    analyze_commons ( electionFilename + ".clean", commons );
    write_commons ( electionFilename + ".clean", electionFilename + ".complete", commons );
    anon_and_map_sourcefile ( electionFilename + ".complete", dmap, amap, electionFilename + ".dict", electionFilename + ".anoncomplete" );

    dconsensFilename = electionFilename.data();
    dconsensFilename += ".consensus" + toStr ( solveModus );
    dscoreFilename = dconsensFilename + ".score";
    dtimeFilename = dconsensFilename + ".time";
    
    electionFilename = electionFilename + ".anoncomplete";
    if ( partitionInstance_flag )
    {
      consensusFilename = electionFilename + ".consensus" + toStr ( solveModus ) + "p";
    }
    else
    {
      consensusFilename = electionFilename + ".consensus" + toStr ( solveModus );
    }
    scoreFilename = consensusFilename + ".score";
    timeFilename = consensusFilename + ".time";
    verboseLogFilename = consensusFilename + ".verbose";
    tableLogFilename = consensusFilename + ".table";
    debugLogFilename = consensusFilename + ".debug";

    vm.loadFromFile ( electionFilename );

    // corrections
    if ( extrMaxsize < 1 )
    {
      extrMaxsize = vm.getCandidatesCount() - 1;
    }
  }

  if ( generateSuggestions_flag )
  {
    if ( constraintFilename_flag )
    {
      vm.loadConstraintsFromFile ( constraintFilename );
    }

    if ( suggestionFilename_flag )
    {
      vm.genSuggestionsForConstrains ( suggestionFilename );
    }

    else
    {
      cerr << "ERROR: You have to say which constraintfile should be used. ('--constraint-file)\n";
      exit ( ERROR_ARGUMENTS_REQUIRED );
    }
  }

  if ( dirtyFilename_flag )
  {
    vm.printDirtyPairsBothDirections ( dirtyFilename );
  }

  if ( majorityAnalyze_flag )
  {
    vm.analyzeMajority ( majorityRatio );
  }

  if ( majorityFilename_flag )
  {
    vm.printMajorityPairs ( majorityFilename );
  }

  if ( nonMajorityFilename_flag )
  {
    vm.printNonMajorityPairsBothDirections ( nonMajorityFilename );
  }

  if ( dirtyFilename_flag )
  {
    vm.printMajorityPairs ( majorityFilename );
  }

  if ( electionFilename_flag )
  {
    if ( partitionInstance_flag )
    {
      if ( !verbose_flag && modiAsString.size() > 0 )
        cout << modiAsString << "\t";
      vector<string> subinstancesFilenames;
      splitInstance ( electionFilename, subinstancesFilenames, extrMaxsize, bestChoiceMethod, keepAllSolutionsWithReductions, 0,  ruleNonDirtyCandidates, ruleCondorcetCandidates, ruleNonDirtySets, ruleCondorcetSets, ruleCondorcetComponents, timelimit, timelimit_flag, verbose_flag, reductionResults_verbose, reductionResults_table, reductionResults_debuglog );
      if ( solveInstance_flag )
      {
        if ( !verbose_flag )
          cout << electionFilename;
          cout << "\t";
        double cputime;
        string consensus;
        long score;
        string prefix = "[solving reduced instance]: ";
        solveSplittedInstances ( solveModus, electionFilename, subinstancesFilenames, 0, timelimit, timelimit_flag, verbose_flag, kcsoreComputation_verbose, kcsoreComputation_table, kcsoreComputation_debuglog, cputime, consensus, score, prefix );
        if ( !verbose_flag )
        {
          cout << kcsoreComputation_table.str();
        }
        writeResultFiles ( consensusFilename, scoreFilename, timeFilename, verboseLogFilename, tableLogFilename, debugLogFilename, consensus, score, cputime, kcsoreComputation_verbose, kcsoreComputation_table, kcsoreComputation_debuglog );
        writeDeanonymizedResultfiles ( consensus, score, cputime, dconsensFilename, dscoreFilename, dtimeFilename, dmap );
      } else
      {
        if ( !verbose_flag )
          cout << reductionResults_table.str();
      }
      solveInstance_flag = false;
        deleteSplittedInstances ( electionFilename, subinstancesFilenames );
    }
    if ( solveInstance_flag )
    {
      double cputime;
      string consensus;
      long score;
      string prefix = "[solving instance]: ";
      SolveParams solveParams = SolveParams ( solveModus, electionFilename, 0, timelimit, timelimit_flag, verbose_flag, kcsoreComputation_verbose, kcsoreComputation_table, kcsoreComputation_debuglog, cputime, consensus, score, prefix );
      solveInstance ( solveParams );
      if ( !verbose_flag )
        cout << kcsoreComputation_table.str();
      writeResultFiles ( consensusFilename, scoreFilename, timeFilename, verboseLogFilename, tableLogFilename, debugLogFilename, consensus, score, cputime, kcsoreComputation_verbose, kcsoreComputation_table, kcsoreComputation_debuglog );
      writeDeanonymizedResultfiles ( consensus, score, cputime, dconsensFilename, dscoreFilename, dtimeFilename, dmap );
    }
    analyzeInstance ( electionFilename, majorityRatio, 0, timelimit, timelimit_flag, verbose_flag, electionAttributes_verbose, electionAttributes_table, electionAttributes_debuglog );
    if ( printInformation_flag )
    {
      if ( !verbose_flag )
        cout << electionAttributes_table.str();
    }
  }
  if ( ( partitionInstance_flag || printInformation_flag ) && !electionFilename_flag )
  {
    cerr << "ERROR: You have to say which electionfile should be analysed. (--election-file)\n";
    exit ( ERROR_ARGUMENTS_REQUIRED );
  }

  if ( testSuggestions_flag )
  {
    if ( electionFilename_flag )
    {
      if ( suggestionFilename_flag )
      {
        vm.loadSuggestionsFromFile ( suggestionFilename );
      }

      else
      {
        cerr << "ERROR: You have to say which suggestionfile should be used. (--suggestion-file)\n";
        exit ( ERROR_ARGUMENTS_REQUIRED );
      }

      cout << "\nTesting the consensus suggestions in [" << suggestionFilename << "] for the election in [" << electionFilename << "]\n";

      printf ( "Number of suggestions: %lu\n", vm.getSuggestionsCount() );
      vm.testSuggestions ( verbose_flag );
    }

    else
    {
      cerr << "ERROR: You have to say what election file should be analysed. (--election-file)\n";
      exit ( ERROR_ARGUMENTS_REQUIRED );
    }
  }
}
