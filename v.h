#pragma once

#include "license.txt"
#include <math.h>

// comment the line below if using this class without glut
#define USING_OPENGL_HELPER_FUNCTIONS
#define CPP11_HAS_INITIALIZER_LIST
//#define CPP11_HAS_VARDIC_ARGUMENTS
#define constexpr /* constant expression keyword, a C++11 feature */

#ifdef USING_OPENGL_HELPER_FUNCTIONS
#include <stdlib.h>	// include stdlib.h before glut.h to easily prevent exit redefinition problems
#include <GL/glut.h>
#include <type_traits>
#define VFLOAT	GLfloat
#else
#define VFLOAT	float
#endif

#ifdef CPP11_HAS_INITIALIZER_LIST
#include <initializer_list>
#endif

// drawing and math methods that require the libraries above
#define V_SIN		sin
#define V_COS		cos
#define V_ACOS		acos

// PI defined to reduce dependency on other math libraries
#define V_PI			(3.14159265358979323846)
#define V_2PI			(V_PI*2)
#define V_HALFPI		(V_PI/2)
#define V_QUARTERPI	(V_PI/4)

/** this constant loop will hopefully be unrolled by the compiler **/
#define FOREACH(a)		for(int a = 0; a < NUM_DIMENSIONS; ++a)
#define V_ARR(a)		((T*)a)
#define X			V_ARR(this)[0]
#define Y			V_ARR(this)[1]
#define Z			V_ARR(this)[2]
#define V_X(a)		V_ARR(a)[0]
#define V_Y(a)		V_ARR(a)[1]

/**
 * a N dimensional vector class, which includes methods to handle
 * geometric calculation, including rotation in various circumstances.
 *
 * typename BASE: what composite of scalars is used to structure memory
 * typename T: what *primitive* type the scalar components are
 *
 * TODO ensure that BASE is a list of the same kind of primitives (possible?)
 * TODO replace template arg T with decltype of BASE's components (possible?)
 *
 * Example use of this class:
 * <code>
 * // components named x and y
 * struct __V2F_BASE{float x,y;__V2F_BASE():x(0),y(0){}};
 * // 2 dimensional x/y structure of floats
 * typedef V<float, __V2F_BASE> V2F;
 * </code>
 * @author mvaganov@hotmail.com December 2012
 */
template<typename BASE, typename T>
class V : public BASE
{
protected:
public:
	/** how many dimensions a V<BASE, T> keeps track of (sizeof(V)/sizeof(T)) */
	static const int NUM_DIMENSIONS = sizeof(BASE)/sizeof(T);
	/** to access this class's type later */
	typedef T type;

	/** @return array containing points of this vector */
	T * data(){return (T*)this;}
	/** @return array containing points of this vector */
	inline const T * dataCONST()const{return (T*)this;}
	/** access by reference */
	template<int INDEX>
	inline T & get(){return data()[INDEX];}

	inline T getX()const{return dataCONST()[0];}
	inline T getY()const{return dataCONST()[1];}
	inline T setX(T a_value){return data()[0] = a_value;}
	inline T setY(T a_value){return data()[1] = a_value;}
	/** mutate */
	template<int INDEX>
	inline void set(const T & a_value){data()[INDEX]=a_value;}
	/** mutate */
	template<int INDEX>
	inline void add(const T & a_value){data()[INDEX]+=a_value;}
	/** access @return data()[a_dimensionField] */
	inline T getField(const int & a_dimensionField)const{return dataCONST()[a_dimensionField];}
	/** mutate data()[a_dimensionField]=a_value; */
	inline void setField(const int a_dimensionField, const T a_value){data()[a_dimensionField]=a_value;}
	/** mutate data()[a_dimensionField]+=a_value; */
	inline void addField(const int a_dimensionField, const T a_value){data()[a_dimensionField]+=a_value;}
	/** @param a_axis {X, Y} */
	inline void flipAxis(const int & a_axis){data()[a_axis]*=-1;}
	/** @return flipped */
	V<BASE, T> flippedAxis(int a_axis){V<BASE, T> p(*this);p.flipAxis(a_axis);return p;}

