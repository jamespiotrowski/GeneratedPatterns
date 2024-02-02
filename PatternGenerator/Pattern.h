#pragma once

#include "Polygon.h"
#include "BitMap.h"

using namespace std;

/**********************************************************************************************
###############################################################################################
##### Pattern utilities
###############################################################################################
**********************************************************************************************/

/* enumerator for all patterns */
enum PatternType
{
	SQUARE
	, RECTANGLE
	, DIAMOND
	, TRIANGLE
	, HORIZONTAL_STRIPES
	, VERTICAL_STRIPES
	, CIRCLE
	, HEXAGON
	, PENTAGON
	, HEPTAGON
	, STAR
	, OCTAGON
	, TRAPEZOID
	, HEART
	, CROSS
	, CRESCENT
	, SPIKE
	, ARROW
	, TILDE
	, ZIGZAG
	, CANE
	, CAT
	, DEFAULT_PATTERN
};

/* Helper function to return the number of scales a pattern requires */
int GetScalesForPattern(const PatternType &pt) {
	switch (pt) {
	case(SQUARE): case(HORIZONTAL_STRIPES): case(VERTICAL_STRIPES): case(CIRCLE): case(STAR) : case(HEART): case(CAT): { return 1;}
	case(RECTANGLE): case(DIAMOND): case(CROSS): case(CRESCENT):case(SPIKE): case(ARROW): case(TILDE): case(ZIGZAG): case(CANE): { return 2; }
	case(TRIANGLE): case(TRAPEZOID): { return 3; }
	case(PENTAGON): { return 5; }
	case(HEXAGON): { return 6; }
	case(HEPTAGON): { return 7; }
	case(OCTAGON): { return 8; }
	default: { return 0; }
	}
	return 0;
}

/* Helper function to get the name of a pattern as a string */
string GetNameForPattern(const PatternType& pt) {
	switch (pt) {
	case(SQUARE): { return "Square"; }
	case(HORIZONTAL_STRIPES): { return "HorizontalStripe"; }
	case(VERTICAL_STRIPES): { return "VerticalStripe"; }
	case(CIRCLE): { return "Circle"; }
	case(STAR): { return "Star"; }
	case(RECTANGLE): { return "Rectangle"; }
	case(DIAMOND): { return "Diamond"; }
	case(TRIANGLE): { return "Triangle"; }
	case(PENTAGON): { return "Pentagon"; }
	case(HEXAGON): { return "Hexagon"; }
	case(HEPTAGON): { return "Heptagon"; }
	case(OCTAGON): { return "Octogon"; }
	case(TRAPEZOID): { return "Trapezoid"; }
	case(HEART): { return "Heart"; }
	case(CROSS): { return "Cross"; }
	case(CRESCENT): { return "Crescent"; }
	case(SPIKE): { return "Spike"; }
	case(ARROW): { return "Arrow"; }
	case(TILDE): { return "Tilde"; }
	case(ZIGZAG): { return "Zigzag"; }
	case(CANE): { return "Cane"; }
	case(CAT): { return "Cat"; }
	default: { return ""; }
	}
	return "";
}


/**************************************************************
* SpecialProcessing
***************************************************************
* Helper function to determine if a shape (pattern) will be unique for all scales
*
* For example, a Square is also a rectangle and a trapezoid
* A rectangle is also a trapezoid.
*
* When generating patterns, i will need to flag certain patterns to indicate that
* any scale may not be appropriate to use for this pattern
*
* Currently stripes are excluded from this because they will not be used in the first
* iteration of this project
**************************************************************/
bool SpecialProcessing(const PatternType& pt) {
	switch(pt){
	case(RECTANGLE): case(TRAPEZOID): case(CRESCENT): { return true; }
	default: { return false; }
	}
	return false;
}

/************************************************************
#############################################################
#   Unit Pattern Class
#############################################################
#
#   This class is the base class that all specific 
#	unit patterns will inherit from. 
#
#	An object of this class will contain an image
#	
************************************************************/
class UnitPattern {
public:

	/**************************************************************
	* DetermineHeightAndWidthWithTrueCenter
	***************************************************************
	* Depending on the dimensions of a 2D pixel grid, the true
	* center of the grid may be one pixel, two pixels, or four. 
	* 
	* Processing is much easier if the image has a single pixel
	* as it's center, so this function alters the height and 
	* width dimension to make that happen.
	**************************************************************/
	static void DetermineHeightAndWidthWithTrueCenter(int& height, int& width) {
		if ((height % 2) == 0) {
			height += 1;
		}

		if ((width % 2) == 0) {
			width += 1;
		}
	}

	/* function to return the center pixel of a 2D grid */
	static void GetCenter(const int& height, const int& width, int& centerHeight, int& centerWidth) {
		centerHeight = height / 2;
		centerWidth = width / 2;
	}

protected:
	char** pattern = nullptr;						/* pointer to the 2D grid */
	int height = 0, width = 0, numberOfScales = 0;	/* dimensions of the image - number of scales pertains to how many ways the an image can be stretched */
	double* scales = nullptr;						/* pointer to the actual scale values */
	bool verticalOffsetAllowed = true;				/* currently unused member that determines if a unit pattern can have space between a vertical partner unit */
	bool horizontalOffsetAllowed = true;			/* currently unused member that determines if a unit pattern can have space between a horizontal partner unit */
	PatternType patternType;						/* identifier for the type of image that is stored here */

private:

	/**************************************************************
	* FillInUntilEdge
	***************************************************************
	* Function used to optimize the filling in of a polygon
	* 
	* if a point is determined to be inside the polygon 
	* AND the edges of that polygon are already plotted,
	* every pixel to the right of the one inside the polygon
	* is also inside until the first edge is encountered. 
	* 
	* this function helps by filling those pixels in without
	* having to check if they are inside. 
	**************************************************************/
	void FillInUntilEdge(const int& y, int& w) {
		while (w < width && pattern[y][w] != 1) {
			pattern[y][w] = 1;
			w += 1;
		}
	}

protected:

	/* helper function to deep copy a set of scales */
	void SetScale(int numberOfScales, const double* scales) {
		this->scales = new double[numberOfScales];
		for (int i = 0; i < numberOfScales; i++) {
			this->scales[i] = scales[i];
		}
	}

	/**************************************************************
	* IsInsidePolygon
	***************************************************************
	* This is an awful manual implementation (AND NOT AT ALL
	* PERFECT) for checking if 
	* a point is inside a polygon. Unfortunately, some shapes
	* may not be well represented as polygons (like a circle)
	* so using the polygon object to make such image is clunky. 
	* 
	* This function only see's use for those difficult to 
	* represent shapes like a circle. 
	* 
	* The loneEdgePoints parameter identifies points at the 
	* tip and base of the shape so that they can ignored as 
	* edges. For those difficult to represent shapes, prior
	* to calling this function you must identify the top-most
	* set of pixels and the bottom-most set of pixels for this
	* to work nicely. 
	**************************************************************/
	bool IsInsidePolygon(Coordinate c, const Array<Coordinate>& loneEdgePoints) {

		/* 
		because we are actually using the plotted edges of the shape 
		to determine if a point is inside, edge-points that are consectutive
		count as the same edge, so an edge will only be counted if one pixel
		is filled in and it's partner is not. 
		*/

		int edges = 0;			/* number of edges that will be counted */
		int sp = width - 1;		/* safety point to not go out of bounds when checking partner pixels */
		for (int w = c.x; w < width; w += 1) { /* for the remainder of the grid starting from a specific point */
			if (w < sp) { /* if we can safely check for a partner pixel */
				if (pattern[c.y][w] == 1 && pattern[c.y][w + 1] != 1) { /* if a pixel and it's partner are contrasting */
					edges += 1; /* we found an edge! */
				}
			}
			else { /* else, we are at the edge and must check for partner in the other direction */
				if (pattern[c.y][w] == 1 && pattern[c.y][w - 1] != 1) { /* if a pixel and it's partner are contrasting */
					edges += 1; /* we found an edge! */
				}
			}
		}

		/* if any lone edge points exist in a right-cast ray of the point, we must subtract our edge count */
		for (int i = 0; i < loneEdgePoints.getSize(); i++) {
			if (loneEdgePoints.at(i).y == c.y && loneEdgePoints.at(i).x >= c.x) {
				edges -= 1;
			}
		}

		return (((edges % 2) == 1));
	}

