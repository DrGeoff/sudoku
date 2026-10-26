#include <cstddef>
#include <exception>
#include <format>
#include <iostream>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "grid.hpp"
#include "inconsistency.hpp"
#include "uniqueperconstraintregion.hpp"
#include "onlyspot.hpp"
#include "lockedtuples.hpp"
#include "hiddentuples.hpp"

#include "intersectreject.hpp"
#include "gridlock.hpp"
#include "xyzwing.hpp"
#include "singlevaluechains.hpp"
#include "multivaluechains.hpp"

using namespace Sudoku;

void printUsage( std::ostream& os, const std::string& programName )
{
	os << "Usage: " << programName << " [OPTION] PUZZLE_FILE\n"
	   << "\n"
	   << "Solve a Sudoku puzzle with human-style techniques and write the solution to\n"
	   << "PUZZLE_FILE.solution.csv.\n"
	   << "\n"
	   << "The puzzle file format is selected by the file name:\n"
	   << "  *.k   SDK: 81 characters, '.' for empty cells; lines starting with '#' or '['\n"
	   << "           are comments\n"
	   << "  *.v   CSV: 9 rows of 9 comma-separated values, '0' for empty cells\n"
	   << "\n"
	   << "Options:\n"
	   << "  -h, --help     display this help and exit\n";
}


template< typename Function >
bool do_grid_function( Grid& grid, Function func, Constraint::Type type )
{
	bool didWork = false;
	std::set<Cell*> changedCells;
	std::set<Cell*> explanatoryCells;
	std::string explanation;
	
	std::vector<ConstraintRegion>& regions = grid.get(type);
	for( ConstraintRegion& region : regions )
	{
		func( region, grid, changedCells, explanatoryCells, explanation );
	}		
	
	if( changedCells.size() > 0 )
	{ 
		std::cout << "Apply " << func.name() << " rule to " << Constraint::typeToStr( type ) << "s.\n"; 
		std::cout << explanation << '\n' << grid << '\n';
		didWork = true;
	}
	return didWork;
}


template< typename Function >
bool do_function( Grid& grid, Function func, Constraint::Type type )
{
	bool didWork = false;
	std::set<Cell*> changedCells;
	std::set<Cell*> explanatoryCells;
	std::string explanation;
	
	std::vector<ConstraintRegion>& regions = grid.get(type);
	for( ConstraintRegion& region : regions )
	{
		func( region, changedCells, explanatoryCells, explanation );
	}	

	if( changedCells.size() > 0 )
	{ 
		std::cout << "Apply " << func.name() << " rule to " << Constraint::typeToStr( type ) << "s.\n"; 
		std::cout << explanation << '\n' << grid << '\n';
		didWork = true;
	}
	return didWork;
}


template< typename Function >
bool do_function( Grid& grid, Function func )
{
	bool didWork = false;
	didWork |= do_function( grid, func, Constraint::Type::square );
	didWork |= do_function( grid, func, Constraint::Type::row );
	didWork |= do_function( grid,func, Constraint::Type::column );
		
	return didWork;
}

template< typename Function >
bool do_grid_function( Grid& grid, Function func, bool doRow, bool doColumn, bool doSquare )
{
	bool didWork = false;
	
	if( doRow )
	{
		didWork |= do_grid_function( grid, func, Constraint::Type::row );	
	}
	if( doColumn )
	{
		didWork |= do_grid_function(grid, func, Constraint::Type::column );	
	}
	if( doSquare )
	{
		didWork |= do_grid_function( grid, func, Constraint::Type::square );	
	}
	
	return didWork;
}