	// vardic template helper functions. hopefully inlined away after compile...
private:
#ifdef CPP11_HAS_VARDIC_ARGUMENTS
	/** terminus for vardic template recursion */
	template<int INDEX, typename... ARGS>
	inline void setVardic(){
		static_assert( (INDEX != NUM_DIMENSIONS-1), "not enough arguments supplied");
	}
	/** vardic template recursion */
	template<int INDEX, typename T_, typename... ARGS>
	inline void setVardic(const T_ a_value, const ARGS ... args)
	{
		static_assert( (INDEX < NUM_DIMENSIONS), "too many arguments supplied");
		set<INDEX>(a_value);
		// tail recursion, hopefully inline-able by the compiler
		setVardic<INDEX+1, ARGS...>(args...);
	}
#endif
public:
#ifdef CPP11_HAS_VARDIC_ARGUMENTS
	/** sets the value of this vector */
	template<typename... ARGS>
	inline void set(const ARGS... args)
	{
		const int NUMARGS = sizeof...(ARGS)-1;
		static_assert( (NUMARGS != NUM_DIMENSIONS), "wrong number of arguments supplied");
		setVardic<0, ARGS...>(args... );
	}
#else
	inline void set(const T x, const T y)
	{
		data()[0] = x;
		data()[1] = y;
	}
#endif
	/** copy another vector */
	inline void set(const V<BASE, T> & a_v)
	{
		FOREACH(i){	setField(i, a_v.dataCONST()[i]);	}
	}
	/** make this a cos/sin of the given degree radian */
	inline void setRadians(const T & a_piRadians)
	{
		setX(V_COS(a_piRadians));
		setY(V_SIN(a_piRadians));
	}


	/** default constructor (calls default constructor of BASE implicitly) */
	V(){}

#ifdef CPP11_HAS_INITIALIZER_LIST
    V( const std::initializer_list <T> & ilist )
    {
        auto it = ilist.begin();
        int index = 0;
        while( it != ilist.end() ) {
        	setField(index++, *it);
            it++;
        }
    }
#endif

#ifdef CPP11_HAS_VARDIC_ARGUMENTS
    template<typename... ARGS>
	/** complete constructor */
	V(const ARGS... args)
	{
		set(args...);
	}
#else
	/** complete constructor */
	V(const T x, const T y)
	{
		set(x,y);
	}

#endif
	/** de-serialization constructor */
	V(T * a_values)
	{
		FOREACH(i){	setField(i, a_values[i]);	}
	}
	/** turns a pi-radians angle into a vector */
	static V<BASE, T> fromPiRadians(T a_piRadians)
	{
		V<BASE, T> v;
		v.setRadians(a_piRadians);
		return v;
	}
	/** copy constructor */
	V(const V<BASE, T> & v){set(v);}

	/**
	 * declares a "global" variable in a function, which is OK in a template!
	 */
	constexpr inline static const V<BASE, T> ZERO()
	{
		static const V<BASE, T> ZERO;
		return ZERO;
	}
	constexpr static const V<BASE, T> ZERO_DEGREES()	{
		static const V<BASE, T> ZERODEGREES(1,0);
		return ZERODEGREES;
	}

	/** sets x and y to zero */
	inline void zero(){*this=ZERO();}

	/**
	 * @return true if both x and y are zero
	 * @note function is not allowed to modify members in V2
	 */
	inline bool isZero() const {
		FOREACH(i){
			if(dataCONST()[i] != 0)
				return false;
		}
		return true;
	}

	/** @return if both x and y are less than the given x and y */
	inline bool isLessThan(const V<BASE, T> & p)const
	{
		FOREACH(i){
			if(V_ARR(this)[i] >= V_ARR(&p)[i])
				return false;
		}
		return true;
	}
	/** @return if both x and y are greaterthan or equl to the given x and y */
	inline bool isGreaterThanOrEqualTo(const V<BASE, T> & p)const
	{
		return !isLessThan(p);
	}

	/** @return the squared length of the vector (avoids sqrt) "quadrance" */
	inline T lengthSq() const
	{
		T sumOfSquares = 0;
		FOREACH(i){
			sumOfSquares += getField(i)*getField(i);
		}
		return sumOfSquares;
	}

	/** @return the length of the vector (uses sqrt) */
	inline T length() const{return sqrt((T)lengthSq());}

