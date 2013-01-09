#pragma once

#define CPP11_VARDIC_TEMPLATES

#ifdef CPP11_VARDIC_TEMPLATES
#include "v.h"
#else

#include "license.txt"
#include <math.h>

// comment the line below if using this class without glut
#define USING_OPENGL_HELPER_FUNCTIONS

#ifdef USING_OPENGL_HELPER_FUNCTIONS
#include <stdlib.h>	// include stdlib.h before glut.h to easily prevent exit redefinition problems
#include <GL/glut.h>
// modify this if changing V2's data types
#define V_GLVERTEX_METHOD	glVertex2fv
#endif

// drawing and math methods that require the libraries above
#define V_SIN		sin
#define V_COS		cos
#define V_ACOS		acos

// PI defined to reduce dependency on other math libraries
#define V_PI			(3.14159265358979323846)
#define V_2PI			(V_PI*2)
#define V_HALFPI		(V_PI/2)
#define V_QUARTERPI		(V_PI/4)

/**
 * a two dimensional vector class, which includes methods to handle 2D 
 * geometric calculation, including rotation in various circumstances.
 * @author mvaganov@hotmail.com April 2010
 */
template<typename VTYPE>
class V2
{
protected:
public:
	/** position in 2 Dimensional space */
	VTYPE x, y;
	/** how many dimensions a V2<VTYPE> keeps track of (sizeof(V2)/sizeof(VTYPE)) */
	static const int NUM_DIMENSIONS = 2;

	/** to access this class's type later */
	typedef VTYPE type;
	/** the first (initial) dimension */
	static const int _X = 0;
	/** the second dimension */
	static const int _Y = 1;

	/** access X */
	inline const VTYPE & getX()const{return x;}
	/** access Y */
	inline const VTYPE & getY()const{return y;}
	/** mutate X */
	inline void setX(const VTYPE & a_value){x=a_value;}
	/** mutate Y */
	inline void setY(const VTYPE & a_value){y=a_value;}
	/** mutate X */
	inline void addX(const VTYPE & a_value){x+=a_value;}
	/** mutate Y */
	inline void addY(const VTYPE & a_value){y+=a_value;}
	/** @return array containing points of this vector ([V2::X], [V2::Y]) */
	inline const VTYPE * getDimensions()const{return &x;}
	/** @return getDimensions()[a_dimensionField] */
	inline VTYPE getField(const int & a_dimensionField)const{return getDimensions()[a_dimensionField];}
	/** getDimensions()[a_dimensionField]=a_value; */
	inline void setField(const int & a_dimensionField, const VTYPE & a_value){(&x)[a_dimensionField]=a_value;}
	/** @param a_axis {X, Y} */
	inline void flipAxis(const int & a_axis){(&x)[a_axis]*=-1;}
	/** @return flipped */
	V2<VTYPE> flippedAxis(int a_axis){V2<VTYPE> p(*this);p.flipAxis(a_axis);return p;}

	/** resets the value of this vector */
	inline void set(const VTYPE & a_x, const VTYPE & a_y){x = a_x;	y = a_y;}
	/** copy another vector */
	inline void set(const V2<VTYPE> & a_v2d){set(a_v2d.x, a_v2d.y);}
	/** make this a cos/sin of the given degree radian */
	inline void set(const VTYPE & a_piRadians){x=V_COS(a_piRadians);y=V_SIN(a_piRadians);}


	/** default constructor */
	V2():x(0.0),y(0.0){}
	/** complete constructor */
	V2(VTYPE a_x, VTYPE a_y):x(a_x),y(a_y){}
	/** de-serialization constructor */
	V2(VTYPE * a_twoValues):x(a_twoValues[0]),y(a_twoValues[1]){}
	/** turns a pi-radians angle into a vector */
	explicit V2(VTYPE a_piRadians):x(V_COS(a_piRadians)), y(V_SIN(a_piRadians)){}
	/** copy constructor */
	V2(const V2<VTYPE> & v):x(v.x),y(v.y){}

