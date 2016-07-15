/*
 *  gridlock.hpp
 *  
 *
 *  Created by Geoffery Ericksson on 15/04/10.
 *
 */
#ifndef GRIDLOCK_HPP_20100415
#define GRIDLOCK_HPP_20100415

#include <vector>
using std::vector;

#include <set>
using std::set;

#include <iterator>
using std::inserter;

#include <algorithm>
using std::copy;

#include "grid.hpp"
#include "constraintregion.hpp"
#include "cell.hpp"

namespace Sudoku {

/** The 2x2 Gridlock simplification is easiest to understand.  Let's examine the scenario where for a given column a candiate value is constrained to 
    only appear in two rows.  If you can find another column where the candidate value must also be confined to the same two rows then you have a Gridlock.
    For example if the first 3 squares are
 
    {7,8,9}, 0, {6,7},  |   0,     0, {1,2,7},  |   0, {1,2,3,7,8,9}, 0
    0      , 0,     0,  |   0,     0,       0,  |   0,             0, 0
    {5,7}  , 0,     0,  |   {7,8}, 0,       0,  |   0,         {7,9}, 0
 
    and there are no further '7's in either column 0 or column 7 (but there are extra '7's in the other columns) then there is a gridlock on the '7's. 
    Because the '7's must appear in rows 0 and 2 in columns 0 and 7, then there cannot be any other uses of '7' in rows 0 and 2.  
	So the candidate set can be reduced to 

	 {7,8,9}, 0,   {6},  |   0,     0,   {1,2},  |   0, {1,2,3,7,8,9}, 0
	 0      , 0,     0,  |   0,     0,       0,  |   0,             0, 0
	 {5,7}  , 0,     0,  |   {8},   0,       0,  |   0,         {7,9}, 0
 
    This extends to 3x3 and 4x4 gridlocks. It is worth noting in these cases that the candidate value need not appear in every single intersection, but 'n' 
    rows/columns must be covered by the cells containing the candidate value. 
*/	
struct Gridlock 
{
	const char* name() const { return "Gridlock"; }
    static const bool usesGrid = true;

	void operator()( ConstraintRegion& cr, Grid& grid, set<Cell*>& changedCells, set<Cell*>& explanatoryCells, string& explanation )
	{
		// First build up a count of the various candidates
		// Create a vector of many elements initialised to zero. Despite the waste of memory, we wont use the first n element, only using '1' - '9'.
		vector<std::size_t> frequency = Constraint::buildCandidateFrequencyTable( cr );
		
		// Check for an NxN gridlock. (in the code I will use nn rather than n)
		// Note that we can stop at 4x4 gridlock (because if there is a 5x5 then I think there exists a complementary 4x4 gridlock [should verify this])
		// Also note that there can be more than one gridlock in a region. Need to be careful of that.
		for( std::size_t nn = 2; nn != 5; ++nn )
		{
			// Loop over the candidate value look for a potential gridlock
			for( char candidateValue = '1'; candidateValue != ':'; ++candidateValue )
			{
				if( frequency[candidateValue] > 1 && frequency[candidateValue] <= nn )
			    {
					doNxNGridlockForValue( nn, candidateValue, cr, grid, changedCells, explanatoryCells, explanation );
			    }
			}
		}
	}

private:	
	vector<const ConstraintRegion*> findPossibleConstraintRegions( const std::size_t nn, const char candidateValue, const vector<ConstraintRegion>& allCR )
	{
		vector<const ConstraintRegion*> possibleConstraintRegions;
		for( vector<ConstraintRegion>::const_iterator regionsIt = allCR.begin(); regionsIt != allCR.end(); ++regionsIt )
		{
			const ConstraintRegion& cr2 = *regionsIt;
			const vector<std::size_t> frequency2 = Constraint::buildCandidateFrequencyTable( cr2 );				
			if( frequency2[candidateValue] > 1 && frequency2[candidateValue] <= nn )
			{
				//cout << "Adding to possible constraint regions: " << Constraint::calculateConstantIndex( cr2 ) << endl;
				possibleConstraintRegions.push_back( &cr2 );
			}
		}
		return possibleConstraintRegions;
	}
	
