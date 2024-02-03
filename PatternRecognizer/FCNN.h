#pragma once

/************************************************
 * Fully Connected Neural Network Class for C++
 * Code Written by James Piotrowski
 * University of Nevada Las Vegas
 * December 2021
************************************************/

/* Headers */
#include <random>
#include <iostream>
#include <pthread.h>
#include <chrono>
#include <ctime>   
#include <string>
#include <fstream>

using namespace std;

/************************************************************
#############################################################
#   Prediction Class
#############################################################
#
#   Small class for prediction objects
#   Used to hold the output of Neural
#   Network.
************************************************************/

class prediction {
private:
    unsigned int size;      /* To hold the size of the prediction */
    double* arr = nullptr;  /* pointer to memory holding the prediction data */

public:
    /* No default constructor needed */
    prediction() = delete;

    /* Param constructor for easy set up */
    prediction(const unsigned int& s) {
        size = s;
        arr = new double[size];
    }

    /* Destructor to clean up dynamic memory */
    ~prediction() {
        if (arr != nullptr) {
            delete[] arr;
        }
    }

    /* Copy Constructor */
    prediction(const prediction& copy) {
        arr = nullptr;
        size = 0;
        (*this) = copy;
    }

    /* Assignment Operator */
    void operator=(const prediction& copy) {
        if (arr != nullptr) {
            delete[] arr;
            arr = nullptr;
        }
        size = copy.size;
        if (size == 0) {
            return;
        }
        arr = new double[size];
        for (unsigned int i = 0; i < size; i++) {
            arr[i] = copy.arr[i];
        }
    }

    /* Bracket Operator for Easy Access */
    double& operator[](const unsigned int& ind) {
        if (ind >= size) {
            exit(1);
        }
        return arr[ind];
    }

    /* Accessor for size */
    unsigned int getSize() const { return size; }

};
/************************************************************
* ///////////////////////////|\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
* \\\\\\\\\\\\\\\\\\\\\\\\\\\|///////////////////////////////
************************************************************/


/************************************************************
#############################################################
#   Fully Connected Neural Network Class
#############################################################
#
#   Class to create fully connected neural network objects.
************************************************************/
class fcnn {
    
    /* Not needed - could be implemented but I dont think default FCNNs are needed
    * and I dont think that copying an fcnn is necessary */
    fcnn() = delete;
    fcnn(const fcnn& copy) = delete;
    void operator=(const fcnn& copy) = delete;

private:

    /*---------------------------------------------*/
    /* Enum for the activation function being used */
    /*---------------------------------------------*/
    enum activationFunction {
        SIGMOID
        , TANH
        , RELU
        , LEAKY_RELU
        , SOFTPLUS
    };

    static string activationFunctionToString(const activationFunction& af) {
        switch (af) {
            case(SIGMOID): { return "SIGMOID"; }
            case(TANH): { return "TANH"; }
            case(RELU): { return "RELU"; }
            case(LEAKY_RELU): { return "LEAKY_RELU"; }
            case(SOFTPLUS): { return "SOFTPLUS"; }
            default: { return ""; }
        }
        return "";
    }

    static activationFunction activationFunctionFromString(const string& af) {
        if (af == "SIGMOID") { return SIGMOID; }
        if (af == "TANH") { return TANH; }
        if (af == "RELU") { return RELU; }
        if (af == "LEAKY_RELU") { return LEAKY_RELU; }
        if (af == "SOFTPLUS") { return SOFTPLUS; }
        return SIGMOID;
    }

    /*---------------------------------------------*/
    /* Enum for certain functions - used for threading */
    /*---------------------------------------------*/
    enum functionToCall {
        TRAIN_THREAD
        , VALIDATE_THREAD
    };

    /*---------------------------------------------*/
    /** Essential FCNN Structure Members **/
    /*---------------------------------------------*/
    unsigned int inputSize = 0;         /* Size of the input */
    unsigned int numLayers = 0;   /* Number of hidden layers including the output layer */
    unsigned int* layerSize = nullptr;  /* Array holds the size for each layer */
    double*** w = nullptr;              /* Weights for each layer  */
    double** b = nullptr;               /* Node Bias */
    double** s = nullptr;               /* Node result prior to activation funtion (s = i[0]*w[0] + i[1]*w[i] ... i[n]*w[n] + b)  */
    double** y = nullptr;               /* Node result after Activation Node */
    double e = 0.1;                     /* Learning Rate */
    double* error = nullptr;            /* Output Error Array */
    double** errStore = nullptr;        /* Place to store error calculations during back prop */
    double*** wN = nullptr;             /* Place to store updated weights */
    double** bN = nullptr;              /* Place to store updated biases */
    unsigned int lastLayer = 0;         /* Index of the output layer */
    bool useSoftMax = false;            /* Bool to indicate if the FCNN will use softmax on the last layer */
    activationFunction af = SIGMOID;    /* Activation function used by the whole network */
    static unsigned seed;               /* Seed for random number generation */
    static default_random_engine generator; /* Random number generator for initialization of weights */
    static normal_distribution<double> distribution; /* Normal distribution for initialization of weights */
    double averagePredictionTime = 0.0;     /* member used to track the average prediction time of the network */
    double averageEpochTime = 0.0;          /* member used to track the average epoch time of the network */

    /*---------------------------------------------*/
    /** Member required for parallel operation **/
    /*---------------------------------------------*/
    static const unsigned int maxThreads = 32000;   /* Maximum possible number of threads */
    unsigned int numThreads = 1;        /* Total number of threads to use during training */
    bool useThreads = false;            /* bool to indicate if the FCNN should use threads */

    /*---------------------------------------------*/
    /** Structs for argument passing in threads **/
    /*---------------------------------------------*/

    /**************************************************************
    *   Helper Node
    ***************************************************************
    * Because threads are being spawned from inside 
    * an object and being called with member functions,
    * we need to pass the object reference to call the right
    * function. I've created a function called "call member
    * function for thread" which can call different functions from
    * the object. To call the function, we need to know 
    * the exact function to call (ftc), the object reference,
    * and the arguments for that function. 
    * 
    * Functions currently being called is the TRAIN_THREAD which
    * is the basic training function for an individual thread
    * and VALIDATE_THREAD which is a function to validate a data
    * set using threads.
    **************************************************************/
    struct helperNode {
        functionToCall ftc = TRAIN_THREAD;
        fcnn* objectReference = nullptr;
        void* arguments = nullptr;
    };

