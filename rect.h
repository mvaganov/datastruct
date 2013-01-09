#pragma once

#include "license.txt"
#include "v2.h"

/**
 * @author mvaganov@hotmail.com
 */
template<typename VT>
struct Rect
{
	/** corners of box are stored as opposed to x/y w/h*/
	VT m_min, m_max;

#define SCALAR_RECT	typename VT::type

	/** @return top-left */
	inline const VT getMin()const{return m_min;}

	/** @return bottom right */
	inline const VT getMax()const{return m_max;}

//	template<int DIMENSION>
//	inline void setMin(const SCALAR_RECT & a_value){
//		m_min.set<DIMENSION>(a_value);
//	}

	/** set the left-edge location (resizes the rectangle) */
	inline void setMinX(const SCALAR_RECT & a_value){	m_min.x = (a_value);	}

	/** set the right-edge location (resizes the rectangle) */
	inline void setMaxX(const SCALAR_RECT & a_value){	m_max.x = a_value;	}

	/** set the top-edge location (resizes the rectangle) */
	inline void setMinY(const SCALAR_RECT & a_value){	m_min.y = (a_value);	}

	/** set the bottom-edge location (resizes the rectangle) */
	inline void setMaxY(const SCALAR_RECT & a_value){	m_max.y = a_value;	}

	bool isEqual(const Rect<VT> & b) const
	{
		return getMinX() == b.getMinX()
			&& getMinY() == b.getMinY()
			&& getMaxX() == b.getMaxX()
			&& getMaxY() == b.getMaxY();
	}

	/** puts all 4 corners of the box into the given array */
	inline void getCorners(VT * a_array) const
	{
		a_array[0].set(getMinX(), getMinY());
		a_array[1].set(getMaxX(), getMinY());
		a_array[2].set(getMinX(), getMaxY());
		a_array[3].set(getMaxX(), getMaxY());
	}

	/** @return y value of top edge */
	inline const SCALAR_RECT getMinY()const{return m_min.getY();}
	/** @return x value of left edge */
	inline const SCALAR_RECT getMinX()const{return m_min.getX();}
	/** @return y value of bottom edge */
	inline const SCALAR_RECT getMaxY()const{return m_max.getY();}
	/** @return x value of right edge */
	inline const SCALAR_RECT getMaxX()const{return m_max.getX();}

	/** set the upper-left corner location (resizes the rectangle) */
	inline void setMin(const VT & p){setMinX(p.x);setMinY(p.y);}
	/** set the lower-right corner location (resizes the rectangle) */
	inline void setMax(const VT & p){setMaxX(p.x);setMaxY(p.y);}

	/** set the width value (moves the max value) */
	inline void setWidth(const SCALAR_RECT a_value){	setMaxX(getMinX()+a_value);	}
	/** set the height value (moves the max value) */
	inline void setHeight(const SCALAR_RECT a_value){	setMaxY(getMinY()+a_value);	}

	inline SCALAR_RECT getWidth()const{return getMaxX()-getMinX();}
	inline SCALAR_RECT getHeight()const{return getMaxY()-getMinY();}
	/** @return x position of rectangle (left edge location) */
	inline const SCALAR_RECT getX() const{	return getMinX();	}
	/** @return y position of rectangle (top edge location) */
	inline const SCALAR_RECT getY() const{	return getMinY();	}
	/** set the x position value (moves the rectangle) */
	inline void setX(const SCALAR_RECT & a_value)
	{
		SCALAR_RECT d = getMaxX() - getMinX();
		setMinX(a_value);	
		setMaxX(a_value+d);
	}
	/** set the y position value (moves the rectangle) */
	inline void setY(const SCALAR_RECT & a_value)
	{
		SCALAR_RECT d = getMaxY() - getMinY();
		setMinY(a_value);
		setMaxY(a_value+d);
	}
	inline void setXY(VT a_positionToMoveRectangleTo)
	{
		setX(a_positionToMoveRectangleTo.x);
		setY(a_positionToMoveRectangleTo.y);
	}
	/** the distance vector between the two opposite Rect points */
	inline VT diagonal()const{return getMax().difference(getMin());}
	
	/** width/height */
	inline VT getDimension()const{return diagonal();}

	void setDimension(const VT & a_dim){setWidth(a_dim.getX());	setHeight(a_dim.getY());}

	inline SCALAR_RECT getArea()const	{return getWidth()*getHeight();}

