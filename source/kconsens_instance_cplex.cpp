/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2011 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ implementation file of kconsens_instance_cplex
 *   Description: Class that transforms the problem instanze into a linear program in cplex LP format.
 *                The linear program will be solved by cplex.
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
#include "kconsens_instance_cplex.hpp"
#include "jobmanagement.hpp"

kconsens_instance_cplex::kconsens_instance_cplex ( stringstream &outdebug, int timelimit_, bool timelimit_flag_ ) : kconsens_instance_candidate_relations_based ( outdebug )
{
  srand ( ( unsigned ) time ( 0 ) );
  this->isSynchronized = false;
  this->timelimit = timelimit_;
  this->timelimit_flag = timelimit_flag_;
}

kconsens_instance_cplex::~kconsens_instance_cplex()
{
  //this->~kconsens_instance();
}

long kconsens_instance_cplex::getCplexKscore ( string cplexOutputFilename )
{
  long score;
  double scoreAsDouble;
  bool lastStringWasObjective = false;
  int countIntegerString = 0;
  ifstream source ( cplexOutputFilename.data() );
  string tmp;
  if ( source != NULL )
  {
    while ( ! ( source.eof() ) )
    {
      source >> tmp;

      if ( countIntegerString > 1 )
      {
        if ( ( tmp == "=" ) && ( lastStringWasObjective ) )
        {
          source >> scoreAsDouble;
          score = ( long ) scoreAsDouble;
          source.close();
          return score;
        }

        if ( tmp == "Objective" )
        {
          lastStringWasObjective = true;
        }
        else
        {
          lastStringWasObjective = false;
        }
      }

      if ( tmp == "Integer" )
      {
        countIntegerString++;
      }
    }
  }
  source.close();
  return -1;
}

int kconsens_instance_cplex::readCandidatePairRelations ( string cplexOutputFilename, CandidateCandidateRelationsMap &L )
{
  ifstream source ( cplexOutputFilename.data() );
  string tmp = "";
  string pretmp = "";
  bool isInVarSection = false;
  if ( source == NULL )
  {
    cerr << "Could not read file: " << cplexOutputFilename << "\n";
  }
  else
  {
    while ( ! ( source.eof() ) )
    {
      source >> tmp;
      if ( isInVarSection )
      {
        if ( pretmp == "All" )
        {
          return 0;
        }
        if ( ( pretmp[0] == 'x' ) && ( tmp == "1.000000" ) )
        {
          size_t posBrLeft = pretmp.find ( '(' );
          size_t posBrRight = pretmp.find ( ')' );
          size_t posComma = pretmp.find ( ',' );
          size_t posCandA = posBrLeft + 1;
          size_t posCandB = posComma + 1;
          size_t lengthCandA = posComma - posBrLeft - 1;
          size_t lengthCandB = posBrRight - posComma - 1;
          string strCandA = pretmp.substr ( posCandA, lengthCandA );
          string strCandB = pretmp.substr ( posCandB, lengthCandB );
          Candidate a;
          Candidate b;
          fromString<Candidate> ( a, strCandA, std::dec );
          fromString<Candidate> ( b, strCandB, std::dec );
          this->addInfo ( a, b, L );
        }
      }
      else
      {
        if ( ( tmp == "Value" ) && ( pretmp == "Solution" ) )
        {
          isInVarSection = true;
        }
      }

      pretmp = tmp;
    }
  }

  return 0;
}


void kconsens_instance_cplex::solve ( stringstream &outverbose, stringstream &outtable )
{
  this->analyzeMajority ( 3. / 4. );
  string cplexFilename = this->filenameOfLoadedElection + ".lp";
  string cplexScriptFilename = this->filenameOfLoadedElection + ".cplex-script";
  string cplexResultFilename = this->filenameOfLoadedElection + ".cplex-result";

  ofstream cplexFile ( cplexFilename.data() );
  outverbose << "Writing CPLEX linear program file\n";
  this->putLpSourceCode ( cplexFile );
  cplexFile.close();

  ofstream cplexScriptFile ( cplexScriptFilename.data() );
  cplexScriptFile << "read " << cplexFilename << " lp \n";
  if ( timelimit_flag )
  {
    cplexScriptFile << "set timelimit " + toStr ( timelimit ) + "\n";
  }
  cplexScriptFile << "optimize" << "\n";
  cplexScriptFile << "display solution variables -" << "\n";
  cplexScriptFile << "quit" << "\n";
  cplexScriptFile.close();

  string cplexCommand = "cplex < " + cplexScriptFilename + " > " + cplexResultFilename;
  outverbose << "Starting CPLEX to solve the linear program\n";
  int res = system ( cplexCommand.data() );

  this->consensScore = getCplexKscore ( cplexResultFilename );
  this->readCandidatePairRelations ( cplexResultFilename, this->L );
  this->relationsToConsens ( this->consens, this->L );
}
