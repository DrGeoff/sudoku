#ifndef CELL_H_20100327
#define CELL_H_20100327

#include <list>
#include <iostream>
using std::ostream;

#include <cassert>

#include "constraintregion.hpp"

namespace Sudoku {

/**
 *  A Cell is an individual location inside the Sudoku grid.  
 *  It contains a list of candidates (i.e., the remaining potential valid solutions)
 *  and understands its location in the grid.
 *
 */
class Cell
{
public:
    typedef std::list<char> CandidateContainer;
    
    Cell( const size_t cellIndex )
        : initial_( false )
        , cellIndex_( cellIndex )
		, row_( cellIndex/9 )   // Note that we are relying on the round down of integer division.
	    , column_( cellIndex_ % 9 )
    {
		// Note 1: the use of zero based indicies for the rows, columns, and squares
		// Note 2: I follow the usual C 2D array format of contiguous memory representing rows. 
		
		const size_t squareRow = row_ / 3;
		const size_t squareColumn = column_ / 3;
		square_ = 3*squareRow + squareColumn;

		// A cell will be initialised with the full set of possible candidates (i.e., 1,2,3,...,9)
		for( char value = '1'; value != ':'; ++value )  // ':' is the character after '9' in ascii
        {
            candidates_.push_back( value );
        }
    }

    // Accessor
    CandidateContainer& candidates()             { return candidates_; }
    const CandidateContainer& candidates() const { return candidates_; }

	size_t index()  const { return cellIndex_; }
	size_t row()    const {	return row_; }
	size_t column() const { return column_; }
	size_t square() const {	return square_; }
	size_t index( Constraint::Type type ) const
	{
		switch( type )
		{
			case Constraint::row    : return row_;       break;
			case Constraint::column : return column_;    break;
			case Constraint::square : return square_;    break;
			case Constraint::grid   : return cellIndex_; break;
			default:                  assert(false);     break;
		}
	}
	

    // Reduce the set of candidates to the specifed value
    void initial( const char value )
    {
        initial_ = true;
        candidates_.clear();  // remove all the unnecessary candidates.
        candidates_.push_back( value );
    }
    
    char initial() { return initial_; }
    
    char value()
    {
		assert( !candidates_.empty() );
        if( candidates_.size() > 1 )
        {
            return '0';
        }
        else
        {
            return *candidates_.begin();
        }
    }

	ostream& writeCellLocationInformation( ostream& os )
	{
		os << '(' << index() << ' ' << row() << ' ' << column() << ' ' << square() << ") ";	
		return os;
	}

	ostream& writeCellShortLocationInformation( ostream& os )
	{
		os << '(' << row() << ',' << column() << ") ";	
		return os;
	}
	
	ostream& writeCandidateValues( ostream& os )
	{
		CandidateContainer::const_iterator it = candidates().begin();
		const CandidateContainer::const_iterator endIt = candidates().end();
		assert( it != endIt );
		
		os << '{' << *it++;
	    
		for( ; it != endIt; ++it )
		{
			os << ' ' << *it;
		}
        os << "} ";				
		return os;
	}	
	
private:
    CandidateContainer candidates_;   // What are the remaining candidate values for this cell.  If only one candidate is left, that is the answer
    bool initial_;                    // Is the value in this cell the one specified in the initial problem
    size_t cellIndex_;                // Where in the overall Sudoku grid does this cell lie (numbers are 0 to 80, top left cell is 0, bottom right is 80, proceeding along the row)
    size_t row_;                      // Finer grained location. What row does this cell belong to.  Note the zero based index.
	size_t column_;                   // Finer grained location. What column does this cell belong to.  Note the zero based index.
	size_t square_;                   // Finer grained location. What square does this cell belong to.  Note the zero based index.
};


inline ostream& operator<<( ostream& os, const Cell& cell )
{
    const Cell::CandidateContainer& candidates = cell.candidates();
    Cell::CandidateContainer::const_iterator it = candidates.begin();
    const Cell::CandidateContainer::const_iterator endIt = candidates.end();

    if( it != endIt )
    {
	os << *it;
        ++it;
    }

    for( ; it != endIt; ++it )
    {
        os << ',' << *it; 
    }

    // Put on any extra padding
    for( unsigned int padIndex = 0; padIndex != 18 - 2 * candidates.size(); ++padIndex )
    {
        os << ' ';
    }

    return os;
}

	
} // namespace Sudoku


#endif