    /**************************************************************
    *   Thread Arguments
    ***************************************************************
    * Struct to hold all the necessary parameters for a thread
    * to do it's work
    **************************************************************/
    struct threadArguments {
        unsigned int threadId = 0;              /* Threads identifier */
        pthread_barrier_t* barrier = nullptr;   /* Barrier array for threads to sync on */
        double** dataInput = nullptr;           /* Pointer to pass the input array along */
        double** dataOutput = nullptr;          /* Pointer to pass the label array along */
        unsigned int numData = 0;               /* Number of data samples */
        unsigned int epochs = 0;                /* Number of epochs */
        prediction* p = nullptr;                /* Pointer to store neural network output */
    };

    /*---------------------------------------------*/
    /** FCNN utility functions **/
    /*---------------------------------------------*/

    /* Function used to deallocate all dynamic memory */
    void deleteAll();

    /* Basic initialization function for FCNN weights and biases */
    double init() const {
        return ((rand() % 2) == 0 ? (((double)(rand() % 1000)) / 100000.0) : (-1.0 * (((double)(rand() % 1000)) / 100000.0)));
    }

    /* Xavier initialization function for FCNN weights and biases */
    double init(const int& numInputs) const {
        return ((distribution(generator)) * sqrt(1.0 / (double)numInputs));
    }

    /* Function used to get the size of the previous layer */
    unsigned int getPreviousSize(const unsigned int& i) const {
        if (i == 0) {
            return inputSize;
        }
        return layerSize[i - 1];
    }

    /* Function used to get the size of the next layer */
    unsigned int getNextSize(const unsigned int& i) const {
        if (i == lastLayer) {
            exit(1); /* should not reach this... */
        }
        return layerSize[i + 1];
    }

    /*---------------------------------------------*/
    /** FCNN utility functions for threads **/
    /*---------------------------------------------*/

    /* Function is used to copy parameters into a thread arguments object */
    static void setThreadArguments(threadArguments &ta, const unsigned int& threadId, pthread_barrier_t* barrier, double** dataInput, double** dataOutput, const unsigned int& numData, const unsigned int& epochs, prediction *p) {
        ta.threadId = threadId;
        ta.barrier = barrier;
        ta.dataInput = dataInput;
        ta.dataOutput = dataOutput;
        ta.numData = numData;
        ta.epochs = epochs;
        ta.p = p;
    }

    /* Function is used determine the range of nodes a thread will be responsible for */
    static void computeThreadRange(unsigned int& minNode, unsigned int& maxNode, const unsigned int& threadNum, const unsigned int& numThreads, const unsigned int& layerSize) {
        minNode = (unsigned int)((double)threadNum * (((double)layerSize) / ((double)numThreads)));
        maxNode = (unsigned int)((double)(threadNum + 1) * (((double)layerSize) / ((double)numThreads)));
    }


    /* This function is what a thread will call to begin it's life.
    *  The object reference is required to call from inside the FCNN
    *  as well as the arguments required for whatever function is being
    *  called 
    * 
    * See helperNode struct & functionToCall enum for more information */
    static void* callMemberFunctionForThread(void* args);

    /*---------------------------------------------*/
    /** FCNN operation functions  **/
    /*---------------------------------------------*/

    /* softmax activation function - only for the last layer (currently) */
    double softMax(const unsigned int& i) {
        double sum = 0.0;
        for (unsigned int j = 0; j < layerSize[lastLayer]; j++) {
            sum += exp(s[lastLayer][j]);
        }
        return exp(s[lastLayer][i]) / sum;
    }

    /* derivative of the softmax with respect to a certain node */
    double softMaxDerivative(const unsigned int& i) {
        /* Some optimization could happen here 
        
            right now each thread computes this sum then does
            thier work. Sum could be computed for all threads just
            once... Not sure if overhead is worth it. Speed seems
            negligible 
        */
        double sum = 0.0;
        for (unsigned int j = 0; j < layerSize[lastLayer]; j++) {
            sum += exp(s[lastLayer][j]);
        }
        double eVal = exp(s[lastLayer][i]);
        return ((sum * eVal) - (eVal * eVal)) / (sum * sum);
    }

    /* This function computes the activation function during predictions */
    double computeActivationFunction(const double& d, const unsigned int& layer, const unsigned int& node) {
        /* If we are in the last layer and using softmax, call this function */
        if (layer == lastLayer && useSoftMax) { return softMax(node); }

        /* Switch on the activation function */
        switch (af) {
            case(SIGMOID): { return (1.0 / (1.0 + exp(-d))); }
            case(RELU): { return (d > 0.0) ? d : 0.0; }
            case(TANH): { return tanh(d); }
            case(LEAKY_RELU): { return (d > 0.0) ? d : 0.1 * d; }
            case(SOFTPLUS): { return log(1 + exp(d)); }
            default: { return 0; }
        }
        return 0;
    }

    /* This function computes the activation function derivative during backprop */
    double computeActivationFunctionDerivative(const double& aaf, const double& baf, const unsigned int& layer, const unsigned int& node) {
        /* aaf means After Activation Function - useful for tanh and sigmoid */
        /* baf means Before Activiation Function - needed for act funcs that dont have themselves in the derivative */

         /* If we are in the last layer and using softmax, call this function */
        if (lastLayer == layer && useSoftMax) { return softMaxDerivative(node); }

        /* Switch on the activation function */
        switch (af) {
            case(SIGMOID): { return (aaf * (1.0 - aaf)); }
            case(RELU): { return (baf > 0.0) ? 1.0 : 0.0; }
            case(LEAKY_RELU): { return (baf > 0.0) ? 1.0 : 0.1; }
            case(TANH): { return 1.0 - (aaf * aaf); }
            case(SOFTPLUS): { return 1.0 / (1.0 + exp(-1.0 * baf)); }
            default: { return 0; }
        }
        return 0;
    }

    /* This predict function used only by threads during training and validation */
    void predict(double* input, unsigned int** threadRange, prediction& p, pthread_barrier_t* barrier);
    
    /* Function used to compute a single node in the NN during forward computation - does the full computation including the activation function */
    void computeForwardNode(const unsigned int& layer, const unsigned int& node, double* inp, const unsigned int& inpSize);

    /* Function used to compute a single node in the NN during forward computation - does the full computation EXCEPT for activation function
    *  This is needed for Softmax because all nodes need to be computed prior to calling the softmax */
    void computeForwardNode_NoActivationFunction(const unsigned int& layer, const unsigned int& node, double* input, const unsigned int& inputSize);

    /* Function used to apply act func to last layer - used together with computeForwardNode_NoActivationFunction */
    void computeForwardNode_ActivationFunction(const unsigned int& layer, const unsigned int& node, double* input, const unsigned int& inputSize);

    /* Function used to back prop on a single node */
    void computeBackwardNode(const int& layer, const unsigned int& node, double* input, const unsigned int& inpSize);

    /* Function used to drive the entire training process using threads - called from public train() function */
    void trainMaster(double** dataInput, double** dataOuput, const unsigned int& numData, const unsigned int& epochs, ostream* out);

