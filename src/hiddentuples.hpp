/*
 * hiddentuples.hpp
 *
 *  Created on: 31/03/2010
 *      Author: geoff
 */

#ifndef HIDDENTUPLES_HPP_2010331
#define HIDDENTUPLES_HPP_2010331

#include <iosfwd>
#include <iterator>

#include <algorithm>
using std::includes;
using std::copy;

#include <vector>
using std::vector;

#include <set>
using std::set;

#include "cell.hpp"
#include "constraintregion.hpp"
#include "combinator.hpp"

namespace Sudoku {

/**  If a n-tuple of candidates appears in exactly n cells of a region then all other candidates in those n cells can be eliminated
 *   For example, if a square had the following candidates
 *
 *   1,2,3,4,5  4,5,6,7,8  4,5,8,9
 *   1,2,3,8,9  6,7,8,9    4,5,6,7
 *   6,7,8,9    4,5,6,8,9  1,2,3,6,7,8
 *
 *   then we can see that the candidates {1,2,3} form a hidden 3-tuple (more commonly called a triple).  We can thus cull the candidates to be
 *
 *   1,2,3      4,5,6,7,8  4,5,8,9
 *   1,2,3      6,7,8,9    4,5,6,7
 *   6,7,8,9    4,5,6,8,9  1,2,3
 *
 *   This algorithm is a generalisation of OnlySpot
 */
struct HiddenTuples : public unary_function< ConstraintRegion, vector<size_t> >
{
	const char* name() const { return "Hidden Tuples"; }
	static const bool usesGrid = false;
	
	void operator()( ConstraintRegion& cr, set<Cell*>& changedCells, set<Cell*>& explanatoryCells, string& explanation )
	{
		// First build up a count of the various candidates
		// Create a vector of many elements initialised to zero. Despite the waste of memory, we wont use the first n element, only using '1' - '9'.
		vector<size_t> frequency = Constraint::buildCandidateFrequencyTable( cr );
		
		// Now see if it is possible to make 'n' n-tuples (in the code I will use nn rather than n)
		// Note that we can stop at 4-tuples (because if there is a 5-tuple then the given 4-tuple will complement it)
		// Also note that there can be more than one n-tuple in a region. Need to be careful of that.
		for( size_t nn = 2; nn != 5; ++nn )
		{
			// Gather together the possible values that could constitute a n-tuple
			// For example, if the values 6 and 7 have a count of 2 then they could
			// possibly be a 2-tuple, or could be part of a 3-tuple (with another value of course)
			// Be careful of the chance of having two 2-tuples in a constraint region.
			vector<char> possibleValuesForNTuple;
			for( char candidateValue = '1'; candidateValue != ':'; ++candidateValue )
			{
				if( nn >= frequency[candidateValue] )
				{
					possibleValuesForNTuple.push_back(candidateValue);
				}
			}
	
			if( possibleValuesForNTuple.size() >= nn )
			{
				// Now check each of the combinations.
				Combinator<char> combinator( possibleValuesForNTuple, nn );
				
				size_t loop = 0;
				while( loop < combinator.size() )
				{
					// Create a n-tuple to test for the existence of
					vector<char> proposedHiddenTuple = combinator.next();
									
					// Examine each candidate set for the containment of the proposed tuple
					vector< ConstraintRegion::iterator > iteratorsToCellsWithProposedTuple;
					for( ConstraintRegion::iterator crIt = cr.begin(); crIt != cr.end(); ++crIt )
					{
						if( includes( (*crIt)->candidates().begin(), (*crIt)->candidates().end(), proposedHiddenTuple.begin(), proposedHiddenTuple.end() ) )
						{
							iteratorsToCellsWithProposedTuple.push_back(crIt);
						}
					}				
					
					// If we find n of the proposed n-tuple then eliminate the other candidate values from the n-tuple cells.
					if( nn == iteratorsToCellsWithProposedTuple.size() )
					{
						bool didWork = false;
						// Iterators to all the cells containing the proposed tuple have been saved
						// Note the double dereference necessary due to iterating over a container of iterators
						for( vector< ConstraintRegion::iterator >::const_iterator itIt = iteratorsToCellsWithProposedTuple.begin(); itIt != iteratorsToCellsWithProposedTuple.end(); ++itIt )
						{
							// If the cell doesn't have any extra candidates apart from the (not-so)-hidden tuple then there is no need to remove anything
							if( (**itIt)->candidates().size() != nn )
							{
								// Delete all existing candidates then reinsert the hidden (but now exposed) candidate values
								(**itIt)->candidates().clear();				
								copy(proposedHiddenTuple.begin(), proposedHiddenTuple.end(), back_insert_iterator<Cell::CandidateContainer>((**itIt)->candidates()) );
								changedCells.insert((**itIt));	
								didWork = true;
							}
						}

						if( didWork )
						{
							ostringstream oss;
							oss << "For " << Constraint::typeToStr( Constraint::calculateType(cr) ) << " " << Constraint::calculateConstantIndex(cr) 
							<< " the candidate values { ";
							copy(proposedHiddenTuple.begin(), proposedHiddenTuple.end(), std::ostream_iterator<char>(oss," "));
							oss << "} are a hidden tuple in cells ";
							
							for( vector< ConstraintRegion::iterator >::const_iterator itIt = iteratorsToCellsWithProposedTuple.begin(); itIt != iteratorsToCellsWithProposedTuple.end(); ++itIt )
							{
								explanatoryCells.insert(**itIt);
								oss << (**itIt)->index() << " ";
							}
							oss << ". Removing other candidates from these cells.\n";
							explanation += oss.str();
							
						}
					}
					
					++loop;
				}
			}
		}
	}
				
};


} // namespace Sudoku

#endif /* HIDDENTUPLES_HPP_2010331 */