	/** sets x and y to zero */
	inline void zero(){x=y=0;}

	/**
	 * declares a "global" variable in a function, which is OK in a template! 
	 * (can't declare globals in headers otherwise)
	 */
	inline static const V2<VTYPE> & ZERO()			{static V2<VTYPE> ZERO(0,0);			return ZERO;}
	inline static const V2<VTYPE> & ZERO_DEGREES()	{static V2<VTYPE> ZERODEGREES(1,0);	return ZERODEGREES;}

	/**
	 * @return true if both x and y are zero
	 * @note function is not allowed to modify members in V2
	 */
	inline bool isZero() const {return x == 0 && y == 0;}

	/** @return if both x and y are less than the given x and y */
	inline bool isLessThan(const V2<VTYPE> & p)const{return x<p.x&&y<p.y;}
	/** @return if both x and y are greaterthan or equl to the given x and y */
	inline bool isGreaterThanOrEqualTo(const V2<VTYPE> & p)const{return x>=p.x&&y>=p.y;}

	/** @return the squared length of the vector (avoids sqrt) "quadrance" */
	inline VTYPE lengthSq() const{return (VTYPE)(x*x+y*y);}

	/** @return the length of the vector (uses sqrt) */
	inline VTYPE length() const{return sqrt((VTYPE)lengthSq());}

	/** @return dot product of this and v2 */
	inline VTYPE dot(const V2<VTYPE> & v2) const
	{
		return (VTYPE)(x * v2.x + y * v2.y);
	}

	/**
	 * @return positive if v2 is clockwise of this vector 
	 * (assume Y points down, X to right)
	 */
	inline VTYPE sign(const V2<VTYPE> & v2) const{return (x*v2.y)-(y*v2.x);}

	/** @return if this point is clockwise of line a->b */
	bool isCW(V2<VTYPE> const & a, V2<VTYPE> const & b)const{
		return difference(a).sign(b.difference(a)) <= 0;
	}
	/** @return if this point is counter-clockwise of line a->b */
	bool isCCW(V2<VTYPE> const & a, V2<VTYPE> const & b)const{
		return difference(a).sign(b.difference(a)) >= 0;
	}

	/** @return true if this point is inside the given triangle */
	bool isInsideTriangle(V2<VTYPE> const & a, V2<VTYPE> const & b, V2<VTYPE> const & c)const{
		VTYPE signab = difference(a).sign(b.difference(a)), 
				signbc = difference(b).sign(c.difference(b)), 
				signac = difference(c).sign(a.difference(c));
		return(((signab>=0) == (signbc>=0)) && ((signbc>=0) == (signac>=0)))
			||(((signab<=0) == (signbc<=0)) && ((signbc<=0) == (signac<=0)));
	}
	/** triangle points are in clock-wise order */
	bool isInsideTriangleCW(V2<VTYPE> const & a, V2<VTYPE> const & b, V2<VTYPE> const & c)const{
		return isCW(a,b) && isCW(b,c) && isCW(c,a);
	}
	/** triangle points are in counter-clock-wise order */
	bool isInsideTriangleCCW(V2<VTYPE> const & a, V2<VTYPE> const & b, V2<VTYPE> const & c)const{
		return isCCW(a,b) && isCCW(b,c) && isCCW(c,a);
	}

	/** @return if the given polygon has all of it's points in clock-wise order. false if CCW or not convex */
	static bool isPolyCW(V2<VTYPE> * const & points, int points_length){
		for(int i = 0; i < points_length; ++i){
			if(!points[(i+2)%points_length].isCW(points[i], points[(i+1)%points_length]))
				return false;
		}
		return true;
	}

