/*
 *  chain.hpp
 *  sudoku
 *
 *  Created by Geoffery Ericksson on 23/05/10.
 *  Copyright 2010 The University of Queensland. All rights reserved.
 *
 */

#ifndef CHAIN_HPP_20100523
#define CHAIN_HPP_20100523


namespace Sudoku {

typedef vector< Cell* > Chain;	
	

ostream& writeRawChain( ostream& os, const Chain& chain, const string& preamble )
{
	os << preamble;
	copy( chain.begin(), chain.end(), std::ostream_iterator<Cell*>(os) );	
	return os;
}

ostream& writeChain( ostream& os, const Chain& chain, const string& preamble = string() )
{
	os << preamble;
	for( Chain::const_iterator it = chain.begin(); it != chain.end(); ++it )
	{
		(*it)->writeCellShortLocationInformation(os);
	}	
	return os;
}

ostream& writeChainWithValues( ostream& os, const Chain& chain, const string& preamble = string() )
{
	os << preamble;
	for( Chain::const_iterator it = chain.begin(); it != chain.end(); ++it )
	{
		(*it)->writeCellShortLocationInformation(os);
		(*it)->writeCandidateValues(os);
	}	
	return os;
}
	
	
// Return true if the chains are identical
bool identicalChain( Chain chain0_clone, Chain chain1_clone, bool print=false )
{
	if( chain0_clone.size() == chain1_clone.size() )
	{
		sort(chain0_clone.begin(), chain0_clone.end());
		sort(chain1_clone.begin(), chain1_clone.end());
		
		if(print)
		{
			writeChain( cout, chain0_clone, "Chain 0 clone : ");
			writeChain( cout, chain1_clone, "Chain 1 clone : ");
		}
		
		if ( equal(chain0_clone.begin(), chain0_clone.end(), chain1_clone.begin() ) )
		{
			if(print){ cout << "Chains are circular.  Found locked pair or a longer circular chain." << endl; }
			return true; 	
		}
	}
	return false;
}	
	
	
}

#endif
