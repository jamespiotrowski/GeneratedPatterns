#include <fstream>
#include "Pattern.h"

using namespace std;

/************************************************************
#############################################################
#   PatternGenerator Class
#############################################################
#
#   Class used to generate a WHOLE data set 
************************************************************/
class PatternGenerator {
private:
	Array<PatternType> patternList; /* classes to generate */
	int totalImages = 0;			/* total images in the data set */

	int unitPatternWidth = 50;		/* size of the units */
	int unitPatternHeight = 50;		/* size of the units */

	int patternWidth = 50;			/* size of the images */
	int patternHeight = 50;			/* size of the images */

	double minScale = 0;			/* used to control how small a unit can be */
	double scaleStep = 0;			/* used to change the scale to generate unique units */
	double maxScale = 0;			/* used to control how large a unit can be */

	int allowedNumberOfScales = 0;	/* used to limit the number of scales allowed to be used for units - I think this is currently unused */

	const int minPixelsAllowed = 30;		/* generating images using math gets tough if the image is too small */
	const int maximumPossibleScales = 8;	/* max possible ways a unit MIGHT be scaled (like.. an octogon) */

	bool clipping = false;			/* can the units be clipped off the edge */
	bool center = true;				/* are the images centered */

	double percentageOfPatternsToKeep = 0.01;	/* percentage of unit combinations to actually generate - used to limit compute */

	string outputDirectory = "";	/* where am I saving this data */
	ofstream dataFile;				/* and here is the file "object" to save to */

	Array<Array<UnitPattern*>> unitPatterns;	/* the set of all unit patterns that can be used to generate patterns */
	Array<Array<int>> unitPatternIndexes;		/* set to assign IDs to the above patterns - used for determining a combination */

	/* cleaner function to return allocated memory */
	void deallocateAllUnitPattens() {
		for (int i = 0; i < unitPatterns.getSize(); i++) {
			for (int j = 0; j < unitPatterns[i].getSize(); j++) {
				if (unitPatterns[i][j] != nullptr) {
					delete unitPatterns[i][j];
					unitPatterns[i][j] = nullptr;
				}
			}
		}
		unitPatterns.reset();
		unitPatternIndexes.reset();
	}
	
	/* function used to standardize all the class members so that things work nicely */
	void cleanAndStandardizeMembers(bool smartScaleDetection, bool enforceBorders) {

		/* The unit pattern must always be smaller than the big pattern */
		if (unitPatternWidth > patternWidth) { patternWidth = unitPatternWidth; }
		if (unitPatternHeight > patternHeight) { patternHeight = unitPatternHeight; }

		/* No dupes */
		patternList.removeDuplicates();

		/* Scales */
		double smallestDimension = (unitPatternWidth < unitPatternHeight) ? unitPatternWidth : unitPatternHeight;
		if (smartScaleDetection) {
			scaleStep = 1.0 / smallestDimension;
			minScale = 0.2;
			maxScale = 0.9;
		}

		/*
			Scale is between 0 and 1
			When the scale changes, there needs to be a visible change in the image

			for example:
				- A scale step of 0.01 is acceptable for 100 pixels because ((0.01) * (100)) = 1 pixel
				- A scale step of 0.01 is NOT acceptable for 50 pixels because ((0.01) * (50)) = 0 pixels

			So, we take 1 / (minimum dimension) and if the scale step is smaller than that, we correct. 
		*/
		if ((1.0 / smallestDimension) > scaleStep) {
			scaleStep = 1.0 / smallestDimension;
		}

		/* we should not make any shapes where the lesser dimension of the shape will be just a few pixels */
		/* Anything dimension that is less than 30 pixels might be too "grainy" to recognize */
		if ((minScale * smallestDimension) < minPixelsAllowed) {
			minScale = (double)minPixelsAllowed / smallestDimension;
		}

		if (enforceBorders) {
			/* we should allow at least 10 pixels to border the picture */
			int allowedBorder = (minPixelsAllowed / 2);
			double border = (smallestDimension)-(smallestDimension * maxScale);
			if (border < allowedBorder) {
				maxScale = (smallestDimension - allowedBorder) / smallestDimension;
			}
		}

	}