	/** @return if the given polygon has all of it's points in counter-clock-wise order. false if CW or not convex */
	static bool isPolyCCW(V2<VTYPE> * const & points, int points_length){
		for(int i = 0; i < points_length; ++i){
			if(!points[(i+2)%points_length].isCCW(points[i], points[(i+1)%points_length]))
				return false;
		}
		return true;
	}
	/** @return if this point is inside the given polygon, who's points are in clock-wise order */
	bool isInsidePolyCW(V2<VTYPE> * const & points, int points_length)const{
		for(int i = 0; i < points_length; ++i){
			if(isCW(points[i], points[(i+1)%points_length]))
				return false;
		}
		return true;
	}
	/** @return if this point is inside the given polygon, who's points are in counter-clock-wise order */
	bool isInsidePolyCCW(V2<VTYPE> * const & points, int points_length)const{
		for(int i = 0; i < points_length; ++i){
			if(isCCW(points[i], points[(i+1)%points_length]))
				return false;
		}
		return true;
	}

	/** @return true if this point is between points A and B */
	bool isBetween(V2<VTYPE> const & a, V2<VTYPE> const & b)const{
		V2<VTYPE> bracket = b.difference(a);
		bracket.setPerp();
		VTYPE signa = difference(a).sign(bracket), 
				signb = difference(b).sign(bracket); 
		return ((signa >= 0) ^ (signb >= 0));
	}
	
	/** @return the vector perpendicular to this one */
	inline V2<VTYPE> perp() const{return V2(-y, x);}

	void setPerp(){VTYPE t=x;x=-y;y=t;}

	/** @param a_length what to set this vector's magnitude to */
	inline void setLength(const VTYPE & a_length)
	{
		VTYPE l = length();
		// "x = (x * a_length) / l" is more precise than "x *= (a_maxLength/l)"
		x = (x * a_length) / l;
		y = (y * a_length) / l;
	}

	/**
	 * @param a_maxLength x and y adjusts so that the length does not exceed this
	 */
	inline void truncate(const VTYPE & a_maxLength)
	{
		VTYPE max2 = a_maxLength*a_maxLength;
		VTYPE ll = lengthSq();
		if(ll > max2)
		{
			VTYPE l = sqrt(ll);
			// "x = (x * a_maxLength) / l" is more precise than "x *= (a_maxLength/l)"
			x = (x * a_maxLength) / l;
			y = (y * a_maxLength) / l;
		}
	}

	/** @return the quadrance (distance squared) between this vector and v */
	inline VTYPE distanceSq(const V2<VTYPE> & v) const
	{
		VTYPE dx = x-v.x, dy = y-v.y;
		return dx*dx+dy*dy;
	}

	/** @return the pythagorean distance between this vector and v */
	inline VTYPE distance(const V2<VTYPE> & v) const{return sqrt(distanceSq(v));}

	/** @return the manhattan distance between this vector and v */
	inline VTYPE distanceManhattan(const V2<VTYPE> & v)const{
		return abs(v.x-x)+abs(v.y-y);
	}

	/** @return the vector that is the reverse of this vector */
	inline V2<VTYPE> getReverse() const{return V2<VTYPE>(-x, -y);}

	/** @return a new V2<VTYPE> that is the sum of this V2<VTYPE> and v */
	inline V2<VTYPE> sum(const V2<VTYPE> & v) const
	{
		return V2<VTYPE>(x+v.x, y+v.y);
	}

	/** @return a new V2<VTYPE> that is the difference of this V2<VTYPE> and v */
	inline V2<VTYPE> difference(const V2<VTYPE> & v) const
	{
		return V2<VTYPE>(x-v.x, y-v.y);
	}

	/** @return a new V2<VTYPE> that is the product of this V2<VTYPE> and v */
	inline V2<VTYPE> product(const V2<VTYPE> & v) const
	{
		return V2<VTYPE>(x*v.x, y*v.y);
	}

	/** @return a new V2<VTYPE> that is the product of this V2<VTYPE> and v */
	inline V2<VTYPE> product(const VTYPE & a_value) const
	{
		return V2<VTYPE>(x*a_value, y*a_value);
	}

	/** @return a new V2<VTYPE> that is the quotient of this V2<VTYPE> and the given V2*/
	inline V2<VTYPE> quotient(const VTYPE & a_value) const
	{
		return V2<VTYPE>(x/a_value, y/a_value);
	}

