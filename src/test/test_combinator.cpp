#include "../combinator.hpp"

#include <cstddef>
#include <iostream>
#include <vector>

int main()
{
	std::vector<char> possibilities = { 'H', 'e', 'l', '1', 'o' };
	
	Sudoku::Combinator<char> combinator( possibilities, 3 );
	
	std::cout << "5C3 = " << combinator.size() << '\n';
	
	std::size_t loop = 0;
	while( loop < combinator.size() )
	{
		// Create a n-tuple to test for the existence of
		std::vector<char> combination = combinator.next();
		
		for( const char c : combination )
		{
			std::cout << c;
		}
		std::cout << '\n';
		
		++loop;
	}

    return 0;
}
