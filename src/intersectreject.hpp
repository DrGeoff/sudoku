/*
 * intersectreject.hpp
 *
 *  Created on: 31/03/2010
 *      Author: geoff
 */

#ifndef INTERSECTREJECT_HPP_2010331
#define INTERSECTREJECT_HPP_2010331

#include <iosfwd>
#include <iterator>
#include <algorithm>

#include <vector>
using std::vector;

#include <set>
using std::set;

#include "constraintregion.hpp"


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
 *  8          1,4        5          |  2          7,9        3          |  6,7        1,4,6,7    1,4,9            
 *  4,9        6          1,3,4,7,9  |  8          5,7,9      5,9        |  2          1,3,4,7    1,4,9            
 *
 */
struct IntersectReject
{
	const char* name() const { return "Intersect Reject"; }
	static const bool usesGrid = true;
	
	enum Type { undefined, row_intersect, column_intersect, square_intersect };
	
	// It is a precondition that UniquePerContraintRegion has been applied immediately before IntersectReject
	void operator()( ConstraintRegion& cr, Grid& grid, set<Cell*>& changedCells, set<Cell*>& explanatoryCells, string& explanation )
	{
		// First build up a count of the various candidates
		// Create a vector of many elements initialised to zero. Despite the waste of memory, we wont use the first n element, only using '1' - '9'.
		vector<size_t> frequency = Constraint::buildCandidateFrequencyTable( cr );
		
		for( char candidateValue = '1'; candidateValue != ':'; ++candidateValue )  // Check each of the numbers to see if it is possible to intersect-reject it.
		{			
			if( frequency[candidateValue] >1 && frequency[candidateValue] <= 3 )  // There must be three or less to possibly fit into a single row or column.  The single frequency is found by OnlySpot.
			{				
				// Examine each candidate set for the containment of the candidate value and check if the candidate values all lie in the same row or same column
				vector< Cell* > pointorsToCellsWithCandidateValue = Constraint::findCellsContainingCandidateValue( cr, candidateValue );
				Type intersectType = calculateIntersectionType( pointorsToCellsWithCandidateValue, cr );  // Check whether or not each of the cells with the candidate value lives in a single line or a single square
            	
	            if( intersectType != undefined )	
				{
					// Fantastic, we have found an intersect reject.
					assert( pointorsToCellsWithCandidateValue.size() == frequency[candidateValue] );
					
					// Figure out the appropriate row or column or square that needs fixing.
					ConstraintRegion* pNeedFixingConstraintRegion = 0;
					if( intersectType == column_intersect )
					{
						pNeedFixingConstraintRegion = &(grid.columns[ pointorsToCellsWithCandidateValue[0]->column() ]);
					}
					else if ( intersectType == row_intersect )
					{
						pNeedFixingConstraintRegion = &(grid.rows[ pointorsToCellsWithCandidateValue[0]->row() ]);						
					}
					else if ( intersectType == square_intersect )
					{
						pNeedFixingConstraintRegion = &(grid.squares[ pointorsToCellsWithCandidateValue[0]->square() ]);
					}
					//cout << "IntersectReject for candidate value "<< candidateValue <<" on square: " << cr[0]->square() << "\n" << cr << endl;
					//cout << "with " << ((intersectType == row_intersect)?"row":"column") << (*pLine)[0]->index() << "\n" << *pLine << endl;
					
					// Remove the candidate value from all the cells in the constraint region that needs fixing
					// Need to be careful not to eliminate from cells in the square( or line in the complementary case) that 
					// are not part of the "reject"
					bool didWork = false;
					const size_t preserveIndexNum = Constraint::calculateConstantIndex( cr );
					const Constraint::Type constraintType = Constraint::calculateType( cr ); 
					for( ConstraintRegion::iterator crIt2 = pNeedFixingConstraintRegion->begin(); crIt2 != pNeedFixingConstraintRegion->end(); ++crIt2 )						
					{
						if( (*crIt2)->index(constraintType) != preserveIndexNum )
						{
							const size_t candidateSizeBeforeRemoval = (*crIt2)->candidates().size();
							(*crIt2)->candidates().remove(candidateValue);
							
							if( candidateSizeBeforeRemoval > (*crIt2)->candidates().size() )
							{
								changedCells.insert(*crIt2);	
								didWork = true;
							}
						}
					}
					if( didWork )
					{
						copy( pointorsToCellsWithCandidateValue.begin(), pointorsToCellsWithCandidateValue.end(), inserter(explanatoryCells,explanatoryCells.end()) );						
						ostringstream oss;
						oss << "Cells ";
						for( vector<Cell*>::const_iterator cellIt = pointorsToCellsWithCandidateValue.begin(); cellIt != pointorsToCellsWithCandidateValue.end(); ++cellIt )
						{
							oss << (*cellIt)->index() << " ";
						}
						
						const size_t removalRegionIndex = Constraint::calculateConstantIndex(*pNeedFixingConstraintRegion);
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
	Type calculateIntersectionType( const vector< Cell* >& pointorsToCellsWithCandidateValue, const ConstraintRegion& cr )
	{
		assert( pointorsToCellsWithCandidateValue.size() >= 2 );
		Type intersectType = undefined;
		
		const Constraint::Type crType = Constraint::calculateType( cr );		
		if( Constraint::square != crType )  // The constraint region is a row or a column so the intersect must be with a square
		{
			const size_t squareOfFirstCell = pointorsToCellsWithCandidateValue[0]->square();
			if( pointorsToCellsWithCandidateValue[1]->square() == squareOfFirstCell )
			{
				//cout << "Square of first cell (index = " <<pointorsToCellsWithCandidateValue[0]->index() << ") matches square of second cell (index = " <<pointorsToCellsWithCandidateValue[1]->index() << endl;
				intersectType = square_intersect;
			}
			if( pointorsToCellsWithCandidateValue.size() == 3 
				&& intersectType != undefined 
			    && pointorsToCellsWithCandidateValue[2]->square() != squareOfFirstCell )
			{
				intersectType = undefined;  // The three values don't lie in the same square								
			}
		}
		else // The constraint region is a square so we have to be careful to handle either a row or column intersection
		{
			const size_t rowOfFirstCell = pointorsToCellsWithCandidateValue[0]->row();
			const size_t columnOfFirstCell = pointorsToCellsWithCandidateValue[0]->column();
			
			if( pointorsToCellsWithCandidateValue[1]->row() == rowOfFirstCell )
			{
				//cout << "Row of first cell (index = " <<pointorsToCellsWithCandidateValue[0]->index() << ") matches row of second cell (index = " <<pointorsToCellsWithCandidateValue[1]->index() << endl;
				intersectType = row_intersect;
			}
			else if( pointorsToCellsWithCandidateValue[1]->column() == columnOfFirstCell )
			{
				//cout << "Column of first cell (index = " <<pointorsToCellsWithCandidateValue[0]->index() << ") matches column of second cell (index = " <<pointorsToCellsWithCandidateValue[1]->index() << endl;
				
				intersectType = column_intersect; 
			}
			// else leave intersect type as undefined (this means the two don't lie in a single line or column)
			
			if( pointorsToCellsWithCandidateValue.size() == 3 && intersectType != undefined )
			{
				if( pointorsToCellsWithCandidateValue[2]->row() == rowOfFirstCell && intersectType == row_intersect )
				{ /* nothing to do */ }
				else if( pointorsToCellsWithCandidateValue[2]->column() == columnOfFirstCell && intersectType == column_intersect)
				{ /* nothing to do */ }
				else
				{
					intersectType = undefined;  // The three values don't lie on a single line or column
				}
			}			
		}
		return intersectType;
	}
	
	string intersectTypeToStr( const Type type )
	{
		switch (type) 
		{
			case undefined:
				return "undefined";
				break;
			case row_intersect:
				return "row intersection";
				break;
			case column_intersect:
				return "column intersection";
				break;
			case square_intersect:
				return "square intersection";
				break;
			default:
				assert(false);
				return "";
				break;
		}		
	}
};

} // namespace Sudoku

#endif /* INTERSECTREJECT_HPP_2010331*/
