#include <iostream>
using std::cout;
using std::endl;

#include <iterator>
using std::back_insert_iterator;
using std::ostream_iterator;

#include <algorithm>
using std::copy;

#include <sstream>
using std::istringstream;

#include <vector>
using std::vector;

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

/** This version of for_each does the binding to the 4 arguments needed for func to work */
template< typename Function >
void for_each( vector<ConstraintRegion>& cont, Function func, set<Cell*>& changedCells, set<Cell*>& explanatoryCells, string& explanation )
{
	vector<ConstraintRegion>::iterator it = cont.begin();
	for( ; it != cont.end(); ++it )
	{
		func( *it, changedCells, explanatoryCells, explanation );
	}
}


template< typename Function >
bool do_grid_function( Grid& grid, Function func, const Constraint::Type type )
{
	bool didWork = false;
	//	apply_merge( grid, func, changedCells, explanatoryCells, explanation );
	set<Cell*> changedCells;
	set<Cell*> explanatoryCells;
	string explanation;
	
	vector<ConstraintRegion>& regions = grid.get(type);
	for( vector<ConstraintRegion>::iterator it = regions.begin(); it != regions.end(); ++it )
	{
		func( *it, grid, changedCells, explanatoryCells, explanation );
	}		
	
	if(changedCells.size() > 0)
	{ 
		cout << "Apply " << func.name() << " rule to " << Constraint::typeToStr( type ) << "s.\n"; 
		//copy (changedCells.begin(), changedCells.end(), ostream_iterator<std::size_t> (cout, " "));
		cout << explanation << endl << grid << endl;
		didWork = true;
	}
	return didWork;
}


template< typename Function >
bool do_function( Grid& grid, Function func, const Constraint::Type type )
{
	bool didWork = false;
	//	apply_merge( grid, func, changedCells, explanatoryCells, explanation );
	set<Cell*> changedCells;
	set<Cell*> explanatoryCells;
	string explanation;
	
	vector<ConstraintRegion>& regions = grid.get(type);
	for( vector<ConstraintRegion>::iterator it = regions.begin(); it != regions.end(); ++it )
	{
		func( *it, changedCells, explanatoryCells, explanation );
	}	

	if(changedCells.size() > 0)
	{ 
		cout << "Apply " << func.name() << " rule to " << Constraint::typeToStr( type ) << "s.\n"; 
		//copy (changedCells.begin(), changedCells.end(), ostream_iterator<std::size_t> (cout, " "));
		cout << explanation << endl << grid << endl;
		didWork = true;
	}
	return didWork;
}


template< typename Function >
bool do_function( Grid& grid, Function func )
{
	bool didWork = false;
	didWork |= do_function( grid, func, Constraint::square );
	didWork |= do_function( grid, func, Constraint::row );
	didWork |= do_function( grid, func, Constraint::column );
		
	return didWork;
}

template< typename Function >
bool do_grid_function( Grid& grid, Function func, bool doRow, bool doColumn, bool doSquare )

{
	bool didWork = false;
	
	if( doRow )
	{
		didWork |= do_grid_function( grid, func, Constraint::row );	
	}
	if( doColumn )
	{
		didWork |= do_grid_function( grid, func, Constraint::column );	
	}
	if( doSquare )
	{
		didWork |= do_grid_function( grid, func, Constraint::square );	
	}
	
	return didWork;
}


int main( int argc, char* argv[] )
{	
    Grid grid;
    assert(argc > 1);
	grid.parse( argv[1] );
    cout << grid << endl;

	//std::for_each(grid.rows.begin(), grid.rows.end(), PrintCellLocationInformation());
    //std::for_each(grid.columns.begin(), grid.columns.end(), PrintCellLocationInformation());
    //std::for_each(grid.squares.begin(), grid.squares.end(), PrintCellLocationInformation());
	
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
				
				if( do_function( grid, HiddenTuples() ) ){ keepSearching = true; ++countRulesApplied; }
				while( do_function( grid, UniquePerConstraintRegion() ) ){ keepSearching = true; ++countRulesApplied; }
				
				if( do_grid_function( grid, XYZWing(), false, false, true ) ){ keepSearching = true; ++countRulesApplied; }
				while( do_function( grid, UniquePerConstraintRegion() ) ){ keepSearching = true; ++countRulesApplied; }
			}

			if( do_grid_function( grid, IntersectReject(), true, true, true ) ){ keepSearching = true; ++countRulesApplied; }
			while( do_function( grid, UniquePerConstraintRegion() ) ){ keepSearching = true; ++countRulesApplied; }

			if( do_grid_function( grid, Gridlock(), true, true, false ) ){ keepSearching = true; ++countRulesApplied; }
			while( do_function( grid, UniquePerConstraintRegion() ) ){ keepSearching = true; ++countRulesApplied; }
		}	
		
		if( do_grid_function( grid, SingleValueChains(), true, false, false ) ){ keepSearching = true; ++countRulesApplied; }
		while( do_function( grid, UniquePerConstraintRegion() ) ){ keepSearching = true; ++countRulesApplied; }
		
		//cout << "Starting MultiValueChains search" << endl;
		if( do_grid_function( grid, MultiValueChains(), true, false, false ) ){ keepSearching = true; ++countRulesApplied; }
		//cout << "Finished MultiValueChains search" << endl;
		while( do_function( grid, UniquePerConstraintRegion() ) ){ keepSearching = true; ++countRulesApplied; }
	}
	
	if( countRulesApplied > 0 )
	{
		std::cout << "Number of rules applied = " << countRulesApplied << std::endl; 
		
		// Check grid for consistency		
		if( do_function( grid, Inconsistency(true) ) )
		{ 
		   /*
			cout << "Grid is inconsistent or incomplete.  Inconsistent/Incomplete cells are: ";
			for( vector<std::size_t>::const_iterator it = inconsistentCells.begin(); it != inconsistentCells.end(); ++it )
			{
				cout << "index = " << *it << ' ' << grid.cells[*it].row() << ' ' << grid.cells[*it].column() << ' ' << grid.cells[*it].square() << "\t";
			}
			cout << endl; */
			return 0;
		}
		
		grid.writeCSV((string(argv[1]) + ".solution.csv").c_str() );		
	}


    return 0;
}