	/* Nice function to fill in a TRUE polygon */
	void FillInPolygon(const Polygon& p) {
		Coordinate c;
		for (int h = 0; h < height; h++) {
			c.y = h;
			for (int w = 0; w < width; w++) {
				c.x = w;
				if (p.isInsidePolygon(c)) {
					pattern[h][w] = 1; /* Could optimize by using fillInUntilEdge, but not general and may not be necesarry */
				}
			}
		}
	}

	/* Scary function to fill in a NOT TRUE polygon, such as a circle */
	void FillInPolygon(const Array<Coordinate>& loneEdgePoints) {
		Coordinate c;
		for (int h = 0; h < height; h++) {
			c.y = h;
			for (int w = 0; w < width; w++) {
				c.x = w;
				if (IsInsidePolygon(c, loneEdgePoints)) {
					pattern[h][w] = 1; /* Could optimize by using fillInUntilEdge, but not general and may not be necesarry */
				}
			}
		}
	}

	/* this.... shouldnt be used. */
	void FillInBruteForce(const Coordinate& c) {
		if (c.x < 0 || c.x >= width || c.y < 0 || c.y >= height) { return; }
		if (pattern[c.y][c.x] != 0) { return; }
		pattern[c.y][c.x] = 1;
		/* Down */
		if ((c.y + 1) < height) {
			if (pattern[c.y + 1][c.x] != 1) {
				Coordinate down(c.y + 1, c.x);
				FillInBruteForce(down);
			}
		}
		/* Up */
		if ((c.y - 1) >= 0) {
			if (pattern[c.y - 1][c.x] != 1) {
				Coordinate up(c.y - 1, c.x);;
				FillInBruteForce(up);
			}
		}
		/* right */
		if ((c.x + 1) < width) {
			if (pattern[c.y][c.x + 1] != 1) {
				Coordinate right(c.y, c.x + 1);
				FillInBruteForce(right);
			}
		}
		/* left */
		if ((c.x - 1) >= 0) {
			if (pattern[c.y][c.x - 1] != 1) {
				Coordinate left(c.y, c.x - 1);
				FillInBruteForce(left);
			}
		}
	}


private:

	/* function to return the memory used by the image */
	void clear() {
		for (int i = 0; i < height; i++) {
			delete[] pattern[i];
		}
		delete[] pattern;
		delete[] scales;
		pattern = nullptr;
		height = 0;
		width = 0;
		numberOfScales = 0;
	}

	/* helper function to cast a bit to a char */
	char ToString(char p) { return (p == 0) ? '0' : '1'; }

	/* helper function to represent pixels as white space(.) or filled in (#) */
	string ToSymbol(char p) { return (p == 0) ? "." : "#"; }

public:

	/* parameter constructor - allocates memory needed */
	UnitPattern(int height, int width, PatternType patternType) : height(height), width(width), patternType(patternType) {
		/* alter height and width so that there is a single pixel center */
		DetermineHeightAndWidthWithTrueCenter(this->height, this->width);

		/* allocate memory */
		pattern = new char* [this->height];
		for (int i = 0; i < this->height; i++) {
			pattern[i] = new char[this->width];
			for (int j = 0; j < this->width; j++) {
				pattern[i][j] = 0; /* default to white space */
			}
		}
	}

	/* destructor */
	~UnitPattern() {
		clear();
	}

	/* assignment operator for a deep copy :) */
	void operator=(const UnitPattern& copy) {
		clear();
		numberOfScales = copy.numberOfScales;
		height = copy.height;
		width = copy.width;
		pattern = new char* [height];
		for (int i = 0; i < height; i++) {
			pattern[i] = new char[width];
			for (int j = 0; j < width; j++) {
				pattern[i][j] = copy.pattern[i][j];
			}
		}

		this->scales = new double[numberOfScales];
		for (int i = 0; i < numberOfScales; i++) {
			scales[i] = copy.scales[i];
		}
	}

	/* copy constructor for a deep copy :) */
	UnitPattern(const UnitPattern& copy) {
		(*this) = copy;
	}

	/* nice print function for debugging */
	void PrintPattern(ostream& out) {
		string s = "";
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				s += ToSymbol(pattern[i][j]);
			}
			s += "\n";
		}
		s += "\n";

		out << s;
	}

	int GetHeight() const { return height; }
	int GetWidth() const { return width; }
	int GetScales() const { return numberOfScales; }
	PatternType GetPatternType() const { return patternType; }
	bool allowsVerticalOffset() const { return verticalOffsetAllowed; }
	bool allowsHorizontalOffset() const { return horizontalOffsetAllowed; }

	/* function to get a copy of the pixel stored at h,w */
	char At(const int& h, const int& w) const {
		return pattern[h][w];
	}

	/* Pure abstract function so you cannot instantiate this object :) */
	virtual void GenerateUnitPattern() = 0; /* This is the function used to draw the shape within the grid */

};

/*
Please excuse the lack of comments for all the derived classes 

These modules were implemented to make a distinct picture and nothing else. 
Much of the code to generate the pictures was just me writing ridiculous code
to get it to work. 

Many of the true polygons have pretty straight forward code that can be followed. 

The circle, heart, cat... well, im sorry.
*/


/**********************************************************************************************
###############################################################################################
##### SQUARE 
###############################################################################################
**********************************************************************************************/
class SquarePattern : public UnitPattern {
private:
	const int scale1 = 0; 

	void GenerateUnitPattern() {
		int centerHeight = 0, centerWidth = 0;
		GetCenter(height, width, centerHeight, centerWidth);
		int radius = (scales[scale1] * ((height < width) ? height : width)) / 2;

		verticalOffsetAllowed = true;
		horizontalOffsetAllowed = true;

		/* Compute the pattern area */
		int startHeight = centerHeight - radius;
		int startWidth = centerWidth - radius;
		int endHeight = centerHeight + radius;
		int endWidth = centerWidth + radius;

		/* Compute the pattern area */
		Coordinate* c = new Coordinate[4];
		c[0] = Coordinate(startHeight, startWidth);
		c[1] = Coordinate(startHeight, endWidth);
		c[2] = Coordinate(endHeight, endWidth);
		c[3] = Coordinate(endHeight, startWidth);

		Array<Edge> edges;
		edges.push(Edge(c[0], c[1]));
		edges.push(Edge(c[1], c[2]));
		edges.push(Edge(c[2], c[3]));
		edges.push(Edge(c[3], c[0]));

		delete[] c;

		Polygon p(edges);
		p.plotPolygon(pattern);
		FillInPolygon(p);
	}

public:

	SquarePattern(int height, int width, const double* s) : UnitPattern(height, width, SQUARE) {
		SetScale(GetScalesForPattern(patternType), s);
		GenerateUnitPattern();
	}

};

/**********************************************************************************************
###############################################################################################
##### DIAMOND
###############################################################################################
**********************************************************************************************/
class DiamondPattern : public UnitPattern {
private:
	const int widthScale = 0;
	const int heightScale = 1;

	void GenerateUnitPattern() {
		int centerHeight = 0, centerWidth = 0;
		GetCenter(height, width, centerHeight, centerWidth);
		int widthRadius = (scales[widthScale] * width) / 2;
		int heightRadius = (scales[heightScale] * height) / 2;

		verticalOffsetAllowed = true;
		horizontalOffsetAllowed = true;

		/* Compute the pattern area */
		Coordinate* c = new Coordinate[4];
		c[0] = Coordinate(centerHeight + heightRadius, centerWidth);
		c[1] = Coordinate(centerHeight, centerWidth - widthRadius);
		c[2] = Coordinate(centerHeight - heightRadius, centerWidth);
		c[3] = Coordinate(centerHeight, centerWidth + widthRadius);

		Array<Edge> edges;
		edges.push(Edge(c[0], c[1]));
		edges.push(Edge(c[1], c[2]));
		edges.push(Edge(c[2], c[3]));
		edges.push(Edge(c[3], c[0]));

		delete[] c;

		Polygon p(edges);
		p.plotPolygon(pattern);
		FillInPolygon(p);
	}

public:

	DiamondPattern(int height, int width, const double* s) : UnitPattern(height, width, DIAMOND) {
		SetScale(GetScalesForPattern(patternType), s);
		GenerateUnitPattern();
	}

};