    /* Function to train the FCNN from an individual thread - this function is called many times from trainMaster */
    void trainIndividual(void* args);

    /* Function used to get stats on the prediction like the error, the answer, and if it was a correct guess */
    void getPredictionStats(double* answer, unsigned int& answerClass, double& error, bool& correctGuess);

    /* Function used to print the results of a validation */
    static void printValidationResults(const unsigned int& outputSize, const double& averagePredictionTime, const double& averageError, const unsigned int& totalCorrect, const unsigned int& numData, const double& accuracy, const unsigned int classCorrectCount[], const unsigned int classCount[], ostream& out);

    /* Function used to drive the entire validation process using threads - called from public validate() function */
    void validateIndividual(void* args);

    /* Function to validate a dataset from an individual thread - this function is called many times from validateIndividual */
    void validateMaster(double** dataInput, double** dataOuput, const unsigned int& numData, ostream* out);

    /*  */
    void allocateFcnn(const unsigned int& inputSize, const unsigned int& numLayers, const unsigned int layerSizes[], const unsigned int& numThreads, const bool& useThreads, const string& actFunc, const bool& useSoftMax);

public:
    
    /* Only constructor for fcnn - used to initialize the entire structure */
    fcnn(const unsigned int& inputSize, const unsigned int& numLayers, const unsigned int layerSizes[], const unsigned int& numThreads, const bool& useThreads, const string& actFunc, const bool& useSoftMax) {
        allocateFcnn(inputSize, numLayers, layerSizes, numThreads, useThreads, actFunc, useSoftMax);
    }
    
    /* Destructor needed to release all dynamic memory */
    ~fcnn() { deleteAll(); }

    /* basic prediction function */
    prediction predict(double* input);

    /* basic train function */
    void train(double** dataInput, double** dataOuput, const unsigned int& numData, const unsigned int& epochs, const double& lr, ostream* out);

    /* basic validate function */
    void validate(double** dataInput, double** dataOuput, const unsigned int& numData, ostream* out);

    /* Getters */
    double getAverageEpochTime() const { return averageEpochTime; }
    double getAveragePredictionTime() const { return averagePredictionTime; }

    /* Only constructor for fcnn - used to initialize the entire structure */
    static void determineMostEfficientModel(const unsigned int& inputSize, const unsigned int& numLayers, const unsigned int layerSizes[], const unsigned int& maxThreads, const string& actFunc, const bool& useSoftMax, const bool& printProgress);

    /*  */
    void exportFcnn(const string& outFile) const;

    /*  */
    void importFcnn(const string& outFile);
};


unsigned fcnn::seed = 0;
default_random_engine fcnn::generator(seed);
normal_distribution<double> fcnn::distribution(0.0, 1.0);

/************************************************************
* ///////////////////////////|\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
* \\\\\\\\\\\\\\\\\\\\\\\\\\\|///////////////////////////////
************************************************************/

/**************************************************************
*  callMemberFunctionForThread
***************************************************************
*  This function is what a thread will call to begin it's life.
*  The object reference is required to call from inside the FCNN
*  as well as the arguments required for whatever function is being
*  called
*
*  See helperNode struct & functionToCall enum for more information
**************************************************************/
void* fcnn::callMemberFunctionForThread(void* args) {
    helperNode* hn = (helperNode*)args; /* Copy the argument reference */
    switch (hn->ftc) { /* Check which function to call */
    case(TRAIN_THREAD): { /* Standard training function for threads */
        hn->objectReference->trainIndividual(hn->arguments);
        break;
    }
    case(VALIDATE_THREAD): { /* Function to validate data sets */
        hn->objectReference->validateIndividual(hn->arguments);
        break;
    }
    default: {
        break;
    }
    }
    pthread_exit(NULL); /* End of threads life */
    return nullptr;
}


/**************************************************************
*  fcnn default constructor
---------------------------------------------------------------
*  A lot of things happening in here. It is nothing fancy,
*  just the dynamic allocation of the structure and init
*  of all members
**************************************************************/
void fcnn::allocateFcnn(const unsigned int& inputSize, const unsigned int& numLayers, const unsigned int layerSizes[], const unsigned int& numThreads, const bool& useThreads, const string& actFunc, const bool& useSoftMax){
    
    this->inputSize = inputSize;
    this->numLayers = numLayers;
    this->numThreads = numThreads;
    this->useThreads = useThreads;
    this->useSoftMax = useSoftMax;

    /* If too many threads cap it */
    if (numThreads > maxThreads) {
        this->numThreads = maxThreads;
    }
    layerSize = new unsigned int[numLayers];          /* allocate layer sizes for deep copy */
    for (unsigned int i = 0; i < numLayers; i++) {    /* for all layers */
        layerSize[i] = layerSizes[i];                 /* deep copy */
    }
    /* allocate fcnn structure */
    s = new double* [numLayers];
    b = new double* [numLayers];
    y = new double* [numLayers];
    errStore = new double* [numLayers];
    bN = new double* [numLayers];
    w = new double** [numLayers];
    wN = new double** [numLayers];
    for (unsigned int i = 0; i < numLayers; i++) {
        s[i] = new double[layerSize[i]];
        b[i] = new double[layerSize[i]];
        y[i] = new double[layerSize[i]];
        for (unsigned int j = 0; j < layerSize[i]; j++) { y[i][j] = 0.0; s[i][j] = 0.0; }
        errStore[i] = new double[layerSize[i]];
        bN[i] = new double[layerSize[i]];
        unsigned int lowerSize = inputSize;
        if (i > 0) {
            lowerSize = layerSize[i - 1];
        }
        w[i] = new double* [lowerSize];
        wN[i] = new double* [lowerSize];
        for (unsigned int j = 0; j < lowerSize; j++) {
            w[i][j] = new double[layerSize[i]];
            wN[i][j] = new double[layerSize[i]];
            for (unsigned int k = 0; k < layerSize[i]; k++) {
                w[i][j][k] = init(lowerSize);   /* weight initialization here */
                wN[i][j][k] = w[i][j][k];
            }
        }
        for (unsigned int j = 0; j < layerSize[i]; j++) {
            b[i][j] = init(lowerSize); /* bias initialization here */
            bN[i][j] = b[i][j];
        }
    }

    /* Last layer for ease of access */
    lastLayer = numLayers - 1;

    /* Error for predictions */
    error = new double[layerSize[lastLayer]];

    /* Activation functions */
    if (actFunc == "SIGMOID") { af = SIGMOID; }
    else if (actFunc == "TANH") { af = TANH; }
    else if (actFunc == "RELU") { af = RELU; }
    else if (actFunc == "LEAKY_RELU") { af = LEAKY_RELU; }
    else if (actFunc == "SOFTPLUS") { af = SOFTPLUS; }
}

