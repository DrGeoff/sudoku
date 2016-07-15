/*
 *  constraintregion.cpp
 *  
 *  Created by Geoffery Ericksson on 13/04/10.
 *
 */

#include "constraintregion.hpp"
#include "cell.hpp"
using namespace Sudoku;

#include <iostream>
using std::ostream;
using std::cout;
using std::endl;

#include <sstream>
using std::ostringstream;

#include <algorithm>
using std::find;

#include <iterator>  // only for debug

ostream& operator<<( ostream& os, const ConstraintRegion& cr )
{
	for( ConstraintRegion::const_iterator crIt = cr.begin(); crIt != cr.end(); ++crIt )
	{
		os << **crIt << " ";
	}
	return os;
}


void PrintCellLocationInformation::operator()( const ConstraintRegion& cr )
{
	for( ConstraintRegion::const_iterator crIt = cr.begin(); crIt != cr.end(); ++crIt )
	{
		(*crIt)->writeCellLocationInformation( cout );
	}
	cout << endl;
}


/**
 *  Constraint definitions
 */
	
/// Is the constraint region a row, column or square?
Constraint::Type Constraint::calculateType( const ConstraintRegion& cr )
{
	if( cr[0]->row() == cr[4]->row() )           { assert(    cr[0]->row() == cr[8]->row()    ); return Constraint::row; }
	else if( cr[0]->column() == cr[4]->column() ){ assert( cr[0]->column() == cr[8]->column() ); return Constraint::column; }
	else if( cr[0]->square() == cr[4]->square() ){ assert( cr[0]->square() == cr[8]->square() ); return Constraint::square; }
	else { assert(false); return static_cast<Type>(-1); }
}

/// If the constraint type is a row then return a column (and vice versa).  Undefined for squares.
Constraint::Type Constraint::oppositeType( const Constraint::Type type )
{
	assert( type != Constraint::square && type != Constraint::grid );
	return ( type == Constraint::row ) ? Constraint::column : Constraint::row;
}
	

/// Return the string representation of the type
string Constraint::typeToStr( const Constraint::Type type )
{
	switch (type) 
	{
	case row:
		return "row";
		break;
	case column:
		return "column";
		break;
	case square:
		return "square";
		break;
	case grid:
		return "grid";
		break;
	default:
		assert(false);
		return "";
		break;
	}
}


/// If the constraint region is a row then return the row number.  Similarly for columns and squares.
std::size_t Constraint::calculateConstantIndex( const ConstraintRegion& cr )
{
	Type regionType = calculateType( cr );
	return cr[0]->index( regionType );
}


/// Return a frequency table of the candidates in the given constraint region	
vector<std::size_t> Constraint::buildCandidateFrequencyTable( const ConstraintRegion& cr )
{
	// Create a vector of many elements initialised to zero. Despite the waste of memory, we wont use the first n element, only using '1' - '9'.
	vector<std::size_t> frequency(':');  
	
	for( ConstraintRegion::const_iterator crIt = cr.begin(); crIt != cr.end(); ++crIt )
	{
		for( Cell::CandidateContainer::const_iterator candidateIt = (*crIt)->candidates().begin(); candidateIt != (*crIt)->candidates().end(); ++candidateIt )
		{						
			++frequency[*candidateIt];  				
			//std::cout << "Updating frequency of value (" << *candidateIt << ") to be " << frequency[*candidateIt] << std::endl;
		}
	}	
	return frequency;
}	

/// Return a frequency table of the candidates in the given constraint region	
vector<std::size_t> Constraint::buildValueFrequencyTable( const ConstraintRegion& cr )
{
	// Create a vector of many elements initialised to zero. Despite the waste of memory, we wont use the first n element, only using '0' - '9'.
	vector<std::size_t> frequency(':');  
	for( ConstraintRegion::const_iterator crIt = cr.begin(); crIt != cr.end(); ++crIt )
	{
		++frequency[(*crIt)->value()];  	
		//std::cout << "Updating frequency of value (" << (*crIt)->value() << ") to be " << frequency[(*crIt)->value()] << std::endl;
	}	
	return frequency;
}	

/// Search the given constraint region and return pointers to cells containing the given candidate value
vector<Cell*> Constraint::findCandidateValue( const ConstraintRegion& cr, const char candidateValue )
{
	vector<Cell*> cellsWithCandidateValue;
	for( ConstraintRegion::const_iterator crIt = cr.begin(); crIt != cr.end(); ++crIt )
	{
		const Cell::CandidateContainer& currentCandidates = (*crIt)->candidates();
		if( find(currentCandidates.begin(), currentCandidates.end(), candidateValue) != currentCandidates.end() )
		{
			cellsWithCandidateValue.push_back( *crIt );
		}
	}	
	return cellsWithCandidateValue;	
}

/// Search the given constraint region and return the row/column/square indexes (specified by indexType) containing the given candidate value
set<std::size_t> Constraint::findCandidateValue( const ConstraintRegion& cr, const char candidateValue, const Type indexType  )
{
	set<std::size_t> indexes;
	for( ConstraintRegion::const_iterator crIt = cr.begin(); crIt != cr.end(); ++crIt )
	{
		const Cell::CandidateContainer& currentCandidates = (*crIt)->candidates();
		if( find(currentCandidates.begin(), currentCandidates.end(), candidateValue) != currentCandidates.end() )
		{
			indexes.insert( (*crIt)->index(indexType) );
		}
	}	
	return indexes;	
}


