#pragma once

using namespace std;

/**********************************************************************************************
###############################################################################################
##### ARRAY CLASS
###############################################################################################
**********************************************************************************************/
template <class T> class Array {
private:

	/* Array Members */
	size_t arraySize = 0;
	size_t maxSize = 0;
	T* arr = nullptr;
	size_t growthFactor = 100;

	/****************************************************************
	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-

	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-
	****************************************************************/
	void resize() {
		maxSize += growthFactor;
		T* newArr = new T[maxSize];
		for (size_t i = 0; i < arraySize; i++) {
			newArr[i] = arr[i];
		}
		delete[] arr;
		arr = newArr;
		newArr = nullptr;
	}

	/****************************************************************
	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-

	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-
	****************************************************************/
	void clear() {
		if (arr != nullptr) {
			delete[] arr;
		}
		maxSize = 0;
		arraySize = 0;
		arr = nullptr;
	}

	/****************************************************************
	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-

	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-
	****************************************************************/
	size_t partition(size_t start, size_t end)
	{
		T pivot = arr[start];
		size_t count = 0;
		for (size_t i = start + 1; i <= end; i++) {
			if (arr[i] <= pivot)
				count++;
		}

		// Giving pivot element its correct position
		size_t pivotIndex = start + count;
		swap(arr[pivotIndex], arr[start]);

		// Sorting left and right parts of the pivot element
		size_t i = start, j = end;

		while (i < pivotIndex && j > pivotIndex) {

			while (arr[i] <= pivot) {
				i++;
			}

			while (arr[j] > pivot) {
				j--;
			}

			if (i < pivotIndex && j > pivotIndex) {
				swap(arr[i++], arr[j--]);
			}
		}

		return pivotIndex;
	}

	void quickSort(size_t start, size_t end)
	{

		// base case
		if (start >= end)
			return;

		// partitioning the array
		size_t p = partition(start, end);

		// Sorting the left part
		quickSort(start, p - 1);

		// Sorting the right part
		quickSort(p + 1, end);
	}

public:

	/****************************************************************
	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-

	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-
	****************************************************************/
	Array() : arraySize(0), maxSize(0), arr(nullptr), growthFactor(1) { }

	/****************************************************************
	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-

	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-
	****************************************************************/
	~Array() { clear(); }

	/****************************************************************
	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-

	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-
	****************************************************************/
	void operator=(const Array& copy) {
		clear();
		arraySize = copy.arraySize;
		maxSize = copy.maxSize;
		arr = new T[maxSize];
		growthFactor = 100;
		for (size_t i = 0; i < arraySize; i++) {
			arr[i] = copy.arr[i];
		}
	}

	/****************************************************************
	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-

	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-
	****************************************************************/
	Array(const Array& copy) {
		(*this) = copy;
	}

	/****************************************************************
	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-

	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-
	****************************************************************/
	void add(T item) {
		if (arraySize == maxSize) {
			resize();
		}
		arr[arraySize] = item;
		arraySize += 1;
	}

	/****************************************************************
	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-

	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-
	****************************************************************/
	T& operator[](size_t i) {
		return arr[i];
	}

	T& at(size_t i) const {
		return arr[i];
	}

	/****************************************************************
	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-

	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-
	****************************************************************/
	size_t getSize() const { return arraySize; }

	/****************************************************************
	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-

	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-
	****************************************************************/
	void reset() { arraySize = 0; }

	/****************************************************************
	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-

	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-
	****************************************************************/
	bool exists(T item) {
		for (size_t i = 0; i < arraySize; i++) {
			if (item == arr[i]) {
				return true;
			}
		}
		return false;
	}

	/****************************************************************
	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-

	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-
	****************************************************************/
	void sort() {
		quickSort(0, arraySize - 1);
	}

	/****************************************************************
	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-

	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-
	****************************************************************/
	void remove(size_t i) {
		if (i < 0 || i >= arraySize) {
			return;
		}
		for (size_t j = i; j < arraySize - 1; j++) {
			arr[j] = arr[j + 1];
		}
		arraySize = arraySize - 1;
	}


	/****************************************************************
	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-

	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-
	****************************************************************/
	void removeDuplicates() { // WARNING THIS WILL SORT YOUR ARRAY
		if (getSize() == 0) {
			return;
		}

		sort();
		size_t i = 0;
		while (i < (arraySize - 1)) {
			if (arr[i] == arr[i + 1]) {
				remove(i + 1);
			}
			else {
				i += 1;
			}
		}
	}

	/****************************************************************
	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-

	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-
	****************************************************************/
	size_t getIndexOfFirstInstance(const T& item) {
		for (size_t i = 0; i < arraySize; i++) {
			if (arr[i] == item) {
				return i;
			}
		}
		return arraySize; // this is meant to be out of bounds to error
	}
};
