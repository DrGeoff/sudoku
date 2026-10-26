#pragma once

#include <iosfwd>

#include <cstddef>

#include <set>

#include <string>

#include <vector>


namespace Sudoku {

/// Forward declaration
class Cell;


/** A ConstraintRegion is any collection of 9 Cells.
*/
using ConstraintRegion = std::vector<Cell*>;

/// Can stream a constraint region
std::ostream& operator<<( std::ostream& os, const ConstraintRegion& cr );
	
	
struct PrintCellLocationInformation
{
    void operator()( const ConstraintRegion& cr );
};
	

/** This struct contains all the functions that would normally be contained in the ConstraintRegion class */
struct Constraint
{
	enum class Type : int { row = 0, column, square, grid };		
	
	/// Is the constraint region a row, column or square?
	static Type calculateType( const ConstraintRegion& cr );
	
	/// If the constraint type is a row then return a column (and vice versa).  Undefined for squares or grid.
	static Type oppositeType( Type type );
	
	/// Return the string representation of the type
	static std::string typeToStr( Type type );
	
	/// If the constraint region is a row then return the row number.  Similarly for columns and squares. Undefined for grid.
	static std::size_t calculateConstantIndex( const ConstraintRegion& cr );	
	
	/// Return a frequency table of the candidates in the given constraint region	
	static std::vector<std::size_t> buildCandidateFrequencyTable( const ConstraintRegion& cr );
	
	/// Return a frequency table of the values in the given constraint region	
	static std::vector<std::size_t> buildValueFrequencyTable( const ConstraintRegion& cr );
	
	/// Search the given constraint region and return pointers to cells containing the given candidate value
	static std::vector<Cell*> findCandidateValue( const ConstraintRegion& cr, char candidateValue );
	
	/// Search the given constraint region and return the row/column/square/grid indexes (specified by indexType) containing the given candidate value
	static std::set<std::size_t> findCandidateValue( const ConstraintRegion& cr
													, char candidateValue
													, Type indexType  );
	
	/// Go through given constraint region and eliminate the candidate value, being careful to leave the preserved ones. 
	static bool eliminate( ConstraintRegion& cr
						 , char candidateValue
						 , std::set<Cell*>& changedCells 
						 , const std::set<std::size_t>& preserveIndexes = std::set<std::size_t>()
						 , Type preserveRegionType = Type::square );
	
	/// Go through given constraint region and eliminate the candidate value, being careful to leave the preserved ones. 
	static bool eliminate( ConstraintRegion& cr
						  , char candidateValue
						  , std::set<Cell*>& changedCells 
						  , const std::set<Cell*>& preserveCells = std::set<Cell*>() );
	
	/// Eliminate the candidate value from any cell in the box that intersects with a row or column of the given cell
	static bool eliminate( const Cell* pCell
						 , ConstraintRegion& box
				         , char candidateValue
						 , const std::string& explanationFinish
						 , std::set<Cell*>& changedCells
						 , std::set<Cell*>& explanatoryCells
				         , std::string& explanation
						 , const std::set<const Cell*>& preserveCells );
	
	/// Go through the given constraint region and find all the cells that contain the given candidate value.
	static std::vector< Cell* > findCellsContainingCandidateValue( const ConstraintRegion& cr, char candidateValue );
						  
};


} // namespace Sudoku