/**********************************************************************************************
###############################################################################################
##### CIRCLE
###############################################################################################
**********************************************************************************************/
class CirclePattern : public UnitPattern {
private:
	const int scale1 = 0;

	int PythagoreanTheorem_Edge(int a_or_b, double c) {
		return sqrt((c * c) - (a_or_b * a_or_b));
	}

	void GenerateUnitPattern() {
		int centerHeight = 0, centerWidth = 0;
		GetCenter(height, width, centerHeight, centerWidth); 
		double radius = (scales[scale1] * ((height < width) ? height : width)) / 2;

		verticalOffsetAllowed = true;
		horizontalOffsetAllowed = true;

		/* Compute the pattern area */
		Array<Coordinate> circlePoints;
		Coordinate coords[8];

		for (int i = 1; i <= (int)radius; i++) {
			int yPoint = PythagoreanTheorem_Edge(i, radius);

			coords[0] = Coordinate(centerHeight - yPoint, centerWidth + i);
			coords[1] = Coordinate(centerHeight - yPoint, centerWidth - i);
			coords[2] = Coordinate(centerHeight + yPoint, centerWidth + i);
			coords[3] = Coordinate(centerHeight + yPoint, centerWidth - i);
			coords[4] = Coordinate(centerHeight - i, centerWidth + yPoint);
			coords[5] = Coordinate(centerHeight - i, centerWidth - yPoint);
			coords[6] = Coordinate(centerHeight + i, centerWidth + yPoint);
			coords[7] = Coordinate(centerHeight + i, centerWidth - yPoint);

			for (int j = 0; j < 8; j++) {
				if (!circlePoints.exists(coords[j])) {
					circlePoints.push(coords[j]);
				}
			}
		}

		coords[0] = Coordinate(centerHeight, centerWidth + radius);
		coords[1] = Coordinate(centerHeight, centerWidth - radius);
		coords[2] = Coordinate(centerHeight + radius, centerWidth);
		coords[3] = Coordinate(centerHeight - radius, centerWidth);
		for (int j = 0; j < 4; j++) {
			if (!circlePoints.exists(coords[j])) {
				circlePoints.push(coords[j]);
			}
		}

		Array<int> edgePointCounter;
		for (int i = 0; i < height; i++) {
			edgePointCounter.push(0);
		}

		for (int i = 0; i < circlePoints.getSize(); i++) {
			edgePointCounter[circlePoints[i].y] += 1;
			pattern[circlePoints[i].y][circlePoints[i].x] = 1;
		}

		int minHeight = 0, maxHeight = 0;
		for (int i = 0; i < edgePointCounter.getSize(); i++) {
			if (edgePointCounter[i] >= 1) {
				minHeight = i;
				break;
			}
		}

		for (int i = ((int)edgePointCounter.getSize()) - 1; i >= 0; i--) {
			if (edgePointCounter[i] >= 1) {
				maxHeight = i;
				break;
			}
		}

		bool foundMinCoord = false, foundMaxCoord = false;
		Coordinate minCoordinate;
		Coordinate maxCoordinate;
		for (int j = 0; j < circlePoints.getSize(); j++) {

			if (minHeight == circlePoints[j].y) {
				if (foundMinCoord) {
					if (minCoordinate.x > circlePoints[j].x) {
						minCoordinate = circlePoints[j];
					}
				}
				else {
					minCoordinate = circlePoints[j];
					foundMinCoord = true;
				}
			}

			if (maxHeight == circlePoints[j].y) {
				if (foundMaxCoord) {
					if (maxCoordinate.x > circlePoints[j].x) {
						maxCoordinate = circlePoints[j];
					}
				}
				else {
					maxCoordinate = circlePoints[j];
					foundMaxCoord = true;
				}
			}

		}

		Array<Coordinate> loneEdgePoints;
		loneEdgePoints.push(minCoordinate);
		loneEdgePoints.push(maxCoordinate);

		FillInPolygon(loneEdgePoints);
	}

public:

	CirclePattern(int height, int width, const double* s) : UnitPattern(height, width, CIRCLE) {
		SetScale(GetScalesForPattern(patternType), s);
		GenerateUnitPattern();
	}

};


/**********************************************************************************************
###############################################################################################
##### TRIANGLE
###############################################################################################
**********************************************************************************************/
class TrianglePattern : public UnitPattern {
private:

	void GenerateUnitPattern() {
		Coordinate centerCoord;
		GetCenter(height, width, centerCoord.y, centerCoord.x);
		
		verticalOffsetAllowed = true;
		horizontalOffsetAllowed = true;

		double t = ((height < width) ? height : width) / 2;

		int sides = 3;

		double split = 360.0 / sides;

		Array<Coordinate> c;
		for (int i = 0; i < sides; i++) {
			c.push(Polygon::ComputePointGivenAngleAndDistance(180.0 + (i * split), scales[i] * t, centerCoord));
		}
		
		Array<Edge> edges;
		for (int i = 0; i < sides; i++) {
			edges.push(Edge(c[i], c[(i+1) % sides]));
		}

		Polygon p(edges);
		p.plotPolygon(pattern);
		FillInPolygon(p);
	}

public:

	TrianglePattern(int height, int width, const double* s) : UnitPattern(height, width, TRIANGLE) {
		SetScale(GetScalesForPattern(patternType), s);
		GenerateUnitPattern();
	}

};


/**********************************************************************************************
###############################################################################################
##### PENTAGON
###############################################################################################
**********************************************************************************************/
class PentagonPattern : public UnitPattern {
private:

	void GenerateUnitPattern() {
		Coordinate centerCoord;
		GetCenter(height, width, centerCoord.y, centerCoord.x); 

		verticalOffsetAllowed = true;
		horizontalOffsetAllowed = true;

		double t = ((height < width) ? height : width) / 2;

		int sides = 5;

		double split = 360.0 / sides;

		Array<Coordinate> c;
		for (int i = 0; i < sides; i++) {
			c.push(Polygon::ComputePointGivenAngleAndDistance(180.0 + (i * split), scales[i] * t, centerCoord));
		}

		Array<Edge> edges;
		for (int i = 0; i < sides; i++) {
			edges.push(Edge(c[i], c[(i + 1) % sides]));
		}

		Polygon p(edges);
		p.plotPolygon(pattern);
		FillInPolygon(p);
	}

public:

	PentagonPattern(int height, int width, const double* s) : UnitPattern(height, width, PENTAGON) {
		SetScale(GetScalesForPattern(patternType), s);
		GenerateUnitPattern();
	}

};

/**********************************************************************************************
###############################################################################################
##### STAR
###############################################################################################
**********************************************************************************************/
class StarPattern : public UnitPattern {
private:
	const int scale1 = 0; 

	void GenerateUnitPattern() {
		Coordinate centerCoord;
		GetCenter(height, width, centerCoord.y, centerCoord.x);	
		int radius = (scales[scale1] * ((height < width) ? height : width)) / 2; 

		verticalOffsetAllowed = true;
		horizontalOffsetAllowed = true;


		double split = 360.0 / 5.0;

		/* Compute the pattern area */
		Coordinate* c = new Coordinate[10];
		c[0] = Polygon::ComputePointGivenAngleAndDistance(180.0 + (0.0 * split), radius, centerCoord);
		c[2] = Polygon::ComputePointGivenAngleAndDistance(180.0 + (1.0 * split), radius, centerCoord);
		c[4] = Polygon::ComputePointGivenAngleAndDistance(180.0 + (2.0 * split), radius, centerCoord);
		c[6] = Polygon::ComputePointGivenAngleAndDistance(180.0 + (3.0 * split), radius, centerCoord);
		c[8] = Polygon::ComputePointGivenAngleAndDistance(180.0 + (4.0 * split), radius, centerCoord);

		c[1] = Polygon::ComputeCentroid(c[0], c[2], centerCoord);
		c[3] = Polygon::ComputeCentroid(c[2], c[4], centerCoord);
		c[5] = Polygon::ComputeCentroid(c[4], c[6], centerCoord);
		c[7] = Polygon::ComputeCentroid(c[6], c[8], centerCoord);
		c[9] = Polygon::ComputeCentroid(c[8], c[0], centerCoord);


		Array<Edge> edges;
		edges.push(Edge(c[0], c[1]));
		edges.push(Edge(c[1], c[2]));
		edges.push(Edge(c[2], c[3]));
		edges.push(Edge(c[3], c[4]));
		edges.push(Edge(c[4], c[5]));
		edges.push(Edge(c[5], c[6]));
		edges.push(Edge(c[6], c[7]));
		edges.push(Edge(c[7], c[8]));
		edges.push(Edge(c[8], c[9]));
		edges.push(Edge(c[9], c[0]));

		delete[] c;

		Polygon p(edges);
		p.plotPolygon(pattern);
		FillInPolygon(p);

	}

public:

	StarPattern(int height, int width, const double* s) : UnitPattern(height, width, STAR) {
		SetScale(GetScalesForPattern(patternType), s);
		GenerateUnitPattern();
	}

};

/**********************************************************************************************
###############################################################################################
##### Horizontal Stripes - this guy works but not used because unlike the others 
###############################################################################################
**********************************************************************************************/
class HorizontalStripePattern : public UnitPattern {
private:
	const int scale1 = 0; 

	void GenerateUnitPattern() {
		Coordinate centerCoord;
		GetCenter(height, width, centerCoord.y, centerCoord.x);					 
		int radius = (scales[scale1] * ((height < width) ? height : width)) / 2; 

		verticalOffsetAllowed = true;
		horizontalOffsetAllowed = false;


		/* Compute the pattern area */
		Coordinate* c = new Coordinate[4];
		c[0] = Coordinate(centerCoord.y + radius, 0);
		c[1] = Coordinate(centerCoord.y - radius, 0);
		c[2] = Coordinate(centerCoord.y - radius, height - 1);
		c[3] = Coordinate(centerCoord.y + radius, height - 1);


		Array<Edge> edges;
		edges.push(Edge(c[0], c[1]));
		edges.push(Edge(c[1], c[2]));
		edges.push(Edge(c[2], c[3]));
		edges.push(Edge(c[3], c[0]));

		delete[] c;

		Polygon p(edges);
		p.plotPolygon(pattern);
		FillInPolygon(p);
	}

public:

	HorizontalStripePattern(int height, int width, const double* s) : UnitPattern(height, width, HORIZONTAL_STRIPES) {
		SetScale(GetScalesForPattern(patternType), s);
		GenerateUnitPattern();
	}

};


/**********************************************************************************************
###############################################################################################
##### Vertical Stripes - this guy works but not used because unlike the others 
###############################################################################################
**********************************************************************************************/
class VerticalStripePattern : public UnitPattern {
private:
	const int scale1 = 0; // Scale that square uses

	void GenerateUnitPattern() {
		Coordinate centerCoord;
		GetCenter(height, width, centerCoord.y, centerCoord.x);					 
		int radius = (scales[scale1] * ((height < width) ? height : width)) / 2; 

		verticalOffsetAllowed = true;
		horizontalOffsetAllowed = false;


		/* Compute the pattern area */
		Coordinate* c = new Coordinate[4];
		c[0] = Coordinate(0, centerCoord.x + radius);
		c[1] = Coordinate(0, centerCoord.x - radius);
		c[2] = Coordinate(width - 1, centerCoord.x - radius);
		c[3] = Coordinate(width - 1, centerCoord.x + radius);


		Array<Edge> edges;
		edges.push(Edge(c[0], c[1]));
		edges.push(Edge(c[1], c[2]));
		edges.push(Edge(c[2], c[3]));
		edges.push(Edge(c[3], c[0]));

		delete[] c;

		Polygon p(edges);
		p.plotPolygon(pattern);
		FillInPolygon(p);
	}

public:

	VerticalStripePattern(int height, int width, const double* s) : UnitPattern(height, width, VERTICAL_STRIPES) {
		SetScale(GetScalesForPattern(patternType), s);
		GenerateUnitPattern();
	}

};

/**********************************************************************************************
###############################################################################################
##### RECTANLGE
###############################################################################################
**********************************************************************************************/
class RectanglePattern : public UnitPattern {
private:
	const int widthScale = 0;
	const int heightScale = 1;

	void GenerateUnitPattern() {
		int centerHeight = 0, centerWidth = 0;
		GetCenter(height, width, centerHeight, centerWidth); 
		int widthRadius = (scales[widthScale] * width) / 2;
		int heightRadius = (scales[heightScale] * height) / 2;

		verticalOffsetAllowed = true;
		horizontalOffsetAllowed = true;

		/* Compute the pattern area */
		int startHeight = centerHeight - heightRadius;
		int startWidth = centerWidth - widthRadius;
		int endHeight = centerHeight + heightRadius;
		int endWidth = centerWidth + widthRadius;

		/* Compute the pattern area */
		Coordinate* c = new Coordinate[4];
		c[0] = Coordinate(startHeight, startWidth);
		c[1] = Coordinate(startHeight, endWidth);
		c[2] = Coordinate(endHeight, endWidth);
		c[3] = Coordinate(endHeight, startWidth);

		Array<Edge> edges;
		edges.push(Edge(c[0], c[1]));
		edges.push(Edge(c[1], c[2]));
		edges.push(Edge(c[2], c[3]));
		edges.push(Edge(c[3], c[0]));

		delete[] c;

		Polygon p(edges);
		p.plotPolygon(pattern);
		FillInPolygon(p);
	}

public:

	RectanglePattern(int height, int width, const double* s) : UnitPattern(height, width, RECTANGLE) {
		SetScale(GetScalesForPattern(patternType), s);
		GenerateUnitPattern();
	}

};


/**********************************************************************************************
###############################################################################################
##### HEXAGON
###############################################################################################
**********************************************************************************************/
class HexagonPattern : public UnitPattern {
private:

	void GenerateUnitPattern() {
		Coordinate centerCoord;
		GetCenter(height, width, centerCoord.y, centerCoord.x); 

		verticalOffsetAllowed = true;
		horizontalOffsetAllowed = true;

		double t = ((height < width) ? height : width) / 2;

		int sides = 6;

		double split = 360.0 / sides;

		Array<Coordinate> c;
		for (int i = 0; i < sides; i++) {
			c.push(Polygon::ComputePointGivenAngleAndDistance(180.0 + (i * split), scales[i] * t, centerCoord));
		}

		Array<Edge> edges;
		for (int i = 0; i < sides; i++) {
			edges.push(Edge(c[i], c[(i + 1) % sides]));
		}

		Polygon p(edges);
		p.plotPolygon(pattern);
		FillInPolygon(p);
	}

public:

	HexagonPattern(int height, int width, const double* s) : UnitPattern(height, width, HEXAGON) {
		SetScale(GetScalesForPattern(patternType), s);
		GenerateUnitPattern();
	}

};


/**********************************************************************************************
###############################################################################################
##### HEPTAGON
###############################################################################################
**********************************************************************************************/
class HeptagonPattern : public UnitPattern {
private:

	void GenerateUnitPattern() {
		Coordinate centerCoord;
		GetCenter(height, width, centerCoord.y, centerCoord.x); 

		verticalOffsetAllowed = true;
		horizontalOffsetAllowed = true;

		double t = ((height < width) ? height : width) / 2;

		int sides = 7;

		double split = 360.0 / sides;

		Array<Coordinate> c;
		for (int i = 0; i < sides; i++) {
			c.push(Polygon::ComputePointGivenAngleAndDistance(180.0 + (i * split), scales[i] * t, centerCoord));
		}

		Array<Edge> edges;
		for (int i = 0; i < sides; i++) {
			edges.push(Edge(c[i], c[(i + 1) % sides]));
		}

		Polygon p(edges);
		p.plotPolygon(pattern);
		FillInPolygon(p);
	}

public:

	HeptagonPattern(int height, int width, const double* s) : UnitPattern(height, width, HEPTAGON) {
		SetScale(GetScalesForPattern(patternType), s);
		GenerateUnitPattern();
	}

};


/**********************************************************************************************
###############################################################################################
##### OCTOGON
###############################################################################################
**********************************************************************************************/
class OctogonPattern : public UnitPattern {
private:

	void GenerateUnitPattern() {
		Coordinate centerCoord;
		GetCenter(height, width, centerCoord.y, centerCoord.x); 

		verticalOffsetAllowed = true;
		horizontalOffsetAllowed = true;

		double t = ((height < width) ? height : width) / 2;

		int sides = 8;

		double split = 360.0 / sides;

		Array<Coordinate> c;
		for (int i = 0; i < sides; i++) {
			c.push(Polygon::ComputePointGivenAngleAndDistance(180.0 + (i * split), scales[i] * t, centerCoord));
		}

		Array<Edge> edges;
		for (int i = 0; i < sides; i++) {
			edges.push(Edge(c[i], c[(i + 1) % sides]));
		}

		Polygon p(edges);
		p.plotPolygon(pattern);
		FillInPolygon(p);
	}

public:

