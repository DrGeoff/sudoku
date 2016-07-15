/*
 *  singlevaluechains.hpp
 *  
 *
 *  Created by Geoffery Ericksson on 16/05/10.
 *
 */
#ifndef SINGLEVALUECHAINS_HPP_20100516
#define SINGLEVALUECHAINS_HPP_20100516

#include <vector>
using std::vector;

#include <set>
using std::set;

#include <iterator>
using std::back_inserter;

#include <algorithm>
using std::set_intersection;

#include "grid.hpp"
#include "constraintregion.hpp"
#include "cell.hpp"
#include "chain.hpp"

namespace Sudoku {

/**
*/	
struct SingleValueChains 
{
	const char* name() const { return "Single-value chains"; }
	static const bool usesGrid = true;

	void operator()( ConstraintRegion& cr, Grid& grid, set<Cell*>& changedCells, set<Cell*>& explanatoryCells, string& explanation )
	{
		for( ConstraintRegion::iterator crIt = cr.begin(); crIt != cr.end(); ++crIt )
		{
			// Loop over the candidate value to see if there are any with a frequency of exactly 2
			for( char candidateValue = '1'; candidateValue != ':'; ++candidateValue )
			{
				doSingleValueChainForValue( candidateValue,  *crIt, grid, changedCells, explanatoryCells, explanation );
			}
		}
	}

private:
	void doSingleValueChainForValue( const char candidateValue, Cell* pStartCell, Grid& grid, set<Cell*>& changedCells, set<Cell*>& explanatoryCells, string& explanation )
	{
		// Find all the chains leading away from each of these cells
		vector< Chain > allChains = findAllChains( candidateValue, pStartCell, grid );
		
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
						  && find(currentCandidates.begin(), currentCandidates.end(), candidateValue) != currentCandidates.end() )
						{
							//std::cout << "Removing "<< candidateValue << "from cell "<< pCellAtIntersection->index() << std::endl; 	
							// Remove the candidate value from the cell at the intersection
							currentCandidates.remove( candidateValue );
							ostringstream oss;
							oss << "Removing " << candidateValue << " from cell " << pCellAtIntersection->index();
							pCellAtIntersection->writeCellShortLocationInformation(oss) << " because of the single value chains {";
				            
							for( Chain::const_iterator it = chain0.begin(); it !=chain0.end(); ++it )
							{
								oss << " "; (*it)->writeCellShortLocationInformation(oss);
				                explanatoryCells.insert(*it);
							}
				            oss << "} {";
						    for( Chain::const_iterator it = chain1.begin(); it !=chain1.end(); ++it )
						    {
								oss << " "; (*it)->writeCellShortLocationInformation(oss);
				                explanatoryCells.insert(*it);
						    }
				            oss << "}\n";
				   
							explanation += oss.str();
							changedCells.insert(pCellAtIntersection);
							
							//identicalChainCheck( allChains[allChains0_index], allChains[allChains1_index], true );
						}
				    }
				}				   
			}
		}
	}
	
	/** This version of findAllChains returns only a subset of all possible chains.  It returns the chains
	    that end in uni-directional links and it only returns chains that cannot be extended.
	 */
	vector< Chain > findAllChains( const char candidateValue, Cell* pStartCell, const Grid& grid )
	{
		vector< Chain > allChains;
		
		for( Constraint::Type constraintType = static_cast<Constraint::Type>(0); constraintType != Constraint::grid; constraintType=static_cast<Constraint::Type>(constraintType+1) )
		{
			Chain chain;
			chain.push_back( pStartCell );
			findChainsInConstraintType( candidateValue, grid, constraintType, chain, allChains );	
		}
		
		return allChains;
	}
	
	void findChainsInConstraintType( const char candidateValue
								   , const Grid& grid
								   , const Constraint::Type searchConstraintType
								   , Chain& currentChain
								   , vector< Chain >& allChains )
	{
		const Cell* const pSearchCell = *currentChain.rbegin();
		// No point in going any further if the candidate value is not in the search cell
		if( find(pSearchCell->candidates().begin(), pSearchCell->candidates().end(), candidateValue) == pSearchCell->candidates().end() ){ return; }
		
		// Get one of the constraint regions that the cell is in for searching
		const ConstraintRegion& cr = grid.get( searchConstraintType )[ pSearchCell->index( searchConstraintType ) ];
		
		// A bi-directional chain will have a frequency of 2 for the candidate value.  A uni-directional link will have a frequency > 2.
		// A chain can only have a uni-directional link as its last element
		vector<std::size_t> frequency = Constraint::buildCandidateFrequencyTable( cr );
		
		if( frequency[candidateValue] >= 2 )
		{
			vector< Cell* > pointorsToCellsWithCandidateValue = Constraint::findCellsContainingCandidateValue( cr, candidateValue );
			assert( pointorsToCellsWithCandidateValue.size() == frequency[candidateValue] );
			
			// If a cell is already in the chain then remove it from the cells that need further exploration
			for( Chain::const_iterator it = currentChain.begin(); it != currentChain.end(); ++it )
			{
				vector< Cell* >::iterator itRemove = find( pointorsToCellsWithCandidateValue.begin(), pointorsToCellsWithCandidateValue.end(), *it);
				if( itRemove != pointorsToCellsWithCandidateValue.end() )
				{
					pointorsToCellsWithCandidateValue.erase( itRemove );
				}
			}
			
			for( vector< Cell* >::const_iterator it = pointorsToCellsWithCandidateValue.begin(); it != pointorsToCellsWithCandidateValue.end(); ++it )
			{
				Chain clone = currentChain;
				clone.push_back( *it );					
				
				// Only if this is a bi-directional link do we need to recurse.
				if( frequency[candidateValue] == 2  ) 
				{
					for( Constraint::Type constraintType = static_cast<Constraint::Type>(0); constraintType != Constraint::grid; constraintType=static_cast<Constraint::Type>(constraintType+1) )
					{
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