bool Constraint::eliminate( ConstraintRegion& cr
						  , const char candidateValue
						  , set<Cell*>& changedCells 
						  , const set<std::size_t>& preserveIndexes
			              , const Constraint::Type preserveRegionType )
{
	/*
	cout << "Preserve indexes are : ";
	std::copy(preserveIndexes.begin(), preserveIndexes.end(), std::ostream_iterator<std::size_t>(std::cout,","));
	cout << endl;
	*/
	bool didWork = false;
	// Go through given constraint region and eliminate the candidate value, being careful to leave the preserved ones. 
	for( ConstraintRegion::iterator crIt = cr.begin(); crIt != cr.end(); ++crIt )						
	{
		//cout << "Examining cell index " << (*crIt)->index() << " with preserve region index " << (*crIt)->index(preserveRegionType) << endl;
		if( find( preserveIndexes.begin(), preserveIndexes.end(), (*crIt)->index(preserveRegionType) ) == preserveIndexes.end() )
		{
			const std::size_t candidateSizeBeforeRemoval = (*crIt)->candidates().size();
			(*crIt)->candidates().remove(candidateValue);
			
			if((*crIt)->candidates().empty())
			{
				std::cout << "Uh oh.  Constraint::eliminate has eliminated " << candidateValue << " from cell " << (*crIt)->index()  
				          << ".  This cell now contains no candidates." << std::endl;
				throw;  // FIXME throw an actual error.
			} 
			
			if( candidateSizeBeforeRemoval > (*crIt)->candidates().size() )
			{
				//cout << "Removing " << candidateValue << " from cell at ";
				//(*crIt)->writeCellLocationInformation( cout ) << endl;
				changedCells.insert(*crIt);	
				didWork = true;
			}
		}
	}
	return didWork;
}


bool Constraint::eliminate( ConstraintRegion& cr
						   , const char candidateValue
						   , set<Cell*>& changedCells 
						   , const set<Cell*>& preserveCells )
{
	bool didWork = false;
	// Go through given constraint region and eliminate the candidate value, being careful to leave the preserved ones. 
	for( ConstraintRegion::iterator crIt = cr.begin(); crIt != cr.end(); ++crIt )						
	{
		//cout << "Examining cell index " << (*crIt)->index() << endl;
		if( find( preserveCells.begin(), preserveCells.end(), *crIt ) == preserveCells.end() )
		{
			const std::size_t candidateSizeBeforeRemoval = (*crIt)->candidates().size();
			(*crIt)->candidates().remove(candidateValue);
			
			if((*crIt)->candidates().empty())
			{
				std::cout << "Uh oh.  Constraint::eliminate has eliminated " << candidateValue << " from cell " << (*crIt)->index()  
				<< ".  This cell now contains no candidates." << std::endl;
				throw;  // FIXME throw an actual error.
			} 
			
			if( candidateSizeBeforeRemoval > (*crIt)->candidates().size() )
			{
				//cout << "Removing " << candidateValue << " from cell at ";
				//(*crIt)->writeCellLocationInformation( cout ) << endl;
				changedCells.insert(*crIt);	
				didWork = true;
			}
		}
	}
	return didWork;
}


bool Constraint::eliminate( const Cell* const pCell
			              , ConstraintRegion& box
			              , const char candidateValue
			              , const string& explanationFinish
		                  , set<Cell*>& changedCells
						  , set<Cell*>& explanatoryCells
						  , string& explanation 
					      , const set<const Cell*>& preserveCells )
{
	bool didWork = false;
	for( ConstraintRegion::iterator crIt = box.begin(); crIt != box.end(); ++crIt )
	{
		if( ((*crIt)->column() == pCell->column() || (*crIt)->row() == pCell->row())
			&& find( preserveCells.begin(), preserveCells.end(), *crIt ) == preserveCells.end() )
		{
			Cell::CandidateContainer::iterator candidateIt = find((*crIt)->candidates().begin(), (*crIt)->candidates().end(), candidateValue);
			if( candidateIt != (*crIt)->candidates().end() )
			{
				assert( (*crIt)->candidates().size() >= 2 );
				(*crIt)->candidates().erase( candidateIt );
				changedCells.insert( *crIt );
				// TODO: insert whole chain into explanatory cells
				
				ostringstream oss;
				oss << "Eliminating candidate value "  << candidateValue 
				<< " from cell ";
				(*crIt)->writeCellLocationInformation(oss) << " due to " << explanationFinish;
				explanation += oss.str();
				didWork = true;
			}					
		}
	}
	return didWork;
}


vector< Cell* > Constraint::findCellsContainingCandidateValue( const ConstraintRegion& cr, const char candidateValue )
{
	vector< Cell* > pointorsToCellsWithCandidateValue;
	
	for( ConstraintRegion::const_iterator crIt = cr.begin(); crIt != cr.end(); ++crIt )
	{
		const Cell::CandidateContainer& currentCandidates = (*crIt)->candidates();
		if( find( (*crIt)->candidates().begin(), (*crIt)->candidates().end(), candidateValue ) != currentCandidates.end() )
		{
			pointorsToCellsWithCandidateValue.push_back(*crIt);
		}
	}
	
	return pointorsToCellsWithCandidateValue;
}


