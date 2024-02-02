#pragma once

using namespace std;

/************************************************************
#############################################################
#   Array Class
#############################################################
#
#   Class used to create dynamic array objects. 
#	Only a few basic operations are included such as 
#	sorting, accessing, resizing, push (add), searching
#	and removing duplicates
************************************************************/
template <class T> class Array {
private:
	
	int arraySize = 0;			/* Member to keep the current number of objects */
	int maxSize = 0;			/* Member to keep the current size of the array */
	T* arr = nullptr;			/* Member to keep the pointer to the array */
	int growthFactor = 100;		/* This member is used to resize the array */

	/**************************************************************
	* resize
	***************************************************************
	* Grows the array by the growth factor by allocating 
	* brand new memory, copying the old items to the new memory, 
	* and returning the old memory
	**************************************************************/
	void resize() {
		maxSize += growthFactor;
		T* newArr = new T[maxSize];
		for (int i = 0; i < arraySize; i++) {
			newArr[i] = arr[i];
		}
		delete[] arr;
		arr = newArr;
		newArr = nullptr;
	}

	/**************************************************************
	* clear
	***************************************************************
	* Deallocates the memory and safely defaults all size members
	**************************************************************/
	void clear() {
		if (arr != nullptr) {
			delete[] arr;
		}
		maxSize = 0;
		arraySize = 0;
		arr = nullptr;
	}

	/**************************************************************
	* partition & quickSort
	***************************************************************
	* Implementation of quick sort 
	**************************************************************/
	int partition(int start, int end)
	{
		T pivot = arr[start];
		int count = 0;
		for (int i = start + 1; i <= end; i++) {
			if (arr[i] <= pivot)
				count++;
		}

		int pivotIndex = start + count;
		swap(arr[pivotIndex], arr[start]);

		int i = start, j = end;
		while (i < pivotIndex && j > pivotIndex) {
			while (arr[i] <= pivot) { i++; }
			while (arr[j] > pivot) { j--; }
			if (i < pivotIndex && j > pivotIndex) {
				swap(arr[i++], arr[j--]);
			}
		}

		return pivotIndex;
	}

	void quickSort(int start, int end)
	{
		if (start >= end) { return; }
		int p = partition(start, end);
		quickSort(start, p - 1);
		quickSort(p + 1, end);
	}

public:

	/**************************************************************
	* default constructor
	**************************************************************/
	Array() : arraySize(0), maxSize(0), arr(nullptr), growthFactor(20) { }

	/**************************************************************
	* destructor
	**************************************************************/
	~Array() { clear(); }

	/**************************************************************
	* assignment operator to make exact copy of RHS with new mem
	**************************************************************/
	void operator=(const Array& copy) {
		clear();
		arraySize = copy.arraySize;
		maxSize = copy.maxSize;
		arr = new T[maxSize];
		growthFactor = 100;
		for (int i = 0; i < arraySize; i++) {
			arr[i] = copy.arr[i];
		}
	}

	/**************************************************************
	* copy constructor to make exact copy of param with new mem
	**************************************************************/
	Array(const Array& copy) {
		(*this) = copy;
	}

	/**************************************************************
	* push
	***************************************************************
	* function to add a new item to the end of the array
	**************************************************************/
	void push(T item) {
		if (arraySize == maxSize) {
			resize();
		}
		arr[arraySize] = item;
		arraySize += 1;
	}

	/**************************************************************
	* bracket operator to access reference to item
	**************************************************************/
	T& operator[](int i) {
		return arr[i];
	}

	/**************************************************************
	* bracket operator to access reference to item
	**************************************************************/
	T& at(int i) const {
		return arr[i];
	}

	/****************************************************************
	* accessor for the array size
	****************************************************************/
	int getSize() const { return arraySize; }

	/****************************************************************
	* function to quickly "clear" the array
	****************************************************************/
	void reset() { arraySize = 0; }

	/****************************************************************
	* function to check for the existance of an item in the array
	****************************************************************/
	bool exists(T item) {
		for (int i = 0; i < arraySize; i++) {
			if (item == arr[i]) {
				return true;
			}
		}
		return false;
	}

	/****************************************************************
	* public function to sort the array by quicksort
	****************************************************************/
	void sort() {
		quickSort(0, arraySize - 1);
	}

	/****************************************************************
	* function to remove an item at a specifed index
	****************************************************************/
	void remove(int i) {
		if (i < 0 || i >= arraySize) {
			return;
		}
		for (int j = i; j < arraySize - 1; j++) {
			arr[j] = arr[j + 1];
		}
		arraySize = arraySize - 1;
	}


	/****************************************************************
	* removeDuplicates
	*****************************************************************
	* function used to remove duplicate items present in the array
	* 
	* this implementation not only removes dupes but will also 
	* sort the array which may be undesired
	****************************************************************/
	void removeDuplicates() { 
		if (arraySize == 0) {
			return;
		}

		sort();
		int i = 0;
		while (i < (arraySize - 1)) {
			if (arr[i] == arr[i + 1]) {
				remove(i + 1);
			}
			else {
				i += 1;
			}
		}
	}

};
