#pragma once

#include "license.txt"
#include "v2.h"

template <typename VTYPE>
class Circle
{
public:
	/** where the circle is centered */
	VTYPE center;
#define SCALAR_CIRCLE	typename VTYPE::type
	/** the radius of the circle */
	SCALAR_CIRCLE radius;

	Circle():radius(0){}
	Circle(Circle<VTYPE> const & c){
		center = c.center;
		radius = c.radius;
	}
	Circle(VTYPE const & center, SCALAR_CIRCLE const & radius){
		this->center = center;
		this->radius = radius;
	}
	bool equals(Circle<VTYPE> const & c)const{
		return radius == c.radius && center.equals(c.center);
	}

	/** @return if this circle collides with the given circle */
	bool isOverlappingWith(Circle<VTYPE> const & c)
	{
		return center.distance(c.center) < radius+c.radius;
	}
	/** @return how much are the radii overlapping (will be negative if there is no overlap) */
	SCALAR_CIRCLE getOverlappingRadius(Circle<VTYPE> const & c)
	{
		return (radius+c.radius - center.distance(c.center));
	}
	/** @return the number of solutions (-1 for infinity) */
	int getIntersection(Circle<VTYPE> const & c, VTYPE * out1, VTYPE * out2) const
	{
		// TODO for spheres, out2 should be of type Plane
		static_assert(VTYPE::NUM_DIMENSIONS==2,"cannot intersect non-2D circles");
		VTYPE delta = c.center.difference(center);
		SCALAR_CIRCLE dist = delta.length();
		if(dist == 0 && c.radius == radius){	// same circle has infinite solutions
			return -1;
		}
		if(dist > radius+c.radius		// too far apart
		|| dist < abs(radius-c.radius))	// too close
			return 0;
		// circles are tangent to each other
		if(dist == radius+c.radius){
			delta.setLength(radius);
			VTYPE * out = (out1 != NULL)?out1:out2;
			out->set(center);
			out->add(delta);
			return 1;
		}
	//	xD = xC + (1/2)(rA2-rB2)/d * (xB-xA)/d = (1/2)(xB+xA) + (1/2)(xB-xA)(rA2-rB2)/d2 
	//	yD = yC + (1/2)(rA2-rB2)/d * (yB-yA)/d = (1/2)(yB+yA) + (1/2)(yB-yA)(rA2-rB2)/d2 
//		x = (1/2)(xB+xA) + (1/2)(xB-xA)(rA2-rB2)/d2 + 2(yB-yA)K/d2
//		y = (1/2)(yB+yA) + (1/2)(yB-yA)(rA2-rB2)/d2 + -2(xB-xA)K/d2
		SCALAR_CIRCLE d2 = dist*dist;
		SCALAR_CIRCLE dr = radius-c.radius;
		SCALAR_CIRCLE ds = radius+c.radius;
		SCALAR_CIRCLE mr2 = radius*radius;
		SCALAR_CIRCLE or2 = c.radius*c.radius;
		SCALAR_CIRCLE K = sqrt((ds*ds-d2)*(d2-(dr*dr)))/4;
		SCALAR_CIRCLE xPartA = (c.center.x+center.x)/2 +(c.center.x-center.x)*(mr2-or2)/(2*d2);
		SCALAR_CIRCLE xPartB = 2*(c.center.y-center.y)*K/d2;
		SCALAR_CIRCLE yPartA = (c.center.y+center.y)/2 +(c.center.y-center.y)*(mr2-or2)/(2*d2);
		SCALAR_CIRCLE yPartB = 2*(c.center.x-center.x)*K/d2;
//		out1.x = (c.v.x+v.x)/2 +(c.v.x-v.x)*(r*r-c.r*c.r)/(2*d2) + 2*(c.v.y-v.y)*K/d2;// +/- 2*(c.v.y-v.y)*K/d2
//		out1.y = (c.v.y+v.y)/2 +(c.v.y-v.y)*(r*r-c.r*c.r)/(2*d2) - 2*(c.v.x-v.x)*K/d2;
		if(out1 != NULL)	out1->set(xPartA+xPartB, yPartA-yPartB);
		if(out2 != NULL)	out2->set(xPartA-xPartB, yPartA+yPartB);
//		out2.x = (c.v.x+v.x)/2 +(c.v.x-v.x)*(r*r-c.r*c.r)/(2*d2) - 2*(c.v.y-v.y)*K/d2;// +/- 2*(c.v.y-v.y)*K/d2
//		out2.y = (c.v.y+v.y)/2 +(c.v.y-v.y)*(r*r-c.r*c.r)/(2*d2) + 2*(c.v.x-v.x)*K/d2;
				//(1/2)(c.vB+v.y) + (1/2)(yB-yA)(rA2-rB2)/d2;// +/- -2(xB-xA)K/d2
		return 2;
	}
	/** set this circle to be circumscribed on the given points */
	void circumscription(VTYPE const & a_p0, VTYPE const & a_p1, VTYPE const & a_p2){
		// TODO circumscription with a tetrahedron
		static_assert(VTYPE::NUM_DIMENSIONS==2,"cannot intersect non-2D circles");
		VTYPE delta0 = a_p1.difference(a_p0);
		VTYPE delta1 = a_p2.difference(a_p1);
		VTYPE mid0 = VTYPE::between(a_p0, a_p1);
		VTYPE mid1 = VTYPE::between(a_p1, a_p2);
		VTYPE mid0Normal = delta0.perp().sum(mid0);
		VTYPE mid1Normal = delta1.perp().sum(mid1);
		SCALAR_CIRCLE dist;
		VTYPE::lineIntersection(mid0, mid0Normal, mid1, mid1Normal, dist, center);
		radius = center.distance(a_p0);
	}
	/** set this circle to be the circular inscription of the given triangle */
	void circularInscription(VTYPE const & p0, VTYPE const & p1, VTYPE const & p2){
		// TODO circularInscription with a tetrahedron
		static_assert(VTYPE::NUM_DIMENSIONS==2,"cannot intersect non-2D circles");
		VTYPE delta0 = p1.difference(p0);
		delta0.normalize();
		VTYPE delta1 = p2.difference(p1);
		delta1.normalize();
		VTYPE delta2 = p0.difference(p2);
		delta2.normalize();
		delta2.setReverse();
		VTYPE midAngle0 = VTYPE::between(delta0, delta2);
		delta0.setReverse();
		VTYPE midAngle1 = VTYPE::between(delta0, delta1);
		VTYPE testPoint;
		SCALAR_CIRCLE dist;
		VTYPE::lineIntersection(p0, p0.sum(midAngle0), p1, p1.sum(midAngle1), dist, center);
		VTYPE::closestPointOnLine(p0, p1, center, testPoint);
		radius = testPoint.distance(center);
	}
	void set(Circle<VTYPE> const & c)
	{
		radius=c.radius;
		center.set(c.center);
	}
	/**
	 * @param boundary a circle that this circle is not allowed to enter, but is entering
	 * @param out __OUT the location that this circle should be at to be oriented the same way, but no longer overlapping the boundary. {@link #center} if this is directly on top of the boundary
	 */
	void getPushOutPosition(Circle<VTYPE> const & boundary, VTYPE & out){
		out.set(center);
		out.subtract(boundary.center);
		if(out.isZero()){
			out.set(center);
		}else{
			out.setLength(radius+boundary.radius);
			out.add(boundary.center);
		}
	}

#ifdef USING_OPENGL_HELPER_FUNCTIONS
// should be 1 or 4
#define _CALC_PI_SEGMENTS	1
	/**
	 * @param a_segments how many segments
	 * @param a_filled whether to use GL_POLYGON or GL_LINE_STRIP
	 */
	static void glDraw(const float & a_x, const float & a_y,
					  const float & a_radius, int a_segments,
					  const bool a_filled)
	{
		// TODO draw a sphere... one circle, for x, y, and z, with one more facing camera...
		static_assert(VTYPE::NUM_DIMENSIONS==2,"cannot draw a non-2D circle");
		// static code only initializes once, and acts as global data otherwise
		static const int MAX_ARC_SEGMENTS = 100;
		static const int MAX_ARC_NODES = MAX_ARC_SEGMENTS+1;
		static VTYPE g_arc[MAX_ARC_NODES];
		static int s_calculatedSegments = 0;

		if(a_segments > MAX_ARC_SEGMENTS)
			a_segments = MAX_ARC_SEGMENTS;
		// recalculate the points of a quarter-circle if needed
		if(s_calculatedSegments != a_segments)
		{
			// don't recalculate a circle of this size next time...
			s_calculatedSegments = a_segments;
			if(a_segments > 0)
			{
				// must result in a normalized (unit-vector) value
				VTYPE normal;
				normal.setRadians((float)(V_2PI/_CALC_PI_SEGMENTS)/(a_segments));
				g_arc[0].set(1, 0);
				VTYPE * lastPoint;
				// calculate all points in the arc as normals
				for(int i = 1; i < s_calculatedSegments; ++i)
				{
					lastPoint = &g_arc[i-1];
					// calculate turn in next point
					g_arc[i].set(
						// x_ = x*cos(theta) - y*sin(theta)
						lastPoint->x*normal.x - lastPoint->y*normal.y,
						// y_ = x*sin(theta) + y*cos(theta)
						lastPoint->x*normal.y + lastPoint->y*normal.x);
				}
			}else{
				s_calculatedSegments = 0;
			}
		}
		glPushMatrix();
		glTranslatef(a_x, a_y, 0);	//a_center.glTranslate();
		glScalef(a_radius, a_radius, 1);
		glBegin(a_filled?GL_POLYGON:GL_LINE_LOOP);
		int i;
		// draw top-right
		for(i = 0; i < s_calculatedSegments; ++i)	g_arc[i].glVertex();
#if _CALC_PI_SEGMENTS == 4
		int lastSegment = s_calculatedSegments-1;
		static const VTYPE TOP(0,1); TOP.glVertex();
		// flip horizontally
		for(i = lastSegment; i >= 0; --i)	g_arc[i].x *= -1;
		// draw top-left
		for(i = lastSegment; i >= 0; --i)	g_arc[i].glVertex();
		// flip vertically
		for(i = lastSegment; i >= 0; --i)	g_arc[i].y *= -1;
		// draw bottom-left
		for(i = 0; i < s_calculatedSegments; ++i)	g_arc[i].glVertex();
		static const VTYPE BOTTOM(0,-1); BOTTOM.glVertex();
		// flip horizontally
		for(i = lastSegment; i >= 0; --i)	g_arc[i].x *= -1;
		// draw bottom-right
		for(i = lastSegment; i >= 0; --i)	g_arc[i].glVertex();
#endif
#undef _CALC_PI_SEGMENTS
		glEnd();
		glPopMatrix();	// undoes the glTranslate and glScale
	}

	inline void glDraw(const int a_segments, bool filled)const
	{
		glDraw(center.x, center.y, radius, a_segments, filled);
	}

	inline void glDraw(bool filled)const
	{
		glDraw(center.x, center.y, radius, 32, filled);
	}
#endif
};

typedef Circle<V2F> CircleF;

#undef SCALAR_CIRCLE
