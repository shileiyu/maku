#ifndef MAKU_RENDER_GEOMETRY_H_
#define MAKU_RENDER_GEOMETRY_H_
#include <ncore/ncore.h>

namespace maku
{
namespace render
{
class Point
{
public:
	static Point Make(int x, int y);
public:
	Point();
	Point(int x, int y);
	Point(const Point& pt);
	~Point();
	void operator=(const Point& pt);
	bool operator==(const Point& pt) const;
	bool operator!=(const Point& pt) const;

	void set_x(int x);
	void set_y(int y);
	void SetLoc(int x, int y);

	int x() const;
	int y() const;

	void Offset(int dx, int dy);
private:
	int x_;
	int y_;
};


class Rect
{
public:
	static Rect Make(int x, int y, int width, int height);
	static Rect Intersect(const Rect & r1, const Rect & r2);
	static Rect Union(const Rect & r1, const Rect & r2);
public:
	Rect();
	Rect(int x, int y, int width, int height);
	~Rect();

	void operator=(const Rect & r);
	bool operator==(const Rect & r) const;
	bool operator!=(const Rect & r) const;

	bool isEmpty() const;

	bool isPointIn(const Point & pt) const;
	bool isPointIn(int x, int y) const;

	void Intersect(const Rect & r);
	void Union(const Rect & r);

	void SetXYWH(int x, int y, int width, int height);
	void SetLTRB(int left, int top, int right, int bottom);
	void SetEmpty();

	int left() const;
	int top() const;
	int right() const;
	int bottom() const;
	int width() const;
	int height() const;

	void Offset(int dx, int dy);

private:
	int left_;
	int top_;
	int right_;
	int bottom_;
};


struct Float4
{
	Float4(float a, float b, float c, float d)
	{
		x = a;
		y = b;
		z = c;
		w = d;
	}
	Float4()
	{
		x = y = z = w = 0;
	}
	float x;
	float y;
	float z;
	float w;
};
struct Float3
{
	Float3(float a, float b, float c)
	{
		x = a;
		y = b;
		z = c;
	}
	float x;
	float y;
	float z;
};

struct Float2
{
	Float2(float a, float b)
	{
		x = a;
		y = b;
	}
	float x;
	float y;
};

struct Matrix
{
	Float4 m0;
	Float4 m1;
	Float4 m2;
	Float4 m3;
};

Matrix MatrixIdentity();

Matrix MatrixOrthographicOffCenterLH(float left, float right, float bottom, float top,
	float nearz, float farz);

Matrix MatrixTranspose(Matrix & m);

Float4 VertorMergeXY(Float4 & V1, Float4 & V2);

Float4 VertorMergeZW(Float4 & V1, Float4 & V2);

}
}
#endif