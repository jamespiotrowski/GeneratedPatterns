#pragma once

#include "Combination.h"
#include <string>
#include <iostream>
#include <cmath>

using namespace std;

#define M_PI 3.141592653589793;

/**************************************************************
* RoundDouble
***************************************************************
* Function to round a double as you would in grade school
* 
* if 0.5 or above, round up. If < 0.5, round down.
**************************************************************/
int RoundDouble(double d) {
	int i = d;
	double decimal = d - i;
	return (decimal >= 0.5) ? i + 1 : i;
}

/************************************************************
#############################################################
#   Coordinate Class
#############################################################
#
#   Class used to represent coordinates in a 2D grid. 
************************************************************/
struct Coordinate {
	int y = 0;	/* height dimension */
	int x = 0;	/* width dimension */

	/* Default constructor */
	Coordinate() { }		

	/* Parameter constructor */
	Coordinate(int y, int x) : x(x), y(y) { }	

	/* ToString function for debugging */
	string ToString() const { return ("{" + to_string(y) + "," + to_string(x) + "}"); }

	/* equality operator to determine if two coords are the same - needed to remove duplicate coords */
	bool operator==(const Coordinate& c) const { return ((y == c.y) && (x == c.x)); }

	/* lte operator to allow for some sorting of an array of coords */
	bool operator<=(const Coordinate& c) const {
		if (this->y < c.y) { return true; }		/* prioritize y */
		if (this->y > c.y) { return false; }	
		return (this->x <= c.x);				/* else, check x */
	}

	/* gt operator to allow for some sorting of an array of coords */
	bool operator>(const Coordinate& c) const { return !((*this) <= c); }	/* literally just NOT the lte */
};

/************************************************************
#############################################################
#   Angle Class
#############################################################
#
#   Class used to represent an angle in a polygon
#
#	This is needed for determining if a point is within 
#	a polygon - angles make that calculation difficult 
#	so this class provides functionality to make decisions 
#	based on the angles attributes. 
************************************************************/
class Angle {
public:
	Coordinate c;			/* Coordinate at which the angle exists */
	bool intersect = true;	/* Field to determine if a horizontal ray at the same height would intersect or "touch" the angle */
	
	/* 
	this intersect field is expected to be populated before the angle is used in any calculation. It cannot be
	inferred from just the single coordinate. In order to infer if the angle is intersected automagically, this class 
	would need to store two edges (3 coordinates)
	*/

	/* default constructor */
	Angle() : c(0, 0), intersect(false) { }

	/* parameter constructor */
	Angle(Coordinate c1, bool i) : c(c1) {
		intersect = i;
	}

	/* assignment operator - not required but was implemented in an attempt to fix a silly bug. It does not hurt existing here */
	void operator=(const Angle& copy) {
		c = copy.c;
		intersect = copy.intersect;
	}

	/* copy constructor - not required but was implemented in an attempt to fix a silly bug. It does not hurt existing here */
	Angle(const Angle& copy) {
		(*this) = copy;
	}

	bool operator==(const Angle& a) { return this->c == a.c; }		/* equality operator - needed to determine if there are duplicate angles for the same polygon */
	bool operator<=(const Angle& a) { return (this->c <= a.c); }	/* lte operator - needed to provide sorting functionality */
	bool operator>(const Angle& a) { return !((*this) <= a); }		/* gt operator - needed to provide sorting functionality */

	/* to string function for easy debugging */
	string ToString() const {
		string s = "[" + c.ToString() + ",";
		if (intersect) {
			s += "i]";
		}
		else {
			s += "t]";
		}
		return s;
	}
};

/************************************************************
#############################################################
#   Edge Class
#############################################################
#
#   Class used to represent an edge in a polygon
#
#	This is needed for determining if a point is within
#	a polygon as well as representing a polygon. This class
#	is used to determine how to draw the edges on a 2D pixel
#	grid. 
************************************************************/
class Edge {
public:
	Coordinate c1, c2;		/* two points that represent the edge */
	double slope = 1;		/* slope of the edge */
	double b = 0;			/* intercept of the edge (if it were a line) */
	bool infSlope = false;	/* if edge is completely vertical, slope is infinite can cannot be represented in double */

	/* default constructor */
	Edge() {}

