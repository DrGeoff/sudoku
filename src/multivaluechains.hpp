/*
 *  multivaluechains.hpp
 *  
 *
 *  Created by Geoffery Ericksson on 14/05/10.
 *
 */
#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "cell.hpp"
#include "chain.hpp"
#include "constraintregion.hpp"
#include "grid.hpp"

namespace Sudoku {

/**
 * We are looking for a Chain of bi-value cells where each link in the chain is two cells in the same Constraint Region with a value in common. 
 * You start with a bi-value cell, say {az}, and look for another bi-value cell in a common Constraint Region with a value in common with it, 
 * say {ab}. After that we look for {bc}, then {cd} and so on. The aim is to find a cell whose other value is z, for example {dz}. 
 * Now {az} might be z but if it's 'a', then {ab} is 'b', {bc} is 'c', {cd} is 'd' and {dz} is 'z', so any cell in the same Constraint Region 
 * as both {az} and {dz} cannot be 'z' since one of them must be. A one link Multi-Value Chain is a Locked Pair. A two link Multi-Value Chain 
 * is a Locked Triple if all three cells {az}, {ab} and {bz} are in the same Constraint Region, and if {az} and {bz} aren't in the same Constraint Region 
 * then it is an XY-Zap. Longer Chains will often enable eliminations not discernible with any other Solving Rule.
 * 
 * The above explanation is shamelessly lifted from http://www.sudoku-help.com/Multi-Value-Chains.htm
 *
 * In the following example we can remove the candidate value 1 from every cell marked with an X:
 *
 *  1,3        0          0          |  0          3,7          0          |  X          X          X                
 *  0          0          0          |  3          0            6,7        |  0          6,8        0 
 *  X          X          X          |  0          0            0          |  0          1          1,8            
 *
 */	
struct MultiValueChains 
{
	std::string_view name() const { return "Multi-value chains"; }
	inline static constexpr bool usesGrid = true;

	void operator()( ConstraintRegion& cr, Grid& grid, std::set<Cell*>& changedCells, std::set<Cell*>& explanatoryCells, std::string& explanation )
	{
		for( Cell* pStartCell : cr )
		{
			for( const char candidateValue : pStartCell->candidates() )
			{
				if( pStartCell->candidates().size() == 2 )
				{	
					// The search candidate is always the other candidate value in the start cell 
					// For example, if the candidates are {4,7} and we are hoping to eliminate 4 from the constraint region then we 
					// need to search for what happens when 7 is the actual value.  Hopefully some chain will show that 4 must turn up elsewhere
					const char searchCandidateValue = (candidateValue == *pStartCell->candidates().begin()) ? *pStartCell->candidates().rbegin() : *pStartCell->candidates().begin();
					assert( pStartCell->candidates().size() == 2 );
					std::vector< Chain > allChains = findAllChains( searchCandidateValue, pStartCell, grid );
					
					// Now go through the chains and see if we can find one that finishes in the same constraint region
					for( const Chain& chain : allChains )
					{
						// Verify that the chain starts and finishes with the same candidate value.
						const char endOfChain = followChain( candidateValue, chain );
						if( endOfChain == candidateValue )
						{
							const Cell* pEndCell = *(chain.rbegin());
                            // Remove from matching constraint regions
							for( int t = 0; t != static_cast<int>(Constraint::Type::grid); ++t )
							{
								const Constraint::Type constraintType = static_cast<Constraint::Type>(t);
								if( pStartCell->index(constraintType) == pEndCell->index(constraintType) )
								{ 
									std::set<std::size_t> preserveIndexes; 
									preserveIndexes.insert( pStartCell->index() );
									preserveIndexes.insert( pEndCell->index() );							
									
									const std::size_t matchIndex = pStartCell->index(constraintType); 
									ConstraintRegion& eliminationRegion = grid.get(constraintType)[ matchIndex ];
									if( Constraint::eliminate( eliminationRegion, candidateValue, changedCells, preserveIndexes, Constraint::Type::grid ) )
									{
										std::ostringstream oss;
										oss << "Eliminating candidate value "  << candidateValue 
											<< " from " << Constraint::typeToStr( constraintType ) << " " << matchIndex
											<< " due to multivalue chain: ";
										writeChainWithValues( oss, chain ) << '\n';
										explanation += oss.str();
										
										// TODO: insert whole chain into explanatory cells
									}
								}
							}
							
							//Remove from intersection of constraint regions		
							if( pStartCell->column() != pEndCell->column() && pEndCell->row() != pStartCell->row() && pStartCell->square() != pEndCell->square() )
							{
								std::ostringstream oss;
								oss << " multivalue chain: ";
								writeChainWithValues( oss, chain ) << '\n';
								std::set<const Cell*> preserveCells;
								preserveCells.insert( pStartCell );
								preserveCells.insert( pEndCell );					
								
								Constraint::eliminate( pStartCell, grid.get( Constraint::Type::square )[pEndCell->square()], candidateValue, oss.str(), changedCells, explanatoryCells, explanation, preserveCells );
								Constraint::eliminate( pEndCell, grid.get( Constraint::Type::square )[pStartCell->square()], candidateValue, oss.str(), changedCells, explanatoryCells, explanation, preserveCells );
							}
						}
					}
				}
			}
		}
	}

private:	
	const char followChain( char startCandidateValue, const Chain& chain )
	{
		char candidateValue = startCandidateValue;
		for( const Cell* cell : chain )
		{
			candidateValue = (*cell->candidates().begin()==candidateValue) ? (*cell->candidates().rbegin()) : (*cell->candidates().begin());
		}
		return candidateValue;
	}
	
