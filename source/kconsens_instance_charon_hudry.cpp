/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2011 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ implementation file of kconsens_instance_gurobi
 *   Description: Class that transforms the problem instance into a a weighted
 *                tournament and solves the median order problem.
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
#include <limits.h>

// project internal
#include "tools.hpp"
#include "randomtools.hpp"
#include "votetyps.hpp"
#include "kemtyps.hpp"
#include "kconsens_instance_charon_hudry.hpp"
#include "jobmanagement.hpp"

kconsens_instance_charon_hudry::kconsens_instance_charon_hudry ( stringstream &outdebug, int timelimit_, bool timelimit_flag_ ) : kconsens_instance ( outdebug )
{
  srand ( ( unsigned ) time ( 0 ) );
  this->isSynchronized = false;
  this->timelimit = timelimit_;
  this->timelimit_flag = timelimit_flag_;
}

kconsens_instance_charon_hudry::~kconsens_instance_charon_hudry()
{
  //this->~kconsens_instance();
}

void kconsens_instance_charon_hudry::solve ( stringstream &outverbose, stringstream &outtable )
{
  string tournamentFilename = this->filenameOfLoadedElection + ".tournament";
  string tournamentResultFilename = this->filenameOfLoadedElection + ".hudry-result";

  ofstream tournamentFile ( tournamentFilename.data() );
  outverbose << "Writing tournament file for median order interpretation.\n";
  this->putTournamentInput( tournamentFile );
  tournamentFile.close();
  
  string timeLimit = "";
  //if ( timelimit_flag )
  //{
  //  timeLimit = toStr ( timelimit );
  //} else
  //{ // Standard timelimit of two seconds.
  //  timeLimit = "2";
  //}

  string charon_hudryCommand = "bash -c 'echo -e \"" + tournamentFilename + "\\n" + timeLimit + "\" | ./linOrder > " + tournamentResultFilename + " 2>/dev/null'";
  outverbose << "Starting charon-hudry-method to solve the median order problem\n";
  //outverbose << "Timelimit of the noising method was set to " << timeLimit << " seconds.\n";
  int res = system ( charon_hudryCommand.data() );
  
  this->consensScore = get_charon_hudry_Kscore ( tournamentResultFilename );
  this->read_charon_hudry_Consensus( tournamentResultFilename, this->consens );  
}

long kconsens_instance_charon_hudry::get_charon_hudry_Kscore ( string charon_hudry_OutputFilename )
{
  long score;
  double scoreAsDouble;
  ifstream source ( charon_hudry_OutputFilename.data() );
  string tmp = "";
  string pretmp = "";
  if ( source == NULL )
  {
    cerr << "Could not read file: " << charon_hudry_OutputFilename << "\n";
  }
  else
  {
    while ( ! ( source.eof() ) )
    {
      source >> tmp;
      if ( ( tmp == "value" ) && ( pretmp == "best" ) )
      {
        source >> tmp; // throw away "is"
        long aboveGuarantee;
        source >> aboveGuarantee;
        score = aboveGuarantee + this->getLowerScoreBound();
        source.close();
        return score;
      }
      pretmp = tmp;
    }
  }
  source.close();
  return -1;
}

int kconsens_instance_charon_hudry::read_charon_hudry_Consensus ( string charon_hudry_OutputFilename, CandidateList &clist )
{
  long score;
  double scoreAsDouble;
  ifstream source ( charon_hudry_OutputFilename.data() );
  string tmp = "";
  string pretmp = "";
  string prepretmp = "";
  clist.clear();
  long candnumerread=0;
  if ( source == NULL )
  {
    cerr << "Could not read file: " << charon_hudry_OutputFilename << "\n";
    return -1;
  }
  else
  {
    while ( ! ( source.eof() ) )
    {
      source >> tmp;
      if ( ( (candnumerread>0) && (candnumerread<=this->getCandidatesCount()) ) || ( ( tmp == "order" ) && ( pretmp == "the" ) && ( prepretmp == "of" ) ) )
      {
        long candname;
        source >> tmp;
//        cout << tmp << " ";
        candname = strtol (tmp.data(), NULL, 0);
        if ( tmp == toStr(candname) )
        {
          candname++; // increase name by one since in ch-method name begin with 0
          clist.push_back(candname);
          candnumerread++;
// 	  cout << "\n Added: " << candname << " to consensus.\n";
        }
      }
      prepretmp = pretmp;
      pretmp = tmp;
      if ( ( tmp == "duration" ) || ( (tmp == pretmp) && ((tmp == prepretmp)) ) )
      {
        break;
      }

    }
  }
  source.close();
  return 0;
}