	/* parameter constructor */
	Edge(Coordinate ca, Coordinate cb) {
		/* picking a "dominate" coordinate to base calculations on  - order matters for equality check */
		if (cb > ca) {
			c1 = ca;
			c2 = cb;
		}
		else {
			c1 = cb;
			c2 = ca;
		}

		/* determine if there is an inf slope */
		if ((c2.x - c1.x) == 0) {
			infSlope = true;
		}
		else { /* if not inf slope, compute line stats */
			slope = (double)(c2.y - c1.y) / (double)(c2.x - c1.x);
			b = (double)c1.y - (slope * (double)c1.x);
		}
	}

	/* equality operator to determine if two edges are the same */
	bool operator==(const Edge& e) {
		return (c1 == e.c1) && (c2 == e.c2);
	}

	/* lte operator to sort edges */
	bool operator<=(const Edge& e) {
		return (c1 <= e.c1) && (c2 <= e.c2);
	}

	/* gt operator to sort edges */
	bool operator>(const Edge& e) {
		return !((*this) <= e);
	}

	/* helper function to check if edges share a point - determine if the pair forms an angle */
	static bool SharesPoint(const Edge& edge1, const Edge& edge2) {
		return (edge1.c1 == edge2.c1) || (edge1.c1 == edge2.c2) || (edge1.c2 == edge2.c1) || (edge1.c2 == edge2.c2);
	}

	/* function to get the coordinate shared between two edges - SHOULD NOT BE CALLED IF NOT CHECKED PREVIOUSLY */
	static Coordinate GetSharedPoint(const Edge& edge1, const Edge& edge2) {
		if (edge1.c1 == edge2.c1) { return edge1.c1; }
		if (edge1.c1 == edge2.c2) { return edge1.c1; }
		if (edge1.c2 == edge2.c1) { return edge1.c2; }
		if (edge1.c2 == edge2.c2) { return edge1.c2; }
		cout << "ERROR: Edge::GetSharedPoint - Edges do not share a point, returning (0,0)" << endl;
		return Coordinate(0, 0);
	}

	/* To string function for an edge for easy debugging */
	string ToString() const {
		string s = "[" + c1.ToString() + " - " + c2.ToString() + "] :";
		if (!infSlope) {
			s += to_string(slope) + " ";
		}
		else {
			s += "(inf) ";
		}
		return s;
	}

	/* function helps determine if the edge is perfectly horizontal */
	bool isSlopeZero() const {
		return slope == 0;
	}

	/* represented as a linear equation, function returns x value if provided y coordinate */
	double getValueAtY(int y) const {
		return ((infSlope) ? c2.x : ((((double)y) - b) / slope));
	}
};

/************************************************************
#############################################################
#   Polygon Class
#############################################################
#
#   Class used to represent a polygon
************************************************************/
class Polygon {
private:
	Array<Coordinate> points;	/* Set of all points when drawn on a 2D pixel grid */
	Array<Edge> edges;			/* Set of all edges in the polygon */
	Array<Angle> angles;		/* Set of all angles in the polygon */

	int maxY = 0, minY = 0, maxX = 0, minX = 0; /* fields used to determine how far a polygon extends within the grid */

public:

	/**************************************************************
	* pointWithinPolygonRange
	***************************************************************
	* helper function to efficiently determine if a point should 
	* be checked as inside/outside the polygon
	* 
	* to determine if a point is within a polygon, a ray must be
	* cast horizontally and the number of intersections with the 
	* polygon is counted. To check EVERY point in the grid 
	* is wildly inefficient. This function quickly determines 
	* if a point has the possibility of intersecting any edges
	* of the ploygon 
	**************************************************************/
	bool pointWithinPolygonRange(const Coordinate& c) const {
		return c.y >= minY && c.y <= maxY && c.x <= maxX && c.x >= minX;
	}

