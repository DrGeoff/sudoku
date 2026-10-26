/*
 *  singlevaluechains.hpp
 *  
 *
 *  Created by Geoffery Ericksson on 16/05/10.
 *
 */
#ifndef SINGLEVALUECHAINS_HPP_20100516
#define SINGLEVALUECHAINS_HPP_20100516

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
*/	
struct SingleValueChains 
{
	std::string_view name() const { return "Single-value chains"; }
	inline static constexpr bool usesGrid = true;

	void operator()( ConstraintRegion& cr, Grid& grid, std::set<Cell*>& changedCells, std::set<Cell*>& explanatoryCells, std::string& explanation )
	{
		for( Cell* cell : cr )
		{
			// Loop over the candidate value to see if there are any with a frequency of exactly 2
			for( char candidateValue = '1'; candidateValue != ':'; ++candidateValue )
			{
				doSingleValueChainForValue( candidateValue,  cell, grid, changedCells, explanatoryCells, explanation );
			}
		}
	}

private:
	void doSingleValueChainForValue( char candidateValue, Cell* pStartCell, Grid& grid, std::set<Cell*>& changedCells, std::set<Cell*>& explanatoryCells, std::string& explanation )
	{
		// Find all the chains leading away from each of these cells
		std::vector< Chain > allChains = findAllChains( candidateValue, pStartCell, grid );
		
		// If we couldn't find at least two chains then there cannot be an intersection
		if( allChains.size() < 2 ) { return; }
		
		// Look for any intersections between a chains
		for( std::size_t allChains0_index = 0; allChains0_index != allChains.size(); ++allChains0_index )
		{
			for( std::size_t allChains1_index = allChains0_index + 1; allChains1_index < allChains.size(); ++allChains1_index )
			{				
				Chain& chain0 = allChains[allChains0_index];
				Chain& chain1 = allChains[allChains1_index];
				
				// An intersection will occur on the last elements of the chain
				if( !identicalChain( chain0, chain1 )  && *chain0.rbegin() == *chain1.rbegin() )
				{
				    // Found an intersection between a chain starting at cell 0 and a chain starting at cell 1
				    // If chain0_intersection_index and chain1_intersection_index are both even or both odd
				    // then we CANNOT remove the candidate value from the cell that is the intersection
				    // If one is even and the other odd then one of the paths always implies the interection cell cannot 
					// contain the candidate value.
				    if( 1 == ((chain0.size()-1)%2 ^ (chain1.size()-1)%2) )
				    {
						Cell* pCellAtIntersection = *chain0.rbegin() ;
					   
						Cell::CandidateContainer& currentCandidates = pCellAtIntersection->candidates();
						if( currentCandidates.size() > 1
						  && std::ranges::contains( currentCandidates, candidateValue ) )
						{
							// Remove the candidate value from the cell at the intersection
							currentCandidates.remove( candidateValue );
							std::ostringstream oss;
							oss << "Removing " << candidateValue << " from cell " << pCellAtIntersection->index();
							pCellAtIntersection->writeCellShortLocationInformation(oss) << " because of the single value chains {";
				            
							for( Cell* cell : chain0 )
							{
								oss << " "; cell->writeCellShortLocationInformation(oss);
				                explanatoryCells.insert(cell);
							}
				            oss << "} {";
						    for( Cell* cell : chain1 )
						    {
								oss << " "; cell->writeCellShortLocationInformation(oss);
				                explanatoryCells.insert(cell);
						    }
				            oss << "}\n";
				   
							explanation += oss.str();
							changedCells.insert(pCellAtIntersection);
						}
				    }
				}				   
			}
		}
	}
	
	/** This version of findAllChains returns only a subset of all possible chains.  It returns the chains
	    that end in uni-directional links and it only returns chains that cannot be extended.
	 */
	std::vector< Chain > findAllChains( char candidateValue, Cell* pStartCell, const Grid& grid )
	{
		std::vector< Chain > allChains;
		
		for( int t = 0; t != static_cast<int>(Constraint::Type::grid); ++t )
		{
			const Constraint::Type constraintType = static_cast<Constraint::Type>(t);
			Chain chain;
			chain.push_back( pStartCell );
			findChainsInConstraintType( candidateValue, grid, constraintType, chain, allChains );	
		}
		
		return allChains;
	}
	
	void findChainsInConstraintType( char candidateValue
								   , const Grid& grid
								   , const Constraint::Type searchConstraintType
								   , Chain& currentChain
								   , std::vector< Chain >& allChains )
	{
		const Cell* pSearchCell = *currentChain.rbegin();
		// No point in going any further if the candidate value is not in the search cell
		if( !std::ranges::contains( pSearchCell->candidates(), candidateValue ) ){ return; }
		
		// Get one of the constraint regions that the cell is in for searching
		const ConstraintRegion& cr = grid.get( searchConstraintType )[ pSearchCell->index( searchConstraintType ) ];
		
		// A bi-directional chain will have a frequency of 2 for the candidate value.  A uni-directional link will have a frequency > 2.
		// A chain can only have a uni-directional link as its last element
		std::vector<std::size_t> frequency = Constraint::buildCandidateFrequencyTable( cr );
		
		if( frequency[candidateValue] >= 2 )
		{
			std::vector< Cell* > pointorsToCellsWithCandidateValue = Constraint::findCellsContainingCandidateValue( cr, candidateValue );
			assert( pointorsToCellsWithCandidateValue.size() == frequency[candidateValue] );
			
			// If a cell is already in the chain then remove it from the cells that need further exploration
			for( const Cell* cell : currentChain )
			{
				std::vector< Cell* >::iterator itRemove = std::find( pointorsToCellsWithCandidateValue.begin(), pointorsToCellsWithCandidateValue.end(), cell);
				if( itRemove != pointorsToCellsWithCandidateValue.end() )
				{
					pointorsToCellsWithCandidateValue.erase( itRemove );
				}
			}
			
			for( Cell* cell : pointorsToCellsWithCandidateValue )
			{
				Chain clone = currentChain;
				clone.push_back( cell );					
				
				// Only if this is a bi-directional link do we need to recurse.
				if( frequency[candidateValue] == 2  ) 
				{
					for( int t = 0; t != static_cast<int>(Constraint::Type::grid); ++t )
					{
						const Constraint::Type constraintType = static_cast<Constraint::Type>(t);
						assert( pointorsToCellsWithCandidateValue.size() == 1 );
						findChainsInConstraintType( candidateValue, grid, constraintType, clone, allChains );	
					}
				}
				else // Only record chains that end in a uni-directional link					
				{
					allChains.push_back( clone );					
				}
			}
		}
	}
};

} // namespace Sudoku

#endif //SINGLEVALUECHAINS_HPP_20100514