	/* helper function to easily get a unit pattern */
	UnitPattern* GetUnitPattern(PatternType p, double* scaleSet) {
		switch (p) {
		case(SQUARE): { return new SquarePattern(unitPatternHeight, unitPatternWidth, scaleSet); }
		case(RECTANGLE): { return new RectanglePattern(unitPatternHeight, unitPatternWidth, scaleSet); }
		case(DIAMOND): { return new DiamondPattern(unitPatternHeight, unitPatternWidth, scaleSet); }
		case(TRIANGLE): { return new TrianglePattern(unitPatternHeight, unitPatternWidth, scaleSet); }
		case(HORIZONTAL_STRIPES): { return new HorizontalStripePattern(unitPatternHeight, unitPatternWidth, scaleSet); }
		case(VERTICAL_STRIPES): { return new VerticalStripePattern(unitPatternHeight, unitPatternWidth, scaleSet); }
		case(CIRCLE): { return new CirclePattern(unitPatternHeight, unitPatternWidth, scaleSet); }
		case(HEXAGON): { return new HexagonPattern(unitPatternHeight, unitPatternWidth, scaleSet); }
		case(PENTAGON): { return new PentagonPattern(unitPatternHeight, unitPatternWidth, scaleSet); }
		case(HEPTAGON): { return new HeptagonPattern(unitPatternHeight, unitPatternWidth, scaleSet); }
		case(STAR): { return new StarPattern(unitPatternHeight, unitPatternWidth, scaleSet); }
		case(OCTAGON): { return new OctogonPattern(unitPatternHeight, unitPatternWidth, scaleSet); }
		case(TRAPEZOID): { return new TrapezoidPattern(unitPatternHeight, unitPatternWidth, scaleSet); }
		case(HEART): { return new HeartPattern(unitPatternHeight, unitPatternWidth, scaleSet); }
		case(CROSS): { return new CrossPattern(unitPatternHeight, unitPatternWidth, scaleSet); }
		case(CRESCENT): { return new CrescentPattern(unitPatternHeight, unitPatternWidth, scaleSet); }
		case(SPIKE): { return new SpikePattern(unitPatternHeight, unitPatternWidth, scaleSet); }
		case(ARROW): { return new ArrowPattern(unitPatternHeight, unitPatternWidth, scaleSet); }
		case(TILDE): { return new TildePattern(unitPatternHeight, unitPatternWidth, scaleSet); }
		case(ZIGZAG): { return new ZigzagPattern(unitPatternHeight, unitPatternWidth, scaleSet); }
		case(CANE): { return new CanePattern(unitPatternHeight, unitPatternWidth, scaleSet); }
		case(CAT): { return new CatPattern(unitPatternHeight, unitPatternWidth, scaleSet); }
		}
		return nullptr;
	}

	/* function to determine how many unit patterns will fit in the pattern - needed to determine how large a combination is */
	int GetNumberOfUnitPatternsPerPattern(const int &verticalOffset, const int &horizontalOffset) const {

		double tUnitHeight = unitPatternHeight + verticalOffset;
		double tUnitWidth = unitPatternWidth + horizontalOffset;

		double tHeightUnits = (double)patternHeight / tUnitHeight;
		double tWidthUnits = (double)patternWidth / tUnitWidth;

		if (clipping) {
			tHeightUnits = ceil(tHeightUnits);
			tWidthUnits = ceil(tWidthUnits);
		}
		else {
			tHeightUnits = floor(tHeightUnits);
			tWidthUnits = floor(tWidthUnits);
		}
		
		return (int)(tHeightUnits * tWidthUnits);
	}

	/* function to generate pattern combinations to use when generating images */
	Array<Array<int>> GetPatternCombinations(int pattern, const int& verticalOffset, const int& horizontalOffset) const {
		int totalUnitsPerPattern = GetNumberOfUnitPatternsPerPattern(verticalOffset, horizontalOffset);
		return SomeCombinations(unitPatternIndexes.at(pattern), totalUnitsPerPattern, percentageOfPatternsToKeep);
	}

	/* function to generate pattern combinations to use when generating images - this one has the percentage as a parameter */
	Array<Array<int>> GetPatternCombinations(int pattern, const int& verticalOffset, const int& horizontalOffset, const double& perc) const {
		int totalUnitsPerPattern = GetNumberOfUnitPatternsPerPattern(verticalOffset, horizontalOffset);
		return SomeCombinations(unitPatternIndexes.at(pattern), totalUnitsPerPattern, perc);
	}

