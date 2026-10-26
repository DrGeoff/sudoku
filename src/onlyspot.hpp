/*
 *  onlyspot.hpp
 *  
 *
 *  Created by Geoffery Ericksson on 29/03/10.
 *
 */
#pragma once

#include <algorithm>
#include <cstddef>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "cell.hpp"
#include "constraintregion.hpp"


namespace Sudoku {

struct OnlySpot 
{
	std::string_view name() const { return "Only Spot"; }
	inline static constexpr bool usesGrid = false;
	
	// If the value only appears once in _any_ of the candidates in the constraint region then that must be the value of the cell  
    void operator()( ConstraintRegion& cr, std::set<Cell*>& changedCells, std::set<Cell*>& explanatoryCells, std::string& explanation )
	{
		// First build up a count of the various candidates
		// Create a vector of many elements initialised to zero. Despite the waste of memory, we wont use the first n element, only using '1' - '9'.
		std::vector<std::size_t> frequency = Constraint::buildCandidateFrequencyTable( cr );

		// Check if the count is 1 for any of them
		for( char candidateValue = '1'; candidateValue != ':'; ++candidateValue )
		{
			if( 1 == frequency[candidateValue] )
			{
				// Find where this value occurs and remove all other candidates for that cell.  
				for( Cell* cell : cr )
				{
					Cell::CandidateContainer& currentCandidates = cell->candidates();
					if( currentCandidates.size() > 1
					   && std::ranges::contains( currentCandidates, candidateValue ) )
					{
						// Delete all candidates then reinsert this candidate value
						currentCandidates.clear();						
						currentCandidates.push_back( candidateValue );
						explanatoryCells.insert(cell);
						std::ostringstream oss;
						oss << "Cell " << cell->index() << " is the only cell in " 
						    << Constraint::typeToStr( Constraint::calculateType(cr) ) << " " << Constraint::calculateConstantIndex(cr) 
						    << " with a candidate value of " << candidateValue << ". Removing other candidate values from this cell.\n";
						explanation += oss.str();
						changedCells.insert(cell);
					}
				}
			}
		}
	}

};
	
} // namespace Sudoku