	/** @return width/height of this rectangle >= x/y of a_size */
	inline bool isDimensionGreaterThan(VT const & a_size)const{return getWidth()>=a_size.x && getHeight()>=a_size.y;}

	/** @return true if this and the given rectangle have identical values */
	inline bool equals(const Rect<VT> & b) const
	{
		return getMin().equals(b.getMin()) && getMax().equals(b.getMax());
	}

	/** re-constructor */
	inline void set(const VT & a_min, const VT & a_max)
	{	setMin(a_min);	setMax(a_max);	}

	/** given points in arbitrary order, makes this an inscribed rectangle */
	void setFromPoints(const VT & a, const VT & b)
	{
		VT _min(((a.x<b.x)?a.x:b.x),((a.y<b.y)?a.y:b.y));
		VT _max(((a.x>b.x)?a.x:b.x),((a.y>b.y)?a.y:b.y));
		set(_min, _max);
	}

	/** sets the rectangle to be all zeros (zero size at the origin) */
	inline void clear(){set(VT::ZERO(),VT::ZERO());}

	/** default constructor, everything is zero */
	inline Rect(){}
	/** */
	inline Rect(const VT & a_min, const VT & a_max){setFromPoints(a_min, a_max);}
	/** @param where to center the box, a square a_radius*2 tall and wide */
	inline Rect(const VT & a_center, const SCALAR_RECT & a_radius)
	{
		VT corner(a_radius, a_radius);
		set(a_center.difference(corner), a_center.sum(corner));
	}
	/** re-constructor */
	inline void set(const VT & a_center, const SCALAR_RECT & a_radius)
	{
		VT corner(a_radius, a_radius);
		set(a_center.sum(corner), a_center.difference(corner));
	}
	inline void set(const Rect<VT> & a_box)
	{
		set(a_box.getMin(), a_box.getMax());
	}
	/** @return the radius of the circle that this rect would be inscribed in */
	inline SCALAR_RECT getRadius()const{return diagonal().quotient(2).length();}
	/** resizes the rectangle by circle-radius (a circle that this rectangle is inscribed in) */
	inline void setRadius(const SCALAR_RECT & a_radius)
	{
		VT diffRad = diagonal();
		if(diffRad.isZero())
		{
			diffRad.set(1,1);
		}
		SCALAR_RECT currentRad = diffRad.quotient(2).length();
		SCALAR_RECT ratio = a_radius/currentRad;
		VT center(getCenter());
		diffRad.multiply(ratio);
		setMin(center.difference(diffRad));
		setMax(center.sum(diffRad));
	}
	inline VT getCenter()const{return VT::between(getMin(),getMax());}
	inline void setCenter(const VT & c)
	{
		VT rad = diagonal();
		rad.divide(2);
		setMin(c.difference(rad));
		setMax(c.sum(rad));
	}

	inline Rect expandBorder(const VT & a_bevel)
	{
		return Rect<VT>(getMin().difference(a_bevel),
			getMax().sum(a_bevel));
	}

	inline bool intersects(const Rect<VT> & b) const
	{
		return!(b.getMaxX() < getMinX() ||	b.getMaxY() < getMinY()
			||	b.getMinX() > getMaxX() ||	b.getMinY() > getMaxY());
	}

	/** @return true if the given Rect is totally contained in this Rect */
	inline bool contains(const Rect<VT> & b) const
	{
		return getMinX() <= b.getMinX() && getMinY() <= b.getMinY()
			&& getMaxX() >= b.getMaxX() && getMaxY() >= b.getMaxY();
	}
	/** @return true if the given point is in this Rect */
	inline bool contains(const VT & p) const
	{
		return getMinX() <= p.x && getMinY() <= p.y
			&& getMaxX() >= p.x && getMaxY() >= p.y;
	}

	static const int BAD_VALUE = -1;
	/** used to determine X and Y. should not be used in setField() or getField() */
	static const int X = 0, Y = 1, Z = 2;
	/** MINX/MINY */
	static const int MIN = VT::NUM_DIMENSIONS*0;
	/** used by getField() and setField() */
	static const int MINX = MIN+X, MINY = MIN+Y, MINZ = MIN+Z;
	/** MAXX/MAXY */
	static const int MAX = VT::NUM_DIMENSIONS*1;
	/** used by getField() and setField() */
	static const int MAXX = MAX+X, MAXY = MAX+Y, MAXZ = MAX+Z;
	/** X Position/Y Position */
	static const int POSITION = VT::NUM_DIMENSIONS*2;
	/** used by getField() and setField() */
	static const int POSITIONX = POSITION+X, POSITIONY = POSITION+Y, POSITIONZ = POSITION+Z;
	/** WIDTH/HEIGHT */
	static const int DIMENSION = VT::NUM_DIMENSIONS*3;
	/** used by getField() and setField() */
	static const int WIDTH = DIMENSION+X, HEIGHT = DIMENSION+Y, DEPTH = DIMENSION+Z;

