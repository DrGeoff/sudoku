#ifndef LOCKEDTUPLES_HPP_20100328
#define LOCKEDTUPLES_HPP_20100328

#include <iosfwd>  // debug only
using std::cout;

#include <iterator>
using std::inserter;

#include <algorithm>
using std::copy;

#include <set>
using std::set;


namespace Sudoku {
	
struct LockedTuples 
{
	const char* name() const { return "Locked Tuples"; }
	static const bool usesGrid = false;
	
	// The locked pairs simplification is easiest to understand.  If two cells only contain say {3,8} as their candidates
	// then 3 and 8 cannot possibly be candidates for other cells in the constraint region.  This extends to locked triples and locked quadruples.
	void operator()( ConstraintRegion& cr, set<Cell*>& changedCells, set<Cell*>& explanatoryCells, string& explanation )
	{
		// Find if a locked pair exists in this constraint region
		for( ConstraintRegion::iterator crIt = cr.begin(); crIt != cr.end(); ++crIt )
		{
			const std::size_t tupleSize = (*crIt)->candidates().size();
			if( tupleSize >= 2 && tupleSize <= 4 )
			{
				//cout << "Cell " << (*crIt)->index() << " has " << tupleSize << " candidates.\t";
				set<Cell*> preserveCells; 
				preserveCells.insert( *crIt );
			    Cell const * const pLockedCell = (*crIt);
				
				// OK crIt point to a possible first cell of a locked pair/triple/quadruple.  See if we can find a partner(s) for it.
				for( ConstraintRegion::iterator crIt2 = crIt + 1; crIt2 != cr.end() ; ++crIt2 )
				{					
					// Note that in using std::equal we are assuming that the candidate container has been sorted (which it was at construction)
					if( (*crIt2)->candidates().size() == tupleSize
					    && std::equal( (*crIt)->candidates().begin(), (*crIt)->candidates().end(), (*crIt2)->candidates().begin() ) )
					{
						// Found another of the tuples
						//cout <<  "Cell " << (*crIt2)->index() << " has " << tupleSize << " candidates.\t";
						//std::copy((*crIt2)->candidates().begin(), (*crIt2)->candidates().end(), std::ostream_iterator<char>(std::cout,","));
						preserveCells.insert( *crIt2 );
					}	
				}

				if( preserveCells.size() == tupleSize ) // OK we have found the locked tuples.  Remove them from the other cells in the constraint region.
				{
					// The preserve cells are the locked pair/triple/quadruple which explain what we are doing
					copy( preserveCells.begin(), preserveCells.end(), inserter(explanatoryCells,explanatoryCells.end()) );						
					
					// Remove every locked candidate.  Look up one of the preserved cells to see what the locked candidates actually are.
					bool didWork = false;
					for( Cell::CandidateContainer::const_iterator lockedValueIt = pLockedCell->candidates().begin(); lockedValueIt != pLockedCell->candidates().end(); ++lockedValueIt )
					{
						didWork |= Constraint::eliminate( cr, *lockedValueIt, changedCells, preserveCells );
					}	
					if( didWork )
					{
						ostringstream oss;
						oss << "Cells ";
						for( set<Cell*>::const_iterator cellIt = preserveCells.begin(); cellIt != preserveCells.end(); ++cellIt )
						{
							oss << (*cellIt)->index() << " ";
						}
						
						oss << " contain the locked candidates { ";
						for( Cell::CandidateContainer::const_iterator lockedValueIt = pLockedCell->candidates().begin(); lockedValueIt != pLockedCell->candidates().end(); ++lockedValueIt )
						{
							oss << *lockedValueIt << ' ';
						}
						oss << "}. We can remove those locked candidates from all other candidates in " 
    						<< Constraint::typeToStr( Constraint::calculateType(cr) ) << " " << Constraint::calculateConstantIndex(cr) << '\n';
						explanation += oss.str();					
					}											
				}
			}
		}
	}
};

} // namespace Sudoku

#endif