	/* function to generate an image based on a combination */
	Pattern GetPattern(int pattern, const int& verticalOffset, const int& horizontalOffset, const Array<int>& combination) const {
		return Pattern(patternList.at(pattern), patternHeight, patternWidth, verticalOffset, horizontalOffset, clipping, center, unitPatterns.at(pattern), combination);
	}

	/*******************************************
	* GenerateAllUnitPatterns
	********************************************
	* function to generate all possible 
	* unit patterns to use when generating
	* images
	*******************************************/
	void GenerateAllUnitPatterns() {

		/* Reset the object */
		deallocateAllUnitPattens();

		/* Get all scales to iterate through */
		Array<double> scales;
		for (double s = minScale; s < maxScale; s += scaleStep) {
			scales.push(s);
		}

		/* honestly, not sure why this is here... gonna assume I was testing something */
		/* scales.at(0) = 0.1; */

		/* Set up an array to store all unit patterns */
		for (int p = 0; p < patternList.getSize(); p++) {
			Array<UnitPattern*> unitPatternSet;
			unitPatterns.push(unitPatternSet);
		}

		/* Make an array to store scales - will need enough space for maximum possible scales
			- At the time of 2023-09-18, the max scales required for a unit pattern is 8 (octogon) 
		*/
		double* scaleForPattern = new double[maximumPossibleScales];

		/* For all scales */
		for (int i = 0; i < scales.getSize(); i++) {

			/* default all scales to be the same
				- This code my change for future iterations of the project
			*/
			for (int j = 0; j < maximumPossibleScales; j++) {
				scaleForPattern[j] = scales[i];
			}

			/* Make a pattern for all pattern types that will be unique with any scale */
			for (int p = 0; p < patternList.getSize(); p++) {
				if (!SpecialProcessing(patternList[p])) {
					unitPatterns[p].push(GetUnitPattern(patternList[p], scaleForPattern));
				}
			}

			/* Make a pattern for all pattern types that will NOT be unique with any scale */
			/* This solution is obviously very hardcoded but will be necesarry for now */
			int firstScale = 0, secondScale = 1, thirdScale = 2;
			for (int p = 0; p < patternList.getSize(); p++) {
				if (SpecialProcessing(patternList[p])) {
					switch (patternList[p]) {
					case(RECTANGLE): {
						if (maximumPossibleScales > secondScale) {
							scaleForPattern[secondScale] = 0.5 * scaleForPattern[firstScale];
						}
						break;
					}
					case(TRAPEZOID): {
						if (maximumPossibleScales > thirdScale) {
							scaleForPattern[secondScale] = 0.5 * scaleForPattern[firstScale];
							scaleForPattern[thirdScale] = 0.5 * scaleForPattern[firstScale];
						}
						break;
					}
					case(CRESCENT): {
						if (maximumPossibleScales > secondScale) {
							scaleForPattern[secondScale] = 0.55;
						}
						break;
					}
					default: { break; }
					}
					unitPatterns[p].push(GetUnitPattern(patternList[p], scaleForPattern));
				}
			}
		}

		for (int i = 0; i < unitPatterns.getSize(); i++) {
			unitPatternIndexes.push(Array<int>());
			for (int j = 0; j < unitPatterns[i].getSize(); j++) {
				unitPatternIndexes[i].push(j);
			}
		}

		delete[] scaleForPattern;
	}

public:
	/* Parameter constructor */
	PatternGenerator(
		Array<PatternType> patternList
		, int unitPatternWidth
		, int unitPatternHeight
		, int patternWidth
		, int patternHeight
		, double minScale
		, double scaleStep
		, double maxScale
		, int allowedNumberOfScales
		, string outputDirectory
		, bool clipping = false
		, bool center = true
		, double percentageOfPatternsToKeep = 0.01
		, bool smartScaleDetection = false
		, bool enforceBorderRequirements = false
	) : patternList(patternList), unitPatternWidth(unitPatternWidth), unitPatternHeight(unitPatternHeight)
		, patternWidth(patternWidth), patternHeight(patternHeight), minScale(minScale), scaleStep(scaleStep)
		, maxScale(maxScale), outputDirectory(outputDirectory), allowedNumberOfScales(allowedNumberOfScales)
		, clipping(clipping), center(center), percentageOfPatternsToKeep(percentageOfPatternsToKeep)
	{
		cleanAndStandardizeMembers(smartScaleDetection, enforceBorderRequirements);
		allowedNumberOfScales = 1; /* Not going to incorporate multiple scales just yet. */
		dataFile.open(outputDirectory + "data.csv");
		dataFile.close();
		GenerateAllUnitPatterns();
	}

