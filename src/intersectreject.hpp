/*
 * intersectreject.hpp
 *
 *  Created on: 31/03/2010
 *      Author: geoff
 */

#ifndef INTERSECTREJECT_HPP_2010331
#define INTERSECTREJECT_HPP_2010331

#include <cassert>
#include <cstddef>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "cell.hpp"
#include "constraintregion.hpp"
#include "grid.hpp"


namespace Sudoku {

/** When we know that all of a particular candidate value for a square (say the number 7) are only in one row (or one column)
 *  then we can eliminate that value (e.g., 7) along the row (or column) in the other squares.  That is if the square restricts the
 *  candidate value in the intersection of a square and a row (or column) then we can reject the candidate on the same row (or column)
 *  in the other squares.
 *
 *  For example, examine the case where the first three squares have the following values (0 means not yet determined).
 *
 *  0,0,0, 0,3,0, 0,0,0
 *  0,0,0, 2,0,7, 4,9,0
 *  9,2,8, 0,0,0, 5,0,0
 *
 *  The bottom row of square one is filled up and 7 cannot appear on the middle row because of the 7 in the middle row in square two.
 *  Thus 7 must be in the top row of square one.  Hence 7 can be eliminated as a candidate for the top row of square three.
 *  Note that the UniquePerConstraintRegion will have already eliminated 7 from the middle row of square three.
 *
 *  The second example is the complementary case where the other cells in a line cannot be the given number and hence we can
 *  remove them from the other cells of the square.  In the first square we see that 9 is a candidate for both the top and bottom rows. 
 *  However, the remainder of the top row cannot have 9 as a candidate so 9 must be in the top row of the first square.
 *  Hence we can eliminate 9 from the bottom row of the first square.
 * 
 *  2          7,9        3,7,9      |  4          1          6          |  5          3,7        8                
 *  8          1,4        5          |  2          7,9        1          |  6,7        1,4,6,7    1,4,9            
 *  4,9        6          1,3,4,7,9  |  8          5,7,9      5,9        |  2          1,3,4,1    1,4,9            
 *
 */
struct IntersectReject
{
	std::string_view name() const { return "Intersect Reject"; }
	inline static constexpr bool usesGrid = true;
	
	enum class Type { undefined, row_intersect, column_intersect, square_intersect };
	
