#pragma once

#include <list>

#include <cassert>
#include <cstddef>
#include <format>
#include <iostream>
#include <stdexcept>
#include <string>

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
    using CandidateContainer = std::list<char>;
    
    explicit Cell( std::size_t cellIndex )
        : initial_( false )
        , cellIndex_( cellIndex )
		, row_( cellIndex/9 )   // Note that we are relying on the round down of integer division.
	    , column_( cellIndex_ % 9 )
    {
		// Note 1: the use of zero based indicies for the rows, columns, and squares
		// Note 2: I follow the usual C 2D array format of contiguous memory representing rows. 
		
		const std::size_t squareRow = row_ / 3;
		const std::size_t squareColumn = column_ / 3;
		square_ = 3*squareRow + squareColumn;

		// A cell will be initialised with the full set of possible candidates (i.e., 1,2,3,...,9)
		candidates_ = { '1', '2', '3', '4', '5', '6', '7', '8', '9' };
    }

    // Accessor
    CandidateContainer& candidates()             { return candidates_; }
    const CandidateContainer& candidates() const { return candidates_; }

	std::size_t index()  const { return cellIndex_; }
	std::size_t row()    const {	return row_; }
	std::size_t column() const { return column_; }
	std::size_t square() const {	return square_; }
	std::size_t index( Constraint::Type type ) const
	{
		switch( type )
		{
			case Constraint::Type::row    : return row_;       break;
			case Constraint::Type::column : return column_;    break;
			case Constraint::Type::square : return square_;    break;
			case Constraint::Type::grid   : return cellIndex_; break;
		}
		throw std::logic_error( "impossible constraint type given" );
	}
	

    // Reduce the set of candidates to the specifed value
    void initial( char value )
    {
        if( value < '1' || value > '9' )
        {
            throw std::invalid_argument( std::format( "invalid cell value '{}': expected '1' to '9'", value ) );
        }
        initial_ = true;
        candidates_.clear();  // remove all the unnecessary candidates.
        candidates_.push_back( value );
    }
    
    bool initial() const { return initial_; }
    
    char value() const
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

	std::ostream& writeCellLocationInformation( std::ostream& os ) const
	{
		os << '(' << index() << ' ' << row() << ' ' << column() << ' ' << square() << ") ";	
		return os;
	}

	std::ostream& writeCellShortLocationInformation( std::ostream& os ) const
	{
		os << '(' << row() << ',' << column() << ") ";	
		return os;
	}
	
	std::ostream& writeCandidateValues( std::ostream& os ) const
	{
		const CandidateContainer& currentCandidates = candidates();
		assert( !currentCandidates.empty() );
		
		os << '{';
		bool first = true;
		for( const char candidate : currentCandidates )
		{
			if( !first )
			{
				os << ' ';
			}
			os << candidate;
			first = false;
		}
        os << "} ";				
		return os;
	}	
	
private:
    CandidateContainer candidates_;   // What are the remaining candidate values for this cell.  If only one candidate is left, that is the answer
    bool initial_;                    // Is the value in this cell the one specified in the initial problem
    std::size_t cellIndex_;                // Where in the overall Sudoku grid does this cell lie (numbers are 0 to 80, top left cell is 0, bottom right is 80, proceeding along the row)
    std::size_t row_;                      // Finer grained location. What row does this cell belong to.  Note the zero based index.
	std::size_t column_;                   // Finer grained location.  What column does this cell belong to.  Note that the zero based index.
	std::size_t square_;                   // Finer grained location. What square does this cell belong to.  Note the zero based index.
};


inline std::ostream& operator<<( std::ostream& os, const Cell& cell )
{
    const Cell::CandidateContainer& candidates = cell.candidates();

    bool first = true;
    for( const char candidate : candidates )
    {
        if( !first )
        {
            os << ',';
        }
        os << candidate;
        first = false;
    }

    // Put on any extra padding
    os << std::string( 18 - 2 * candidates.size(), ' ' );

    return os;
}

	
} // namespace Sudoku