	/** @return a new V2<VTYPE> that is the quotient of this V2<VTYPE> and the given V2<VTYPE> */
	inline V2<VTYPE> quotient(const V2<VTYPE> & v) const
	{
		return V2<VTYPE>(x/v.x, y/v.y);
	}

	/** @return this V2<VTYPE> after adding v */
	inline V2<VTYPE> & add(const V2<VTYPE> & v)
	{
		x += v.x;
		y += v.y;
		return *this;
	}

	/** @return this V2<VTYPE> after subtracting v */
	inline V2<VTYPE> & subtract(const V2<VTYPE> & v)
	{
		x -= v.x;
		y -= v.y;
		return *this;
	}

	/** @return this V2<VTYPE> after multiplying v */
	inline V2<VTYPE> & multiply(const VTYPE & a_value)
	{
		x *= a_value;
		y *= a_value;
		return *this;
	}

	/** @return this V2<VTYPE> after multiplying v */
	inline V2<VTYPE> & multiply(const V2<VTYPE> & v)
	{
		x *= v.x;
		y *= v.y;
		return *this;
	}

	/** @return this V2<VTYPE> after dividing v */
	inline V2<VTYPE> & divide(const VTYPE & a_value)
	{
		x /= a_value;
		y /= a_value;
		return *this;
	}

	/** @return this V2<VTYPE> after dividing v */
	inline V2<VTYPE> & divide(const V2<VTYPE> & v)
	{
		x /= v.x;
		y /= v.y;
		return *this;
	}

	/** @return if this V2<VTYPE> is euqal to v */
	inline bool isEqual(const V2<VTYPE> & v) const
	{
		return (x == v.x && y == v.y);
	}

	/** @return true if this point is within a_radius from a_point */
	inline bool isWithin(const VTYPE & a_radius, const V2<VTYPE> & a_point) const
	{
		VTYPE rr = a_radius*a_radius;
		return this->distanceSq(a_point) <= rr;
	}

	/** @return if this point is between the rectangle inscribed by the given corners */
	inline bool isBetweenRect(const V2<VTYPE> & a, const V2<VTYPE> & b)
	{
		return((a.x <= b.x)?(x>=a.x&&x<=b.x):(x>=b.x&&x<=a.x))
			&&((a.y <= b.y)?(y>=a.y&&y<=b.y):(y>=b.y&&y<=a.y));
	}

	/** @return true if the given vector is equivalant to this one */
	inline bool equals(const V2<VTYPE> & v)const{return x == v.x && y == v.y;}

	/** forces vector to have a length of 1 */
	inline V2<VTYPE> & normalize()
	{
		divide(length());
		return *this;
	}

	/** make this direction vector go in the opposite direction */
	void setReverse()
	{
		x *= -1;
		y *= -1;
	}

	/** a normalized verson of this vector */
	inline V2<VTYPE> normal() const
	{
		V2<VTYPE> norm(*this);
		return norm.normalize();
	}

	/** @return radians between these normalized vector is */
	inline VTYPE piRadians(const V2<VTYPE> & v) const
	{
		return V_COS(dot(v));
	}

	/** @return radians that this normalized vector is */
	inline VTYPE piRadians() const
	{
		return (y<0?-1:1)*piRadians(ZERO_DEGREES());
	}
	
	/** @return how many degrees (standard 360 degree scale) this is */
	inline VTYPE degrees() const
	{
		return ((y > 0)?-1:1)*V_ACOS(x)*180/V_PI;
	}

	/** @param a_normal cos(theta),sin(theta) as x,y values */
	inline void rotate(const V2<VTYPE> & a_normal)
	{
		VTYPE len = length();	// remember length data
		// normalize()	// turn vector into simple angle, lose length data
		divide(len);	// same as normalize, but one less function call
		// calculate turn in-place within a new data structure
		V2<VTYPE> turned(
			// x_ = x*cos(theta) - y*sin(theta)
			x*a_normal.x - y*a_normal.y, 
			// y_ = x*sin(theta) + y*cos(theta)
			x*a_normal.y + y*a_normal.x);
		// memory copy of structure
		*this = turned;
		// put length data back into normalized vector
		multiply(len);
	}

