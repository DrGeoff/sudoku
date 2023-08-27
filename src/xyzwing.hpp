/*
 *  xyzwing.hpp
 *  sudoku
 *
 *  Created by Geoffery Ericksson on 2/06/10.
 *
 */


#ifndef XYZWING_HPP_20100602
#define XYZWING_HPP_20100602

#include <vector>
using std::vector;

#include <iterator>
using std::back_inserter;

#include <algorithm>
using std::set_difference;

#include <set>
using std::set;
#include "grid.hpp"
#include "constraintregion.hpp"
#include "cell.hpp"

namespace Sudoku {

	
struct XYZWing 
{
	const char* name() const { return "XYZ Wing"; }
	static const bool usesGrid = true;
	
	// Only call this with squares.
	void operator()( ConstraintRegion& cr, Grid& grid, set<Cell*>& changedCells, set<Cell*>& explanatoryCells, string& explanation )
	{
		assert( Constraint::calculateType(cr) == Constraint::square );
		
		Cell* pXYZCell = 0;
		while( (pXYZCell = findXYZ( cr, pXYZCell )) )
		{
			// Without loss of generality we can impose the condition that the XZ cell lies in the same square as the XYZ cell
			Cell* pXZCell = 0;
			while( (pXZCell = findXZ( cr, pXYZCell, pXZCell )) )
			{
				// The YZ cell must now _not_ be in the same square as the XYZ cell (if it was this would be a locked triple instead)
				Cell* pYZCell = 0;
				while( (pYZCell = findYZ( grid, pXYZCell, pXZCell, pYZCell )) )
				{
					// Figure out the actual value of Z
					vector< char > intersection;
					set_intersection( pXZCell->candidates().begin(), pXZCell->candidates().end(), pYZCell->candidates().begin(), pYZCell->candidates().end(), back_inserter(intersection) );
					assert( intersection.size() == 1 );
					const char candidateZ = intersection[0];
					
					// eliminate Z in the intersection of the square and row/column of YZ
					ostringstream oss;
					oss << "XYZ wing composed of XYZ ";
					pXYZCell->writeCellLocationInformation(oss);
					pXYZCell->writeCandidateValues(oss);
					oss << " XZ ";
					pXZCell->writeCellLocationInformation(oss);
					pXZCell->writeCandidateValues(oss);
					oss << " YZ ";
					pYZCell->writeCellLocationInformation(oss);
					pYZCell->writeCandidateValues(oss) << "\n";
					
					set<const Cell*> preserveCells;
					preserveCells.insert( pXYZCell );
					preserveCells.insert( pXZCell );					
					Constraint::eliminate( pYZCell, grid.get( Constraint::square )[pXYZCell->square()], candidateZ, oss.str(), changedCells, explanatoryCells, explanation, preserveCells );
				}
			}
		}
	}
	
private:	
	Cell* findYZ( const Grid& grid, const Cell* pXYZCell, const Cell* pXZCell, const Cell* pYZCell )  
	{
		assert( pXYZCell->candidates().size() == 3 );
		assert( pXZCell->candidates().size() == 2 );
		
		// Don't know yet if X is candidate1 or candidate3
		char candidate1 = *pXZCell->candidates().begin();
		char candidateY = 0; // This is easily determined as the value _not_ in the 
		char candidate3 = *pXZCell->candidates().rbegin();
		
		vector< char > difference;
		set_difference( pXYZCell->candidates().begin(), pXYZCell->candidates().end(), pXZCell->candidates().begin(), pXZCell->candidates().end(), back_inserter(difference) );
		assert( difference.size() == 1 );
		candidateY = difference[0];
		
		// Need to search the row and column that the XYZ cell occupies
		for( Constraint::Type crType = static_cast<Constraint::Type>(0); crType != Constraint::square; crType = static_cast<Constraint::Type>(crType + 1) )
		{
			// If the previous pYZCell comes from being found in a column then fastforward onto checking columns
			if( pYZCell && (crType == Constraint::row) && (pYZCell->column() == pXYZCell->column()) )
			{
				crType = static_cast<Constraint::Type>(crType + 1);
			}
			
			ConstraintRegion cr = grid.get(crType)[ pXYZCell->index(crType) ];			
			ConstraintRegion::iterator crIt = cr.begin();
			
			// If we have already tested a YZ from the row/column that we are about to search through then fast-forward crIt to start at pYZCell
			if( pYZCell && (find(cr.begin(), cr.end(), pYZCell) != cr.end() ) )
			{
				while( (*crIt) != pYZCell && crIt != cr.end() )
				{
					++crIt;
				}
				if( crIt != cr.end() )
				{
					++crIt;
				}
			}
			
			for( ; crIt != cr.end(); ++crIt )
			{
				const Cell::CandidateContainer currentCandidates = (*crIt)->candidates();
				if( currentCandidates.size() == 2
				   && find( currentCandidates.begin(), currentCandidates.end(), candidateY ) != currentCandidates.end() )
				{
					// << "Candidates is 2 and candidateY " << candidateY << " is in candidates." << endl;
					if( find( currentCandidates.begin(), currentCandidates.end(), candidate1 ) != currentCandidates.end()
					   || find( currentCandidates.begin(), currentCandidates.end(), candidate3 ) != currentCandidates.end() )
					{
						// The YZ cell must now _not_ be in the same square as the XYZ cell (if it was this would be a locked triple instead)
						if( (*crIt)->square() != pXYZCell->square() )
						{
							return *crIt;	
						}
					}
				}
			}														
		}
		return 0;
	}
	
	Cell* findXZ( ConstraintRegion& cr, const Cell* pXYZCell, const Cell* pXZCell  )  
	{
		ConstraintRegion::iterator crIt = cr.begin();
		
		if( pXZCell != 0 )
		{
			while( (*crIt) != pXZCell && crIt != cr.end() )
			{
				++crIt;
			}
			if( crIt == cr.end() )
			{
				return 0;					
			}
			++crIt;
		}
		
		for( ; crIt != cr.end(); ++crIt )
		{
			// First find a cell containing an XZ
			if( (*crIt)->candidates().size() == 2
			   && includes( pXYZCell->candidates().begin(), pXYZCell->candidates().end(), (*crIt)->candidates().begin(), (*crIt)->candidates().end() )
			  )
			{
				return *crIt;
			}
		}
		return 0;
	}
	
	Cell* findXYZ( ConstraintRegion& cr, const Cell* pXYZCell ) 
	{
		ConstraintRegion::iterator crIt = cr.begin();

		if( pXYZCell != 0 )
		{
			while( (*crIt) != pXYZCell && crIt != cr.end() )
			{
				++crIt;
			}
			if( crIt == cr.end() )
			{
				return 0;					
			}
			++crIt;
		}
		
		for( ; crIt != cr.end(); ++crIt )
		{
			// First find a cell containing an XYZ
			if( (*crIt)->candidates().size() == 3 )
			{
				return *crIt;
			}
		}
		
		return 0;
	}
};
	
} // namespace Sudoku 

#endif // XYZWING_HPP_20100602