	/* destructor */
	~PatternGenerator() {
		deallocateAllUnitPattens();
	}

	/* function to generate a data set */
	void MakePatterns(bool makeBMPs, bool saveToFile) {
		
		if (saveToFile) {
			dataFile.open(outputDirectory + "data.csv", ios_base::app);
		}

		unsigned int verticalOffset = 0;
		unsigned int horizontalOffset = 0;

		/* determine the number of vertical offsets to use */
		double pd = patternHeight;
		double upd = unitPatternHeight;
		int totalFit = patternHeight / unitPatternHeight;
		unsigned int verticalSteps = (totalFit > 1) ? ceil((((pd / 2.0) + 1.0) - upd)) : 0;

		/* determine the number of horizontal offsets to use */
		pd = patternWidth;
		upd = unitPatternWidth;
		totalFit = patternWidth / unitPatternWidth;
		unsigned int horizontalSteps = (totalFit > 1) ? ceil((((pd / 2.0) + 1.0) - upd)) : 0;

		int tImgs = 0; /* total images */
		/* for all vertical offset */
		for(verticalOffset = 0; verticalOffset <= verticalSteps; verticalOffset += 1){
			/* for all horizontal offset */
			for (horizontalOffset = 0; horizontalOffset <= horizontalSteps; horizontalOffset += 1) {
				/* for each pattern */
				for (int currentPattern = 0; currentPattern < patternList.getSize(); currentPattern++) {
					/* Generate the possible combinations */
					Array<Array<int>> allCombinations = GetPatternCombinations(currentPattern, verticalOffset, horizontalOffset);
					string outputFile;
					string currentPatternString = GetNameForPattern(patternList.at(currentPattern));
					/* For all combinations */
					for (int i = 0; i < allCombinations.getSize(); i++) {
						/* Generate a pattern */
						Pattern p = GetPattern(currentPattern, verticalOffset, horizontalOffset, allCombinations[i]);
						if (makeBMPs) {
							outputFile = outputDirectory + currentPatternString + "_" + to_string(tImgs) + ".bmp";
							p.SavePatternToBmp(outputFile);
						}
						if (saveToFile) {
							dataFile << p.GetRawDataAsString() << endl;
						}
						tImgs += 1;
					}
				}
			}
		}

		if (saveToFile) {
			dataFile.close();
		}
	}

	/* helper function to generate sample images for the paper
		- Same as above except instead of generating all combinations, it just
		picks a random sample to generate
	*/
	void MakePatternSamples(bool makeBMPs, bool saveToFile) {

		if (saveToFile) {
			dataFile.open(outputDirectory + "data.csv", ios_base::app);
		}

		unsigned int verticalOffset = 0;
		unsigned int horizontalOffset = 0;

		double pd = patternHeight;
		double upd = unitPatternHeight;
		int totalFit = patternHeight / unitPatternHeight;
		unsigned int verticalSteps = (totalFit > 1) ? ceil((((pd / 2.0) + 1.0) - upd)) : 0;

		pd = patternWidth;
		upd = unitPatternWidth;
		totalFit = patternWidth / unitPatternWidth;
		unsigned int horizontalSteps = (totalFit > 1) ? ceil((((pd / 2.0) + 1.0) - upd)) : 0;

		cout << verticalSteps << " , " << horizontalSteps << endl;

		int tImgs = 0;
		for (verticalOffset = 0; verticalOffset <= verticalSteps; verticalOffset += 1) {
			for (horizontalOffset = 0; horizontalOffset <= horizontalSteps; horizontalOffset += 1) {
				for (int currentPattern = 0; currentPattern < patternList.getSize(); currentPattern++) {
					int totalUnitsPerPattern = GetNumberOfUnitPatternsPerPattern(verticalOffset, horizontalOffset);
					double percToKeep = 1.0;
					if (totalUnitsPerPattern >= 9) {
						percToKeep = 0.0001;
					}
					else if (totalUnitsPerPattern >= 6) {
						percToKeep = 0.01;
					}
					Array<Array<int>> allCombinations = GetPatternCombinations(currentPattern, verticalOffset, horizontalOffset, percToKeep);
					string outputFile;
					string currentPatternString = GetNameForPattern(patternList.at(currentPattern));
					if (allCombinations.getSize() > 0) {
						int randomSample = rand() % allCombinations.getSize();
						Pattern p = GetPattern(currentPattern, verticalOffset, horizontalOffset, allCombinations[randomSample]);
						if (makeBMPs) {
							outputFile = outputDirectory + currentPatternString + "_" + to_string(tImgs) + ".bmp";
							p.SavePatternToBmp(outputFile);
						}
						if (saveToFile) {
							dataFile << p.GetRawDataAsString() << endl;
						}
						tImgs += 1;
					}
				}
			}
		}

		if (saveToFile) {
			dataFile.close();
		}
	}

