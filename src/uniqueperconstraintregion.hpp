/*
 * uniqueperconstraintregion.hpp
 *
 *  Created on: 27/03/2010
 *      Author: geoff
 */

#ifndef UNIQUEPERCONSTRAINTREGION_HPP_20100327
#define UNIQUEPERCONSTRAINTREGION_HPP_20100327

#include <set>
using std::set;

#include <sstream>
using std::ostringstream;

#include "constraintregion.hpp"


namespace Sudoku {

struct UniquePerConstraintRegion 
{
	const char* name() const { return "Unique Per Constraint Region"; }
	static const bool usesGrid = false;
	
	// If the set of candidates contains only one element then we can remove that element from all other candidates in the constraint region
	void operator()( ConstraintRegion& cr, set<Cell*>& changedCells, set<Cell*>& explanatoryCells, string& explanation )
	{
		// Check if a cell has only one element
		for( ConstraintRegion::iterator crIt = cr.begin(); crIt != cr.end(); ++crIt )
		{
			if( (*crIt)->candidates().size() == 1 )
			{
				// Found a final value in a cell, so remove that value from all other constraint regions				
				set<Cell*> preserveCells; 
				preserveCells.insert( *crIt );
				const bool didWork = Constraint::eliminate( cr, (*crIt)->value(), changedCells, preserveCells );
				if( didWork )
				{
					explanatoryCells.insert(*crIt);
					ostringstream oss;
					oss << "Since cell " << (*crIt)->index() << " has the value " << (*crIt)->value() 
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