	/** @param a_value {MINX, MINY, MAXX, MAXY} */
	inline const SCALAR_RECT getField(int const & a_value) const
	{
		//return ((VTYPE*)this)[a_value];
		switch(a_value)
		{
		case MINX:	return getMinX();
		case MINY:	return getMinY();
		case MAXX:	return getMaxX();
		case MAXY:	return getMaxY();
		case POSITIONX:	return getX();
		case POSITIONY:	return getY();
		case WIDTH:		return getWidth();
		case HEIGHT:	return getHeight();
		}
		return 0;
	}
	/**
	 * a more data-driven way to call setMinX, setMaxX, setMinY, or setMaxY
	 * @param a_dimension {MINX,MAXX,MINY,MAXY}
	 */
	inline void setField(const int & a_dimension, const SCALAR_RECT & a_value)
	{
		//((VTYPE*)this)[a_dimension] = a_value;
		switch(a_dimension)
		{
		case MINX:	setMinX(a_value);	break;
		case MINY:	setMinY(a_value);	break;
		case MAXX:	setMaxX(a_value);	break;
		case MAXY:	setMaxY(a_value);	break;
		case POSITIONX:	setX(a_value);	break;
		case POSITIONY:	setY(a_value);	break;
		case WIDTH:		setWidth(a_value);	break;
		case HEIGHT:	setHeight(a_value);	break;
		}

	}

	/**
	 * used to grab the opposite side of the range of the given dimension. 
	 * eg: MINX being passed in returns MAXX. MAXY passed in returns MINY.
	 * @return MINX<->MAXX, MINY<->MAXY
	 */
	//static const int oppositeSide(const int & a_field)
	//{
	//	if(a_field >= MIN && a_field < MAX)	// if it is MIN
	//		return a_field-(MIN+MAX);
	//	else if(a_field >= MAX && a_field < DIMENSION)	// if it is MAX
	//		return a_field-(MAX+MIN);
	//	else return a_field-DIMENSION;
	//}

	/**
	 * @param a_range {MINX,MAXX,MINY,MAXY}
	 * @return true if this box and the given box share the passed in range 
	 * (x or y), meaing their width/height would overlap
	 */
	inline bool commonRange(const Rect<VT> * a_him, int a_range) const
	{
		switch(a_range)
		{
		case MINX:
		case MAXX:	return (getMaxY() >= a_him->getMinY() && getMinY() <= a_him->getMaxY());
		case MINY:
		case MAXY:	return (getMaxX() >= a_him->getMinX() && getMinX() <= a_him->getMaxX());
		}
		return false;
	}
	/**
	 * @param a_range {MINX,MAXX,MINY,MAXY}
	 * @return true if this box and the given box share the passed in range, excluding equality
	 * (x or y), meaing their width/height would overlap
	 */
	inline bool nonOrthogonalCommonRange(const Rect<VT> * a_him, int a_range) const
	{
		switch(a_range)
		{
		case MINX:
		case MAXX:	return (getMaxY() > a_him->getMinY() && getMinY() < a_him->getMaxY());
		case MINY:
		case MAXY:	return (getMaxX() > a_him->getMinX() && getMinX() < a_him->getMaxX());
		}
		return false;
	}
	/**
	 * @return true if this box and the given box share the passed in range, excluding equality
	 * (x or y), meaing their width/height would overlap
	 */
	inline bool nonOrthogonalIntersect(const Rect<VT> & b) const
	{
		return nonOrthogonalCommonRange(&b, X) && nonOrthogonalCommonRange(&b, Y);
	}

	/**
	 */
	bool isEdgeTouchingEdge(const int & a_myEdge, 
		const Rect<VT> * a_him, const int & a_hisEdge,
		SCALAR_RECT a_touchRange = 1.0/1024) const
	{
		if(a_touchRange == 0.0)
			return (nonOrthogonalCommonRange(a_him, a_myEdge) 
			&& getField(a_myEdge) == a_him->getField(a_hisEdge));
		return (commonRange(a_him, a_myEdge)
			&& (abs(getField(a_myEdge) - a_him->getField(a_hisEdge)) < a_touchRange));
	}

