#ifndef GRID_HPP_20100327
#define GRID_HPP_20100327

#include "cell.hpp"
#include "constraintregion.hpp"

#include <iosfwd> 
using std::ostream;

#include <string>
using std::string;

#include <vector>
using std::vector;

#include <fstream>
using std::ifstream;
using std::ofstream;



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
    	for( size_t index = 0; index != 81; ++index )
    	{
    		cells.push_back(Cell(index));
    	}

    	// Allocate some memory for the rows, columns, squares (since a constraint region is only a typedef and not a real class)
        for( size_t constraintIndex = 0; constraintIndex != 9; ++constraintIndex )
        {
        	rows[constraintIndex].resize(9);
        	columns[constraintIndex].resize(9);
        	squares[constraintIndex].resize(9);
        }

        // Put the cells into the appropriate rows, columns and squares.
        // Note the use of zero based indicies.
        // I follow the usual C 2D array format of contiguous memory representing rows
        for( size_t constraintIndex = 0; constraintIndex != 9; ++constraintIndex )
        {
        	ConstraintRegion& currentRow = rows[constraintIndex];
        	ConstraintRegion& currentCol = columns[constraintIndex];
        	ConstraintRegion& currentSq  = squares[constraintIndex];

        	// The 9 squares of the grid are numbered 0,1,2 as the top, 3,4,5 as the middle, and 6,7,8, as the bottom
        	// square_start_cell is the cell number of the top left hand element of the square
        	const size_t squareStartCell = (constraintIndex/3) * 27 + (constraintIndex%3) * 3; // Note that we are relying on the integer division round towards zero.

        	// Each constraint region must have 9 cells
            for( size_t cellIndex = 0; cellIndex != 9; ++cellIndex )
            {
            	const size_t rowCellIndex = constraintIndex*9+cellIndex; // e.g., row 0 will have cells 0,1,...,8
            	currentRow[cellIndex] = &cells[rowCellIndex];
            	const size_t columnCellIndex = constraintIndex + cellIndex*9; // e.g., col 0 will have cells 0,9,...,72
            	currentCol[cellIndex] = &cells[columnCellIndex];

            	// squares are a little trickier.  The 0th square has cells
            	// 0,1,2
            	// 9,10,11
            	// 18,19,20

            	// The square offset is the amount you have to add onto the square_start_cell to get to the "cellIndex"th element of that square.
            	const size_t squareOffset = (cellIndex/3) * 9 + (cellIndex%3); // Note that we are relying on the integer division round towards zero. Hence /3 * 3 does not cancel to 1.
            	const size_t squareCellIndex = squareStartCell + squareOffset;
            	currentSq[cellIndex]  = &cells[squareCellIndex];
            }
        }
    }
	
	void parse( const string& filename )
	{
		if( *filename.rbegin() == 'v' )
		{
			parseCSV( filename );			
		}
		else if( *filename.rbegin() == 'k' )
		{
			parseSDK( filename );
		}
		else throw "Unparsable file format";
	}
	
	void parseSDK( const string& filename )
	{
		ifstream ifs( filename.c_str() );
		std::string line;
		size_t index = 0;
		
		// Extract a single line from the file
		while( getline( ifs,line ) && index < 81 ) 
		{	
			if( *line.begin() == '#' || *line.begin() == '[' ) 
			{
				continue;
			}
			
			for( std::string::const_iterator it = line.begin(); it != line.end(); ++it )
			{
				if( *it == '\r' || *it == ' ' || *it == '\t' )
				{
					continue;
				}
				if( *it != '.'  )
				{
					cells[index++].initial( *it );		
				}
				else
				{
					++index;
				}				
			}
		}		
	}

	void parseCSV( const string& filename )
	{
		ifstream ifs( filename.c_str() );
		std::string line;
		size_t index = 0;
		
		// Extract a single line from the file
		while( getline( ifs,line ) && index < 81 ) 
		{		
			// Extract the ',' delimited elements from the file
			istringstream iss(line);
			std::string element;
			
			while( getline( iss, element, ',' ) )
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
	}
	
	void writeCSV( const string& filename )
	{
		ofstream fout( filename.c_str() );
		for( size_t rowIndex = 0; rowIndex != 9; ++rowIndex )
		{
			for( size_t colIndex = 0; colIndex != 8; ++colIndex )
			{
				fout << rows[rowIndex][colIndex]->value() << ',';
			}
			fout << rows[rowIndex][8]->value()<< '\n';
		}
	}
	
	vector<ConstraintRegion>& get( const Constraint::Type type )
	{
		switch( type )
		{
			case Constraint::row:
				return rows;
				break;
			case Constraint::column:
				return columns;
				break;
			case Constraint::square:
				return squares;
				break;
			default:
				throw;
				break;
		}
	}

	const vector<ConstraintRegion>& get( const Constraint::Type type ) const
	{
		switch( type )
		{
			case Constraint::row:
				return rows;
				break;
			case Constraint::column:
				return columns;
				break;
			case Constraint::square:
				return squares;
				break;
			default:
				throw;
				break;
		}
	}
	
	Cell* get( size_t row_index, size_t column_index )
	{
		vector<Cell*>& row = get( Constraint::row )[row_index];
		return row[column_index];
	}
	
	const Cell* get( size_t row_index, size_t column_index ) const
	{
		const vector<Cell*>& row = get( Constraint::row )[row_index];
		return row[column_index];
	}
	
	
    vector<Cell> cells;
    vector<ConstraintRegion> rows;
    vector<ConstraintRegion> columns;
    vector<ConstraintRegion> squares;
};


ostream& operator<<( ostream& os, const Grid& grid )
{
	for( size_t rowIndex = 0; rowIndex != 9; ++rowIndex )
	{
		if(rowIndex%3 == 0)
		{
			for(size_t loop = 0; loop < 178; ++loop)
			{
				os << '-';
			}
			os << '\n';
		}

		for( size_t colIndex = 0; colIndex != 9; ++colIndex )
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