/**************************************************************
*  deleteAll
---------------------------------------------------------------
*  Deallocate everything
**************************************************************/
void fcnn::deleteAll() {
    for (unsigned int i = 0; i < numLayers; i++) {
        unsigned int lowerSize = inputSize;
        if (i > 0) {
            lowerSize = layerSize[i - 1];
        }
        for (unsigned int j = 0; j < lowerSize; j++) {
            delete[] w[i][j];
            delete[] wN[i][j];
        }
        delete[] s[i];
        delete[] b[i];
        delete[] y[i];
        delete[] errStore[i];
        delete[] bN[i];
        delete[] w[i];
        delete[] wN[i];
    }
    delete[] layerSize;
    delete[] s;
    delete[] b;
    delete[] y;
    delete[] errStore;
    delete[] bN;
    delete[] w;
    delete[] wN;
    delete[] error;

    inputSize = 0;
    numLayers = 0;
    layerSize = nullptr;
    w = nullptr;
    b = nullptr;
    s = nullptr;
    y = nullptr;
    errStore = nullptr;
    wN = nullptr;
    bN = nullptr;
    error = nullptr;
}

/******************************************************************************
 * exportFcnn
-------------------------------------------------------------------------------
 * num layers
 * layer 1
 * layer 2
 * layer ...
 * input size
 * act function
 * softmax output
 * weights & biases by layer....
*******************************************************************************/
void fcnn::exportFcnn(const string& outFile) const {
    ofstream outputFile;
    outputFile.open(outFile);
    if (!outputFile.is_open()) {
        cout << "Unable to open export file" << endl;
        exit(1);
    }
    outputFile << numLayers << endl;
    for (unsigned int i = 0; i < numLayers; i++) {
        outputFile << layerSize[i] << endl;
    }
    outputFile << inputSize << endl;
    outputFile << activationFunctionToString(af) << endl;
    outputFile << (useSoftMax ? "TRUE" : "FALSE") << endl;

    unsigned int pSize = inputSize;
    for (unsigned int i = 0; i < numLayers; i++) {
        if (i > 0) {
            pSize = layerSize[i - 1];
        }
        for (unsigned int j = 0; j < layerSize[i]; j++) {
            for (unsigned int k = 0; k < pSize; k++) {
                outputFile << w[i][k][j] << endl;
            }
            outputFile << b[i][j] << endl;
        }
    }
    outputFile.close();
}

/******************************************************************************
 * importFcnn
-------------------------------------------------------------------------------
 * num layers
 * layer 1
 * layer 2
 * layer ...
 * input size
 * act function
 * softmax output
 * weights & biases by layer....
*******************************************************************************/
void fcnn::importFcnn(const string& inFileName) {
    deleteAll();
    ifstream inFile;
    inFile.open(inFileName);
    if (!inFile.is_open()) {
        cout << "Unable to open import file" << endl;
        exit(1);
    }
    inFile >> numLayers;
    unsigned int* layers = new unsigned int[numLayers];
    for (unsigned int i = 0; i < numLayers; i++) {
        inFile >> layers[i];
    }
    inFile >> inputSize;

    string afs;
    inFile >> afs;
    //af = activationFunctionFromString(afs);

    string s;
    inFile >> s;
    useSoftMax = (s == "TRUE");

    allocateFcnn(inputSize, numLayers, layers, numThreads, useThreads, afs, useSoftMax);

    unsigned int pSize = inputSize;
    for (unsigned int i = 0; i < numLayers; i++) {
        if (i > 0) {
            pSize = layerSize[i - 1];
        }
        for (unsigned int j = 0; j < layerSize[i]; j++) {
            for (unsigned int k = 0; k < pSize; k++) {
                inFile >> w[i][k][j];
            }
            inFile >> b[i][j];
        }
    }
    inFile.close();
}


/******************************************************************************
 * computeForwardNode
-------------------------------------------------------------------------------
 * computes a single node going forward
 * include activation function
*******************************************************************************/
void fcnn::computeForwardNode(const unsigned int& layer, const unsigned int& node, double* input, const unsigned int& inputSize) {
    s[layer][node] = 0;
    for (unsigned int j = 0; j < inputSize; j++) {
        s[layer][node] += w[layer][j][node] * input[j];
    }
    s[layer][node] += b[layer][node];
    y[layer][node] = computeActivationFunction(s[layer][node], layer, node);
}

/******************************************************************************
 * computeForwardNode_NoActivationFunction
-------------------------------------------------------------------------------
 * computes a single node going forward
 * no activation function
*******************************************************************************/
void fcnn::computeForwardNode_NoActivationFunction(const unsigned int& layer, const unsigned int& node, double* input, const unsigned int& inputSize) {
    s[layer][node] = 0;
    for (unsigned int j = 0; j < inputSize; j++) {
        s[layer][node] += w[layer][j][node] * input[j];
    }
    s[layer][node] += b[layer][node];
}

/******************************************************************************
 * computeForwardNode_SoftmaxLastLayer
-------------------------------------------------------------------------------
 * apply activation function to node
*******************************************************************************/
void fcnn::computeForwardNode_ActivationFunction(const unsigned int& layer, const unsigned int& node, double* input, const unsigned int& inputSize) {
    y[layer][node] = computeActivationFunction(s[layer][node], layer, node);
}

/******************************************************************************
 * computeBackwardNode FUNCTION
-------------------------------------------------------------------------------
 *
*******************************************************************************/
void fcnn::computeBackwardNode(const int& layer, const unsigned int& node, double* input, const unsigned int& inputSize) {
    if (layer >= 0) {
        double sumZ = 0;
        for (unsigned int i = 0; i < inputSize; i++) {
            wN[layer + 1][node][i] = w[layer + 1][node][i] - ((errStore[layer + 1][i]) * y[layer][node]);
            sumZ += errStore[layer + 1][i] * w[layer + 1][node][i];
        }
        errStore[layer][node] = sumZ * computeActivationFunctionDerivative(y[layer][node], s[layer][node], layer, node);
        bN[layer][node] = b[layer][node] - errStore[layer][node];
    }
    else {
        for (unsigned int i = 0; i < inputSize; i++) {
            wN[0][node][i] = w[0][node][i] - ((errStore[0][i]) * input[node]);
        }
    }
}

