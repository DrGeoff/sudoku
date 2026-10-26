/*
 *  inconsistency.hpp
 *  
 *
 *  Created by Geoffery Ericksson on 15/04/10.
 *
 */

#ifndef INCONSISTENCY_HPP_20100415
#define INCONSISTENCY_HPP_20100415

#include <cstddef>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "cell.hpp"
#include "constraintregion.hpp"


namespace Sudoku {

/** Find a list of cells that contradict (according to the Sudoku rules) other cells. 
    If there are no contradictions then return an empty vector.
*/	
struct Inconsistency 
{
	// Can examine cells for '0' you want to check that each cell actually have a value.
	// This ability is useful at the end of a sudoku solve but not helpful before the completion.
	explicit Inconsistency( bool testForZero = false ) : testForZero_(testForZero) {}
	
	std::string_view name() const { return "Inconsistency"; }
	inline static constexpr bool usesGrid = false;

	void operator()( ConstraintRegion& cr, std::set<Cell*>& inconsistentCells, std::set<Cell*>& explanatoryCells, std::string& explanation )
	{
		// First build up a count of the various candidates
		// Create a vector of many elements initialised to zero. Despite the waste of memory, we wont use the first n element, only using '0' - '9'.
		std::vector<std::size_t> frequency = Constraint::buildValueFrequencyTable( cr );
		
		// Now check if there are too many of any given value
		for( char value = (testForZero_?'0':'1'); value != ':'; ++value ) 
		{
			if( frequency[value] > 1 || (testForZero_ && '0' == value && frequency['0'] > 0) )
			{
				// There must exist a duplicate value or a cell that hasn't been completed
				// Now need to track down the cell and report it
    			std::ostringstream oss;
				oss << "Since cells ";
				for( Cell* cell : cr )
				{
					if( cell->value() == value )
					{
						inconsistentCells.insert( cell );
						explanatoryCells.insert( cell );
						oss << cell->index() << " "; 
					}
				}		
				oss << " have the same value (" << value << ") there is an inconsistency in " 
				    << Constraint::typeToStr( Constraint::calculateType(cr) ) << " " << Constraint::calculateConstantIndex(cr) << '\n';
				explanation += oss.str();
			}
		}
	}
	
private:
	bool testForZero_;
};

} // namespace Sudoku

#endif //INCONSISTENCY_HPP_20100415
