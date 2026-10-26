/*
 * xyzwing.hpp
 *  sudoku
 *
 *  Created by Geoffery Ericksson on 2/06/10.
 *
 */


#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "cell.hpp"
#include "constraintregion.hpp"
#include "grid.hpp"

namespace Sudoku {

	
struct XYZWing 
{
	std::string_view name() const { return "XYZ Wing"; }
	inline static constexpr bool usesGrid = true;
	
	// Only call this with squares.
	void operator()( ConstraintRegion& cr, Grid& grid, std::set<Cell*>& changedCells, std::set<Cell*>& explanatoryCells, std::string& explanation )
	{
		assert( Constraint::calculateType(cr) == Constraint::Type::square );
		
		Cell* pXYZCell = nullptr;
		while( (pXYZCell = findXYZ( cr, pXYZCell )) )
		{
			// Without loss of generality we can impose the condition that the XZ cell lies in the same square as the XYZ cell
			Cell* pXZCell = nullptr;
			while( (pXZCell = findXZ( cr, pXYZCell, pXZCell )) )
			{
				// The YZ cell must now _not_ be in the same square as the XYZ cell (if it was this would be a locked triple instead)
				Cell* pYZCell = nullptr;
				while( (pYZCell = findYZ( grid, pXYZCell, pXZCell, pYZCell )) )
				{
					// Figure out the actual value of Z
					std::vector< char > intersection;
					std::set_intersection( pXZCell->candidates().begin(), pXZCell->candidates().end(), pYZCell->candidates().begin(), pYZCell->candidates().end(), std::back_inserter(intersection) );
					assert( intersection.size() == 1 );
					const char candidateZ = intersection[0];
					
					// eliminate Z in the intersection of the square and row/column of YZ
					std::ostringstream oss;
					oss << "XYZ wing composed of XYZ ";
					pXYZCell->writeCellLocationInformation(oss);
					pXYZCell->writeCandidateValues(oss);
					oss << " XZ ";
					pXZCell->writeCellLocationInformation(oss);
					pXZCell->writeCandidateValues(oss);
					oss << " YZ ";
					pYZCell->writeCellLocationInformation(oss);
					pYZCell->writeCandidateValues(oss) << '\n';
					
					std::set<const Cell*> preserveCells;
					preserveCells.insert( pXYZCell );
					preserveCells.insert( pXZCell );					
					Constraint::eliminate( pYZCell, grid.get( Constraint::Type::square )[pXYZCell->square()], candidateZ, oss.str(), changedCells, explanatoryCells, explanation, preserveCells );
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
		
		std::vector< char > difference;
		std::set_difference( pXYZCell->candidates().begin(), pXYZCell->candidates().end(), pXZCell->candidates().begin(), pXZCell->candidates().end(), std::back_inserter(difference) );
		assert( difference.size() == 1 );
		candidateY = difference[0];
		
		// Need to search the row and column that the XYZ cell occupies
		for( int t = 0; t != static_cast<int>(Constraint::Type::square); ++t )
		{
			const Constraint::Type crType = static_cast<Constraint::Type>(t);
			// If the previous pYZCell comes from being found in a column then fastforward onto checking columns
			if( pYZCell && (crType == Constraint::Type::row) && (pYZCell->column() == pXYZCell->column()) )
			{
				continue;
			}
			
			ConstraintRegion cr = grid.get(crType)[ pXYZCell->index(crType) ];			
			ConstraintRegion::iterator crIt = cr.begin();
			
			// If we have already tested a YZ from the row/column that we are about to search through then fast-forward crIt to start at pYZCell
			if( pYZCell && std::ranges::contains( cr, pYZCell ) )
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
				   && std::ranges::contains( currentCandidates, candidateY ) )
				{
					if( std::ranges::contains( currentCandidates, candidate1 )
					   || std::ranges::contains( currentCandidates, candidate3 ) )
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
		return nullptr;
	}
	
	Cell* findXZ( ConstraintRegion& cr, const Cell* pXYZCell, const Cell* pXZCell  )  
	{
		ConstraintRegion::iterator crIt = cr.begin();
		
		if( pXZCell != nullptr )
		{
			while( (*crIt) != pXZCell && crIt != cr.end() )
			{
				++crIt;
			}
			if( crIt == cr.end() )
			{
				return nullptr;					
			}
			++crIt;
		}
		
		for( ; crIt != cr.end(); ++crIt )
		{
			// First find a cell containing an XZ
			if( (*crIt)->candidates().size() == 2
			   && std::includes( pXYZCell->candidates().begin(), pXYZCell->candidates().end(), (*crIt)->candidates().begin(), (*crIt)->candidates().end() )
			  )
			{
				return *crIt;
			}
		}
		return nullptr;
	}
	
	Cell* findXYZ( ConstraintRegion& cr, const Cell* pXYZCell ) 
	{
		ConstraintRegion::iterator crIt = cr.begin();

		if( pXYZCell != nullptr )
		{
			while( (*crIt) != pXYZCell && crIt != cr.end() )
			{
				++crIt;
			}
			if( crIt == cr.end() )
			{
				return nullptr;					
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
		
		return nullptr;
	}
};
	
} // namespace Sudoku 

