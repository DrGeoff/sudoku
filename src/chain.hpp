/*
 *  chain.hpp
 *  sudoku
 *
 *  Created by Geoffery Ericksson on 23/05/10.
 *  Copyright 2010 The University of Queensland. All rights reserved.
 *
 */

#pragma once

#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "cell.hpp"

namespace Sudoku {

using Chain = std::vector< Cell* >;	
	

std::ostream& writeRawChain( std::ostream& os, const Chain& chain, const std::string& preamble )
{
	os << preamble;
	std::copy( chain.begin(), chain.end(), std::ostream_iterator<Cell*>(os) );	
	return os;
}

std::ostream& writeChain( std::ostream& os, const Chain& chain, const std::string& preamble = std::string() )
{
	os << preamble;
	for( const Cell* cell : chain )
	{
		cell->writeCellShortLocationInformation(os);
	}	
	return os;
}

std::ostream& writeChainWithValues( std::ostream& os, const Chain& chain, const std::string& preamble = std::string() )
{
	os << preamble;
	for( const Cell* cell : chain )
	{
		cell->writeCellShortLocationInformation(os);
		cell->writeCandidateValues(os);
	}	
	return os;
}
	
	
// Return true if the chains are identical
bool identicalChain( Chain chain0_clone, Chain chain1_clone, bool print=false )
{
	if( chain0_clone.size() == chain1_clone.size() )
	{
		std::sort(chain0_clone.begin(), chain0_clone.end());
		std::sort(chain1_clone.begin(), chain1_clone.end());
		
		if(print)
		{
			writeChain( std::cout, chain0_clone, "Chain 0 clone : " );
			writeChain( std::cout, chain1_clone, "Chain 1 clone : " );
		}
		
		if ( chain0_clone == chain1_clone )
		{
			if(print){ std::cout << "Chains are circular.  Found locked pair or a longer circular chain." << '\n'; }
			return true; 	
		}
	}
	return false;
}	
	
	
}

