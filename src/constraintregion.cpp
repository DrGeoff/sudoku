/*
 *  constraintregion.cpp
 *  
 *  Created by Geoffery Ericksson on 13/04/10.
 *
 */

#include "constraintregion.hpp"
#include "cell.hpp"

#include <algorithm>
#include <cassert>
#include <format>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace Sudoku {

std::ostream& operator<<( std::ostream& os, const ConstraintRegion& cr )
{
	for( const Cell* cell : cr )
	{
		os << *cell << " ";
	}
	return os;
}


void PrintCellLocationInformation::operator()( const ConstraintRegion& cr )
{
	for( const Cell* cell : cr )
	{
		cell->writeCellLocationInformation( std::cout );
	}
	std::cout << '\n';
}


/**
 *  Constraint definitions
 */
	
/// Is the constraint region a row, column or square?
Constraint::Type Constraint::calculateType( const ConstraintRegion& cr )
{
	if( cr[0]->row() == cr[4]->row() )           { assert(    cr[0]->row() == cr[8]->row()    ); return Constraint::Type::row; }
	else if( cr[0]->column() == cr[4]->column() ){ assert( cr[0]->column() == cr[8]->column() ); return Constraint::Type::column; }
	else if( cr[0]->square() == cr[4]->square() ){ assert( cr[0]->square() == cr[8]->square() ); return Constraint::Type::square; }
	else { throw std::logic_error( "constraint region is not a row, column or square" ); }
}

/// If the constraint type is a row then return a column (and vice versa).  Undefined for squares.
Constraint::Type Constraint::oppositeType( Constraint::Type type )
{
	assert( type != Constraint::Type::square && type != Constraint::Type::grid );
	return ( type == Constraint::Type::row ) ? Constraint::Type::column : Constraint::Type::row;
}
	

/// Return the string representation of the type
std::string Constraint::typeToStr( Constraint::Type type )
{
	switch (type) 
	{
	case Constraint::Type::row:
		return "row";
	case Constraint::Type::column:
		return "column";
	case Constraint::Type::square:
		return "square";
	case Constraint::Type::grid:
		return "grid";
	}
	assert( false );
	return "";
}


/// If the constraint region is a row then return the row number.  Similarly for columns and squares.
std::size_t Constraint::calculateConstantIndex( const ConstraintRegion& cr )
{
	Constraint::Type regionType = calculateType( cr );
	return cr[0]->index( regionType );
}


/// Return a frequency table of the candidates in the given constraint region	
std::vector<std::size_t> Constraint::buildCandidateFrequencyTable( const ConstraintRegion& cr )
{
	// Create a vector of many elements initialised to zero. Despite the waste of memory, we wont use the first n element, only using '1' - '9'.
	std::vector<std::size_t> frequency(':');  
	
	for( const Cell* cell : cr )
	{
		for( const char candidate : cell->candidates() )
		{						
			++frequency[candidate];  				
		}
	}	
	return frequency;
}	

/// Return a frequency table of the candidates in the given constraint region	
std::vector<std::size_t> Constraint::buildValueFrequencyTable( const ConstraintRegion& cr )
{
	// Create a vector of many elements initialised to zero. Despite the waste of memory, we wont use the first n element, only using '0' - '9'.
	std::vector<std::size_t> frequency(':');  
	for( const Cell* cell : cr )
	{
		++frequency[cell->value()];  	
	}	
	return frequency;
}	

/// Search the given constraint region and return pointers to cells containing the given candidate value
std::vector<Cell*> Constraint::findCandidateValue( const ConstraintRegion& cr, char candidateValue )
{
	std::vector<Cell*> cellsWithCandidateValue;
	for( Cell* cell : cr )
	{
		const Cell::CandidateContainer& currentCandidates = cell->candidates();
		if( std::ranges::contains( currentCandidates, candidateValue ) )
		{
			cellsWithCandidateValue.push_back( cell );
		}
	}	
	return cellsWithCandidateValue;	
}

