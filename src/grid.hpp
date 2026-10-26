#ifndef GRID_HPP_20100327
#define GRID_HPP_20100327

#include "cell.hpp"
#include "constraintregion.hpp"

#include <cstddef>
#include <fstream>
#include <format>
#include <iosfwd>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>



namespace Sudoku {

struct Grid
{
    Grid()
        : cells()
        , rows(9)
        , columns(9)
        , squares(9)
    {
    	// Allocate all the cells
		cells.reserve(81);
    	for( std::size_t index = 0; index != 81; ++index )
    	{
    		cells.push_back(Cell(index));
    	}

    	// Allocate some memory for the rows, columns, squares (since a constraint region is only a typedef and not a real class)
        for( std::size_t constraintIndex = 0; constraintIndex != 9; ++constraintIndex )
        {
        	rows[constraintIndex].resize(9);
        	columns[constraintIndex].resize(9);
        	squares[constraintIndex].resize(9);
        }

        // Put the cells into the appropriate rows, columns and squares.
        // Note the use of zero based indicies.
        // I follow the usual C 2D array format of contiguous memory representing rows
        for( std::size_t constraintIndex = 0; constraintIndex != 9; ++constraintIndex )
        {
        	ConstraintRegion& currentRow = rows[constraintIndex];
        	ConstraintRegion& currentCol = columns[constraintIndex];
        	ConstraintRegion& currentSq  = squares[constraintIndex];

        	// The 9 squares of the grid are numbered 0,1,2 as the top, 3,4,5 as the middle, and 6,7,8, as the bottom
        	// square_start_cell is the cell number of the top left hand element of the square
        	const std::size_t squareStartCell = (constraintIndex/3) * 27 + (constraintIndex%3) * 3; // Note that we are relying on the integer division round towards zero.

        	// Each constraint region must have 9 cells
            for( std::size_t cellIndex = 0; cellIndex != 9; ++cellIndex )
            {
            	const std::size_t rowCellIndex = constraintIndex*9+cellIndex; // e.g., row 0 will have cells 0,1,...,8
            	currentRow[cellIndex] = &cells[rowCellIndex];
            	const std::size_t columnCellIndex = constraintIndex + cellIndex*9; // e.g., col 0 will have cells 0,9,...,72
            	currentCol[cellIndex] = &cells[columnCellIndex];

            	// squares are a little trickier.  The 0th square has cells
            	// 0,1,2
            	// 9,10,11
            	// 18,19,20

            	// The square offset is the amount you have to add onto the square_start_cell to get to the "cellIndex"th element of that square.
            	const std::size_t squareOffset = (cellIndex/3) * 9 + (cellIndex%3); // Note that we are relying on the integer division round towards zero. Hence /3 * 3 does not cancel to 1.
            	const std::size_t squareCellIndex = squareStartCell + squareOffset;
            	currentSq[cellIndex]  = &cells[squareCellIndex];
            }
        }
    }
	
	void parse( const std::string& filename )
	{
		if( filename.ends_with('v') )
		{
			parseCSV( filename );			
		}
		else if( filename.ends_with('k') )
		{
			parseSDK( filename );
		}
		else
		{
			throw std::runtime_error( std::format( "unrecognised puzzle file format '{}': the file name must end in '.k' (SDK) or '.v' (CSV)", filename ) );
		}
	}
	
	void parseSDK( const std::string& filename )
	{
		std::ifstream ifs( filename );
		if( !ifs )
		{
			throw std::runtime_error( std::format( "cannot open puzzle file '{}'", filename ) );
		}
		std::string line;
		std::size_t index = 0;
		
		// Extract a single line from the file
		while( std::getline( ifs,line ) && index < 81 ) 
		{	
			if( line.starts_with('#') || line.starts_with('[') ) 
			{
				continue;
			}
			
			for( const char c : line )
			{
				if( c == '\r' || c == ' ' || c == '\t' )
				{
					continue;
				}
				if( c != '.'  )
				{
					cells[index++].initial( c );		
				}
				else
				{
					++index;
				}				
			}
		}
		if( index != 81 )
		{
			throw std::runtime_error( std::format( "puzzle file '{}' contains {} cells, expected 81", filename, index ) );
		}
	}

	void parseCSV( const std::string& filename )
	{
		std::ifstream ifs( filename );
		if( !ifs )
		{
			throw std::runtime_error( std::format( "cannot open puzzle file '{}'", filename ) );
		}
		std::string line;
		std::size_t index = 0;
		
		// Extract a single line from the file
		while( std::getline( ifs,line ) && index < 81 ) 
		{		
			// Extract the ',' delimited elements from the file
			std::istringstream iss(line);
			std::string element;
			
			while( std::getline( iss, element, ',' ) )
			{
				if( !element.empty() && !(element[0]=='0') )
				{
					cells[index++].initial( element[0] );		
				}
				else
				{
					++index;
				}
			}
		}
		if( index != 81 )
		{
			throw std::runtime_error( std::format( "puzzle file '{}' contains {} cells, expected 81", filename, index ) );
		}
	}
	
	void writeCSV( const std::string& filename )
	{
		std::ofstream fout( filename );
		if( !fout )
		{
			throw std::runtime_error( std::format( "cannot open solution file '{}' for writing", filename ) );
		}
		for( std::size_t rowIndex = 0; rowIndex != 9; ++rowIndex )
		{
			for( std::size_t colIndex = 0; colIndex != 8; ++colIndex )
			{
				fout << rows[rowIndex][colIndex]->value() << ',';
			}
			fout << rows[rowIndex][8]->value()<< '\n';
		}
		fout.flush();
		if( !fout )
		{
			throw std::runtime_error( std::format( "failed to write solution file '{}'", filename ) );
		}
	}
	
	std::vector<ConstraintRegion>& get( Constraint::Type type )
	{
		switch( type )
		{
			case Constraint::Type::row:
				return rows;
			case Constraint::Type::column:
				return columns;
			case Constraint::Type::square:
				return squares;
			default:
				break;
		}
		throw std::logic_error( "unknown constraint type" );
	}

	const std::vector<ConstraintRegion>& get( Constraint::Type type ) const
	{
		switch( type )
		{
			case Constraint::Type::row:
				return rows;
			case Constraint::Type::column:
				return columns;
			case Constraint::Type::square:
				return squares;
			default:
				break;
		}
		throw std::logic_error( "unknown constraint type" );
	}
	
	Cell* get( std::size_t row_index, std::size_t column_index )
	{
		std::vector<Cell*>& row = get( Constraint::Type::row )[row_index];
		return row[column_index];
	}
	
	const Cell* get( std::size_t row_index, std::size_t column_index ) const
	{
		const std::vector<Cell*>& row = get( Constraint::Type::row )[row_index];
		return row[column_index];
	}
	
	
    std::vector<Cell> cells;
    std::vector<ConstraintRegion> rows;
    std::vector<ConstraintRegion> columns;
    std::vector<ConstraintRegion> squares;
};


std::ostream& operator<<( std::ostream& os, const Grid& grid )
{
	for( std::size_t rowIndex = 0; rowIndex != 9; ++rowIndex )
	{
		if(rowIndex%3 == 0)
		{
			os << std::string(178, '-') << '\n';
		}

		for( std::size_t colIndex = 0; colIndex != 9; ++colIndex )
		{
			(colIndex != 0 && colIndex%3 == 0) ? os << "  |  " : os << "  ";
			os << *grid.rows[rowIndex][colIndex];
		}
		os << '\n';
	}
    return os;
}


} // namespace Sudoku

#endif