	/** @return dot product of this and v2 */
	inline T dot(const V<BASE, T> & v) const
	{
		T dotProduct = 0;
		FOREACH(i){
			dotProduct += getField(i)*v.getField(i);
		}
		return dotProduct;
	}

// Begin 2D vectors ONLY ///////////////////////////////////////////////////////
#define TEST2D	(NUM_DIMENSIONS==2)
#define TEST3D	(NUM_DIMENSIONS==3)
#define TEST4D	(NUM_DIMENSIONS==4)
	/**
	 * @return positive if v2 is clockwise of this vector
	 * (assume Y points down, X to right)
	 */
	T sign(const V<BASE, T> & v2) const
	{
		static_assert(TEST2D,"cannot test sign on non-2D vector");
		return (X*V_Y(&v2)) - (Y*V_X(&v2));
	}

	/** @return if this point is clockwise of line a->b */
	bool isCW(V<BASE, T> const & a, V<BASE, T> const & b)const{
		static_assert(TEST2D,"cannot test CW on non-2D vector");
		return difference(a).sign(b.difference(a)) <= 0;
	}
	/** @return if this point is counter-clockwise of line a->b */
	bool isCCW(V<BASE, T> const & a, V<BASE, T> const & b)const{
		static_assert(TEST2D,"cannot test CCW on non-2D vector");
		return difference(a).sign(b.difference(a)) >= 0;
	}

	/** @return true if this point is inside the given triangle */
	bool isInsideTriangle(V<BASE, T> const & a, V<BASE, T> const & b, V<BASE, T> const & c)const{
		static_assert(TEST2D,"cannot test isInsideTriangle on non-2D vector");
		T signab = difference(a).sign(b.difference(a)),
				signbc = difference(b).sign(c.difference(b)),
				signac = difference(c).sign(a.difference(c));
		return(((signab>=0) == (signbc>=0)) && ((signbc>=0) == (signac>=0)))
			||(((signab<=0) == (signbc<=0)) && ((signbc<=0) == (signac<=0)));
	}
	/** triangle points are in clock-wise order */
	bool isInsideTriangleCW(V<BASE, T> const & a, V<BASE, T> const & b, V<BASE, T> const & c)const{
		static_assert(TEST2D,"cannot test isInsideTriangleCW on non-2D vector");
		return isCW(a,b) && isCW(b,c) && isCW(c,a);
	}
	/** triangle points are in counter-clock-wise order */
	bool isInsideTriangleCCW(V<BASE, T> const & a, V<BASE, T> const & b, V<BASE, T> const & c)const{
		static_assert(TEST2D,"cannot test isInsideTriangleCCW on non-2D vector");
		return isCCW(a,b) && isCCW(b,c) && isCCW(c,a);
	}

	/** @return if the given polygon has all of it's points in clock-wise order. false if CCW or not convex */
	static bool isPolyCW(V<BASE, T> * const & points, int points_length){
		static_assert(TEST2D,"cannot test isPolyCW on non-2D vector");
		for(int i = 0; i < points_length; ++i){
			if(!points[(i+2)%points_length].isCW(points[i], points[(i+1)%points_length]))
				return false;
		}
		return true;
	}

	/** @return if the given polygon has all of it's points in counter-clock-wise order. false if CW or not convex */
	static bool isPolyCCW(V<BASE, T> * const & points, int points_length){
		static_assert(TEST2D,"cannot test isPolyCCW on non-2D vector");
		for(int i = 0; i < points_length; ++i){
			if(!points[(i+2)%points_length].isCCW(points[i], points[(i+1)%points_length]))
				return false;
		}
		return true;
	}
	/** @return if this point is inside the given polygon, who's points are in clock-wise order */
	bool isInsidePolyCW(V<BASE, T> * const & points, int points_length)const{
		static_assert(TEST2D,"cannot test isInsidePolyCW on non-2D vector");
		for(int i = 0; i < points_length; ++i){
			if(isCW(points[i], points[(i+1)%points_length]))
				return false;
		}
		return true;
	}
	/** @return if this point is inside the given polygon, who's points are in counter-clock-wise order */
	bool isInsidePolyCCW(V<BASE, T> * const & points, int points_length)const{
		static_assert(TEST2D,"cannot test isInsidePolyCCW on non-2D vector");
		for(int i = 0; i < points_length; ++i){
			if(isCCW(points[i], points[(i+1)%points_length]))
				return false;
		}
		return true;
	}

	/** @return true if this point is between points A and B */
	bool isBetween(V<BASE, T> const & a, V<BASE, T> const & b)const{
		static_assert(TEST2D,"cannot test isBetween on non-2D vector");
		V<BASE, T> bracket = b.difference(a);
		bracket.setPerp();
		T signa = difference(a).sign(bracket),
				signb = difference(b).sign(bracket);
		return ((signa >= 0) ^ (signb >= 0));
	}