/******************************************************************************
 * PREDICTION FUNCTION
-------------------------------------------------------------------------------
 * This member function takes input for the neural network
 * and makes a prediction.
*******************************************************************************/
prediction fcnn::predict(double* input) {
    
    prediction p(layerSize[lastLayer]); /* Init prediction array */
    double* inp = nullptr;      /* dynamic pointer to switch the input array */
    unsigned int inpSize = 0;   /* dynamic variable to swtich the input size */

    /* Hidden layer computation */
    for (unsigned int i = 0; i < lastLayer; i++) {
        inp = (i > 0) ? y[i - 1] : input;
        inpSize = (i > 0) ? layerSize[i - 1] : inputSize;
        for (unsigned int j = 0; j < layerSize[i]; j++) {
            computeForwardNode(i, j, inp, inpSize);
        }
    }

    /* last layer */
    inp = (lastLayer > 0) ? y[lastLayer - 1] : input;
    inpSize = (lastLayer > 0) ? layerSize[lastLayer - 1] : inputSize;
    
    /* if no soft max */
    if (!useSoftMax) {
        for (unsigned int j = 0; j < layerSize[lastLayer]; j++) {
            computeForwardNode(lastLayer, j, inp, inpSize);
            p[j] = y[lastLayer][j];
        }
    }
    /* else, soft max */
    else {
        for (unsigned int j = 0; j < layerSize[lastLayer]; j++) {
            computeForwardNode_NoActivationFunction(lastLayer, j, inp, inpSize);
        }
        for (unsigned int j = 0; j < layerSize[lastLayer]; j++) {
            computeForwardNode_ActivationFunction(lastLayer, j, inp, inpSize);
            p[j] = y[lastLayer][j];
        }
    }
    
    /* all done! */
    return p;
}

/******************************************************************************
 * TRAIN FUNCTION
-------------------------------------------------------------------------------
*  This function trains the entire network on a data set
*******************************************************************************/
void fcnn::train(double** dataInput, double** dataOutput, const unsigned int& numData, const unsigned int& epochs = 50, const double& lr = 0.1, ostream* out = nullptr) {
    
    e = lr; /* set up the learning rate */

    /* If using threads, we gotta go to a different function */
    if (useThreads) {
        trainMaster(dataInput, dataOutput, numData, epochs, out);
    }
    /* Single thread operation */
    else {
        /* Timing var for epoch timing */
        std::chrono::duration<double> elapsed_seconds;  
        double totalTime = 0;
        averageEpochTime = 0;
        /* For all epochs */
        for (unsigned int epc = 0; epc < epochs; epc++) {
            auto start = std::chrono::system_clock::now();  /* Start the timer */
            /* for all data instances */
            for (unsigned int d = 0; d < numData; d++) {

                /* make a prediction */
                prediction p = predict(dataInput[d]);

                /* compute the error and begin back prop on last layer */
                for (unsigned int i = 0; i < layerSize[lastLayer]; i++) {
                    error[i] = p[i] - dataOutput[d][i];
                    errStore[lastLayer][i] = (e * error[i] * computeActivationFunctionDerivative(y[lastLayer][i], s[lastLayer][i], lastLayer, i));
                    bN[lastLayer][i] = b[lastLayer][i] - errStore[lastLayer][i];
                }

                /* back prop for all hidden layers */
                if (numLayers > 1) {
                    for (int h = numLayers - 2; h >= 0; h--) {
                        for (unsigned int j = 0; j < layerSize[h]; j++) {
                            computeBackwardNode(h, j, nullptr, layerSize[h + 1]);
                        }
                    }
                }

                /* back prop for input layer */
                for (unsigned int j = 0; j < inputSize; j++) {
                    computeBackwardNode(-1, j, dataInput[d], layerSize[0]);
                }
               
                /* Reassign new weights and biases */
                for (unsigned int i = 0; i < numLayers; i++) {
                    unsigned int lowerSize = (i > 0) ? layerSize[i - 1] : inputSize;
                    for (unsigned int k = 0; k < layerSize[i]; k++) {
                        for (unsigned int j = 0; j < lowerSize; j++) {
                            w[i][j][k] = wN[i][j][k];
                        }
                        b[i][k] = bN[i][k];
                    }
                }
            }

            auto end = std::chrono::system_clock::now();    /* Stop timer */
            elapsed_seconds = end - start;                  /* Compute time */
            averageEpochTime += elapsed_seconds.count();    /* Add in time */

            if (out != nullptr) {
                string s = "Finished epoch " + to_string(epc) + " in " + to_string(elapsed_seconds.count()) + " seconds.\n";
                *out << s;
            }
        }
        averageEpochTime /= (double)epochs;
    }
}

/******************************************************************************
 * printValidationResults
-------------------------------------------------------------------------------
 * function to print out some validation results
*******************************************************************************/
void fcnn::printValidationResults(const unsigned int& outputSize, const double& averagePredictionTime, const double& averageError, const unsigned int& totalCorrect, const unsigned int& numData, const double& accuracy, const unsigned int classCorrectCount[], const unsigned int classCount[], ostream& out){
    
    string results = string("-----------------------------------\n")
        + "|  VALIDATION RESULTS             |\n" +
        +"-----------------------------------\n"
        + "| Overall Average Prediction Time: " + to_string(averagePredictionTime) + "\n"
        + "| Overall Average Error:           " + to_string(averageError) + "\n"
        + "| Overall Accuracy:                " + to_string(totalCorrect) + "/" + to_string(numData) + "(" + to_string(accuracy) + "%)\n";

    for (unsigned int i = 0; i < outputSize; i++) {
        double classCorrectAcc = (double)classCorrectCount[i] / (double)classCount[i] * 100.0;

        results += "|  - Class " + to_string(i) + " Accuracy: " + to_string(classCorrectCount[i]) + "/" + to_string(classCount[i]) + " (" + to_string(classCorrectAcc) + "%)\n";
    }

    out << results;
}

/******************************************************************************
 * getPredictionStats
-------------------------------------------------------------------------------
 * function checks the current prediction and returns some stats
*******************************************************************************/
void fcnn::getPredictionStats(double* answer, unsigned int& answerClass, double& error, bool& correctGuess) {
    
    double* guess = y[lastLayer];   /* Network output */
    double sumError = 0.0;          /* Overall error of prediction */
    unsigned int answerIndex = 0;   /* The actual answer index */
    double answerValue = answer[0]; /* The actual answer value */
    unsigned int guessIndex = 0;    /* The guessed answer index */
    double guessValue = guess[0];   /* The guessed answer value */

    /* For all output nodes */
    for (unsigned int i = 0; i < layerSize[lastLayer]; i++) {
        /* RMSE - sum part */
        double indError = answer[i] - guess[i];
        sumError += (indError * indError);

        /* Determine the answer */
        if (answerValue < answer[i]) {
            answerValue = answer[i];
            answerIndex = i;
        }

        /* Determine the guess */
        if (guessValue < guess[i]) {
            guessValue = guess[i];
            guessIndex = i;
        }
    }

    error = sqrt(sumError);     /* RMSE - square root part */
    answerClass = answerIndex;  /* Return the actual class */
    correctGuess = (guessIndex == answerIndex); /* return whether or not there was a correct guess */
}

