/*
 *  inconsistency.hpp
 *  
 *
 *  Created by Geoffery Ericksson on 15/04/10.
 *
 */

#ifndef INCONSISTENCY_HPP_20100415
#define INCONSISTENCY_HPP_20100415

#include <vector>
using std::vector;

#include <set>
using std::set;

#include <sstream>
using std::ostringstream;


namespace Sudoku {

/** Find a list of cells that contradict (according to the Sudoku rules) other cells. 
    If there are no contradictions then return an empty vector.
*/	
struct Inconsistency 
{
	// Can examine cells for '0' you want to check that each cell actually have a value.
	// This ability is useful at the end of a sudoku solve but not helpful before the completion.
	Inconsistency( const bool testForZero = false ) : testForZero_(testForZero) {}
	
	const char* name() const { return "Inconsistency"; }
	static const bool usesGrid = false;

	void operator()( ConstraintRegion& cr, set<Cell*>& inconsistentCells, set<Cell*>& explanatoryCells, string& explanation )
	{
		// First build up a count of the various candidates
		// Create a vector of many elements initialised to zero. Despite the waste of memory, we wont use the first n element, only using '0' - '9'.
		vector<std::size_t> frequency = Constraint::buildValueFrequencyTable( cr );
		
		// Now check if there are too many of any given value
		for( char value = (testForZero_?'0':'1'); value != ':'; ++value ) 
		{
			if( frequency[value] > 1 || (testForZero_ && '0' == value && frequency['0'] > 0) )
			{
				// There must exist a duplicate value or a cell that hasn't been completed
				// Now need to track down the cell and report it
    			ostringstream oss;
				oss << "Since cells ";
				for( ConstraintRegion::iterator crIt = cr.begin(); crIt != cr.end(); ++crIt )
				{
					if( (*crIt)->value() == value )
					{
						inconsistentCells.insert( *crIt );
						explanatoryCells.insert( *crIt );
						oss << (*crIt)->index() << " "; 
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


