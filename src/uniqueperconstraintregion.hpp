/*
 * uniqueperconstraintregion.hpp
 *
 *  Created on: 27/03/2010
 *      Author: geoff
 */

#ifndef UNIQUEPERCONSTRAINTREGION_HPP_20100327
#define UNIQUEPERCONSTRAINTREGION_HPP_20100327

#include <set>
#include <sstream>
#include <string>
#include <string_view>

#include "cell.hpp"
#include "constraintregion.hpp"


namespace Sudoku {

struct UniquePerConstraintRegion 
{
	std::string_view name() const { return "Unique Per Constraint Region"; }
	inline static constexpr bool usesGrid = false;
	
	// If the set of candidates contains only one element then we can remove that element from all other candidates in the constraint region
	void operator()( ConstraintRegion& cr, std::set<Cell*>& changedCells, std::set<Cell*>& explanatoryCells, std::string& explanation )
	{
		// Check if a cell has only one element
		for( Cell* cell : cr )
		{
			if( cell->candidates().size() == 1 )
			{
				// Found a final value in a cell, so remove that value from all other constraint regions				
				std::set<Cell*> preserveCells; 
				preserveCells.insert( cell );
				const bool didWork = Constraint::eliminate( cr, cell->value(), changedCells, preserveCells );
				if( didWork )
				{
					explanatoryCells.insert(cell);
					std::ostringstream oss;
					oss << "Since cell " << cell->index() << " has the value " << cell->value() 
					    << " we can remove that value from all other candidates in " 
					    << Constraint::typeToStr( Constraint::calculateType(cr) ) << " " << Constraint::calculateConstantIndex(cr) << '\n';
					explanation += oss.str();					
				}
			}
		}
	}
};

} // namespace Sudoku


#endif /* UNIQUEPERCONSTRAINTREGION_HPP_ */