/******************************************************************************
 * validate FUNCTION
-------------------------------------------------------------------------------
 * function to validate a data set
*******************************************************************************/
void fcnn::validate(double** dataInput, double** dataOutput, const unsigned int& numData, ostream* out) {

    /* if using threads, we need to go to a different function */
    if (useThreads) {
        validateMaster(dataInput, dataOutput, numData, out);
        return;
    }

    /* if data set is not empty */
    if (numData > 0) {
        std::chrono::duration<double> elapsed_seconds;                  /* timing variable for average prediction time */
        double totalPredictionTime = 0;                                 /* timing variable for average prediction time */
        unsigned int totalCorrect = 0;                                  /* total correct guesses */
        double totalError = 0.0;                                        /* total error */
        unsigned int outputSize = layerSize[lastLayer];                 /* output size */
        unsigned int* classCount = new unsigned int[outputSize];        /* keeps track of the number of instances in a class */
        unsigned int* classCorrectCount = new unsigned int[outputSize]; /* keeps track of the number of correctly guessed instances in a class */

        /* Init */
        for (unsigned int i = 0; i < outputSize; i++) {
            classCount[i] = 0;
            classCorrectCount[i] = 0;
        }

        /* for all data instances */
        for (unsigned int i = 0; i < numData; i++) {
            
            auto start = std::chrono::system_clock::now();  /* begin time */
            prediction p = predict(dataInput[i]);           /* predict */
            auto end = std::chrono::system_clock::now();    /* stop time */
            elapsed_seconds = (end - start);                /* compute time */
            totalPredictionTime += elapsed_seconds.count(); /* add to total time */

            /* variables to get results of getPredictionStats */
            double guessError = 0.0;        
            unsigned int answerClass = 0;
            bool correctGuess = true;
            
            /* call function */
            getPredictionStats(dataOutput[i], answerClass, guessError, correctGuess);

            /* count 1 for encountered class  */
            classCount[answerClass] += 1;
            if (correctGuess) { /* if correct */
                classCorrectCount[answerClass] += 1; /* count 1 for correct guess */
                totalCorrect += 1;
            }
            totalError += guessError;

        }

        /* Compute stats */
        double averagePredictionTime = totalPredictionTime / (double)numData;
        double averageError = totalError / (double)numData;
        double accuracy = (double)totalCorrect / double(numData) * 100.0;

        if (out != nullptr) {
            /* Print them */
            printValidationResults(outputSize, averagePredictionTime, averageError, totalCorrect, numData, accuracy, classCorrectCount, classCount, *out);
        }

        /* save average prediction time */
        this->averagePredictionTime = averagePredictionTime;

        delete[] classCorrectCount;
        delete[] classCount;
    }
}

/******************************************************************************
############################################################################### 
############################################################################### 
    THIS IS THREAD STUFF - BIG COMMENT TO KEEP SEPERATE
############################################################################### 
############################################################################### 
*******************************************************************************/

/******************************************************************************
 * predict (threads)
-------------------------------------------------------------------------------
 * prediction function for threads. Only a certain range of threads is
 * handled
*******************************************************************************/
void fcnn::predict(double* input, unsigned int** threadRange, prediction& p, pthread_barrier_t* barrier) {

    double* inp = nullptr;
    unsigned int inpSize = 0;

    /* hidden layer computations */
    for (unsigned int i = 0; i < lastLayer; i++) {
        inp = (i > 0) ? y[i - 1] : input;
        inpSize = (i > 0) ? layerSize[i - 1] : inputSize;
        for (unsigned int j = threadRange[i][0]; j < threadRange[i][1]; j++) {
            computeForwardNode(i, j, inp, inpSize);
        }
        /* wait at the end of each layer */
        if (barrier != nullptr) { pthread_barrier_wait(barrier); }
    }

    inp = (lastLayer > 0) ? y[lastLayer - 1] : input;
    inpSize = (lastLayer > 0) ? layerSize[lastLayer - 1] : inputSize;

    /* if no soft max, plough on ahead */
    if (!useSoftMax) {
        for (unsigned int j = threadRange[lastLayer][0]; j < threadRange[lastLayer][1]; j++) {
            computeForwardNode(lastLayer, j, inp, inpSize);
            p[j] = y[lastLayer][j];
        }
    }
    /* if soft max */
    else {
        /* compute everything except for the activation function */
        for (unsigned int j = threadRange[lastLayer][0]; j < threadRange[lastLayer][1]; j++) {
            computeForwardNode_NoActivationFunction(lastLayer, j, inp, inpSize);
        }
        /* sync up with everyone */
        if (barrier != nullptr) { pthread_barrier_wait(barrier); }
        /* compute the activation function (which will be softmax) */
        for (unsigned int j = threadRange[lastLayer][0]; j < threadRange[lastLayer][1]; j++) {
            computeForwardNode_ActivationFunction(lastLayer, j, inp, inpSize);
            p[j] = y[lastLayer][j];
        }
    }
    /* sync up with everyone */
    /* Only necessary if num layers is 1 (i.e. no hidden layers) */
    if (numLayers == 1) {
        if (barrier != nullptr) { pthread_barrier_wait(barrier); }
    }
}

/******************************************************************************
 * trainMaster FUNCTION
-------------------------------------------------------------------------------
 * train function for threads. The master sets them up and sends them out
*******************************************************************************/
void fcnn::trainMaster(double** dataInput, double** dataOuput, const unsigned int& numData, const unsigned int& epochs, ostream* out = nullptr) {
    
    /* If threads wider than output, we should update. */
    if (numThreads > layerSize[lastLayer]) {
        numThreads = layerSize[lastLayer];
    }

    /* Allocate threads */
    pthread_t* threads = new pthread_t[numThreads];

    /* Variable for output */
    unsigned int threadResult = 0;

    /* Barrier used to keep threads from moving on without one another */
    pthread_barrier_t barrierSet[2];
    pthread_barrier_init(&barrierSet[0], NULL, numThreads);
    pthread_barrier_init(&barrierSet[1], NULL, numThreads + 1);

    /* Thread Arguments */
    threadArguments* ta = new threadArguments[numThreads];
    helperNode* th = new helperNode[numThreads];
    prediction *p = new prediction(layerSize[lastLayer]);

    /* Timing var for epoch timing */
    std::chrono::duration<double> elapsed_seconds;
    double totalTime = 0;
    averageEpochTime = 0;

    auto start = std::chrono::system_clock::now();  /* Start the timer */
   
    /* Load up threads and send them off */
    for (unsigned int i = 0; i < numThreads; i++) {
        setThreadArguments(ta[i], i, barrierSet, dataInput, dataOuput, numData, epochs, p);
        th[i].ftc = TRAIN_THREAD;
        th[i].objectReference = this;
        th[i].arguments = (void*)(ta + i);
        threadResult = pthread_create((threads + i), NULL, callMemberFunctionForThread, (void*)(th + i));
    }

    /* Meet up at each epoch */
    for (unsigned int i = 0; i < epochs; i++) {
        pthread_barrier_wait(&barrierSet[1]);
        auto end = std::chrono::system_clock::now();    /* Stop timer */
        elapsed_seconds = end - start;                  /* Compute time */
        averageEpochTime += elapsed_seconds.count();    /* Add in time */
        if (out != nullptr) {
            string s = "Finished epoch " + to_string(i) + " in " + to_string(elapsed_seconds.count()) + " seconds.\n";
            *out << s;
        }
        start = end;  /* Start the timer */
    }

    /* Bring everyone back together */
    for (unsigned int i = 0; i < numThreads; i++) {
        threadResult = pthread_join(*(threads + i), NULL);
    }

    averageEpochTime /= (double)epochs;

    delete[] ta;
    delete[] threads;
    delete[] th;
}