	OctogonPattern(int height, int width, const double* s) : UnitPattern(height, width, OCTAGON) {
		SetScale(GetScalesForPattern(patternType), s);
		GenerateUnitPattern();
	}

};


/**********************************************************************************************
###############################################################################################
##### TRAPEZOID
###############################################################################################
**********************************************************************************************/
class TrapezoidPattern : public UnitPattern {
private:
	const int width1Scale = 0;
	const int width2Scale = 1;
	const int heightScale = 2;

	void GenerateUnitPattern() {
		Coordinate centerCoord;
		GetCenter(height, width, centerCoord.y, centerCoord.x); 

		verticalOffsetAllowed = true;
		horizontalOffsetAllowed = true;

		int t = ((height < width) ? height : width) / 2;

		int bottomWidth = scales[width1Scale] * t;
		int topWidth = scales[width2Scale] * t;
		int tHeight = scales[heightScale] * t;
	
		Array<Edge> edges;
		edges.push(Edge(Coordinate(centerCoord.y - tHeight, centerCoord.x - topWidth), Coordinate(centerCoord.y - tHeight, centerCoord.x + topWidth)));
		edges.push(Edge(Coordinate(centerCoord.y - tHeight, centerCoord.x + topWidth), Coordinate(centerCoord.y + tHeight, centerCoord.x + bottomWidth)));
		edges.push(Edge(Coordinate(centerCoord.y + tHeight, centerCoord.x + bottomWidth), Coordinate(centerCoord.y + tHeight, centerCoord.x - bottomWidth)));
		edges.push(Edge(Coordinate(centerCoord.y + tHeight, centerCoord.x - bottomWidth), Coordinate(centerCoord.y - tHeight, centerCoord.x - topWidth)));

		Polygon p(edges);
		p.plotPolygon(pattern);
		FillInPolygon(p);
	}

public:

	TrapezoidPattern(int height, int width, const double* s) : UnitPattern(height, width, TRAPEZOID) {
		SetScale(GetScalesForPattern(patternType), s);
		GenerateUnitPattern();
	}

};


/**********************************************************************************************
###############################################################################################
##### HEART
###############################################################################################
**********************************************************************************************/
class HeartPattern : public UnitPattern {
private:
	const int s1 = 0;

	void GenerateUnitPattern() {
		Coordinate centerCoord;
		GetCenter(height, width, centerCoord.y, centerCoord.x); 

		verticalOffsetAllowed = true;
		horizontalOffsetAllowed = true;

		double t = ((height < width) ? height : width) / 2;

		double halfRadius = t * scales[s1];
		double td2 = halfRadius / 2.0;
		
		Coordinate centerFanCoord;
		int sidesPerFan = 10;
		double split = 360.0 / sidesPerFan;
		double currentAngle = 0;
		Array<Coordinate> c;

		c.push(Coordinate(centerCoord.y + halfRadius, centerCoord.x));

		/* RIGHT FAN */
		centerFanCoord = Coordinate(centerCoord.y - td2, centerCoord.x + td2);
		for (currentAngle = 220; currentAngle < 360; currentAngle += split) {
			c.push(Polygon::ComputePointGivenAngleAndDistance(180.0 + currentAngle, td2, centerFanCoord));
		}
		for (currentAngle = 0; currentAngle < 90; currentAngle += split) {
			c.push(Polygon::ComputePointGivenAngleAndDistance(180.0 + currentAngle, td2, centerFanCoord));
		}				

		/* LEFT FAN */
		centerFanCoord = Coordinate(centerCoord.y - td2, centerCoord.x - td2);
		for (currentAngle = 270; currentAngle < 360; currentAngle += split) {
			c.push(Polygon::ComputePointGivenAngleAndDistance(180.0 + currentAngle, td2, centerFanCoord));
		}
		for (currentAngle = 0; currentAngle <= 140; currentAngle += split) {
			c.push(Polygon::ComputePointGivenAngleAndDistance(180.0 + currentAngle, td2, centerFanCoord));
		}

		Array<Edge> edges;
		for (int i = 0; i < c.getSize(); i++) {
			edges.push(Edge(c[i], c[ (i+1) % c.getSize() ]));
		}

		Polygon p(edges);
		p.plotPolygon(pattern);
		FillInPolygon(p);

	}

public:

	HeartPattern(int height, int width, const double* s) : UnitPattern(height, width, HEART) {
		SetScale(GetScalesForPattern(patternType), s);
		GenerateUnitPattern();
	}

};


/**********************************************************************************************
###############################################################################################
##### CROSS
###############################################################################################
**********************************************************************************************/
class CrossPattern : public UnitPattern {
private:
	const int s1 = 0;
	const int s2 = 0;

	void GenerateUnitPattern() {
		Coordinate centerCoord;
		GetCenter(height, width, centerCoord.y, centerCoord.x); 

		double limbScale = scales[s2];
		if (limbScale >= scales[s1]) {
			limbScale = scales[s1] / 3;
		}

		verticalOffsetAllowed = true;
		horizontalOffsetAllowed = true;

		double t = ((height < width) ? height : width) / 2;

		double halfRadius = t * scales[s1];
		double limbRadius = t * limbScale;

		Array<Coordinate> c;
		c.push(Coordinate(centerCoord.y - halfRadius, centerCoord.x + limbRadius));
		c.push(Coordinate(centerCoord.y - limbRadius, centerCoord.x + limbRadius));
		c.push(Coordinate(centerCoord.y - limbRadius, centerCoord.x + halfRadius));
		c.push(Coordinate(centerCoord.y + limbRadius, centerCoord.x + halfRadius));
		c.push(Coordinate(centerCoord.y + limbRadius, centerCoord.x + limbRadius));
		c.push(Coordinate(centerCoord.y + halfRadius, centerCoord.x + limbRadius));
		c.push(Coordinate(centerCoord.y + halfRadius, centerCoord.x - limbRadius));
		c.push(Coordinate(centerCoord.y + limbRadius, centerCoord.x - limbRadius));
		c.push(Coordinate(centerCoord.y + limbRadius, centerCoord.x - halfRadius));
		c.push(Coordinate(centerCoord.y - limbRadius, centerCoord.x - halfRadius));
		c.push(Coordinate(centerCoord.y - limbRadius, centerCoord.x - limbRadius));
		c.push(Coordinate(centerCoord.y - halfRadius, centerCoord.x - limbRadius));

		Array<Edge> edges;
		for (int i = 0; i < c.getSize(); i++) {
			edges.push(Edge(c[i], c[(i + 1) % c.getSize()]));
		}

		Polygon p(edges);
		p.plotPolygon(pattern);

		/******************
			Relatively garbage code to fill in difficult shapes
		**********************/
		Array<Coordinate> loneEdgePoints;
		Array<int> edgePointCounter;
		for (int i = 0; i < height; i++) {
			edgePointCounter.push(0);
		}

		for (int i = 0; i < c.getSize(); i++) {
			edgePointCounter[c[i].y] += 1;
			pattern[c[i].y][c[i].x] = 1;
		}

		int minHeight = 0, maxHeight = 0;
		for (int i = 0; i < edgePointCounter.getSize(); i++) {
			if (edgePointCounter[i] >= 1) {
				minHeight = i;
				break;
			}
		}

		for (int i = ((int)edgePointCounter.getSize()) - 1; i >= 0; i--) {
			if (edgePointCounter[i] >= 1) {
				maxHeight = i;
				break;
			}
		}

		bool foundMinCoord = false, foundMaxCoord = false;
		Coordinate minCoordinate;
		Coordinate maxCoordinate;
		for (int j = 0; j < c.getSize(); j++) {

			if (minHeight == c[j].y) {
				if (foundMinCoord) {
					if (minCoordinate.x > c[j].x) {
						minCoordinate = c[j];
					}
				}
				else {
					minCoordinate = c[j];
					foundMinCoord = true;
				}
			}

			if (maxHeight == c[j].y) {
				if (foundMaxCoord) {
					if (maxCoordinate.x > c[j].x) {
						maxCoordinate = c[j];
					}
				}
				else {
					maxCoordinate = c[j];
					foundMaxCoord = true;
				}
			}

		}

		loneEdgePoints.push(minCoordinate);
		loneEdgePoints.push(maxCoordinate);

		FillInPolygon(loneEdgePoints);
	}

public:

	CrossPattern(int height, int width, const double* s) : UnitPattern(height, width, CROSS) {
		SetScale(GetScalesForPattern(patternType), s);
		GenerateUnitPattern();
	}

};


/**********************************************************************************************
###############################################################################################
##### CRESCENT
###############################################################################################
**********************************************************************************************/
class CrescentPattern : public UnitPattern {
private:
	const int s1 = 0;
	const int s2 = 1;

	void GenerateUnitPattern() {
		Coordinate centerCoord;
		GetCenter(height, width, centerCoord.y, centerCoord.x); 

		verticalOffsetAllowed = true;
		horizontalOffsetAllowed = true;

		double t = ((height < width) ? height : width) / 2;

		double halfRadius = t * scales[s1];

		int angleStep = 20;

		Array<Coordinate> c;
		for (double i = 180; i <= 360; i += angleStep) {
			Coordinate newC = Polygon::ComputePointGivenAngleAndDistance(i, halfRadius, centerCoord);
			if (!c.exists(newC)) {
				c.push(newC);
			}
		}

		int distanceFromCenter = 0;
		for (int i = c.getSize() - 2; i >= 0; i--) {
			distanceFromCenter = centerCoord.x - c[i].x;
			c.push(Coordinate(c[i].y, c[i].x + (distanceFromCenter * scales[s2])));
		}

		Array<Edge> edges;
		for (int i = 0; i < c.getSize(); i++) {
			edges.push(Edge(c[i], c[(i + 1) % c.getSize()]));
		}

		Polygon p(edges);
		p.plotPolygon(pattern);
		FillInPolygon(p);

	}

public:

	CrescentPattern(int height, int width, const double* s) : UnitPattern(height, width, CRESCENT) {
		SetScale(GetScalesForPattern(patternType), s);
		GenerateUnitPattern();
	}

};


/**********************************************************************************************
###############################################################################################
##### SPIKE
###############################################################################################
**********************************************************************************************/
class SpikePattern : public UnitPattern {
private:
	const int s1 = 0;
	const int s2 = 1;

	void GenerateUnitPattern() {
		Coordinate centerCoord;
		GetCenter(height, width, centerCoord.y, centerCoord.x); 

		verticalOffsetAllowed = true;
		horizontalOffsetAllowed = true;

		double t = ((height < width) ? height : width) / 2;

		double halfRadius1 = t * scales[s1];
		double halfRadius2 = t * scales[s2];

		if (halfRadius1 == halfRadius2) {
			halfRadius2 = halfRadius1 / 2.0;
		}

		int angleStep = 20;

		bool swap = false;
		Array<Coordinate> c;
		for (double i = 0; i < 360; i += angleStep) {
			Coordinate newC = Polygon::ComputePointGivenAngleAndDistance(i + 2, (swap) ? halfRadius1 : halfRadius2, centerCoord);
			c.push(newC);
			swap = !swap;
		}

		Array<Edge> edges;
		for (int i = 0; i < c.getSize(); i++) {
			edges.push(Edge(c[i], c[(i + 1) % c.getSize()]));
		}

		Polygon p(edges);
		p.plotPolygon(pattern);
		FillInPolygon(p);

	}

public:

	SpikePattern(int height, int width, const double* s) : UnitPattern(height, width, SPIKE) {
		SetScale(GetScalesForPattern(patternType), s);
		GenerateUnitPattern();
	}

};


/**********************************************************************************************
###############################################################################################
##### ARROW
###############################################################################################
**********************************************************************************************/
class ArrowPattern : public UnitPattern {
private:
	const int s1 = 0;
	const int s2 = 1;

	void GenerateUnitPattern() {
		Coordinate centerCoord;
		GetCenter(height, width, centerCoord.y, centerCoord.x); 

		verticalOffsetAllowed = true;
		horizontalOffsetAllowed = true;

		double t = ((height < width) ? height : width) / 2;

		double heightRadius = t * scales[s1];
		double widthRadius = t * scales[s2];
		double stemRadius = widthRadius / 2;

		Array<Coordinate> c;
		c.push(Coordinate(centerCoord.y - heightRadius, centerCoord.x));
		c.push(Coordinate(centerCoord.y, centerCoord.x + widthRadius));
		c.push(Coordinate(centerCoord.y, centerCoord.x + stemRadius));
		c.push(Coordinate(centerCoord.y + heightRadius, centerCoord.x + stemRadius));
		c.push(Coordinate(centerCoord.y + heightRadius, centerCoord.x - stemRadius));
		c.push(Coordinate(centerCoord.y, centerCoord.x - stemRadius));
		c.push(Coordinate(centerCoord.y, centerCoord.x - widthRadius));

		Array<Edge> edges;
		for (int i = 0; i < c.getSize(); i++) {
			edges.push(Edge(c[i], c[(i + 1) % c.getSize()]));
		}

		Polygon p(edges);
		p.plotPolygon(pattern);

		/******************
			Relatively garbage code to fill in difficult shapes
		**********************/
		Array<Coordinate> loneEdgePoints;
		Array<int> edgePointCounter;
		for (int i = 0; i < height; i++) {
			edgePointCounter.push(0);
		}

		for (int i = 0; i < c.getSize(); i++) {
			edgePointCounter[c[i].y] += 1;
			pattern[c[i].y][c[i].x] = 1;
		}

		int minHeight = 0, maxHeight = 0;
		for (int i = 0; i < edgePointCounter.getSize(); i++) {
			if (edgePointCounter[i] >= 1) {
				minHeight = i;
				break;
			}
		}

		for (int i = ((int)edgePointCounter.getSize()) - 1; i >= 0; i--) {
			if (edgePointCounter[i] >= 1) {
				maxHeight = i;
				break;
			}
		}

		bool foundMinCoord = false, foundMaxCoord = false;
		Coordinate minCoordinate;
		Coordinate maxCoordinate;
		for (int j = 0; j < c.getSize(); j++) {

			if (minHeight == c[j].y) {
				if (foundMinCoord) {
					if (minCoordinate.x > c[j].x) {
						minCoordinate = c[j];
					}
				}
				else {
					minCoordinate = c[j];
					foundMinCoord = true;
				}
			}

			if (maxHeight == c[j].y) {
				if (foundMaxCoord) {
					if (maxCoordinate.x > c[j].x) {
						maxCoordinate = c[j];
					}
				}
				else {
					maxCoordinate = c[j];
					foundMaxCoord = true;
				}
			}

		}

		loneEdgePoints.push(minCoordinate);
		loneEdgePoints.push(maxCoordinate);

		FillInPolygon(loneEdgePoints);
	}

public:

	ArrowPattern(int height, int width, const double* s) : UnitPattern(height, width, ARROW) {
		SetScale(GetScalesForPattern(patternType), s);
		GenerateUnitPattern();
	}

};


/**********************************************************************************************
###############################################################################################
##### TILDE
###############################################################################################
**********************************************************************************************/
class TildePattern : public UnitPattern {
private:
	const int s1 = 0;
	const int s2 = 1;

	void GenerateUnitPattern() {
		Coordinate centerCoord;
		GetCenter(height, width, centerCoord.y, centerCoord.x); 

		verticalOffsetAllowed = true;
		horizontalOffsetAllowed = true;

		double t = ((height < width) ? height : width) / 2;

		double halfRadius1 = t * scales[s1] / 2.0;
		double halfRadius2 = t * scales[s2] / 2.0;

		int angleStep = 1;

		Array<Coordinate> c;

		for (double i = 270; i >= 90; i -= angleStep) {
			Coordinate newC = Polygon::ComputePointGivenAngleAndDistance(i, halfRadius1, Coordinate(centerCoord.y + halfRadius2, centerCoord.x - halfRadius1));
			c.push(newC);
		}

		for (double i = 270; i < 360; i += angleStep) {
			Coordinate newC = Polygon::ComputePointGivenAngleAndDistance(i, halfRadius1, Coordinate(centerCoord.y + halfRadius2, centerCoord.x + halfRadius1));
			c.push(newC);
		}
		for (double i = 0; i < 90; i += angleStep) {
			Coordinate newC = Polygon::ComputePointGivenAngleAndDistance(i, halfRadius1, Coordinate(centerCoord.y + halfRadius2, centerCoord.x + halfRadius1));
			c.push(newC);
		}

		c.removeDuplicates();

		int startingPoint = c.getSize() - 1;
		for (int j = 1; j < halfRadius2; j++) {
			for (int i = startingPoint; i >= 0; i--) {
				c.push(Coordinate(c[i].y - j, c[i].x));
			}
		}
		
		for (int i = 0; i < c.getSize(); i++) {
			pattern[c[i].y][c[i].x] = 1;
		}

	}

public:

