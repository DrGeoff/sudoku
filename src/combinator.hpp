/*
 *  combinator.hpp
 *  
 *
 *  Created by Geoffery Ericksson on 4/04/10.
 *
 */

#ifndef COMBINATOR_HPP_20100404
#define COMBINATOR_HPP_20100404

#include <vector>
using std::vector;

#include <cassert>

//#include<iostream>  // only for debugging

namespace Sudoku {

// For a given container, create the possible combinations
template <typename PossibilitiesValueType >
struct Combinator
{
	Combinator( const vector<PossibilitiesValueType>& possibilities, const size_t numberOfChoices ) 
	    : possibilities_(possibilities)
	    , numberOfChoices_(numberOfChoices)
	    , currentIndexes_() 
	    {}
	
	// Return nCr
	size_t size() const
	{
		assert( numberOfChoices_ <= possibilities_.size() );
		size_t result = 1;
		for( size_t loop = possibilities_.size(); loop > possibilities_.size() - numberOfChoices_; --loop )
		{
			result *= loop;
		}
		for( size_t loop = numberOfChoices_; loop > 0; --loop )
		{
			result /= loop;
		}
		return result;
	}
	
	vector<PossibilitiesValueType> first()
	{
		currentIndexes_.clear();
		for( size_t loop = 0; loop < numberOfChoices_; ++loop )
		{
			currentIndexes_.push_back(loop);
		}
		
		return createCurrentCombination();
	}
	
	vector<PossibilitiesValueType> next()
	{
		if(currentIndexes_.empty())
		{
			return first();
		}
		else
		{
			// Don't reuse possibilities when sampling (i.e., not a permutation but a combination) and preserve the sorting. 
			// For example, 5C3 must go through the sequence of indexes,
			// 0 1 2, 0 1 3, 0 1 4, 0 2 3, 0 2 4, 0 3 4, 1 2 3, 1 2 4, 1 3 4, 2 3 4
			
			// Find an iterator to the left-most element that must change
			vector<size_t>::reverse_iterator rit = currentIndexes_.rbegin();
			size_t rIndex = 0;  // A reverse index corresponding to the reverse iterator
			
			// This line is tricky. To demystify, note in the above example that the middle element has a maximum value of 3.
			// This can be calculated by noting that the reverse index is 1 for the middle element and 5 -1 -1 gives 3.
			// Similarly for the left element, the reverse index is 2 and 5 -1 -2 = 2.  
			// Whenever an index is at these maximum values, you must overflow to the right to correctly set those indexes.
			for( ; rit != currentIndexes_.rend() && *rit == (possibilities_.size()-1-rIndex); ++rIndex, ++rit ){}
			++(*rit); // The above loop guarantees that this wont overflow
			
			// Fix up all the overflows to the right of this index.
			for( size_t loop = numberOfChoices_-rIndex; loop < numberOfChoices_; ++loop )
			{
				currentIndexes_[loop] = currentIndexes_[loop-1] + 1;
			}			
			
			return createCurrentCombination();
		}
	}

private:
	vector<PossibilitiesValueType> createCurrentCombination() const
	{
		vector<PossibilitiesValueType> result;
		for( size_t loop = 0; loop < numberOfChoices_; ++loop )
		{
			result.push_back(possibilities_[currentIndexes_[loop]]);
			//cout << loop << ' ' << possibilities_[currentIndexes_[loop]] << endl;
		}
		assert( result.size() == numberOfChoices_ );
		return result;							
	}
	
private:
	const vector<PossibilitiesValueType>& possibilities_;
	const size_t numberOfChoices_;
	vector< size_t > currentIndexes_;  // Keep track of what indexes into possibilities the current combination is using.
};
							 
							 
} // namespace Sudoku

#endif // COMBINATOR_HPP_20100404