	/**************************************************************
	* ComputeStraitLine
	***************************************************************
	* function computes every coordinate on a line drawn from 
	* coord 1 to coord 2. Because an image is a 2D grid, a point 
	* can not be perfectly represented as pixels are whole (i.e. 
	* no partial pixels). This function counts the number of pixels
	* between the points of the edge, determines each pixels 
	* unknown dimension, and then rounds it to a whole number. 
	**************************************************************/
	static Array<Coordinate> ComputeStraitLine(Coordinate c1, Coordinate c2) {
		
		/* an array of coordinates will represent a line on the 2d grid */
		Array<Coordinate> line;	

		/* if y coords are the same, slope is zero and x coords simply increment */
		if (c1.y == c2.y) {
			int greaterX = (c2.x > c1.x) ? c2.x : c1.x;		/* determine right-most x */
			int lesserX = (c2.x > c1.x) ? c1.x : c2.x;		/* determine left-most x */
			for (int i = lesserX + 1; i < greaterX; i++) {	/* for all pixels between the two points */
				line.push(Coordinate(c2.y, i));	/* add a new coordinate with constant y and increasing x */
			}
		}
		/* else, we have a line with a slope! */
		else {

			/* need to determine which distance is greater. if plotting on the lesser distance, points will be missed */
			int numCoordinatesY = (c2.y > c1.y) ? c2.y - c1.y : c1.y - c2.y;	/* determine number of pixels between each y */
			int numCoordinatesX = (c2.x > c1.x) ? c2.x - c1.x : c1.x - c2.x;	/* determine number of pixels between each x */
			int numCoordinates = 0;

			/* fields used to represent the line drawn between the two points */
			double slope = 0;
			double b = 0;
			bool infSlope = false;

			/* compute the slope */
			if ((c2.x - c1.x) == 0) {
				infSlope = true;
			}
			else {
				slope = (double)(c2.y - c1.y) / (double)(c2.x - c1.x);
				b = c1.y - (slope * c1.x);
			}

			/* if greater y distance */
			if (numCoordinatesY > numCoordinatesX) {
				numCoordinates = numCoordinatesY;
				int startY = (c2.y > c1.y) ? c1.y : c2.y;	/* determine where to start */
				Coordinate c;
				double d;
				for (int i = 0; i < numCoordinates; i += 1) {	/* for all pixels to compute */
					c.y = i + startY; /* assign incrementing y value */
					c.x = (infSlope) ? c2.x : RoundDouble((((double)(c.y)) - b) / slope); /* use the linear equation to determine the x */
					line.push(c);	/* add coordinate to line */
				}
			} /* if greator x distance */
			else {
				numCoordinates = numCoordinatesX;
				int startX = (c2.x > c1.x) ? c1.x : c2.x; /* determine where to start */
				Coordinate c;
				double d;
				for (int i = 0; i < numCoordinates; i += 1) { /* for all pixels to compute */
					c.x = i + startX; /* assign incrementing x value */
					c.y = RoundDouble((((double)(c.x) * slope) + b)); /* use the linear equation to determine the y */
					line.push(c); /* add coordinate to line */
				} 
			}
		}
		/* it is (maybe..?) possible the the end-points of the line were already included. if they werent, make sure to include them */
		if (!line.exists(c1)) { line.push(c1); }
		if (!line.exists(c2)) { line.push(c2); }

		/* line is ready for plotting! */
		return line;
	}

	/**************************************************************
	* ComputePointGivenAngleAndDistance
	***************************************************************
	* computes a coordinate given a starting point, angle, 
	* and distance. good for drawing things like circles 
	* and hexagons. 
	* 
	* Angle is based from the x = 0 line
	**************************************************************/
	static Coordinate ComputePointGivenAngleAndDistance(double angle, double distance, Coordinate startingPoint) {
		double piCalc = angle * M_PI;
		piCalc = piCalc / 180.0;
		double x = startingPoint.x + (distance * sin(piCalc));
		double y = startingPoint.y + (distance * cos(piCalc));
		return Coordinate(RoundDouble(y), RoundDouble(x));
	}

	/* compute centroid of 3 points */
	static Coordinate ComputeCentroid(Coordinate c1, Coordinate c2, Coordinate c3) {
		double new_x = ((double)(c1.x + c2.x + c3.x)) / 3.0;
		double new_y = ((double)(c1.y + c2.y + c3.y)) / 3.0;
		return Coordinate(RoundDouble(new_y), RoundDouble(new_x));
	}

	/* determines if a ray drawn from coord c will intersect OR touch the angle a */
	static bool inLineWithAngle(const Coordinate& c, const Angle& a) {
		return (c.y == a.c.y) && (c.x <= a.c.x);
	}

	/* determines if a ray drawn from coord c will intersect the angle a */
	static bool intersectsAngle(const Coordinate& c, const Angle& a) {
		if (c.y == a.c.y) {
			if (c.x <= a.c.x) {
				return a.intersect;
			}
		}
		return false;
	}

	/* determines if a ray drawn from coord c will intersect the edge e */
	static bool intersectsEdge(const Coordinate& c, const Edge& e) {
		if ((c.y <= e.c1.y && c.y >= e.c2.y) || (c.y <= e.c2.y && c.y >= e.c1.y)) {
			double x = e.getValueAtY(c.y);
			return x >= ((double)c.x);
		}
		return false;
	}