/******************************************************************************
 * trainIndividual (threads)
-------------------------------------------------------------------------------
 * train function for threads. Only a certain range of threads is
 * handled
*******************************************************************************/
void fcnn::trainIndividual(void* args) {
    threadArguments* ta = (threadArguments*)args;
    unsigned int threadId = ta->threadId;
    double** dataInput = ta->dataInput;
    double** dataOutput = ta->dataOutput;
    unsigned int numData = ta->numData;
    unsigned int epochs = ta->epochs;
    pthread_barrier_t *barrier = ta->barrier;
    prediction* p = ta->p;
    unsigned int** threadRange = new unsigned int*[numLayers + 1];
    threadRange[numLayers] = new unsigned int[2]; /* Start(1)& End(2)*/

    computeThreadRange(threadRange[numLayers][0], threadRange[numLayers][1], threadId, numThreads, inputSize);

    for (unsigned int i = 0; i < numLayers; i++) {
        threadRange[i] = new unsigned int[2]; /* Start(1)& End(2)*/
        computeThreadRange(threadRange[i][0], threadRange[i][1], threadId, numThreads, layerSize[i]);
    }

    for (unsigned int epc = 0; epc < epochs; epc++) {
        for (unsigned int d = 0; d < numData; d++) {

            /* Forward Computation (Prediction) */
            predict(dataInput[d], threadRange, *p, &barrier[0]);

            /* Set up back prop using the prediction */
            for (unsigned int i = threadRange[lastLayer][0]; i < threadRange[lastLayer][1]; i++) {
                error[i] = p->operator[](i) - dataOutput[d][i];
                errStore[lastLayer][i] = (e * error[i] * computeActivationFunctionDerivative(y[lastLayer][i], s[lastLayer][i], lastLayer, i));
                bN[lastLayer][i] = b[lastLayer][i] - errStore[lastLayer][i];
            }
            pthread_barrier_wait(&barrier[0]);

            /* Back prop for all hidden layers */
            if (numLayers > 1) {
                for (int h = numLayers - 2; h >= 0; h--) {
                    for (unsigned int j = threadRange[h][0]; j < threadRange[h][1]; j++) {
                        computeBackwardNode(h, j, nullptr, layerSize[h + 1]);
                    }
                    pthread_barrier_wait(&barrier[0]);
                }
            }

            unsigned int size = layerSize[0];
            /* For input layer */
            for (unsigned int j = threadRange[numLayers][0]; j < threadRange[numLayers][1]; j++) {
                for (unsigned int i = 0; i < size; i++) {
                    wN[0][j][i] = w[0][j][i] - ((errStore[0][i]) * dataInput[d][j]);
                }
            }
            pthread_barrier_wait(&barrier[0]);

            /* Reassignment */
            for (unsigned int i = 0; i < numLayers; i++) {
                /* Determine the size of the previous layer */
                unsigned int lowerSize = inputSize;
                if (i > 0) {
                    lowerSize = layerSize[i - 1];
                }
                /* Reassign weights */
                for (unsigned int k = threadRange[i][0]; k < threadRange[i][1]; k++) {
                    for (unsigned int j = 0; j < lowerSize; j++) { 
                        w[i][j][k] = wN[i][j][k];
                    }
                    b[i][k] = bN[i][k];
                }
            } 
            pthread_barrier_wait(&barrier[0]);
        }
        /* sync up with the master */
        pthread_barrier_wait(&barrier[1]);
    }
}

/******************************************************************************
 * validateMaster (threads)
-------------------------------------------------------------------------------
 * validate function for threads. this is the master controller
*******************************************************************************/
void fcnn::validateMaster(double** dataInput, double** dataOutput, const unsigned int& numData, ostream* out) {
    
    string results = "";

    if (numData > 0) {

        std::chrono::duration<double> elapsed_seconds;
        unsigned int totalTime = 0;
        double totalPredictionTime = 0;

        unsigned int totalCorrect = 0;
        double totalError = 0.0;

        unsigned int outputSize = layerSize[lastLayer];
        unsigned int* classCount = new unsigned int[outputSize];
        unsigned int* classCorrectCount = new unsigned int[outputSize];

        for (unsigned int i = 0; i < outputSize; i++) {
            classCount[i] = 0;
            classCorrectCount[i] = 0;
        }

        /* If threads wider than output, we should update. */
        if (numThreads > layerSize[lastLayer]) {
            numThreads = layerSize[lastLayer];
        }
        /* Allocate threads */
        pthread_t* threads = new pthread_t[numThreads];
        /* Variable for output */
        unsigned int threadResult = 0;
        /* Barrier used to keep threads from moving on without one another */
        pthread_barrier_t barrierSet[2];
        pthread_barrier_init(&barrierSet[0], NULL, numThreads);
        pthread_barrier_init(&barrierSet[1], NULL, numThreads + 1);
        /* Thread Arguments */
        threadArguments* ta = new threadArguments[numThreads];
        helperNode* th = new helperNode[numThreads];
        prediction *p = new prediction(layerSize[lastLayer]);
        /* Load up threads and send them off */
        for (unsigned int i = 0; i < numThreads; i++) {
            setThreadArguments(ta[i], i, barrierSet, dataInput, dataOutput, numData, 0, p);
            th[i].ftc = VALIDATE_THREAD;
            th[i].objectReference = this;
            th[i].arguments = (void*)(ta + i);
            threadResult = pthread_create((threads + i), NULL, callMemberFunctionForThread, (void*)(th + i));
        }

        double guessError = 0.0;
        unsigned int answerClass = 0;
        bool correctGuess = true;
        auto start = std::chrono::system_clock::now();

        pthread_barrier_wait(&barrierSet[1]);   /* Prediction started barrier */

        for (unsigned int i = 0; i < numData - 1; i++) {
            pthread_barrier_wait(&barrierSet[1]);   /* Prediction finished barrier */
            auto end = std::chrono::system_clock::now();
            elapsed_seconds = (end - start);
            totalPredictionTime += elapsed_seconds.count();

            getPredictionStats(dataOutput[i], answerClass, guessError, correctGuess);

            start = std::chrono::system_clock::now();
            pthread_barrier_wait(&barrierSet[1]);   /* Prediction started barrier */

            classCount[answerClass] += 1;
            if (correctGuess) {
                classCorrectCount[answerClass] += 1;
                totalCorrect += 1;
            }
            totalError += guessError;
        }

        pthread_barrier_wait(&barrierSet[1]);   /* Prediction finished barrier */
        auto end = std::chrono::system_clock::now();
        elapsed_seconds = (end - start);
        totalPredictionTime += elapsed_seconds.count();

        getPredictionStats(dataOutput[numData - 1], answerClass, guessError, correctGuess);

        classCount[answerClass] += 1;
        if (correctGuess) {
            classCorrectCount[answerClass] += 1;
            totalCorrect += 1;
        }
        totalError += guessError;

        for (unsigned int i = 0; i < numThreads; i++) {
            threadResult = pthread_join(*(threads + i), NULL);
        }

        delete[] ta;
        delete[] threads; 
        delete[] th;

        double averagePredictionTime = totalPredictionTime / (double)numData;
        double averageError = totalError / (double)numData;
        double accuracy = (double)totalCorrect / double(numData) * 100.0;

        if (out != nullptr) {
            printValidationResults(outputSize, averagePredictionTime, averageError, totalCorrect, numData, accuracy, classCorrectCount, classCount, *out);
        }

        this->averagePredictionTime = averagePredictionTime;

        delete[] classCorrectCount;
        delete[] classCount;
    }
}