	/**
	 * @param a_normal cos(theta),sin(theta) as x,y values 
	 * NOTE: only use if this is a unit vector!
	 */
	void rotateUnitVectors(V2<VTYPE> a_normal)
	{
		// x_ = x*cos(theta) - y*sin(theta)
		VTYPE x0 = x*a_normal.x - y*a_normal.y; 
		// y_ = x*sin(theta) + y*cos(theta)
		VTYPE y0 = x*a_normal.y + y*a_normal.x;
		x = x0;
		y = y0;
	}

	/** @param a_degreePiRadians in piRadians */
	inline void rotate(const VTYPE & a_degreePiRadians)
	{
		rotate(V2<VTYPE>(V_COS(a_degreePiRadians), V_SIN(a_degreePiRadians)));
	}

	/**
	 * @param a_fwd rotate this point's x axis to match this vector
	 * @param a_side rotate this point's y axis to match this vector
	 */
	inline V2<VTYPE> toWorldSpace(const V2<VTYPE> & a_fwd, const V2<VTYPE> & a_side, 
		const V2<VTYPE> & a_pos) const
	{
		return V2<VTYPE>(
			(a_fwd.x*x) + (a_side.x*y) + (a_pos.x),
			(a_fwd.y*x) + (a_side.y*y) + (a_pos.y));
	}

	/**
	 * @param a_fwd rotate this point's x axis to match this vector
	 * @param a_side rotate this point's y axis to match this vector
	 */
	inline V2<VTYPE> toWorldSpace(const V2<VTYPE> & a_fwd, const V2<VTYPE> & a_side, 
		const V2<VTYPE> & a_pos, const V2<VTYPE> & a_scale) const
	{
		return V2<VTYPE>(
			(a_scale.x*a_fwd.x*x) + (a_scale.y*a_side.x*y) + (a_pos.x),
			(a_scale.x*a_fwd.y*x) + (a_scale.y*a_side.y*y) + (a_pos.y));
	}

	/**
	 * @param a_fwd rotate this point's x axis to match this vector
	 * @param a_side rotate this point's y axis to match this vector
	 */
	inline V2<VTYPE> toWorldSpace(const V2<VTYPE> & a_fwd, const V2<VTYPE> & a_side) const
	{
		return V2<VTYPE>((a_fwd.x*x) + (a_side.x*y), (a_fwd.y*x) + (a_side.y*y));
	}

	/**
	 * @param a_fwd vector of translated x dimension
	 * @param a_side vector of translated y dimension
	 * @param a_origin origin to translate this point in relation to
	 */
	inline V2<VTYPE> toLocalSpace(const V2<VTYPE> & a_fwd, const V2<VTYPE> & a_side, const V2<VTYPE> & a_origin) const
	{
		VTYPE tx = -a_origin.dot(a_fwd);
		VTYPE ty = -a_origin.dot(a_side);
		return V2<VTYPE>(
			(a_fwd.x*x)  + (a_fwd.y*y)  + (tx),
			(a_side.x*x) + (a_side.y*y) + (ty));
	}

	/**
	 * organizes a list of 2D vectors into a circular curve (arc)
	 * @param a_startVector what to start at (normalized vector)
	 * @param a_angle the angle to increment the arc by V2(piRadians)
	 * @param a_list the list of 2D vectors to map along the arc
	 * @param a_arcs the number of elements in a_list
	 */
	static void arc(V2<VTYPE> const & a_startVector, V2<VTYPE> const & a_angle, V2<VTYPE> * const & a_list, int const & a_arcs)
	{
		VTYPE len = a_startVector.length();	// remember length data
		a_list[0] = a_startVector;				// copy starting point for calculations
		a_list[0].divide(len);					// normalize starting point
		V2<VTYPE> * lastPoint = &a_list[0];	// faster memory reference than a_list[i-1]
		// calculate all points in the arc as normals (faster: no division)
		for(int i = 1; i < a_arcs; ++i)
		{
			// calculate rotation in next point
			(lastPoint+1)->set(
				// x_ = x*cos(theta) - y*sin(theta)
				lastPoint->x*a_angle.x - lastPoint->y*a_angle.y, 
				// y_ = x*sin(theta) + y*cos(theta)
				lastPoint->x*a_angle.y + lastPoint->y*a_angle.x);
			++lastPoint;
		}
		if(len != 1)
		{
			// put length data back into normalized vector
			for(int i = 0; i < a_arcs; ++i)
			{
				// embarassingly parallel
				a_list[i].multiply(len);
			}
		}
	}

