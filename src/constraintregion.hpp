#ifndef CONSTRAINTREGION_HPP_20100327
#define CONSTRAINTREGION_HPP_20100327

#include <iosfwd>
using std::ostream;

#include <vector>
using std::vector;

#include <set>
using std::set;

#include <string>
using std::string;


namespace Sudoku {

/// Forward declaration
class Cell;


/** A ConstraintRegion is any collection of 9 Cells.
*/
typedef vector<Cell*>  ConstraintRegion;

/// Can stream a constraint region
ostream& operator<<( ostream& os, const ConstraintRegion& cr );
	
	
struct PrintCellLocationInformation
{
    void operator()( const ConstraintRegion& cr );
};
	

/** This struct contains all the functions that would normally be contained in the ConstraintRegion class */
struct Constraint
{
	enum Type { row=0, column, square, grid };		
	
	/// Is the constraint region a row, column or square?
	static Type calculateType( const ConstraintRegion& cr );
	
	/// If the constraint type is a row then return a column (and vice versa).  Undefined for squares or grid.
	static Type oppositeType( const Constraint::Type type );
	
	/// Return the string representation of the type
	static string typeToStr( const Constraint::Type type );
	
	/// If the constraint region is a row then return the row number.  Similarly for columns and squares. Undefined for grid.
	static std::size_t calculateConstantIndex( const ConstraintRegion& cr );	
	
	/// Return a frequency table of the candidates in the given constraint region	
	static vector<std::size_t> buildCandidateFrequencyTable( const ConstraintRegion& cr );
	
	/// Return a frequency table of the values in the given constraint region	
	static vector<std::size_t> buildValueFrequencyTable( const ConstraintRegion& cr );
	
	/// Search the given constraint region and return pointers to cells containing the given candidate value
	static vector<Cell*> findCandidateValue( const ConstraintRegion& cr, const char candidateValue );
	
	/// Search the given constraint region and return the row/column/square/grid indexes (specified by indexType) containing the given candidate value
	static set<std::size_t> findCandidateValue( const ConstraintRegion& cr, const char candidateValue, const Type indexType  );
	
	/// Go through given constraint region and eliminate the candidate value, being careful to leave the preserved ones. 
	static bool eliminate( ConstraintRegion& cr
						 , const char candidateValue
						 , set<Cell*>& changedCells 
						 , const set<std::size_t>& preserveIndexes = set<std::size_t>()
						 , const Type preserveRegionType = square );
	
	/// Go through given constraint region and eliminate the candidate value, being careful to leave the preserved ones. 
	static bool eliminate( ConstraintRegion& cr
						  , const char candidateValue
						  , set<Cell*>& changedCells 
						  , const set<Cell*>& preserveCells = set<Cell*>() );
	
	/// Eliminate the candidate value from any cell in the box that intersects with a row or column of the given cell
	static bool eliminate( const Cell* const pCell
						 , ConstraintRegion& box
				         , const char candidateValue
						 , const string& explanationFinish
						 , set<Cell*>& changedCells
						 , set<Cell*>& explanatoryCells
				         , string& explanation
						 , const set<const Cell*>& preserveCells );
	
	/// Go through the given constraint region and find all the cells that contain the given candidate value.
	static vector< Cell* > findCellsContainingCandidateValue( const ConstraintRegion& cr, const char candidateValue );
						  
};


} // namespace Sudoku


#endif

