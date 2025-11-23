/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2011 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ implementation file of kconsens_instance_glpk
 *   Description: Class that transforms the problem instanze to a linear program in "GNU MathProg Language".
 *                The linear program will be solved by glpk.
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
#include "kemtyps.hpp"
#include "kconsens_instance_glpk.hpp"
#include "jobmanagement.hpp"

kconsens_instance_glpk::kconsens_instance_glpk ( stringstream &outdebug, int timelimit_, bool timelimit_flag_ ) : kconsens_instance_candidate_relations_based ( outdebug )
{
  srand ( ( unsigned ) time ( 0 ) );
  this->isSynchronized = false;
  this->timelimit = timelimit_;
  this->timelimit_flag = timelimit_flag_;
}

kconsens_instance_glpk::~kconsens_instance_glpk()
{
  //this->~kconsens_instance();
}

long kconsens_instance_glpk::getGlpsolKscore ( string glpsolOutputFilename )
{
  long score;
  bool isOpt = false;
  //    for ( int i = 1; i < 7; i++ )
  //    {
  ifstream source ( glpsolOutputFilename.data() );
  string tmp;
  if ( source != NULL )
  {
    while ( ! ( source.eof() ) )
    {
      source >> tmp;
      if ( tmp == "UNDEFINED" )
      {
        return -1;
      }
      if ( tmp == "OPTIMAL" )
      {
        isOpt = true;
      }
      if ( tmp == "=" )
      {
        break;
      }
    }
    if ( isOpt )
    {
      source >> score;
      source.close();
      return score;
    }
  }
  source.close();
  //    }
  return -1;
}

int kconsens_instance_glpk::readCandidatePairRelations ( string cplexOutputFilename, CandidateCandidateRelationsMap &L )
{
  ifstream source ( cplexOutputFilename.data() );
  string tmp = "";
  string pretmp = "";
  string prepretmp = "";
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
        if ( pretmp == "Integer" )
        {
          return 0;
        }
        if ( ( prepretmp[0] == 'x' ) && ( pretmp == "*" ) && ( tmp == "1" ) )
        {
          size_t posBrLeft = prepretmp.find ( '[' );
          size_t posBrRight = prepretmp.find ( ']' );
          size_t posComma = prepretmp.find ( ',' );
          size_t posCandA = posBrLeft + 1;
          size_t posCandB = posComma + 1;
          size_t lengthCandA = posComma - posBrLeft - 1;
          size_t lengthCandB = posBrRight - posComma - 1;
          string strCandA = prepretmp.substr ( posCandA, lengthCandA );
          string strCandB = prepretmp.substr ( posCandB, lengthCandB );
          Candidate a;
          Candidate b;
          fromString<Candidate> ( a, strCandA, std::dec );
          fromString<Candidate> ( b, strCandB, std::dec );
          this->addInfo ( a, b, L );
        }
      }
      else
      {
        if ( ( tmp == "Activity" ) && ( pretmp == "name" ) && ( prepretmp == "Column" ) )
        {
          isInVarSection = true;
        }
      }
      prepretmp = pretmp;
      pretmp = tmp;
    }
  }

  return 0;
}

void kconsens_instance_glpk::solve ( stringstream &outverbose, stringstream &outtable )
{
  this->analyzeMajority ( 3. / 4. );
  string glpkFilename = this->filenameOfLoadedElection + ".mathprog";
  string glpkResultFilename = this->filenameOfLoadedElection + ".glpk-result";
  ofstream glpkFile ( glpkFilename.data() );
  this->putMathProgSourceCode ( glpkFile );
  glpkFile.close();

  string timelimstr = "";
  if ( timelimit_flag )
  {
    timelimstr = " --tmlim " + toStr ( timelimit );
  }
  string glpsolCommand = "glpsol --binarize --math " + glpkFilename + timelimstr + " -o " + glpkResultFilename + " > /dev/null";
  int res = system ( glpsolCommand.data() );

  this->consensScore = getGlpsolKscore ( glpkResultFilename );
  this->readCandidatePairRelations ( glpkResultFilename, this->L );
  this->relationsToConsens ( this->consens, this->L );
}