	/**
	 * @return {MINX, MINY, MAXX, MAXY} if this Rectangle touches a_him. 
	 * BAD_VALUE (-1) if they don't touch.
	 */
	int getEdgeTouching(const Rect<VT> * a_him, SCALAR_RECT const & a_neighborRange = 1.0/1024) const
	{
		int myEdge, hisEdge;
		for(myEdge = 0; myEdge < POSITION; ++myEdge)
		{
			hisEdge = (myEdge + MAX) % POSITION;
			if(isEdgeTouchingEdge(myEdge, a_him, hisEdge, a_neighborRange))
				return myEdge;
		}
		return BAD_VALUE;
	}

	static const int NUM_DIMENSIONS = VT::NUM_DIMENSIONS;

	/** @return true if the width and height are both zero */
	inline bool isZero()const{return getWidth()==0 && getHeight()==0;}

	/** @return true if the width and height are both greater than zero */
	inline bool isValid()const{return getWidth()>0&&getHeight()>0;}

	/** adds to the rectangle's size to include the given point (may resize the rectangle) */
	void add(VT const & p)
	{
		if(p.getX() < this->getMinX())	this->setMinX(p.getX());
		if(p.getX() > this->getMaxX())	this->setMaxX(p.getX());
		if(p.getY() < this->getMinY())	this->setMinY(p.getY());
		if(p.getY() > this->getMaxY())	this->setMaxY(p.getY());
	}

	//** adds a given rectangle's area to this rectangle (may resize the rectangle) */
	void add(Rect const & r)
	{
		if(r.getMinX() < getMinX())	setMinX(r.getMinX());
		if(r.getMaxX() > getMaxX())	setMaxX(r.getMaxX());
		if(r.getMinY() < getMinY())	setMinY(r.getMinY());
		if(r.getMaxY() > getMaxY())	setMaxY(r.getMaxY());
	}
	/** makes this rectangle only the intersection with the given rectangle */
	void clip(const Rect & r)
	{
		if(r.getMinX() > getMinX())	setMinX(r.getMinX());
		if(r.getMaxX() < getMaxX())	setMaxX(r.getMaxX());
		if(r.getMinY() > getMinY())	setMinY(r.getMinY());
		if(r.getMaxY() < getMaxY())	setMaxY(r.getMaxY());
	}
	/** @return what this rectangle would be if it were clipped */
	Rect clipped(const Rect & a_clippingRect)const
	{
		Rect r(*this);
		r.clip(a_clippingRect);
		return r;
	}

	/** reduces the rectangles size in all directions by the given value */
	void inset(const SCALAR_RECT & a_border){
		VT rad(a_border,a_border);
		setMin(getMin().sum(rad));setMax(getMax().difference(rad));
	}

	/** @return what this rectangle would be if it were inset the given value */
	Rect insetted(const SCALAR_RECT & a_border)const
	{
		Rect insetRect(*this);
		insetRect.inset(a_border);
		return insetRect;
	}

	/** @return a rectangle moved over a_direction units (this rectangle is a unit-size) */
	Rect unitsOver(const VT & a_direction)const
	{
		return Rect(getX()+getWidth()*a_direction.getX(),
			getY()+getHeight()*a_direction.getY(),getWidth(),getHeight());
	}

	/** add a_delta to the rectangle's position */
	void move(const VT & a_delta)
	{
		setMin(getMin().sum(a_delta));setMax(getMax().sum(a_delta));
	}
	/** @return this rectangle if it were moved a_delta */
	Rect moved(const VT & a_delta)const
	{
		Rect r(*this);
		r.move(a_delta);
		return r;
	}

	/** force this rectangle to overlap as much area as possible with the given rectangle */
	void keepBound(const Rect & area)
	{
		bool isSmallerThanArea;
		// go through X and Y dimensions
		for(int d = 0; d < 2; ++d)
		{
			isSmallerThanArea = getField(DIMENSION+d) < area.getField(DIMENSION+d);
			if(isSmallerThanArea
			?(getField(MIN+d) <= area.getField(MIN+d))
			:(getField(MIN+d) >= area.getField(MIN+d)))
				setField(d, area.getField(d));
			else if(isSmallerThanArea 
			?(getField(MAX+d) >= area.getField(MAX+d))
			:(getField(MAX+d) <= area.getField(MAX+d)))
				setField(d, area.getField(MAX+d)-getField(DIMENSION+d));
		}
	}
	/** force this rectangle out of the given rectangle */
	void keepOut(const Rect & area)
	{
		if(intersects(area))
		{
			for(int d = 0; d < VT::NUM_DIMENSIONS; ++d)
			{
				if(getField(MAX+d) > area.getField(MIN+d) 
				&& getField(MIN+d) < area.getField(MIN+d))
					setField(d, area.getField(MIN+d)-getField(DIMENSION+d));
				if(getField(MIN+d) < area.getField(MAX+d)
				&& getField(MAX+d) > area.getField(MAX+d))
					setField(d, area.getField(MAX+d));
			}
		}
	}
	/** multiply the dimensions and position of this rectangle */
	void multiply(const SCALAR_RECT & a_value){
		setMin(getMin().product(a_value));
		setMax(getMax().product(a_value));
	}