	// It is a precondition that UniquePerContraintRegion has been applied immediately before IntersectReject
	void operator()( ConstraintRegion& cr, Grid& grid, std::set<Cell*>& changedCells, std::set<Cell*>& explanatoryCells, std::string& explanation )
	{
		// First build up a count of the various candidates
		// Create a vector of many elements initialised to zero. Despite the waste of memory, we wont use the first n element, only using '1' - '9'.
		std::vector<std::size_t> frequency = Constraint::buildCandidateFrequencyTable( cr );
		
		for( char candidateValue = '1'; candidateValue != ':'; ++candidateValue )  // Check each of the numbers to see if it is possible to intersect-reject it.
		{			
			if( frequency[candidateValue] >1 && frequency[candidateValue] <= 3 )  // There must be three or less to possibly fit into a single row or column.  The single frequency is found by OnlySpot.
			{				
				// Examine each candidate set for the containment of the candidate value and check if the candidate values all lie in the same row or same column
				std::vector< Cell* > pointorsToCellsWithCandidateValue = Constraint::findCellsContainingCandidateValue( cr, candidateValue );
				Type intersectType = calculateIntersectionType( pointorsToCellsWithCandidateValue, cr );  // Check whether or not each of the cells with the candidate value lives in a single line or a single square
            	
	            if( intersectType != Type::undefined )	
				{
					// Fantastic, we have found an intersect reject.
					assert( pointorsToCellsWithCandidateValue.size() == frequency[candidateValue] );
					
					// Figure out the appropriate row or column or square that needs fixing.
					ConstraintRegion* pNeedFixingConstraintRegion = nullptr;
					if( intersectType == Type::column_intersect )
					{
						pNeedFixingConstraintRegion = &(grid.columns[ pointorsToCellsWithCandidateValue[0]->column() ]);
					}
					else if ( intersectType == Type::row_intersect )
					{
						pNeedFixingConstraintRegion = & ( grid.rows[ pointorsToCellsWithCandidateValue[ 0 ] ->row() ]);						
					}
					else if ( intersectType == Type::square_intersect )
					{
						pNeedFixingConstraintRegion = &( grid.squares[ pointorsToCellsWithCandidateValue [ 0 ] ->square() ]);
					}
					
					// Remove the candidate value from all the cells in the constraint region that needs fixing
					// Need to be careful not to eliminate from cells in the square( or line in the complementary case) that 
					// are not part of the "reject"
					bool didWork = false;
					const std::size_t preserveIndexNum = Constraint::calculateConstantIndex( cr );
					const Constraint::Type constraintType = Constraint::calculateType( cr ); 
					for( Cell* cell : *pNeedFixingConstraintRegion )						
					{
						if( cell->index(constraintType) != preserveIndexNum )
						{
							const std::size_t candidateSizeBeforeRemoval = cell->candidates().size();
							cell->candidates().remove(candidateValue);
							
							if( candidateSizeBeforeRemoval > cell->candidates().size() )
							{
								changedCells.insert(cell);	
								didWork = true;
							}
						}
					}
					if( didWork )
					{
						explanatoryCells.insert( pointorsToCellsWithCandidateValue.begin(), pointorsToCellsWithCandidateValue.end() );						
						std::ostringstream oss;
						oss << "Cells ";
						for( const Cell* cell : pointorsToCellsWithCandidateValue )
						{
							oss << cell->index() << " ";
						}
						
						const std::size_t removalRegionIndex = Constraint::calculateConstantIndex(*pNeedFixingConstraintRegion);
						const Constraint::Type removalRegionType = Constraint::calculateType(*pNeedFixingConstraintRegion);
						oss << "are the cells which must contain the candidate value "  << candidateValue 
						<< " for " << Constraint::typeToStr(removalRegionType) << " " << removalRegionIndex 
						<< " due to a " << intersectTypeToStr(intersectType) << " with " << Constraint::typeToStr(constraintType) << " " << preserveIndexNum
						<< ". Removing " << candidateValue << " from other cells in " 
						<< Constraint::typeToStr(removalRegionType) << " " << removalRegionIndex << '\n';
						explanation += oss.str();	
					}
				}
			}
		}
	}
	
private:
	Type calculateIntersectionType( const std::vector< Cell* >& pointorsToCellsWithCandidateValue, const ConstraintRegion& cr )
	{
		assert( pointorsToCellsWithCandidateValue.size() >= 2 );
		Type intersectType = Type::undefined;
		
		const Constraint::Type crType = Constraint::calculateType( cr );		
		if( Constraint::Type::square != crType )  // The constraint region is a row or a column so the intersect must be with a square
		{
			const std::size_t squareOfFirstCell = pointorsToCellsWithCandidateValue[0]->square();
			if( pointorsToCellsWithCandidateValue[1]->square() == squareOfFirstCell )
			{
				intersectType = Type::square_intersect;
			}
			if( pointorsToCellsWithCandidateValue.size() == 3 
				&& intersectType != Type::undefined 
			    && pointorsToCellsWithCandidateValue[2]->square() != squareOfFirstCell )
			{
				intersectType = Type::undefined;  // The three values don't lie in the same square								
			}
		}
		else // The constraint region is a square so we have to be careful to handle either a row or column intersection
		{
			const std::size_t rowOfFirstCell = pointorsToCellsWithCandidateValue[0]->row();
			const std::size_t columnOfFirstCell = pointorsToCellsWithCandidateValue[0]->column();
			
			if( pointorsToCellsWithCandidateValue [ 1 ] ->row() == rowOfFirstCell )
			{
				intersectType = Type::row_intersect;
			}
			else if( pointorsToCellsWithCandidateValue [ 1 ] ->column() == columnOfFirstCell )
			{
				intersectType = Type::column_intersect; 
			}
			// else leave intersect type as undefined (this means the two don't lie in a single line or column)
			
			if( pointorsToCellsWithCandidateValue.size() == 3 && intersectType != Type::undefined )
			{
				if( pointorsToCellsWithCandidateValue[2]->row() == rowOfFirstCell && intersectType == Type::row_intersect )
				{ /* nothing to do */ }
				else if( pointorsToCellsWithCandidateValue[2]->column() == columnOfFirstCell && intersectType == Type::column_intersect)
				{ /* nothing to do */ }
				else
				{
					intersectType = Type::undefined;  // The three values don't lie on a single line or column
				}
			}			
		}
		return intersectType;
	}
	
	std::string intersectTypeToStr( Type type )
	{
		switch (type) 
		{
			case Type::undefined:
				return "undefined";
			case Type::row_intersect:
				return "row intersection";
			case Type::column_intersect:
				return "column intersection";
			case Type::square_intersect:
				return "square intersection";
			default:
				assert(false);
				return "";
		}		
	}
};

} // namespace Sudoku

#endif /* INTERSECTREJECT_HPP_2010331*/