	/**************************************************************
	* isInsidePolygon
	***************************************************************
	* determines if a point is within the polygon 
	**************************************************************/
	bool isInsidePolygon(const Coordinate& c) const {
		/* first, if the coord is within some of the polygons dimensions */
		if (pointWithinPolygonRange(c)) {
			int intersections = 0;
			/* iterate over all edges in the set */
			for (int i = 0; i < edges.getSize(); i++) {
				/* If ray would intersect an edge */
				if (intersectsEdge(c, edges.at(i))) { 
					intersections += 1;
				}
			}
			/* iterate over all angles in the set */
			for (int i = 0; i < angles.getSize(); i++) {
				/* If ray would intersect OR touch an angle */
				if (inLineWithAngle(c, angles.at(i))) { 
					/* If ray would intersect an angle */
					if (intersectsAngle(c, angles.at(i))) {
						/* this means the edge DID intersect the polygon */
						intersections -= 1; /* would have double counted for two edges meeting, sub 1 */
					}
					else {
						/* this means the edge didnt intersect the polygon */
						intersections -= 2; /* would have double counted for two edges meeting, sub 2 (because it doesnt intersect the angle) */
					}
				}
			}
			/* if intersections is not even, c is within polygon */
			return ((intersections % 2) != 0);
		}
		return false;
	}

	/* parameter constructor */
	Polygon(Array<Edge> e) : edges(e) {
		/* Remove dupes from list of edges */
		edges.removeDuplicates();

		/* Get all points to plot */
		for (int i = 0; i < edges.getSize(); i++) {
			Array<Coordinate> line = ComputeStraitLine(edges[i].c1, edges[i].c2);
			for (int j = 0; j < line.getSize(); j++) {
				points.push(line[j]);
			}
		}

		/* Remove dupes */
		points.removeDuplicates();

		/* Find angles */
		for (int i = 0; i < edges.getSize(); i++) {
			for (int j = 0; j < edges.getSize(); j++) {
				if (i == j) { /* must check each edge with every other edge. if same edge, skip */
					continue;
				}
				/* If edges share a point */
				if (Edge::SharesPoint(edges[i], edges[j])) {
					/* Get the point */
					Coordinate c = Edge::GetSharedPoint(edges[i], edges[j]);
					/* Get the other 2 coordinates */
					Coordinate other1 = (edges[i].c1 == c) ? edges[i].c2 : edges[i].c1;
					Coordinate other2 = (edges[j].c1 == c) ? edges[j].c2 : edges[j].c1;
					/* Determine if a ray would intersect the two edges or just touch it */
					bool intersection = ((other1.y >= c.y) && (other2.y <= c.y)) || ((other1.y <= c.y) && (other2.y >= c.y));
					Angle a(c, intersection); /* create the angle */
					angles.push(a); /* add to set of angles */
				}
			}
		}
		/* Remove dupes */
		angles.removeDuplicates();

		/* determine the general area that the polygon occupies */
		if (points.getSize() > 0) {
			maxY = points[0].y;
			minY = points[0].y;
			maxX = points[0].x;
			minX = points[0].x;
			for (int i = 1; i < points.getSize(); i++) {
				if (points[i].y > maxY) { maxY = points[i].y; }
				if (points[i].y < minY) { minY = points[i].y; }
				if (points[i].x > maxX) { maxX = points[i].x; }
				if (points[i].x < minX) { minX = points[i].x; }
			}
		}
		else {
			maxY = 0;
			minY = 0;
			maxX = 0;
			minX = 0;
		}
	}

	/* function to plot all points of a polygon on a grid */
	void plotPolygon(char** grid) {
		for (int i = 0; i < points.getSize(); i++) {
			grid[points[i].y][points[i].x] = 1;
		}
	}

	/* print function for debugging */
	void printPolygon() {
		string p = "Points : { ";
		for (int i = 0; i < points.getSize(); i++) {
			p += points[i].ToString() + " ";
		}
		p += " }\n";

		string e = "Edges : { ";
		for (int i = 0; i < edges.getSize(); i++) {
			e += edges[i].ToString() + " ";
		}
		e += " }\n";

		string a = "Angles : { ";
		for (int i = 0; i < angles.getSize(); i++) {
			a += angles[i].ToString() + " ";
		}
		a += " }\n";

		string r = "Width Range: {" + to_string(minX) + " - " + to_string(maxX)
			+ "} | Height Range: {" + to_string(minY) + " - " + to_string(maxY) + "}\n";

		string t = p + e + a + r;
		cout << t << endl;
	}

};