	TildePattern(int height, int width, const double* s) : UnitPattern(height, width, TILDE) {
		SetScale(GetScalesForPattern(patternType), s);
		GenerateUnitPattern();
	}

};


/**********************************************************************************************
###############################################################################################
##### ZIGZAG
###############################################################################################
**********************************************************************************************/
class ZigzagPattern : public UnitPattern {
private:
	const int s1 = 0;
	const int s2 = 1;

	void GenerateUnitPattern() {
		Coordinate centerCoord;
		GetCenter(height, width, centerCoord.y, centerCoord.x); 

		verticalOffsetAllowed = true;
		horizontalOffsetAllowed = true;

		double t = ((height < width) ? height : width) / 2;

	
		double halfRadius1 = t * scales[s1];
		double halfRadius2 = t * scales[s2] / 1.5;

		Array<Coordinate> c;

		c.push(Coordinate(centerCoord.y, centerCoord.x - halfRadius1));
		c.push(Coordinate(centerCoord.y + halfRadius2, centerCoord.x - (halfRadius1/3)));
		c.push(Coordinate(centerCoord.y, centerCoord.x + (halfRadius1 / 3)));
		c.push(Coordinate(centerCoord.y + halfRadius2, centerCoord.x + halfRadius1));
		c.push(Coordinate(centerCoord.y, centerCoord.x + halfRadius1));
		c.push(Coordinate(centerCoord.y - halfRadius2, centerCoord.x + (halfRadius1 / 3)));
		c.push(Coordinate(centerCoord.y, centerCoord.x - (halfRadius1 / 3)));
		c.push(Coordinate(centerCoord.y - halfRadius2, centerCoord.x - halfRadius1));


		Array<Edge> edges;
		for (int i = 0; i < c.getSize(); i++) {
			edges.push(Edge(c[i], c[(i + 1) % c.getSize()]));
		}

		Polygon p(edges);
		p.plotPolygon(pattern);
		FillInPolygon(p);

	}

public:

	ZigzagPattern(int height, int width, const double* s) : UnitPattern(height, width, ZIGZAG) {
		SetScale(GetScalesForPattern(patternType), s);
		GenerateUnitPattern();
	}

};


/**********************************************************************************************
###############################################################################################
##### CANE
###############################################################################################
**********************************************************************************************/
class CanePattern : public UnitPattern {
private:
	const int s1 = 0;
	const int s2 = 1;

	void GenerateUnitPattern() {
		Coordinate centerCoord;
		GetCenter(height, width, centerCoord.y, centerCoord.x); 

		verticalOffsetAllowed = true;
		horizontalOffsetAllowed = true;

		double t = ((height < width) ? height : width) / 2;

		double heightRadius = t * scales[s1];
		double widthRadius = t * scales[s2];
		double thickness = widthRadius / 4.0;

		if (heightRadius == widthRadius) {
			widthRadius /= 2;
		}

		Array<Coordinate> c;

		c.push(Coordinate(centerCoord.y + heightRadius, centerCoord.x - widthRadius));
		c.push(Coordinate(centerCoord.y - heightRadius, centerCoord.x - widthRadius));
		c.push(Coordinate(centerCoord.y - heightRadius, centerCoord.x + widthRadius));
		c.push(Coordinate(centerCoord.y - heightRadius / 4.0, centerCoord.x + widthRadius));
		c.push(Coordinate(centerCoord.y - heightRadius / 4.0, centerCoord.x + widthRadius - thickness));
		c.push(Coordinate(centerCoord.y - heightRadius + thickness, centerCoord.x + widthRadius - thickness));
		c.push(Coordinate(centerCoord.y - heightRadius + thickness, centerCoord.x - widthRadius + thickness));
		c.push(Coordinate(centerCoord.y + heightRadius, centerCoord.x - widthRadius + thickness));
		
		Array<Edge> edges;
		for (int i = 0; i < c.getSize(); i++) {
			edges.push(Edge(c[i], c[(i + 1) % c.getSize()]));
		}

		Polygon p(edges);
		p.plotPolygon(pattern);
		FillInPolygon(p);

	}

public:

	CanePattern(int height, int width, const double* s) : UnitPattern(height, width, CANE) {
		SetScale(GetScalesForPattern(patternType), s);
		GenerateUnitPattern();
	}

};


/**********************************************************************************************
###############################################################################################
##### CAT
###############################################################################################
**********************************************************************************************/
class CatPattern : public UnitPattern {
private:
	const int s1 = 0;

	void GenerateUnitPattern() {
		Coordinate centerCoord;
		GetCenter(height, width, centerCoord.y, centerCoord.x); 

		verticalOffsetAllowed = true;
		horizontalOffsetAllowed = true;

		double t = ((height < width) ? height : width) / 2;

		double radius = t * scales[s1];
		double halfRadius = radius / 2.0;
		double fourthRadius = radius / 4.0;
		double eigthRadius = radius / 8.0;
	
		Array<Coordinate> c;

		c.push(Coordinate(centerCoord.y + radius, centerCoord.x - fourthRadius));	/* Left Chin */
		c.push(Coordinate(centerCoord.y + radius, centerCoord.x + fourthRadius));	/* Right chin */
		c.push(Coordinate(centerCoord.y + halfRadius, centerCoord.x + halfRadius + fourthRadius)); /* right cheek */
		c.push(Coordinate(centerCoord.y, centerCoord.x + halfRadius + fourthRadius + (eigthRadius / 2))); /* right face */
		c.push(Coordinate(centerCoord.y - fourthRadius, centerCoord.x + halfRadius + fourthRadius)); /* right ear connect */
		c.push(Coordinate(centerCoord.y - radius, centerCoord.x + halfRadius));		/* Right ear tip */
		c.push(Coordinate(centerCoord.y - halfRadius, centerCoord.x + fourthRadius));	/* right top of head */
		c.push(Coordinate(centerCoord.y - halfRadius, centerCoord.x - fourthRadius));	/* left top of head */
		c.push(Coordinate(centerCoord.y - radius, centerCoord.x - halfRadius));		/* left ear tip */
		c.push(Coordinate(centerCoord.y - fourthRadius, centerCoord.x - halfRadius - fourthRadius));	/* left ear connect */
		c.push(Coordinate(centerCoord.y, centerCoord.x - halfRadius - fourthRadius - (eigthRadius / 2))); /* left face */
		c.push(Coordinate(centerCoord.y + halfRadius, centerCoord.x - halfRadius - fourthRadius)); /* left cheek */

		Array<Edge> edges;
		for (int i = 0; i < c.getSize(); i++) {
			edges.push(Edge(c[i], c[(i + 1) % c.getSize()]));
		}

		Polygon p(edges);
		p.plotPolygon(pattern);
		FillInPolygon(p);

	}

public:

	CatPattern(int height, int width, const double* s) : UnitPattern(height, width, CANE) {
		SetScale(GetScalesForPattern(patternType), s);
		GenerateUnitPattern();
	}

};

/************************************************************
#############################################################
#   Pattern Class
#############################################################
#
#   Objects of this class contain an image. The image
#	will be made up of 1 to many unit patterns
#
************************************************************/
class Pattern {
private:
	int height = 0;				/* height of the image */
	int width = 0;				/* width of the image */
	int horizontalOffset = 0;	/* space between unit patterns horizontally */
	int verticalOffset = 0;		/* space between unit patterns vertically */
	bool clipping = false;		/* are unit patterns allowed to clip off the edge */
	bool centerPattern = true;	/* should unit patterns be centered on the image */
	PatternType patternType = DEFAULT_PATTERN;	/* classification of the image */
		
	char** canvas = nullptr;	/* pointer for the image */