/******************************************************************************
 * validateIndividual (threads)
-------------------------------------------------------------------------------
 * validate function for threads. this is for indivdual threads where
 * only a certain range of nodes is handled 
*******************************************************************************/
void fcnn::validateIndividual(void* args) {
    threadArguments* ta = (threadArguments*)args;
    unsigned int threadId = ta->threadId;
    double** dataInput = ta->dataInput;
    double** dataOutput = ta->dataOutput;
    unsigned int numData = ta->numData;
    unsigned int epochs = ta->epochs;
    pthread_barrier_t* barrier = ta->barrier;
    prediction* p = ta->p;
    unsigned int** threadRange = new unsigned int* [numLayers + 1];
    threadRange[numLayers] = new unsigned int[2]; /* Start (1) & End (2) */

    computeThreadRange(threadRange[numLayers][0], threadRange[numLayers][1], threadId, numThreads, inputSize);

    for (unsigned int i = 0; i < numLayers; i++) {
        threadRange[i] = new unsigned int[2]; /* Start (1) & End (2) */
        computeThreadRange(threadRange[i][0], threadRange[i][1], threadId, numThreads, layerSize[i]);
    }

    for (unsigned int d = 0; d < numData; d++) {
        pthread_barrier_wait(&barrier[1]);
        /* Forward Computation (Prediction) */
        predict(dataInput[d], threadRange, *p, &barrier[0]);
        /* sync with master */
        pthread_barrier_wait(&barrier[1]);
    }
}

/******************************************************************************
 * determineMostEfficientModel 
-------------------------------------------------------------------------------
 * 
*******************************************************************************/
void fcnn::determineMostEfficientModel(const unsigned int& inputSize, const unsigned int& numLayers, const unsigned int layerSizes[], const unsigned int& maxThreads, const string& actFunc, const bool& useSoftMax, const bool& printProgress) {
    
    /* Check threads to make sure feasible */
    unsigned int newMaxThreads = maxThreads;
    unsigned int outputSize = layerSizes[numLayers - 1];
    if (maxThreads > outputSize) {
        newMaxThreads = layerSizes[numLayers - 1];
        if (printProgress) {
            cout << "Number of threads greater than the output size of the network is currently not supported." << endl;
            cout << "Reducing thread limit to " << newMaxThreads << endl;
        }
    }

    /* Set up some mock data - doesnt need to have meaning, just testing speed */
    const unsigned int testingInstances = 1000;
    double** input = new double* [testingInstances];
    double** output = new double* [testingInstances];
    for (unsigned int i = 0; i < testingInstances; i++) {
        input[i] = new double[inputSize];
        output[i] = new double[outputSize];
        for (unsigned int j = 0; j < inputSize; j++) {
            input[i][j] = rand();
        }
        for (unsigned int j = 0; j < outputSize; j++) {
            output[i][j] = rand();
        }
    }

    /* Tracking vars */
    bool useThreads = false;
    double* avgEpochTime = new double[maxThreads + 1];
    double* avgPredictTime = new double[maxThreads + 1];

    /* fcnn training params */
    const unsigned int numEpochs = 10;
    const double learningRate = 0.1;

    /* default Fcnn with no threads */
    {
        if (printProgress) {
            cout << "Testing default fcnn efficiency..." << endl;
        }

        fcnn defaultFcnn(inputSize, numLayers, layerSizes, 0, useThreads, actFunc, useSoftMax);
        defaultFcnn.train(input, output, testingInstances, numEpochs, learningRate, nullptr);
        defaultFcnn.validate(input, output, testingInstances, nullptr);
        avgEpochTime[0] = defaultFcnn.getAverageEpochTime();
        avgPredictTime[0] = defaultFcnn.getAveragePredictionTime();
    }

    useThreads = true;
    /* threads */
    for (unsigned int nt = 1; nt <= newMaxThreads; nt++) {
        if (printProgress) {
            cout << "Testing threaded fcnn efficiency... " << (nt) << endl;
        }
        fcnn tFcnn(inputSize, numLayers, layerSizes, nt, useThreads, actFunc, useSoftMax);
        tFcnn.train(input, output, testingInstances, numEpochs, learningRate, nullptr);
        tFcnn.validate(input, output, testingInstances, nullptr);
        avgEpochTime[nt] = tFcnn.getAverageEpochTime();
        avgPredictTime[nt] = tFcnn.getAveragePredictionTime();
    }

    /* print out everyone */
    cout << "[Default FCNN] - Epoch Time : " << avgEpochTime[0] << " | Prediction Time : " << avgPredictTime[0] << endl;
    for (unsigned int nt = 1; nt <= newMaxThreads; nt++) {
        cout << "[Threaded FCNN(" << nt << ")] - Epoch Time : " << avgEpochTime[nt] << " | Prediction Time : " << avgPredictTime[nt] << endl;
    }

    for (unsigned int i = 0; i < testingInstances; i++) {
        delete[] input[i];
        delete[] output[i];
    }
    delete[] input;
    delete[] output;
    delete[] avgEpochTime;
    delete[] avgPredictTime;
}