	/** @return the vector perpendicular to this one */
	inline V<BASE, T> perp() const
	{
		static_assert(TEST2D,"cannot get perp from non-2D vector");
		return V<BASE, T>(-Y, X);
	}

	void setPerp()
	{
		static_assert(TEST2D,"cannot set perp on non-2D vector");
		T t=X;X=-Y;Y=t;
	}

	/** @return radians between these normalized vector is */
	inline T piRadians(const V<BASE, T> & v) const
	{
		static_assert(TEST2D,"cannot convert to pi radians on non-2D vector");
		return V_COS(dot(v));
	}

	/** @return radians that this normalized vector is */
	inline T piRadians() const
	{
		static_assert(TEST2D,"cannot convert to pi radians on non-2D vector");
		return (Y<0?-1:1)*piRadians(ZERO_DEGREES());
	}

	/** @return how many degrees (standard 360 degree scale) this is */
	inline T degrees() const
	{
		static_assert(TEST2D,"cannot convert to degrees on non-2D vector");
		return ((Y > 0)?-1:1)*V_ACOS(X)*180/V_PI;
	}

	/** @param a_normal cos(theta),sin(theta) as x,y values */
	inline void rotate(const V<BASE, T> & a_normal)
	{
		static_assert(TEST2D,"cannot rotate non-2D vector using unit vector");
		T len = length();	// remember length data
		// normalize()	// turn vector into simple angle, lose length data
		divide(len);	// same as normalize, but one less function call
		// calculate turn in-place within a new data structure
		V<BASE, T> turned(
			// x_ = x*cos(theta) - y*sin(theta)
			X*V_X(&a_normal) - Y*V_Y(&a_normal),
			// y_ = x*sin(theta) + y*cos(theta)
			X*V_Y(&a_normal) + Y*V_X(&a_normal));
		// memory copy of structure
		*this = turned;
		// put length data back into normalized vector
		multiply(len);
	}

	/**
	 * @param a_normal cos(theta),sin(theta) as x,y values
	 * NOTE: only use if this is a unit vector!
	 */
	void rotateUnitVectors(V<BASE, T> a_normal)
	{
		static_assert(TEST2D,"cannot rotate non-2D vector using unit vector");
		// x_ = x*cos(theta) - y*sin(theta)
		T x0 = X*V_X(&a_normal) - Y*V_Y(&a_normal);
		// y_ = x*sin(theta) + y*cos(theta)
		T y0 = X*V_Y(&a_normal) + Y*V_X(&a_normal);
		X = x0;
		Y = y0;
	}

	/** @param a_degreePiRadians in piRadians */
	inline void rotate(const T & a_degreePiRadians)
	{
		static_assert(TEST2D,"cannot rotate non-2D vector using unit vector");
		rotate(V<BASE, T>(V_COS(a_degreePiRadians), V_SIN(a_degreePiRadians)));
	}

	/**
	 * @param a_fwd rotate this point's x axis to match this vector
	 * @param a_side rotate this point's y axis to match this vector
	 */
	inline V<BASE, T> toWorldSpace(const V<BASE, T> & a_fwd, const V<BASE, T> & a_side,
		const V<BASE, T> & a_pos) const
	{
		static_assert(TEST2D,"cannot rotate non-2D vector using unit vector");
		return V<BASE, T>(
			(V_X(&a_fwd)*X) + (V_X(&a_side)*Y) + (V_X(&a_pos)),
			(V_Y(&a_fwd)*X) + (V_Y(&a_side)*Y) + (V_Y(&a_pos)));
	}

	/**
	 * @param a_fwd rotate this point's x axis to match this vector
	 * @param a_side rotate this point's y axis to match this vector
	 */
	inline V<BASE, T> toWorldSpace(const V<BASE, T> & a_fwd, const V<BASE, T> & a_side,
		const V<BASE, T> & a_pos, const V<BASE, T> & a_scale) const
	{
		static_assert(TEST2D,"cannot rotate non-2D vector using unit vector");
		return V<BASE, T>(
			(V_X(&a_scale)*V_X(&a_fwd)*X) + (V_Y(&a_scale)*V_X(&a_side)*Y) + (V_X(&a_pos)),
			(V_X(&a_scale)*V_Y(&a_fwd)*X) + (V_Y(&a_scale)*V_Y(&a_side)*Y) + (V_Y(&a_pos)));
	}