	/**
	 * This version of findAllChains returns all possible bi-directional link chains.  
	 * It even returns chains that can be extended further ( and will also return that further extension)
	 */
	std::vector< Chain > findAllChains( char searchCandidateValue, Cell* pStartCell, const Grid& grid )
	{
		std::vector< Chain > allChains;
		for( int t = 0; t != static_cast<int>(Constraint::Type::grid); ++t )
		{
			const Constraint::Type constraintType = static_cast<Constraint::Type>(t);
			Chain chain;
			assert( pStartCell->candidates().size() == 2 );
			chain.push_back( pStartCell );
			findChainsInConstraintType( searchCandidateValue, grid, constraintType, chain, allChains );	
		}				
		return allChains;
	}
	
	void findChainsInConstraintType(  char searchCandidateValue
									, const Grid& grid
									, const Constraint::Type searchConstraintType
									, Chain& currentChain
									, std::vector< Chain >& allChains )
	{
		const Cell* pSearchCell = *currentChain.rbegin();
		
		// If the current cell doesn't contain exactly two candidates then we can't search any further down this chain.
		assert( pSearchCell->candidates().size() == 2 );
		
		// Get one of the constraint regions that the cell is in for searching
		const ConstraintRegion& cr = grid.get( searchConstraintType )[ pSearchCell->index( searchConstraintType ) ];

		std::vector< Cell* > pointorsToCellsWithCandidateValue = Constraint::findCellsContainingCandidateValue( cr, searchCandidateValue );
		
		// If a cell is already in the chain then remove it from the cells that need further exploration
		for( const Cell* cell : currentChain )
		{
			std::vector< Cell* >::iterator itRemove = std::find( pointorsToCellsWithCandidateValue.begin(), pointorsToCellsWithCandidateValue.end(), cell);
			if( itRemove != pointorsToCellsWithCandidateValue.end() )
			{
				pointorsToCellsWithCandidateValue.erase( itRemove );
			}
		}
		
		// There may be many cells containing the candidate value but we can only go further with the chain if we can find a cell with exactly 2 candidates.
		for( Cell* cell : pointorsToCellsWithCandidateValue )
		{				
			const Cell::CandidateContainer& nextSearchCellCandidates = cell->candidates();
			if( nextSearchCellCandidates.size() == 2 ) 
			{
				Chain clone = currentChain;							
				clone.push_back( cell );		
				for( int t = 0; t != static_cast<int>(Constraint::Type::grid); ++t )
				{
					const Constraint::Type constraintType = static_cast<Constraint::Type>(t);
					findChainsInConstraintType( (*nextSearchCellCandidates.begin()==searchCandidateValue)?(*nextSearchCellCandidates.rbegin()):(*nextSearchCellCandidates.begin()), grid, constraintType, clone, allChains );	

					// Don't keep a chain which only has one cell (useless) or two cells (a locked pair) and don't keep a duplicate record
					if( clone.size() > 2 && !std::ranges::contains( allChains, clone ) )
					{
						allChains.push_back( clone );	
					}
				}
			}
		}
	}
};

} // namespace Sudoku