/// Search the given constraint region and return the row/column/square indexes (specified by indexType) containing the given candidate value
std::set<std::size_t> Constraint::findCandidateValue( const ConstraintRegion& cr, char candidateValue, Constraint::Type indexType  )
{
	std::set<std::size_t> indexes;
	for( const Cell* cell : cr )
	{
		const Cell::CandidateContainer& currentCandidates = cell->candidates();
		if( std::ranges::contains( currentCandidates, candidateValue ) )
		{
			indexes.insert( cell->index(indexType) );
		}
	}	
	return indexes;	
}


bool Constraint::eliminate( ConstraintRegion& cr
						  , char candidateValue
						  , std::set<Cell*>& changedCells 
						  , const std::set<std::size_t>& preserveIndexes
			              , Constraint::Type preserveRegionType )
{
	bool didWork = false;
	// Go through given constraint region and eliminate the candidate value, being careful to leave the preserved ones. 
	for( Cell*& cell : cr )						
	{
		if( !preserveIndexes.contains( cell->index(preserveRegionType) ) )
		{
			const std::size_t candidateSizeBeforeRemoval = cell->candidates().size();
			cell->candidates().remove(candidateValue);
			
			if(cell->candidates().empty())
			{
				throw std::runtime_error( std::format( "Constraint::eliminate has eliminated '{}' from cell {}. This cell now contains no candidates.", candidateValue, cell->index() ) );
			} 
			
			if( candidateSizeBeforeRemoval > cell->candidates().size() )
			{
				changedCells.insert(cell);	
				didWork = true;
			}
		}
	}
	return didWork;
}


bool Constraint::eliminate( ConstraintRegion& cr
						   , char candidateValue
						   , std::set<Cell*>& changedCells 
						   , const std::set<Cell*>& preserveCells )
{
	bool didWork = false;
	// Go through given constraint region and eliminate the candidate value, being careful to leave the preserved ones. 
	for( Cell*& cell : cr )						
	{
		if( !preserveCells.contains( cell ) )
		{
			const std::size_t candidateSizeBeforeRemoval = cell->candidates().size();
			cell->candidates().remove(candidateValue);
			
			if(cell->candidates().empty())
			{
				throw std::runtime_error( std::format( "Constraint::eliminate has eliminated '{}' from cell {}. This cell now contains no candidates.", candidateValue, cell->index() ) );
			} 
			
			if( candidateSizeBeforeRemoval > cell->candidates().size() )
			{
				changedCells.insert(cell);	
				didWork = true;
			}
		}
	}
	return didWork;
}


bool Constraint::eliminate( const Cell* pCell
			              , ConstraintRegion& box
			              , char candidateValue
			              , const std::string& explanationFinish
		                  , std::set<Cell*>& changedCells
						  , std::set<Cell*>& explanatoryCells
						  , std::string& explanation 
					      , const std::set<const Cell*>& preserveCells )
{
	bool didWork = false;
	for( Cell*& cell : box )
	{
		if( (cell->column() == pCell->column() || cell->row() == pCell->row())
			&& !preserveCells.contains( cell ) )
		{
			Cell::CandidateContainer::iterator candidateIt = std::find(cell->candidates().begin(), cell->candidates().end(), candidateValue);
			if( candidateIt != cell->candidates().end() )
			{
				assert( cell->candidates().size() >= 2 );
				cell->candidates().erase( candidateIt );
				changedCells.insert( cell );
				// TODO: insert whole chain into explanatory cells
				
				std::ostringstream oss;
				oss << "Eliminating candidate value "  << candidateValue 
				<< " from cell ";
				cell->writeCellLocationInformation(oss) << " due to " << explanationFinish;
				explanation += oss.str();
				didWork = true;
			}					
		}
	}
	return didWork;
}


std::vector< Cell* > Constraint::findCellsContainingCandidateValue( const ConstraintRegion& cr, char candidateValue )
{
	std::vector< Cell* > pointersToCellsWithCandidateValue;
	
	for( Cell* cell : cr )
	{
		const Cell::CandidateContainer& currentCandidates = cell->candidates();
		if( std::ranges::contains( currentCandidates, candidateValue ) )
		{
			pointersToCellsWithCandidateValue.push_back(cell);
		}
	}
	
	return pointersToCellsWithCandidateValue;
}

} // namespace Sudoku
