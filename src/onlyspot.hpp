/*
 *  onlyspot.hpp
 *  
 *
 *  Created by Geoffery Ericksson on 29/03/10.
 *
 */
#ifndef ONLYSPOT_HPP_20100329
#define ONLYSPOT_HPP_20100329

#include <vector>
using std::vector;

#include <set>
using std::set;

#include <iterator>
using std::insert_iterator;

#include <algorithm>
using std::copy;

#include "uniqueperconstraintregion.hpp"
#include "constraintregion.hpp"

#include <sstream>
using std::ostringstream;


namespace Sudoku {

struct OnlySpot 
{
	const char* name() { return "Only Spot"; }
	static const bool usesGrid = false;
	
	// If the value only appears once in _any_ of the candidates in the constraint region then that must be the value of the cell  
    void operator()( ConstraintRegion& cr, set<Cell*>& changedCells, set<Cell*>& explanatoryCells, string& explanation )
	{
		// First build up a count of the various candidates
		// Create a vector of many elements initialised to zero. Despite the waste of memory, we wont use the first n element, only using '1' - '9'.
		vector<std::size_t> frequency = Constraint::buildCandidateFrequencyTable( cr );

		// Check if the count is 1 for any of them
		for( char candidateValue = '1'; candidateValue != ':'; ++candidateValue )
		{
			if( 1 == frequency[candidateValue] )
			{
				//std::cout << "Only spot found " << candidateValue << std::endl;
				// Find where this value occurs and remove all other candidates for that cell.  
				for( ConstraintRegion::iterator crIt = cr.begin(); crIt != cr.end(); ++crIt )
				{
					//std::cout << "Examining cell " << (*crIt)->index() << std::endl; 	
					Cell::CandidateContainer& currentCandidates = (*crIt)->candidates();
					if( currentCandidates.size() > 1
					   && find(currentCandidates.begin(), currentCandidates.end(), candidateValue) != currentCandidates.end() )
					{
						//std::cout << "Removing "<< candidateValue << "from cell "<< (*crIt)->index() << std::endl; 	
						
						// Delete all candidates then reinsert this candidate value
						currentCandidates.clear();						
						currentCandidates.push_back( candidateValue );
						explanatoryCells.insert(*crIt);
						ostringstream oss;
						oss << "Cell " << (*crIt)->index() << " is the only cell in " 
						    << Constraint::typeToStr( Constraint::calculateType(cr) ) << " " << Constraint::calculateConstantIndex(cr) 
						    << " with a candidate value of " << candidateValue << ". Removing other candidate values from this cell.\n";
						explanation += oss.str();
						changedCells.insert(*crIt);
					}
				}
			}
		}
	}

};
	
} // namespace Sudoku

#endif