	/**
	 * ensures wraps this V2's x/y values around the given rectangular range (like a torroid)
	 * @param a_min the minimum x/y
	 * @param a_max the maximum x/y
	 */
	inline void wrapAround(V2<VTYPE> const & a_min, V2<VTYPE> const & a_max)
	{
		VTYPE width = a_max.x - a_min.x;
		VTYPE height= a_max.y - a_min.y;
		while(x < a_min.x){	x += width;	}
		while(x > a_max.x){	x -= width;	}
		while(y < a_min.y){	y +=height;	}
		while(y > a_max.y){	y -=height;	}
	}

	/** @return the position half-way between line a->b */
	inline static V2<VTYPE> between(V2<VTYPE> const & a, V2<VTYPE> const & b)
	{
		V2<VTYPE> average = b.sum(a);
		average.divide(2.0);
		return average;
	}

	/**
	 * @param A,B line 1
	 * @param C,D line 2
	 * @param point __OUT to the intersection of line AB and CD
	 * @param dist __OUT the distance along line AB to the intersection
	 * @return true if intersection occurs between the lines
	 */
	static inline bool lineIntersection(const V2<VTYPE> & A, const V2<VTYPE> & B, 
										const V2<VTYPE> & C, const V2<VTYPE> & D, 
										VTYPE & dist, V2<VTYPE> & point)
	{
		VTYPE rTop = (A.y-C.y)*(D.x-C.x)-(A.x-C.x)*(D.y-C.y);
		VTYPE rBot = (B.x-A.x)*(D.y-C.y)-(B.y-A.y)*(D.x-C.x);
		VTYPE sTop = (A.y-C.y)*(B.x-A.x)-(A.x-C.x)*(B.y-A.y);
		VTYPE sBot = (B.x-A.x)*(D.y-C.y)-(B.y-A.y)*(D.x-C.x);
		if ( (rBot == 0) || (sBot == 0))
		{
			//lines are parallel
			return false;
		}
		VTYPE r = rTop/rBot;
		VTYPE s = sTop/sBot;
		dist = A.distance(B) * r;
		point = A.sum(B.difference(A).product(r));
		return ( (r > 0) && (r < 1) && (s > 0) && (s < 1) );
	}

	/**
	 * @param a_out_closestPoint will be closest point to a_point on line AB
	 * @return true if a_out_closestPoint is actually on line AB
	 */
	static bool closestPointOnLine(const V2<VTYPE> & A, const V2<VTYPE> & B, 
		const V2<VTYPE> & a_point, V2<VTYPE> & a_out_closestPoint)
	{
		V2<VTYPE> r = B.difference(A).perp();
		V2<VTYPE> d = r.product(2).sum(a_point);
		V2<VTYPE> c = a_point.difference(r);
		VTYPE dist;
		bool intersected = lineIntersection(A, B, c, d, dist, a_out_closestPoint);
		return intersected;
	}

	/** @return if circle (a_point,a_radius) crosses line (A,B) */
	static bool lineCrossesCircle(const V2<VTYPE> & A, const V2<VTYPE> & B, 
		const V2<VTYPE> & a_point, const VTYPE & a_radius, V2<VTYPE> & a_out_closePoint)
	{
		bool connectionOnLine = closestPointOnLine(A, B, a_point, a_out_closePoint);
		return(connectionOnLine && a_out_closePoint.distance(a_point) <= a_radius)
			|| a_point.distance(A) < a_radius
			|| a_point.distance(B) < a_radius;
	}

