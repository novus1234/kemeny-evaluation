/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2011 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ implementation file of kconsens_instance_gurobi
 *   Description: Class that transforms the problem instanze into a linear program in cplex LP format.
 *                The linear program will be solved by gurobi.
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
#include "kconsens_instance_gurobi.hpp"
#include "jobmanagement.hpp"

kconsens_instance_gurobi::kconsens_instance_gurobi ( stringstream &outdebug, int timelimit_, bool timelimit_flag_ ) : kconsens_instance_candidate_relations_based ( outdebug )
{
  srand ( ( unsigned ) time ( 0 ) );
  this->isSynchronized = false;
  this->timelimit = timelimit_;
  this->timelimit_flag = timelimit_flag_;
}

kconsens_instance_gurobi::~kconsens_instance_gurobi()
{
  //this->~kconsens_instance();
}

long kconsens_instance_gurobi::getGurobiKscore ( string cplexOutputFilename )
{
  long score;
  double scoreAsDouble;
  ifstream source ( cplexOutputFilename.data() );
  string tmp = "";
  string pretmp = "";
  if ( source == NULL )
  {
    cerr << "Could not read file: " << cplexOutputFilename << "\n";
  }
  else
  {
    while ( ! ( source.eof() ) )
    {
      source >> tmp;
      if ( ( tmp == "objective" ) && ( pretmp == "Best" ) )
      {
        string strObjective;
        source >> strObjective;
        size_t posComma = strObjective.find ( ',' );
        string strDouble = strObjective.substr ( 0, posComma );
        fromString<double> ( scoreAsDouble, strDouble, std::scientific );
        score = ( long ) scoreAsDouble;
        source.close();
        return score;
      }
      pretmp = tmp;
    }
  }
  source.close();
  return -1;
}

int kconsens_instance_gurobi::readCandidatePairRelations ( string cplexOutputFilename, CandidateCandidateRelationsMap &L )
{
  ifstream source ( cplexOutputFilename.data() );
  string tmp = "";
  string pretmp = "";
  if ( source == NULL )
  {
    cerr << "Could not read file: " << cplexOutputFilename << "\n";
  }
  else
  {
    while ( ! ( source.eof() ) )
    {
      source >> tmp;
      if ( ( pretmp[0] == 'x' ) && ( tmp == "1" ) )
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
      pretmp = tmp;
    }
  }

  return 0;
}


void kconsens_instance_gurobi::solve ( stringstream &outverbose, stringstream &outtable )
{
  this->analyzeMajority ( 3. / 4. );
  string gurobiFilename = this->filenameOfLoadedElection + ".lp";
  string gurobiVarResultsFilename = this->filenameOfLoadedElection + ".gurobi-result.sol";
  string gurobiResultFilename = this->filenameOfLoadedElection + ".gurobi-result";

  ofstream gurobiFile ( gurobiFilename.data() );
  outverbose << "Writing GUROBI linear program file\n";
  this->putLpSourceCode ( gurobiFile );
  gurobiFile.close();

  string timelimstr = "";
  if ( timelimit_flag )
  {
    timelimstr = " TimeLimit=" + toStr ( timelimit );
  }

  string gurobiCommand = "gurobi_cl" + timelimstr + " ResultFile=" + gurobiVarResultsFilename + " " + gurobiFilename + " > " + gurobiResultFilename;
  outverbose << "Starting GUROBI to solve the linear program\n";
  int res = system ( gurobiCommand.data() );

  this->consensScore = getGurobiKscore ( gurobiResultFilename );
  this->readCandidatePairRelations ( gurobiVarResultsFilename, this->L );
  this->relationsToConsens ( this->consens, this->L );
}
