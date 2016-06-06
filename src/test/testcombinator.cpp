#include "../combinator.hpp"
using namespace Sudoku;

#include <iostream>
#include <iterator>
#include <vector>
using namespace std;

int main( int argc, char* argv[] )
{
	vector<char> possibilities;
	possibilities.push_back('H');
	possibilities.push_back('e');
	possibilities.push_back('l');
	possibilities.push_back('1');
	possibilities.push_back('o');
	
	/*
	cout << "Hel1o in reverse\n";
	for( vector<char>::reverse_iterator rit = possibilities.rbegin(); rit != possibilities.rend(); ++rit )
	{
		cout << *rit << '\t' ;
	}
	cout << endl;
	*/
	
	Combinator combinator( possibilities, 3 );
	
	cout << "5C2 = " << combinator.size() << endl;
	
	size_t loop = 0;
	while( loop < combinator.size() )
	{
		// Create a n-tuple to test for the existence of
		vector<char> combination = combinator.next();
		
		for( vector<char>::const_iterator it = combination.begin(); it != combination.end(); ++it )
		{
			cout << *it;
		}
		cout << endl;
		
		++loop;
	}

    return 0;
}

