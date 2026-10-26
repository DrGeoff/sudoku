#pragma once

#include <cstddef>
#include <set>
#include <sstream>
#include <string>
#include <string_view>

#include "cell.hpp"
#include "constraintregion.hpp"


namespace Sudoku {
	
struct LockedTuples 
{
	std::string_view name() const { return "Locked Tuples"; }
	inline static constexpr bool usesGrid = false;
	
	// The locked pairs simplification is easiest to understand.  If two cells only contain say {3,8} as their candidates
	// then 3 and 8 cannot possibly be candidates for other cells in the constraint region.  This extends to locked triples and locked quadruples.
	void operator()( ConstraintRegion& cr, std::set<Cell*>& changedCells, std::set<Cell*>& explanatoryCells, std::string& explanation )
	{
		// Find if a locked pair exists in this constraint region
		for( std::size_t i = 0; i != cr.size(); ++i )
		{
			Cell* cell = cr[i];
			const std::size_t tupleSize = cell->candidates().size();
			if( tupleSize >= 2 && tupleSize <= 4 )
			{
				std::set<Cell*> preserveCells; 
				preserveCells.insert( cell );
			    const Cell* pLockedCell = cell;
				
				// OK cell points to a possible first cell of a locked pair/triple/quadruple.  See if we can find a partner(s) for it.
				for( std::size_t j = i + 1; j != cr.size(); ++j )
				{
					Cell* partner = cr[j];
					// Note that in using operator== we are assuming that the candidate container has been sorted (which it was at construction)
					if( partner->candidates().size() == tupleSize
					    && partner->candidates() == cell->candidates() )
					{
						// Found another of the tuples
						preserveCells.insert( partner );
					}	
				}

				if( preserveCells.size() == tupleSize ) // OK we have found the locked tuples.  Remove them from the other cells in the constraint region.
				{
					// The preserve cells are the locked pair/triple/quadruple which explain what we are doing
					explanatoryCells.insert( preserveCells.begin(), preserveCells.end() );						
					
					// Remove every locked candidate.  Look up one of the preserved cells to see what the locked candidates actually are.
					bool didWork = false;
					for( const char lockedValue : pLockedCell->candidates() )
					{
						didWork |= Constraint::eliminate( cr, lockedValue, changedCells, preserveCells );
					}	
					if( didWork )
					{
						std::ostringstream oss;
						oss << "Cells ";
						for( const Cell* preserveCell : preserveCells )
						{
							oss << preserveCell->index() << " ";
						}
						
						oss << " contain the locked candidates { ";
						for( const char lockedValue : pLockedCell->candidates() )
						{
							oss << lockedValue << ' ';
						}
						oss << "}. We can remove those locked candidates from all other candidates in " 
    						<< Constraint::typeToStr( Constraint::calculateType(cr) ) << " " << Constraint::calculateConstantIndex(cr) << '\n';
						explanation += oss.str();					
					}											
				}
			}
		}
	}
};

} // namespace Sudoku

