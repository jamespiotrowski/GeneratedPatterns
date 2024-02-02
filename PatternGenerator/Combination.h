#pragma once

#include "Array.h"

using namespace std;

/**************************************************************
* AllCombinations (work implementation)
***************************************************************
* Recursive function to generate all combinations of a 
* specified length. 
* 
* vals is the items that can be used in the combination
* 
* currentCombination is where an instance of a combination
* is built. It happens recursively. currentCombination will
* be finished once the number of recursions matches the 
* specified length
* 
* combinations is a array to hold all possible combinations
**************************************************************/
template<typename T>
void AllCombinations(const int &length, const Array<T> &vals, const Array<T> &currentCombination, Array<Array<T>> &combinations) {
	/* Base Case */
	if (currentCombination.getSize() == length) {
		combinations.push(currentCombination); /* add combination to list */
		return;
	}
	for (int i = 0; i < vals.getSize(); i++) {
		Array<T> newArr = currentCombination;					/* make a new copy of the current combination */
		newArr.push(vals.at(i));								/* add the current item to the array */
		AllCombinations(length, vals, newArr, combinations);	/* recurse */
	}
	return;
}

/**************************************************************
* AllCombinations (interface implementation)
***************************************************************
* Function to interface with when ready to generate all 
* combinations 
* 
* The user does not need to provide 3 of the necessary 
* parameters used specifically for recursion, thus this 
* function sets up those parameters and calls the work
* implementation. 
**************************************************************/
template<typename T>
Array<Array<T>> AllCombinations(const Array<T>& vals, const unsigned int& len) {
	Array<Array<T>> c;
	if (vals.getSize() == 0) {
		return c;
	}
	Array<T> currentCombination;
	AllCombinations(len, vals, currentCombination, c);
	return c;
}

/**************************************************************
* SomeCombinations (work implementation)
***************************************************************
* Recursive function to generate some combinations of a
* specified length.
*
* vals is the items that can be used in the combination
*
* currentCombination is where an instance of a combination
* is built. It happens recursively. currentCombination will
* be finished once the number of recursions matches the
* specified length
*
* combinations is a array to hold all possible combinations
* 
* percNum is the numerator of a fraction (X)
* 
* percDen is the denominator of a fraction (Y)
* 
* numCombinations tracks the number of combos generated 
* 
* this function will keep every X out of Y combinations
**************************************************************/
template<typename T>
void SomeCombinations(const int& length, const Array<T>& vals, const Array<T>& currentCombination, Array<Array<T>>& combinations, const int& percNum, const int& percDen, int& numCombinations) {
	/* Base Case */
	if (currentCombination.getSize() == length) {		/* if combo is finished */
		if ((numCombinations % percDen) < percNum) {	/* if combo will be kept */
			combinations.push(currentCombination);		/* add it to the array */
		}
		return;
	}
	for (int i = 0; i < vals.getSize(); i++) {
		numCombinations += 1;
		Array<T> newArr = currentCombination;	/* make a new copy of the current combination */
		newArr.push(vals.at(i));				/* add the current item to the array */
		SomeCombinations(length, vals, newArr, combinations, percNum, percDen, numCombinations); /* recurse */
	}
	return;
}

/**************************************************************
* SomeCombinations (interface implementation)
***************************************************************
* Function to interface with when ready to generate some
* combinations
*
* The user does not need to provide 6 of the necessary
* parameters used specifically for recursion, thus this
* function sets up those parameters and calls the work
* implementation.
* 
* percentage is a parameter used to limit the number of
* combos generated. It will be slightly altered from c
* to (1 / floor(1 / c)) so that determining which
* combinations to keep is more simple and robust. 
* 
* i.e. it is easier to implement a function to keep every 
* 1 out of 3 combos compared to a function that keeps 33% of 
* all combos. 
**************************************************************/
template<typename T>
Array<Array<T>> SomeCombinations(const Array<T>& vals, const unsigned int& len, const double& percentage = 1.0) {
	Array<Array<T>> c;
	if (vals.getSize() == 0 || percentage > 1.0 || percentage <= 0.0) {
		return c;
	}
	/* Set up fields to represent percentage as a fraction */
	int numerator = 1; 
	int denominator = 1;

	denominator = 1.0 / percentage;							/* divide 1 by the percentage and take the floor*/
	denominator = (denominator < 1.0) ? 1.0 : denominator;	/* if less than 1, set to 1 */

	int numCombinations = 0;		/* field for recursion - needed to determine which combos to keep */
	Array<T> currentCombination;	/* field for recursion - needed to store a recursive chains instance of a combo */

	/* if the total number of combos will be less than 100, keep them all - used for making sample images */
	if (pow(vals.getSize(), len) < 100 || (numerator == denominator)) {
		return AllCombinations(vals, len);
	}

	/* begin recursion */
	SomeCombinations(len, vals, currentCombination, c, numerator, (int)denominator, numCombinations);
	return c;
}