	/* function to return memory used */
	void clear() {
		for (int i = 0; i < height; i++) {
			delete[] canvas[i];
		}
		delete[] canvas;
		canvas = nullptr;
		height = 0;
		width = 0;
		horizontalOffset = 0;
		verticalOffset = 0;
	}

	/* helper function to convert a bit to a char */
	char ToString(char p) { return (p == 0) ? '0' : '1'; }

	/* helper function to represent pixels as whitespace of filled-space in a terminal */
	string ToSymbol(char p) { return (p == 0) ? "." : "#"; }

public:
	/* default ctor */
	Pattern() { }

	/* parameter constructor */
	Pattern(PatternType patternType,int height, int width, int verticalOffset, int horizontalOffset, bool clipping, bool center, const Array<UnitPattern*>& unitPatterns, const Array<int> &patternSet)
		: verticalOffset(verticalOffset), horizontalOffset(horizontalOffset)
		, height(height), width(width)
		, centerPattern(center), clipping(clipping)
		, patternType(patternType)
	{
		/* make sure dimensions will allow for a true center */
		UnitPattern::DetermineHeightAndWidthWithTrueCenter(height, width);

		/* allocate memory needed */
		canvas = new char* [height];
		for (unsigned h = 0; h < height; h++) {
			canvas[h] = new char[width];
			for (int w = 0; w < width; w++) {
				canvas[h][w] = 0;
			}
		}

		/* provided the set of unit patterns (and the ones to use), plot the image */
		GeneratePattern(unitPatterns, patternSet);
	}

	/* parameter ctor to generate pattern using just one unit pattern */
	Pattern(PatternType patternType, int height, int width, int verticalOffset, int horizontalOffset, bool clipping, bool center, UnitPattern* unitPattern)
		: verticalOffset(verticalOffset), horizontalOffset(horizontalOffset)
		, height(height), width(width) 
		, centerPattern(center), clipping(clipping)
		, patternType(patternType)
	{
		/* make sure dimensions will allow for a true center */
		UnitPattern::DetermineHeightAndWidthWithTrueCenter(height, width);

		/* allocate memory needed */
		canvas = new char* [height];
		for (unsigned h = 0; h < height; h++) {
			canvas[h] = new char[width];
			for (int w = 0; w < width; w++) {
				canvas[h][w] = 0;
			}
		}

		/* using just the "first" unit pattern in the "set" */
		Array<int> pattern;
		pattern.push(0);
		
		/* creating a set of a single unit pattern */
		Array<UnitPattern*> up;
		up.push(unitPattern);

		/* plot the image */
		GeneratePattern(up, pattern);
	}

	/* Destructor */
	~Pattern() {
		clear();
	}

	/* assignment operator for deep copy */
	void operator=(const Pattern& copy) {
		clear();
		verticalOffset = copy.verticalOffset;
		horizontalOffset = copy.verticalOffset;
		height = copy.height;
		width = copy.width;
		clipping = copy.clipping;
		centerPattern = copy.centerPattern;
		canvas = new char* [height];
		for (int i = 0; i < height; i++) {
			canvas[i] = new char[width];
			for (int j = 0; j < width; j++) {
				canvas[i][j] = copy.canvas[i][j];
			}
		}
	}

	/* copy constructor for deep copy */
	Pattern(const Pattern& copy) {
		(*this) = copy;
	}

	/* helper function for debugging */
	void PrintPattern(ostream& out) {
		string s = "";
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				s += ToSymbol(canvas[i][j]);
			}
			s += "\n";
		}
		s += "\n";

		out << s;
	}

	/**************************************************************
	* GeneratePattern
	***************************************************************
	* Given a set of different unit patterns and the specific ones
	* to use, plot them on the canvas 
	**************************************************************/
	void GeneratePattern(const Array<UnitPattern*>& unitPatterns, const Array<int>& patternSet) {
		
		if (unitPatterns.getSize() == 0) {
			return;
		}

		/* Unit Pattern Dimensions */
		int unitHeight = unitPatterns.at(0)->GetHeight();
		int unitWidth = unitPatterns.at(0)->GetWidth();

		/* Total space taken up by a unit and it's offset */
		double tHeightSpace = (unitHeight + verticalOffset);
		double tWidthSpace = (unitWidth + horizontalOffset);

		/* The number of total (unit + offset) that can fit in the pattern space */
		double tHeightUnits;
		double tWidthUnits;

		/* if clipping is allowed */
		if (clipping) {
			/* take the ceiling of the number of units that can fit in this pattern */
			tHeightUnits = ceil((double)height / tHeightSpace);
			tWidthUnits = ceil((double)width / tWidthSpace);
		}
		else { /* if no clipping */
			/* take the floor of the number of units that can fit in this pattern */
			tHeightUnits = floor((double)height / tHeightSpace);
			tWidthUnits = floor((double)width / tWidthSpace);
		}

		/* The space needed to FULLY fit everything */
		double virtualHeight = tHeightUnits * tHeightSpace;
		double virtualWidth = tWidthUnits * tWidthSpace;

		/* Where the pattern begins */
		int startHeight = 0;
		int startWidth = 0;

		/* Where the pattern ends */
		int endHeight = tHeightUnits * tHeightSpace;
		int endWidth = tWidthUnits * tWidthSpace;

		/* if centering the pattern */
		if (centerPattern) {
			/* we must adjust where the start/end height/width are */
			double heightDiff = virtualHeight - height;
			double widthDiff = virtualWidth - width;
	
			startHeight = (((-1.0 * heightDiff) / 2.0) + ((double)verticalOffset / 2.0));
			startWidth = (((-1.0 * widthDiff) / 2.0) + ((double)horizontalOffset / 2.0));

			endHeight = height + ((heightDiff / 2.0) - ((double)verticalOffset / 2.0));
			endWidth = width + ((widthDiff / 2.0) - ((double)horizontalOffset / 2.0));

		}

		int heightStep = unitHeight + verticalOffset;	/* The amount of height steps to take whilst pasting the unit pattern */
		int widthStep = unitWidth + horizontalOffset;	/* The amount of width steps to take whilst pasting the unit pattern */
		int innerHeightLimit = 0;	/* Used inside the loop to limit the inner for loops */
		int innerWidthLimit = 0;	/* Used inside the loop to limit the inner for loops */

		int slider = 0; /* used to switch between unit patterns in the set */

		/* for the number of unit patterns height-wise */
		for (int oH = startHeight; oH < endHeight; oH += heightStep) {
			innerHeightLimit = oH + unitHeight;
			innerHeightLimit = (innerHeightLimit > height) ? height : innerHeightLimit;
			/* for the number of unit patterns width-wise */
			for (int oW = startWidth; oW < endWidth; oW += widthStep) {
				innerWidthLimit = oW + unitWidth;
				innerWidthLimit = (innerWidthLimit > width) ? width : innerWidthLimit;
				/* copy the unit pattern onto the canvas */
				for (int iH = ((oH >= 0) ? oH : 0); iH < innerHeightLimit; iH += 1) {
					for (int iW = ((oW >= 0) ? oW : 0); iW < innerWidthLimit; iW += 1) {
						canvas[iH][iW] = unitPatterns.at(patternSet.at(slider))->At(iH - oH, iW - oW);
					}
				}
				/* switch to the next unit pattern */
				slider = ((slider + 1) % (unitPatterns.getSize()));
			}
		}

		/* drops mic */
	}

	/* function to represent image as a string of bits - used to export data for ML */
	string GetRawDataAsString() const {
		string s = GetNameForPattern(patternType) + "," + to_string(height) + "," + to_string(width) + ",";
		for (int h = 0; h < height; h++) {
			for (int w = 0; w < width; w++) {
				s += to_string(canvas[h][w]);
			}
		}
		return s;
	}

	/* function to copy canvas into a pixel vector and use the bitmap class which I got from Kevin Buffardi (check the file) */
	void SavePatternToBmp(string fileName) {
		PixelMatrix pm;
		int colorVal;
		for (int h = 0; h < height; h++) {
			pm.push_back(vector<Pixel>());
			for (int w = 0; w < width; w++) {
				colorVal = (canvas[h][w] == 0) ? 255 : 0;
				Pixel p;
				p.red = colorVal;
				p.blue = colorVal;
				p.green = colorVal;
				pm[h].push_back(p);
			}
		}

		Bitmap bm;
		bm.fromPixelMatrix(pm);

		bm.save(fileName);
	}

};