    void doNxNGridlockForValue( const std::size_t nn, const char candidateValue, ConstraintRegion& cr, Grid& grid, set<Cell*>& changedCells, set<Cell*>& explanatoryCells, string& explanation )
	{ 
		const Constraint::Type constraintType = Constraint::calculateType( cr );  // Are we trying to find matching columns or rows?
		const Constraint::Type oppositeType = Constraint::oppositeType(constraintType); // Calculate the opposite row/column type from "cr".  That is if "cr" is a row then opposite is column.
		
		vector<ConstraintRegion>& sameConstraintTypes = grid.get( constraintType );  // Get access to all the rows or columns 
		
		// Loop over the same type constraint regions and collect any potential columns/rows for the gridlock.
		vector<const ConstraintRegion*> possibleConstraintRegions = findPossibleConstraintRegions( nn, candidateValue, sameConstraintTypes );
		
		// If we have found enough regions to potentially form an NxN gridlock then examine each of the possible combinations
		if( possibleConstraintRegions.size() >= nn )
		{
			// Could have picked up too many possibilities so now check each of the possible combination of constraint regions found.
			Combinator<const ConstraintRegion*> combinator( possibleConstraintRegions, nn );
			
			set<std::size_t> preserveIndexes;  // This is useful later so that we don't eliminate data that forms part of the NxN Gridlock
			set<std::size_t> oppositeIndexes;  // Allowed to delete from the columns/rows with these indexes
			std::size_t loop = 0;
			bool found = false;
			while( loop < combinator.size() && !found )
			{
				// Create a n-tuple to test for the existence of
				vector<const ConstraintRegion*> potentialGridlock = combinator.next();
				
				// Create the preserve indexes and the opposite indexes that can have the candidate value eliminated from
				preserveIndexes.clear();
				oppositeIndexes.clear();
				for( std::vector<const ConstraintRegion*>::const_iterator regionsPtrIt = potentialGridlock.begin(); regionsPtrIt != potentialGridlock.end(); ++regionsPtrIt )
				{
					assert( *regionsPtrIt != 0 );
					const ConstraintRegion& cr2 = **regionsPtrIt;
					const std::size_t cr2ConstantIndex = Constraint::calculateConstantIndex( cr2 );
					const vector<std::size_t> frequency2 = Constraint::buildCandidateFrequencyTable( cr2 );				
					if( frequency2[candidateValue] > 1 && frequency2[candidateValue] <= nn )
					{
						preserveIndexes.insert( cr2ConstantIndex );
						
						set<std::size_t> moreOppositeIndexes = Constraint::findCandidateValue( cr2, candidateValue, oppositeType );
						copy( moreOppositeIndexes.begin(), moreOppositeIndexes.end(), inserter(oppositeIndexes,oppositeIndexes.end()) );
					}
				}
				
				// If the number of potential gridlock columns (including the given "cr") equals nn then there really is a gridlock
				found = ( preserveIndexes.size() == nn && oppositeIndexes.size() == nn ) ? true : false;
				++loop;
			}
			
			// Can now eliminate from the opposite type.  I.e., if cr is a column then we can eliminate from the rows.
			if( found )
			{
				// TODO: Somehow need to fill out the explanatory cells.  Might have to rewrite the algorithm to find set<Cell*> rather than indexes.
				
				// For each opposite region type remove the candidate, being careful to preserve the gridlock data
				bool didWork = false;
				vector<ConstraintRegion>& eliminationRegions = grid.get( oppositeType );
				for( set<std::size_t>::const_iterator opIndexIt = oppositeIndexes.begin(); opIndexIt != oppositeIndexes.end(); ++opIndexIt )
				{
					ConstraintRegion& cr3 = eliminationRegions[ *opIndexIt ];
					didWork |= Constraint::eliminate( cr3, candidateValue, changedCells, preserveIndexes, constraintType );
				}			
				
				if( didWork )
				{
					ostringstream oss;
					oss << nn << "x" << nn << " gridlock on candidate value = " << candidateValue
						<< " for " << Constraint::typeToStr( constraintType ) << "s";
					for( set<std::size_t>::const_iterator sit = preserveIndexes.begin(); sit != preserveIndexes.end(); ++sit )
					{
						oss << " " << *sit;
					}
					oss << ". Removing candidate value from " << Constraint::typeToStr( oppositeType ) << "s";
					for( set<std::size_t>::const_iterator opIndexIt = oppositeIndexes.begin(); opIndexIt != oppositeIndexes.end(); ++opIndexIt )
					{
						oss << " " << *opIndexIt;
					}
					oss << "\n";
						
					explanation += oss.str();	
				}
				
			}
		}
	}	
};

} // namespace Sudoku

#endif //GRIDLOCK_HPP_20100415