	/* helper function to generate all unit pattern images for viewing */
	void SaveUnitPatternPNGs() {
		string outputFile;
		int tImgs = 0;
		for (int p = 0; p < patternList.getSize(); p++) {
			string currentPattern = GetNameForPattern(patternList[p]);
			for (int i = 0; i < unitPatterns[p].getSize(); i++) {
				UnitPattern* up = unitPatterns[p][i];
				outputFile = outputDirectory + currentPattern + "_" + to_string(tImgs) + ".bmp";
				Pattern p = Pattern(up->GetPatternType(), unitPatternHeight, unitPatternWidth, 0, 0, clipping, center, up);
				p.SavePatternToBmp(outputFile);
				tImgs += 1;
			}
		}
	}
};

int main()
{
	Array<PatternType> patternList;
	patternList.push(SQUARE);
	patternList.push(RECTANGLE);
	patternList.push(TRAPEZOID);
	patternList.push(TRIANGLE);
	patternList.push(PENTAGON);
	patternList.push(STAR);
	patternList.push(CIRCLE);
	patternList.push(DIAMOND);
	patternList.push(HEXAGON);
	patternList.push(OCTAGON);
	patternList.push(HEPTAGON);
	patternList.push(HEART);
	patternList.push(CROSS);
	patternList.push(CRESCENT);
	patternList.push(SPIKE);
	patternList.push(ARROW);
	patternList.push(TILDE);
	patternList.push(ZIGZAG);
	patternList.push(CANE);
	patternList.push(CAT);

	PatternGenerator pg(
		patternList
		, 50	// Unit Pattern height
		, 50	// Unit Pattern width
		, 465	// Total Pattern height
		, 65	// Total Pattern width
		, 0.2	// Starting scale
		, 0.3	// Scale step
		, 0.97	// Ending scale
		, 1		// Scales allowed
		, "C:\\Users\\james\\Code\\CPP\\MachineLearningCPP\\MachineLearningCPP\\PatternGenerator\\Output\\" // Output Directory
		, false	// clipping - making this default for now
		, true	// center
		, 1.0	// percentage of combinations to keep
		, false	// smart scale detection
		, false	// enforce borders
	);

	pg.MakePatterns(false, true);

	// pg.MakePatternSamples(true, true);

	/*
	int uph = 50;	// Unit Pattern height
	int upw = 50;	// Unit Pattern width
	int tph = 465;	// Total Pattern height
	int tpw = 65;	// Total Pattern width
	double m = 0.2;	// Starting scale	
	double s = 0.3;	// Scale step
	double l = 0.97;	// Ending scale
	double c = 1.0;

	int totalFit = tpw / upw;
	int g = (totalFit > 1) ? ceil((((tpw / 2.0) + 1.0) - upw)) : 0;

	totalFit = tph / uph;
	int v = (totalFit > 1) ? ceil((((tph / 2.0) + 1.0) - uph)) : 0;

	int d = 0;

	int u = floor((l - m) / s);

	cout << v << "," << g << endl;

	for (unsigned int i = 0; i <= v; i++) {
		for (unsigned int j = 0; j <= g; j++) {
			int vu = (tph / (uph + i));
			int hu = (tpw / (upw + j));
			int tu = vu * hu;
			// power
			int val = 1;
			for (unsigned int k = 0; k < tu; k++) {
				val *= u;
			}

			d += (val * c);
		}
	}

	d *= 20;

	cout << d << endl;

	//double scales[6] = { 0.8, 0.4, 0.8, 0.4, 0.8, 0.4 };
	//HexagonPattern op(300, 300 ,scales);
	//Pattern p(HEXAGON, 301, 301, 0, 0, false, true, &op);
	//p.SavePatternToBmp("C:\\Users\\james\\Code\\CPP\\MachineLearningCPP\\MachineLearningCPP\\PatternGenerator\\WriteUp\\Imagesh4.bmp");

	*/

	return 0;
}