	/**
	 * @param a_fwd rotate this point's x axis to match this vector
	 * @param a_side rotate this point's y axis to match this vector
	 */
	inline V<BASE, T> toWorldSpace(const V<BASE, T> & a_fwd, const V<BASE, T> & a_side) const
	{
		static_assert(TEST2D,"cannot rotate non-2D vector using unit vector");
		return V<BASE, T>(
				(V_X(&a_fwd)*X) + (V_X(&a_side)*Y), (V_Y(&a_fwd)*X) + (V_Y(&a_side)*Y));
	}

	/**
	 * @param a_fwd vector of translated x dimension
	 * @param a_side vector of translated y dimension
	 * @param a_origin origin to translate this point in relation to
	 */
	inline V<BASE, T> toLocalSpace(const V<BASE, T> & a_fwd, const V<BASE, T> & a_side, const V<BASE, T> & a_origin) const
	{
		static_assert(TEST2D,"cannot rotate non-2D vector using unit vector");
		T tx = -a_origin.dot(a_fwd);
		T ty = -a_origin.dot(a_side);
		return V<BASE, T>(
			(V_X(&a_fwd)*X)  + (V_Y(&a_fwd)*Y)  + (tx),
			(V_X(&a_side)*X) + (V_Y(&a_side)*Y) + (ty));
	}