int main( int argc, char* const argv[] )
{
	std::string programName = "sudoku";
	if( argc > 0 && argv[0] != nullptr )
	{
		const std::string_view argv0 = argv[0];
		const std::size_t slashPos = argv0.find_last_of('/');
		programName = ( slashPos == std::string_view::npos ) ? std::string( argv0 ) : std::string( argv0.substr( slashPos + 1 ) );
	}
	
	std::vector<std::string> positional;
	for( int i = 1; i < argc; ++i )
	{
		const std::string_view arg = argv[i];
		if( arg == "-h" || arg == "--help" )
		{
			printUsage( std::cout, programName );
			return 0;
		}
		if( arg.starts_with('-') )
		{
			std::println( std::cerr, "error: unknown option '{}'", arg );
			printUsage( std::cerr, programName );
			return 2;
		}
		positional.push_back( std::string( arg ) );
	}
	if( positional.size() != 1 )
	{
		std::println( std::cerr, "error: expected exactly one puzzle file, got {}", positional.size() );
		printUsage( std::cerr, programName );
		return 2;
	}
	const std::string puzzleFile = positional[0];
	
	Grid grid;
	try
	{
		grid.parse( puzzleFile );
	}
	catch( const std::exception& e )
	{
		std::println( std::cerr, "error: {}", e.what() );
		return 1;
	}
	
	std::cout << grid << '\n';
	
	try
	{
		// Check grid for consistency
		if( do_function( grid, Inconsistency() ) )
		{		
			return 0;
		}
		
		std::size_t countRulesApplied = 0;
		bool keepSearching = false;
		while( do_function( grid, UniquePerConstraintRegion() ) ){ keepSearching = true; ++countRulesApplied; }
		
		while( keepSearching )
		{
			while( keepSearching )
			{
				while( keepSearching )
				{
					while( keepSearching )
					{
						keepSearching = false;
						if( do_function( grid, OnlySpot() ) ){ keepSearching = true; ++countRulesApplied; }
						while( do_function( grid, UniquePerConstraintRegion() ) ){ keepSearching = true; ++countRulesApplied; }						
					}
					
					if( do_function( grid, LockedTuples() ) ){ keepSearching = true; ++countRulesApplied; }
					while( do_function( grid, UniquePerConstraintRegion() ) ){ keepSearching = true; ++countRulesApplied; }
					
					if( do_function( grid, HiddenTuples() ) ){ keepSearching=true; ++countRulesApplied; }
					while( do_function( grid,UniquePerConstraintRegion() ) ){ keepSearching=true; ++countRulesApplied; }
					
					if( do_grid_function( grid, XYZWing(), false, false, true ) ){ keepSearching=true; ++countRulesApplied; }
					while( do_function( grid, UniquePerConstraintRegion() ) ){ keepSearching = true; ++countRulesApplied; }
				}
				
				if( do_grid_function( grid, IntersectReject(), true, true, true ) ){ keepSearching=true; ++countRulesApplied; }
				while( do_function( grid, UniquePerConstraintRegion() ) ){ keepSearching = true; ++countRulesApplied; }
				
				if( do_grid_function( grid, Gridlock(), true, true, false ) ){ keepSearching = true; ++countRulesApplied; }
				while( do_function( grid, UniquePerConstraintRegion() ) ){ keepSearching=true; ++countRulesApplied; }
			}	
			
			if( do_grid_function( grid, SingleValueChains(), true, false, false ) ){ keepSearching = true; ++countRulesApplied; }
			while( do_function( grid, UniquePerConstraintRegion() ) ){ keepSearching=true; ++countRulesApplied; }
			
			if( do_grid_function( grid, MultiValueChains(), true, false, false ) ){ keepSearching = true; ++countRulesApplied; }
			while( do_function( grid, UniquePerConstraintRegion() ) ){ keepSearching=true; ++countRulesApplied; }
		}
		
		if( countRulesApplied > 0 )
		{
			std::cout << "Number of rules applied = " << countRulesApplied << '\n'; 
			
			// Check grid for consistency		
			if( do_function( grid, Inconsistency(true) ) )
			{ 
				return 0;
			}
			
			grid.writeCSV( std::format( "{}.solution.csv", puzzleFile ) );		
		}
	}
	catch( const std::exception& e )
	{
		std::println( std::cerr, "error: {}", e.what() );
		return 1;
	}
	
	return 0;
}
