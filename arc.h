#pragma once

#include "circle.h"

template <typename VTYPE>
class ArcSegment : public Circle<VTYPE>
{
public:
//#define VTYPE	V2F
#define SCALAR	typename VTYPE::type
	/** where the arc begins and ends */
	float startAngle, angleTotal;
	/** how large a percentage of this arch is hollow, like a doughnut. 0 to 1 */
	float percentHollow;

	ArcSegment(V2F center, float radius,
			float startAngle, float angleTotal, float percentHollow)
		:Circle<VTYPE>(center, radius),
		 startAngle(startAngle),angleTotal(angleTotal), percentHollow(percentHollow)
	{
	}

	ArcSegment():Circle<VTYPE>(V2F(10,10), 10),startAngle(.1),angleTotal(V_PI), percentHollow(0.75){}

	/**
	 * @param a_angleRadians 
	 * @param a_radius 
	 * @return length of the hypotinuse of an isosceles triangle
	 */
	static float angleHypotinuse(float a_angleRadians, float a_radius)
	{
		V2F v = V2F::fromPiRadians(a_angleRadians);
		v *= a_radius;
		return v.length();
	}

	static float radiusToGetHypotinuse()
	{
		// TODO math to do the cool animation.
	}

	void static glDraw(const float & a_x, const float & a_y,
			  const float a_radius,
			  const float startAngle,
			  const float angleTotal,
			  const float percentHollow,
			  int a_segments,
			  const bool a_filled)
	{
		static_assert(VTYPE::NUM_DIMENSIONS==2,"cannot draw a non-2D circle");
		// static code only initializes once, and acts as global data otherwise
		static const int MAX_ARC_SEGMENTS = 100;
		static const int MAX_ARC_NODES = MAX_ARC_SEGMENTS+1;
		static VTYPE g_arc[MAX_ARC_NODES];
		static int s_calculatedSegments = 0;
		if(a_segments == 0 && angleTotal != 0)
			a_segments = 1;
		if(a_segments > MAX_ARC_SEGMENTS)
			a_segments = MAX_ARC_SEGMENTS;
		// recalculate the points of a quarter-circle if needed
		if(true)//s_calculatedSegments != a_segments)
		{
			// don't recalculate a circle of this size next time...
			s_calculatedSegments = a_segments;
			if(a_segments > 0)
			{
				// must result in a normalized (unit-vector) value
				VTYPE g_normal = VTYPE::fromPiRadians((float)(
						//V_2PI
						angleTotal
						)/(a_segments));
//					g_arc[0].set(1, 0);
				VTYPE centerP = VTYPE::fromPiRadians(startAngle);
				g_arc[0].set(centerP);
				VTYPE * lastPoint;
				// calculate all points in the arc as normals
				for(int i = 1; i <= s_calculatedSegments; ++i)
				{
					lastPoint = &g_arc[i-1];
					// calculate turn in next point
					g_arc[i].set(
						// x_ = x*cos(theta) - y*sin(theta)
						lastPoint->x*g_normal.x - lastPoint->y*g_normal.y,
						// y_ = x*sin(theta) + y*cos(theta)
						lastPoint->x*g_normal.y + lastPoint->y*g_normal.x);
				}
			}else{
				//s_calculatedSegments = 0;
			}
		}
		glPushMatrix();
		glTranslatef(a_x, a_y, 0);	//a_center.glTranslate();
		glScalef(a_radius, a_radius, 1);
		//glBegin(a_filled?GL_POLYGON:GL_LINE_LOOP);
		glBegin(a_filled?GL_TRIANGLE_STRIP:GL_LINE_STRIP);
		int i;
		VTYPE vmin, vmax;
		for(i = 0; i < s_calculatedSegments+1; ++i){
//			float size;
//			// small to big
//			size = (float)i/s_calculatedSegments;
//			// small to big to small
////			float half = (s_calculatedSegments+1)/2;
////			if(i <= half)
////				size = (float)i/(half);
////			else
////				size = (float)(half-(i-half))/(half);
//			vmax = g_arc[i] * (((1-percentHollow)*size)+percentHollow);
//			vmin = g_arc[i] * percentHollow;
//			vmin.glVertex();
//			vmax.glVertex();
			g_arc[i].glVertex();
			g_arc[i] *= percentHollow;
			g_arc[i].glVertex();
		}
		glEnd();
		glPopMatrix();	// undoes the glTranslate and glScale
	}

	void setRadiusInner(float a_innerRad){
		percentHollow = a_innerRad/this->radius;
	}

	void setRadii(float a_innerRad, float a_outerRad)
	{
		this->radius = a_outerRad;
		percentHollow = a_innerRad/a_outerRad;
	}

	float getRadiusInner() const
	{
		return this->radius*percentHollow;
	}

	float getRadiusOuter() const
	{
		return this->radius;
	}

	void setRadiusOuter(float a_outerRad){
		float inner = getRadiusInner();
		this->radius = a_outerRad;
		setRadiusInner(inner);
	}

	void setAngles(float a_startAngle, float a_endAngle)
	{
		startAngle = a_startAngle;
		angleTotal = a_endAngle-a_startAngle;
	}

	void glDraw(bool a_filled) const
	{
		glDraw(this->center.x, this->center.y, this->radius,
				startAngle, angleTotal,
				percentHollow, angleTotal*32/V_2PI, a_filled);

	}

	bool contains(VTYPE const point) const
	{
		float dist = this->center.distance(point);
		if(dist <= this->radius && dist >= this->radius*percentHollow)
		{
			VTYPE A(this->center), B(this->center);
			A += VTYPE(startAngle);
			B += VTYPE(startAngle+angleTotal);
			if(angleTotal < V_PI)
				return point.isCW(this->center, A)
					&& point.isCCW(this->center, B);
			else
				return !(point.isCCW(this->center, A)
						&& point.isCW(this->center, B));
		}
		return false;
	}
};

typedef ArcSegment<V2F> ArcF;

#undef SCALAR
#undef VTYPE