	/** @return if the line crosses an edge. does not test if contains line */
	bool crossesLine(const VT & A, const VT & B)const
	{
		SCALAR_RECT dist;
		VT point,
			mM(getMin().getX(), getMax().getY()),
			Mm(getMax().getX(), getMin().getY());
		bool intersection = 
			VT::lineIntersection(A, B, getMin(), mM, dist, point) ||
			VT::lineIntersection(A, B, mM, getMax(), dist, point) ||
			VT::lineIntersection(A, B, getMax(), Mm, dist, point) ||
			VT::lineIntersection(A, B, Mm, getMin(), dist, point);
		return intersection;
	}
	/** expand the rectangle in the given direction. if (-3, 5) is passed, minX will move -3, and maxy will move +5 */
	void expand(const VT & a_direction)
	{
		int delta, whichField;
		for(int d = 0; d < VT::NUM_DIMENSIONS; ++d)
		{
			if((delta = a_direction.getField(d)))
			{
				whichField = (delta>0)?MAX:MIN;
				setField(whichField+d, getField(whichField+d)+delta);
			}
		}
	}
	static const int 
		DIR_MINX = (1<<0),
		DIR_MINY = (1<<1),
		DIR_MAXX = (1<<2),
		DIR_MAXY = (1<<3);
	/**
	 * @param rect adds ths given rect to this
	 * @return 0 for no change, or a bitwise combination of {@link #DIR_MINY}, {@link #DIR_MAXY}, {@link #DIR_MINX} {@link #DIR_MAXX}
	 */
	int addToWhatSide(Rect<VT> const & rect)
	{
		if(getMinX() == 0 && getMinY() == 0 && getMaxX() == 0 && getMaxY() == 0){set(rect);return DIR_MINX|DIR_MINY|DIR_MAXX|DIR_MAXY;}
		int bigger = 0;
		if(rect.getMinX() < getMinX()){setMinX(rect.getMinX());bigger|=DIR_MINX;}
		if(rect.getMinY() < getMinY()){setMinY(rect.getMinY());bigger|=DIR_MINY;}
		if(rect.getMaxX() > getMaxX()){setMaxX(rect.getMaxX());bigger|=DIR_MAXX;}
		if(rect.getMaxY() > getMaxY()){setMaxY(rect.getMaxY());bigger|=DIR_MAXY;}
		return bigger;
	}
	/**
	 * @param rect check if the given rect is bigger than this
	 * @return 0 for not bigger, or a bitwise combination of {@link #DIR_MINY}, {@link #DIR_MAXY}, {@link #DIR_MINX} {@link #DIR_MAXX}
	 */
	int isSmallerThan(Rect<VT> const & rect)const
	{
		int bigger = 0;
		if(rect.getMinX() < getMinX()){bigger|=DIR_MINX;}
		if(rect.getMinY() < getMinY()){bigger|=DIR_MINY;}
		if(rect.getMaxX() > getMaxX()){bigger|=DIR_MAXX;}
		if(rect.getMaxY() > getMaxY()){bigger|=DIR_MAXY;}
		return bigger;
	}

#ifdef USING_OPENGL_HELPER_FUNCTIONS
	inline void draw(bool filled) const
	{
		glBegin(filled?GL_POLYGON:GL_LINE_LOOP);
		VT tr(getMaxX(), getMinY());
		VT bl(getMinX(), getMaxY());
		getMin().glVertex();
		tr.glVertex();
		getMax().glVertex();
		bl.glVertex();
		glEnd();
	}
	inline void draw() const
	{
		draw(true);
	}
#endif
};

#undef SCALAR_RECT

/** standard 2 dimensional Rect using floats */
typedef Rect<V2F> RectF;