	/**
	 * organizes a list of 2D vectors into a circular curve (arc)
	 * @param a_startVector what to start at (normalized vector)
	 * @param a_angle the angle to increment the arc by V2(piRadians)
	 * @param a_list the list of 2D vectors to map along the arc
	 * @param a_arcs the number of elements in a_list
	 */
	static void arc(V<BASE, T> const & a_startVector, V<BASE, T> const & a_angle, V<BASE, T> * const & a_list, int const & a_arcs)
	{
		static_assert(TEST2D,"cannot rotate non-2D vector using unit vector");
		T len = a_startVector.length();	// remember length data
		a_list[0] = a_startVector;				// copy starting point for calculations
		a_list[0].divide(len);					// normalize starting point
		V<BASE, T> * lastPoint = &a_list[0];	// faster memory reference than a_list[i-1]
		// calculate all points in the arc as normals (faster: no division)
		for(int i = 1; i < a_arcs; ++i)
		{
			// calculate rotation in next point
			(lastPoint+1)->set(
				// x_ = x*cos(theta) - y*sin(theta)
				V_X(lastPoint)*V_X(&a_angle) - V_Y(lastPoint)*V_Y(&a_angle),
				// y_ = x*sin(theta) + y*cos(theta)
				V_X(lastPoint)*V_Y(&a_angle) + V_Y(lastPoint)*V_X(&a_angle));
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
	 * @param A,B line 1
	 * @param C,D line 2
	 * @param point __OUT to the intersection of line AB and CD
	 * @param dist __OUT the distance along line AB to the intersection
	 * @return true if intersection occurs between the lines
	 */
	static inline bool lineIntersection(const V<BASE, T> & A, const V<BASE, T> & B,
										const V<BASE, T> & C, const V<BASE, T> & D,
										T & dist, V<BASE, T> & point)
	{
		static_assert(TEST2D,"cannot intersect non-2D lines");
		T rTop = (A.y-C.y)*(D.x-C.x)-(A.x-C.x)*(D.y-C.y);
		T rBot = (B.x-A.x)*(D.y-C.y)-(B.y-A.y)*(D.x-C.x);
		T sTop = (A.y-C.y)*(B.x-A.x)-(A.x-C.x)*(B.y-A.y);
		T sBot = (B.x-A.x)*(D.y-C.y)-(B.y-A.y)*(D.x-C.x);
		if ( (rBot == 0) || (sBot == 0))
		{
			//lines are parallel
			return false;
		}
		T r = rTop/rBot;
		T s = sTop/sBot;
		dist = A.distance(B) * r;
		point = A.sum(B.difference(A).product(r));
		return ( (r > 0) && (r < 1) && (s > 0) && (s < 1) );
	}

	/**
	 * @param a_out_closestPoint will be closest point to a_point on line AB
	 * @return true if a_out_closestPoint is actually on line AB
	 */
	static bool closestPointOnLine(const V<BASE, T> & A, const V<BASE, T> & B,
		const V<BASE, T> & a_point, V<BASE, T> & a_out_closestPoint)
	{
		static_assert(TEST2D,"cannot intersect non-2D lines");
		V<BASE, T> r = B.difference(A).perp();
		V<BASE, T> d = r.product(2).sum(a_point);
		V<BASE, T> c = a_point.difference(r);
		T dist;
		bool intersected = lineIntersection(A, B, c, d, dist, a_out_closestPoint);
		return intersected;
	}

	/** @return if circle (a_point,a_radius) crosses line (A,B) */
	static bool lineCrossesCircle(const V<BASE, T> & A, const V<BASE, T> & B,
		const V<BASE, T> & a_point, const T & a_radius, V<BASE, T> & a_out_closePoint)
	{
		static_assert(TEST2D,"cannot intersect non-2D lines");
		bool connectionOnLine = closestPointOnLine(A, B, a_point, a_out_closePoint);
		return(connectionOnLine && a_out_closePoint.distance(a_point) <= a_radius)
			|| a_point.distance(A) < a_radius
			|| a_point.distance(B) < a_radius;
	}

//  END  2D vectors ONLY ///////////////////////////////////////////////////////

	/** @param a_length what to set this vector's magnitude to */
	inline void setLength(const T & a_length)
	{
		T l = length();
		FOREACH(i){
			setField(i, (getField(i) * a_length) / l);
		}
	}

	/**
	 * @param a_maxLength x and y adjusts so that the length does not exceed this
	 */
	inline void truncate(const T & a_maxLength)
	{
		T max2 = a_maxLength*a_maxLength;
		T ll = lengthSq();
		if(ll > max2)
		{
			T l = sqrt(ll);
			setLength(l);
		}
	}

	/** @return the quadrance (distance squared) between this vector and v */
	inline T distanceSq(const V<BASE, T> & v) const
	{
		V<BASE, T> d = difference(v);
		return d.lengthSq();
	}

	/** @return the pythagorean distance between this vector and v */
	inline T distance(const V<BASE, T> & v) const{return sqrt(distanceSq(v));}

	/** @return the manhattan distance between this vector and v */
	inline T distanceManhattan(const V<BASE, T> & v)const{
		T dist = 0;
		FOREACH(i){
			dist += abs(v.getField(i)-getField(i));
		}
		return dist;
	}

#define NEWRESULT(r, i, ex)	\
V<BASE, T> r;	\
FOREACH(i){	\
	r.setField(i, ex); \
}
#define COMPOUND_(v, i, ex)	\
FOREACH(i){	\
	data()[i] ex; \
}
	/** @return the vector that is the reverse of this vector */
	inline V<BASE, T> getReverse() const
	{
		NEWRESULT(r, i, -getField(i))
		return r;
	}

	/** @return a new V<BASE, T> that is the sum of this V<BASE, T> and v */
	inline V<BASE, T> sum(const V<BASE, T> & v) const
	{
		NEWRESULT(r, i, getField(i)+v.getField(i))
		return r;
	}

	/** @return a new V<BASE, T> that is the difference of this V<BASE, T> and v */
	inline V<BASE, T> difference(const V<BASE, T> & v) const
	{
		NEWRESULT(r, i, getField(i)-v.getField(i))
		return r;
	}

	/** @return a new V<BASE, T> that is the product of this V<BASE, T> and v */
	inline V<BASE, T> product(const V<BASE, T> & v) const
	{
		NEWRESULT(r, i, getField(i)*v.getField(i))
		return r;
	}

	/** @return a new V<BASE, T> that is the product of this V<BASE, T> and v */
	inline V<BASE, T> product(const T & a_value) const
	{
		NEWRESULT(r, i, getField(i)*a_value)
		return r;
	}

	/** @return a new V<BASE, T> that is the quotient of this V<BASE, T> and the given V2*/
	inline V<BASE, T> quotient(const T & a_value) const
	{
		NEWRESULT(r, i, getField(i)/a_value)
		return r;
	}

	/** @return a new V<BASE, T> that is the quotient of this V<BASE, T> and the given V<BASE, T> */
	inline V<BASE, T> quotient(const V<BASE, T> & v) const
	{
		NEWRESULT(r, i, getField(i)/v.getField(i))
		return r;
	}

	/** @return this V<BASE, T> after adding v */
	inline V<BASE, T> & add(const V<BASE, T> & v)
	{
		COMPOUND_(v, i, += v.getField(i))
		return *this;
	}

	/** @return this V<BASE, T> after subtracting v */
	inline V<BASE, T> & subtract(const V<BASE, T> & v)
	{
		COMPOUND_(v, i, -= v.getField(i))
		return *this;
	}

	/** @return this V<BASE, T> after multiplying v */
	inline V<BASE, T> & multiply(const T & a_value)
	{
		COMPOUND_(v, i, *= a_value)
		return *this;
	}

	/** @return this V<BASE, T> after multiplying v */
	inline V<BASE, T> & multiply(const V<BASE, T> & v)
	{
		COMPOUND_(v, i, *= v.getField(i))
		return *this;
	}

	/** @return this V<BASE, T> after dividing v */
	inline V<BASE, T> & divide(const T & a_value)
	{
		COMPOUND_(v, i, /= a_value)
		return *this;
	}

	/** @return this V<BASE, T> after dividing v */
	inline V<BASE, T> & divide(const V<BASE, T> & v)
	{
		COMPOUND_(v, i, /= v.getField(i))
		return *this;
	}

	/** @return if this V<BASE, T> is equal to v */
	inline bool isEqual(const V<BASE, T> & v) const
	{
		FOREACH(i){
			if(getField(i) != v.getField(i))
				return false;
		}
		return true;
	}

	/** @return true if this point is within a_radius from a_point */
	inline bool isWithin(const T & a_radius, const V<BASE, T> & a_point) const
	{
		T rr = a_radius*a_radius;
		return this->distanceSq(a_point) <= rr;
	}

	inline static void seperateMinMax(V<BASE, T> & min, V<BASE, T> & max)
	{
		T temp;
		FOREACH(i){
			if(min.getField(i) > max.getField(i)){
				temp = min.getField(i);
				min.setField(i, max.getField(i));
				max.setField(i, temp);
			}
		}
	}

	/** @return if this point is between the rectangle inscribed by the given corners */
	inline bool isBetweenRect(const V<BASE, T> & a, const V<BASE, T> & b)
	{
		V<BASE, T> min_(a);
		V<BASE, T> max_(b);
		seperateMinMax(a,b);
		FOREACH(i){
			if(getField(i) < min_.getField(i) || getField(i) > max_.getField(i))
				return false;
		}
		return true;
	}

	/** @return true if the given vector is equivalant to this one */
	inline bool equals(const V<BASE, T> & v)const
	{
		FOREACH(i){
			if(getField(i) != v.getField(i))
				return false;
		}
		return true;
	}

	/** forces vector to have a length of 1 */
	inline V<BASE, T> & normalize()
	{
		divide(length());
		return *this;
	}

	/** make this direction vector go in the opposite direction */
	void setReverse()
	{
		FOREACH(i){
			setField(i, getField(i)*-1);
		}
	}

	/** a normalized verson of this vector */
	inline V<BASE, T> normal() const
	{
		V<BASE, T> norm(*this);
		return norm.normalize();
	}

	/**
	 * ensures wraps this V2's x/y values around the given rectangular range (like a torroid)
	 * @param a_min the minimum x/y
	 * @param a_max the maximum x/y
	 */
	inline void wrapAround(V<BASE, T> const & a_min, V<BASE, T> const & a_max)
	{
		V<BASE, T> size = a_max - a_min;
		FOREACH(i){
			while(getField(i) < a_min.getField(i))
			{	addField(i, size.getField(i));	}
			while(getField(i) > a_max.getField(i))
			{	addField(i, -size.getField(i));	}
		}
	}

	/** @return the position half-way between line a->b */
	inline static V<BASE, T> between(V<BASE, T> const & a, V<BASE, T> const & b)
	{
		V<BASE, T> average = b.sum(a);
		average.divide(2.0);
		return average;
	}

	// overloaded operators
	inline V<BASE, T> operator-(){return this->getReverse();}
	inline V<BASE, T> operator+=(V<BASE, T> const & rhs){return add(rhs);}
	inline V<BASE, T> operator-=(V<BASE, T> const & rhs){return subtract(rhs);}
	inline V<BASE, T> operator*=(T const & rhs){return multiply(rhs);}
	inline V<BASE, T> operator/=(T const & rhs){return divide(rhs);}
	inline V<BASE, T> operator+(V<BASE, T> const & rhs)const{return sum(rhs);}
	inline V<BASE, T> operator-(V<BASE, T> const & rhs)const{return difference(rhs);}
	inline V<BASE, T> operator*(T const & rhs)const{return product(rhs);}
	inline V<BASE, T> operator/(T const & rhs)const{return quotient(rhs);}
	inline bool operator==(V<BASE, T> const & rhs) const{return isEqual(rhs);}
	inline bool operator!=(V<BASE, T> const & rhs) const{return !isEqual(rhs);}

#ifdef USING_OPENGL_HELPER_FUNCTIONS
// OpenGL/GLUT specific functions

	/** calls glVertex<N,T>v on this data structure */
	inline void glVertex() const
	{
//		glVertex2f(getX(), getY());

//		if(std::is_integral<T>::value){
//				  if(sizeof(T) == sizeof(GLshort)){
//				if(TEST2D)	glVertex2sv((GLshort*)dataCONST());
//				if(TEST3D)	glVertex3sv((GLshort*)dataCONST());
//				if(TEST4D)	glVertex4sv((GLshort*)dataCONST());
//			}else if(sizeof(T) == sizeof(GLint)){
//				if(TEST2D)	glVertex2iv((GLint*)dataCONST());
//				if(TEST3D)	glVertex3iv((GLint*)dataCONST());
//				if(TEST4D)	glVertex4iv((GLint*)dataCONST());
//			}
//		}else{

				  if(sizeof(T) == sizeof(GLfloat)){
				if(TEST2D)	glVertex2fv((GLfloat*)dataCONST());
				if(TEST3D)	glVertex3fv((GLfloat*)dataCONST());
				if(TEST4D)	glVertex4fv((GLfloat*)dataCONST());
			}else if(sizeof(T) == sizeof(GLdouble)){
				if(TEST2D)	glVertex2dv((GLdouble*)dataCONST());
				if(TEST3D)	glVertex3dv((GLdouble*)dataCONST());
				if(TEST4D)	glVertex4dv((GLdouble*)dataCONST());
			}

//		}
	}

#define DO_(theThing, nonvalue)\
if(sizeof(T) == sizeof(GLfloat)){	\
	if(TEST2D)	theThing##f((GLfloat)X, (GLfloat)Y, (GLfloat)nonvalue);	\
	else		theThing##f((GLfloat)X, (GLfloat)Y, (GLfloat)Z);	\
}else if(sizeof(T) == sizeof(GLdouble)){	\
	if(TEST2D)	theThing##d((GLdouble)X, (GLdouble)Y, (GLdouble)nonvalue);	\
	else		theThing##d((GLdouble)X, (GLdouble)Y, (GLdouble)Z);	\
}

	/**
	 * translates the open GL rendering context.
	 * @note: Dont forget to push and pop the matrix!
	 */
	inline void glTranslate() const
	{
		DO_(glTranslate, 0)
	}

	/**
	 * scale the open GL rendering context.
	 * @note: Dont forget to push and pop the matrix!
	 */
	inline void glScale() const
	{
		DO_(glScale, 1)
	}
#undef DO_
	/**
	 * rotate the open GL rendering context.
	 * @note: Dont forget to push and pop the matrix!
	 */
	inline void glRotate() const
	{
		if(sizeof(T) == sizeof(GLfloat)){	\
			if(TEST2D)	glRotatef(-degrees(), 0,0,1);	\
			else		glRotatef(-degrees(), 0,0,1);	\
		}else if(sizeof(T) == sizeof(GLdouble)){	\
			if(TEST2D)	glRotated(-degrees(), 0,0,1);	\
			else		glRotated(-degrees(), 0,0,1);	\
		}
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

	inline bool glDrawTo(const V<BASE, T> & a_next) const
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
	static inline void glVertexList(V<BASE, T> * a_list, const int & a_count)
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
			glRasterPos2f (X, Y+linesPrinted*lineHeight);
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

struct __V2F_BASE{VFLOAT x,y;__V2F_BASE():x(0),y(0){}};
/** V2 using float, useful for all kinds of games and simulations */
typedef V<__V2F_BASE, VFLOAT> V2F;

struct __V2I_BASE{int x,y;__V2I_BASE():x(0),y(0){}};
/** V2 using int, useful for grids and col/row coordinates */
typedef V<__V2I_BASE, int> V2I;

#undef X
#undef Y
#undef Z
#undef FOREACH
#undef NEWRESULT
#undef COMPOUND_
#undef V_SIN
#undef V_COS
#undef V_ACOS
#undef TEST2D
#undef TEST3D
#undef TEST4D
//#undef VFLOAT
#undef V_ARR