	// overloaded operators
	inline V2<VTYPE> operator-(){return this->getReverse();}
	inline V2<VTYPE> operator+=(V2<VTYPE> const & rhs){return add(rhs);}
	inline V2<VTYPE> operator-=(V2<VTYPE> const & rhs){return subtract(rhs);}
	inline V2<VTYPE> operator*=(VTYPE const & rhs){return multiply(rhs);}
	inline V2<VTYPE> operator/=(VTYPE const & rhs){return divide(rhs);}
	inline V2<VTYPE> operator+(V2<VTYPE> const & rhs)const{return sum(rhs);}
	inline V2<VTYPE> operator-(V2<VTYPE> const & rhs)const{return difference(rhs);}
	inline V2<VTYPE> operator*(VTYPE const & rhs)const{return product(rhs);}
	inline V2<VTYPE> operator/(VTYPE const & rhs)const{return quotient(rhs);}
	inline bool operator==(V2<VTYPE> const & rhs) const{return isEqual(rhs);}
	inline bool operator!=(V2<VTYPE> const & rhs) const{return !isEqual(rhs);}

#ifdef USING_OPENGL_HELPER_FUNCTIONS
// OpenGL/GLUT specific functions

	/** calls glVertex2fv on this data structure */
	inline void glVertex() const
	{
		V_GLVERTEX_METHOD((VTYPE*)this);
	}

	/**
	 * translates the open GL rendering context.
	 * @note: Dont forget to push and pop the matrix!
	 */
	inline void glTranslate() const
	{
		glTranslatef((GLfloat)x, (GLfloat)y, (GLfloat)0);
	}

	/**
	 * scale the open GL rendering context.
	 * @note: Dont forget to push and pop the matrix!
	 */
	inline void glScale() const
	{
		glScalef((GLfloat)x, (GLfloat)y, (GLfloat)1);
	}

	/**
	 * rotate the open GL rendering context.
	 * @note: Dont forget to push and pop the matrix!
	 */
	inline void glRotate() const
	{
		glRotatef(-degrees(), 0, 0, 1);
	}

	/**
	 * draw an OpenGL line from (0,0) to (V2.x, V2.y)
	 * @return true if something was drawn
	 */
	inline void glDraw() const
	{
		glBegin(GL_LINES);
		ZERO().glVertex();
		glVertex();
		glEnd();
	}

	inline bool glDrawTo(const V2<VTYPE> & a_next) const
	{
		glBegin(GL_LINES);
		glVertex();
		a_next.glVertex();
		glEnd();
		return true;
	}

	/**
	 * @param GLenum_mode draws using this GL enum. eg: GL_LINES, GL_LINE_LOOP, GL_LINE_STRIP
	 */
	static inline void glVertexList(V2<VTYPE> * a_list, const int & a_count)
	{
		for(int i = 0; i < a_count; ++i)
		{
			a_list[i].glVertex();
		}
	}

	/** @param a_string to draw at this coordinate */
	void glDrawString(const char * a_string)
	{
		const int lineHeight = 12;
		int linesPrinted = 0, index = 0;
		do{
			glRasterPos2f (x, y+linesPrinted*lineHeight);
			while(a_string[index]){
				if(a_string[index] != '\n' && a_string[index] != '\r')
					glutBitmapCharacter (GLUT_BITMAP_HELVETICA_10, a_string[index]);
				index++;
				if(a_string[index] == '\n'){
					++linesPrinted;
					break;
				}
			}
		}while(a_string[index]);
	}
#endif
};

// V2 using float, useful for all kinds of games and simulations
typedef V2<float> V2F;
// V2 using double, useful for high-precision simulations
typedef V2<double> V2D;
// V2 using int, useful for grids, tables, manhattan distance
typedef V2<int> V2I;

#endif
