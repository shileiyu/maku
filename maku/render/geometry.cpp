#include "geometry.h"

#include <algorithm>

namespace maku
{
namespace render
{

Point::Point()
	:x_(0),
	y_(0)
{
}

Point::Point(int x, int y)
{
	x_ = x;
	y_ = y;
}

Point::Point(const Point& pt)
{
	x_ = pt.x_;
	y_ = pt.y_;
}

Point::~Point()
{
}

Point Point::Make(int x, int y)
{
	Point r(x, y);
	return r;
}

void Point::operator=(const Point& pt)
{
	x_ = pt.x_;
	y_ = pt.y_;
}

bool Point::operator==(const Point& pt) const
{
	return (x_ == pt.x_) && (y_ == pt.y_);
}

bool Point::operator!=(const Point& pt) const
{
	return (x_ != pt.x_) || (y_ != pt.y_);
}

void Point::set_x(int x)
{
	x_ = x;
}

void Point::set_y(int y)
{
	y_ = y;
}

void Point::SetLoc(int x, int y)
{
	x_ = x;
	y_ = y;
}

int Point::x() const
{
	return x_;
}

int Point::y() const
{
	return y_;
}

void Point::Offset(int dx, int dy)
{
	x_ += dx;
	y_ += dy;
}


Rect Rect::Make(int x, int y, int width, int height)
{
	Rect r(x, y, width, height);
	return r;
}

Rect Rect::Intersect(const Rect & r1, const Rect & r2)
{
	Rect r;
	r = r1;
	r.Intersect(r2);
	return r;
}

Rect Rect::Union(const Rect & r1, const Rect & r2)
{
	Rect r;
	r = r1;
	r.Union(r2);
	return r;
}

Rect::Rect()
{
	left_ = 0;
	top_ = 0;
	right_ = 0;
	bottom_ = 0;
}

Rect::Rect(int x, int y,
	int width, int height)
{
	SetXYWH(x, y, width, height);
}

Rect::~Rect()
{
}

void Rect::operator=(const Rect & r)
{
	left_ = r.left();
	right_ = r.right();
	top_ = r.top();
	bottom_ = r.bottom();
}

bool Rect::operator==(const Rect & r) const
{
	return (left_ == r.left()) && (right_ == r.right()) && (top_ == r.top()) && (bottom_ == r.bottom());
}

bool Rect::operator!=(const Rect & r) const
{
	return (left_ != r.left()) || (right_ != r.right()) || (top_ != r.top()) || (bottom_ != r.bottom());
}

bool Rect::isEmpty() const
{
	bool zero_width = (right_ - left_) == 0;
	bool zero_height = (bottom_ - top_) == 0;

	return zero_width | zero_height;
}

bool Rect::isPointIn(const Point & pt) const
{
	return isPointIn(pt.x(), pt.y());
}

bool Rect::isPointIn(int x, int y) const
{
	if (left_ <= x &&
		x < right_ &&
		top_ <= y &&
		y < bottom_)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Rect::Intersect(const Rect & r)
{
	if (!isEmpty() && !r.isEmpty() &&
		r.left_ < r.right_ &&
		r.top_ < r.bottom_ &&
		left_ < r.right_ &&
		right_ > r.left_ &&
		top_ < r.bottom_ &&
		bottom_ > r.top_)
	{
		top_ = std::max(top_, r.top_);
		left_ = std::max(left_, r.left_);
		right_ = std::min(right_, r.right_);
		bottom_ = std::min(bottom_, r.bottom_);
	}
	else
	{
		top_ = 0;
		left_ = 0;
		right_ = 0;
		bottom_ = 0;
	}

	return;
}

void Rect::Union(const Rect & r)
{
	if (r.isEmpty())
	{
		//Do nothing because r is empty
	}
	else if (isEmpty())
	{
		top_ = r.top_;
		left_ = r.left_;
		right_ = r.right_;
		bottom_ = r.bottom_;
	}
	else
	{
		top_ = std::min(top_, r.top_);
		left_ = std::min(left_, r.left_);
		right_ = std::max(right_, r.right_);
		bottom_ = std::max(bottom_, r.bottom_);
	}

	return;
}

void Rect::SetXYWH(int x, int y, int width, int height)
{
	left_ = x;
	top_ = y;
	right_ = x + width;
	bottom_ = y + height;
}

void Rect::SetLTRB(int left, int top, int right, int bottom)
{
	left_ = left;
	top_ = top;
	right_ = right;
	bottom_ = bottom;
}

void Rect::SetEmpty()
{
	SetLTRB(0, 0, 0, 0);
}

int Rect::left() const
{
	return left_;
}

int Rect::top() const
{
	return top_;
}

int Rect::right() const
{
	return right_;
}

int Rect::bottom() const
{
	return bottom_;
}

int Rect::width() const
{
	return abs(right_ - left_);
}

int Rect::height() const
{
	return abs(bottom_ - top_);
}

void Rect::Offset(int dx, int dy)
{
	left_ += dx;
	right_ += dx;
	top_ += dy;
	bottom_ += dy;
}



Matrix MatrixIdentity()
{
	Matrix matrix;
	matrix.m0.x = 1.0f;
	matrix.m0.y = 0;
	matrix.m0.z = 0;
	matrix.m0.w = 0;

	matrix.m1.x = 0;
	matrix.m1.y = 1.0f;
	matrix.m1.z = 0;
	matrix.m1.w = 0;

	matrix.m2.x = 0;
	matrix.m2.y = 0;
	matrix.m2.z = 1.0f;
	matrix.m2.w = 0;

	matrix.m3.x = 0;
	matrix.m3.y = 0;
	matrix.m3.z = 0;
	matrix.m3.w = 1.0f;
	return matrix;
}

Matrix MatrixOrthographicOffCenterLH(float left, float right, float bottom, float top,
	float nearz, float farz)
{
	Matrix matrix;
	float fReciprocalWidth = 1.0f / (right - left);
	float fReciprocalHeight = 1.0f / (top - bottom);
	float fRange = 1.0f / (farz - nearz);

	matrix.m0.x = fReciprocalWidth * 2;
	matrix.m0.y = 0;
	matrix.m0.z = 0;
	matrix.m0.w = 0;

	matrix.m1.x = 0;
	matrix.m1.y = fReciprocalHeight * 2;
	matrix.m1.z = 0;
	matrix.m1.w = 0;

	matrix.m2.x = 0;
	matrix.m2.y = 0;
	matrix.m2.z = fRange;
	matrix.m2.w = 0;

	matrix.m3.x = -(left + right) * fReciprocalWidth;
	matrix.m3.y = -(top + bottom) * fReciprocalHeight;
	matrix.m3.z = -nearz * fRange;
	matrix.m3.w = 1.0f;
	return matrix;
}

Matrix MatrixTranspose(Matrix & m)
{
	Matrix p;
	Matrix MT;
	p.m0 = VertorMergeXY(m.m0, m.m2);
	p.m1 = VertorMergeXY(m.m1, m.m3);
	p.m2 = VertorMergeZW(m.m0, m.m2);
	p.m3 = VertorMergeZW(m.m1, m.m3);

	MT.m0 = VertorMergeXY(p.m0, p.m1);
	MT.m1 = VertorMergeZW(p.m0, p.m1);
	MT.m2 = VertorMergeXY(p.m2, p.m3);
	MT.m3 = VertorMergeZW(p.m2, p.m3);
	return MT;
}

Float4 VertorMergeXY(Float4 & V1, Float4 & V2)
{
	Float4 result;
	result.x = V1.x;
	result.y = V2.x;
	result.z = V1.y;
	result.w = V2.y;
	return result;
}

Float4 VertorMergeZW(Float4 & V1, Float4 & V2)
{
	Float4 result;
	result.x = V1.z;
	result.y = V2.z;
	result.z = V1.w;
	result.w = V2.w;
	return result;
}

}
}