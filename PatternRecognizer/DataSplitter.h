#include <random>
#include <iostream>
#include "Array.h"

/************************************************************
#############################################################
#   className
#############################################################
#
************************************************************/
class DataSplitter{
private:
    long seed = 0;


    Array<unsigned int> classLabels;
    unsigned int *labels = nullptr;
    unsigned int size = 0;

    unsigned int numClasses = 0;
    Array<unsigned int> classCount;

    Array<unsigned int> trainingSet;
    Array<unsigned int> testingSet;
    
    void analyzeData();

public:

    /* These should not be needed or used */
    DataSplitter() = delete;
    DataSplitter(const DataSplitter& copy) = delete;
    void operator=(const DataSplitter& copy) = delete;

    /* Constructors and Destructors */
    DataSplitter(const unsigned int labels[], const unsigned int& size, long seed);
    DataSplitter(const unsigned int labels[], const unsigned int& size);
    ~DataSplitter();

    /* Required Functions */
    void splitData(const double& splitPercentage);
    void getSplitData(unsigned int*& trainingSetPtr, unsigned int& trainingCount, unsigned int*& testingSetPtr, unsigned int& testingCount);
};
/************************************************************
* ///////////////////////////|\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
* \\\\\\\\\\\\\\\\\\\\\\\\\\\|///////////////////////////////
************************************************************/


/******************************************************************************
 * functionName 
-------------------------------------------------------------------------------
 * 
*******************************************************************************/
DataSplitter::DataSplitter(const unsigned int labels[], const unsigned int& size, long seed) : seed(seed), size(size) {
    this->labels = new unsigned int[size];
    for(unsigned int i = 0; i < size; i++){
        this->labels[i] = labels[i];
    }
    analyzeData();
}

/******************************************************************************
 * functionName 
-------------------------------------------------------------------------------
 * 
*******************************************************************************/
DataSplitter::DataSplitter(const unsigned int labels[], const unsigned int& size) : seed(0), size(size) {
    this->labels = new unsigned int[size];
    for(unsigned int i = 0; i < size; i++){
        this->labels[i] = labels[i];
    }
    analyzeData();
}

DataSplitter::~DataSplitter(){
    if(labels != nullptr){
        delete[] labels;
    }
}

/******************************************************************************
 * functionName 
-------------------------------------------------------------------------------
 * 
*******************************************************************************/
void DataSplitter::analyzeData(){

    classCount.reset();
    classLabels.reset();
    
    /* Figure out the number of classes */
    for(unsigned int i = 0; i < size; i++){
        if(!classLabels.exists(labels[i])){
            numClasses += 1;
            classLabels.add(labels[i]);
        }
    }

    /* Set up a class counter for each class */
    for(unsigned int i = 0; i < numClasses; i++){
        classCount.add(0);
    }

    /* Count the number of classes */
    for(unsigned int i = 0; i < size; i++){
        unsigned int index = classLabels.getIndexOfFirstInstance(labels[i]);
        if (index < classLabels.getSize()) {
            classCount[index] += 1;
        }
    }

    for (unsigned int i = 0; i < classCount.getSize(); i++) {
        cout << classCount[i] << endl;
    }

}


/******************************************************************************
 * functionName
-------------------------------------------------------------------------------
 *
*******************************************************************************/
void DataSplitter::splitData(const double& splitPercentage){

    Array<unsigned int> testingCount = classCount;
    Array<unsigned int> trainingCount = classCount;
    Array<unsigned int> currentTestingCount = classCount;
    Array<unsigned int> currentTrainingCount = classCount;
    for (size_t i = 0; i < classCount.getSize(); i++) {
        testingCount[i] = classCount[i] * splitPercentage;
        trainingCount[i] = classCount[i] - testingCount[i];
        currentTestingCount[i] = 0;
        currentTrainingCount[i] = 0;
    }

    trainingSet.reset();
    testingSet.reset();

    for (unsigned int i = 0; i < size; i++) {
        size_t index = classLabels.getIndexOfFirstInstance(labels[i]);
        if (currentTestingCount[index] < testingCount[index]) {
            testingSet.add(i);
            currentTestingCount[index] += 1;
        }
        else {
            trainingSet.add(i);
            currentTrainingCount[index] += 1;
        }

    }
}

/******************************************************************************
 * functionName
-------------------------------------------------------------------------------
 *
*******************************************************************************/
void DataSplitter::getSplitData(unsigned int*& trainingSetPtr, unsigned int& trainingCount, unsigned int*& testingSetPtr, unsigned int& testingCount) {
    trainingSetPtr = new unsigned int[trainingSet.getSize()];
    testingSetPtr = new unsigned int[testingSet.getSize()];

    trainingCount = trainingSet.getSize();
    testingCount = testingSet.getSize();

    for (unsigned int i = 0; i < testingCount; i++) {
        testingSetPtr[i] = testingSet[i];
    }

    for (unsigned int i = 0; i < trainingCount; i++) {
        trainingSetPtr[i] = trainingSet[i];
    }
}

/******************************************************************************
 * functionName
-------------------------------------------------------------------------------
 *
*******************************************************************************/
template<typename t>
void shuffleData(t *arr, const unsigned int& arrSize, const unsigned int& passThroughs = 3) {
    unsigned int iterations = arrSize * passThroughs;
    unsigned int i1, i2;
    t temp;
    for (unsigned int i = 0; i < iterations; i++) {
        i1 = rand() % arrSize;
        i2 = rand() % arrSize;
        while (i1 == i2) {
            i2 = rand() % arrSize;
        }
        temp = arr[i1];
        arr[i1] = arr[i2];
        arr[i2] = temp;
    }
}
