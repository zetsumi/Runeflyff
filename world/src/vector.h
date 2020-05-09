#ifndef __vector_h__
#define __vector_h__

#include <cmath>

template < class T = float >
class vector3d
{
public:
    T x,y,z;
	void set(T a, T b, T c)
	{
		x=a;
		y=b;
		z=c;
	}
    vector3d(){}
	vector3d(T a,T b, T c):x(a),y(b),z(c){}
	vector3d(const vector3d<T>& a):x(a.x),y(a.y),z(a.z){}
	vector3d<T>& operator=(const vector3d<T>& a)
    {
		x=a.x;
		y=a.y;
		z=a.z;
        return *this;
    }
    vector3d operator+(const vector3d<T>& a) const
    {
        return vector3d(x+a.x, y+a.y, z+a.z);
    }
    vector3d operator-(const vector3d<T>& a) const
    {
        return vector3d(x-a.x, y-a.y, z-a.z);
    }
    vector3d<T>& operator+=(const vector3d<T>& a)
    {
        x+=a.x;
        y+=a.y;
        z+=a.z;
        return *this;
    }
    vector3d<T>& operator-=(const vector3d<T>& a)
    {
        x-=a.x;
        y-=a.y;
        z-=a.z;
        return *this;
    }
    vector3d operator*(T a) const
    {
        return vector3d<T>(x*a, y*a, z*a);
    }
    vector3d operator/(T a) const
    {
        return vector3d<T>(x/a,y/a,z/a);
    }
    vector3d<T>& operator*=(T a)
    {
        x*=a;
        y*=a;
        z*=a;
        return *this;
    }
    vector3d<T>& operator/=(T a)
    {
        x/=a;
        y/=a;
        z/=a;
        return *this;
    }
    T operator *(const vector3d<T> &a) const
    {
        return x*a.x+y*a.y+z*a.z;
    }
    vector3d operator% (const vector3d<T> &a) const
    {
        return vector3d(y*a.z-z*a.y, z*a.x-x*a.z, x*a.y-y*a.x);
    }
    T length() const
    {
        return sqrt(x*x+y*y+z*z);
    }
    T sqrlength() const
    {
        return x*x+y*y+z*z;
    }
	vector3d<T>& operator+()
	{
		return *this;
	}
	const vector3d<T>& operator+() const
	{
		return *this;
	}
	vector3d<T> operator-() const
	{
		return vector3d<T>(-x,-y,-z);
	}
	bool operator==(const vector3d<T>& a) const
	{
		return((x==a.x)&&(y==a.y)&&(z==a.z));
	}
	bool operator!=(const vector3d<T>& a) const
	{
		return((x!=a.x)||(y!=a.y)||(z!=a.z));
	}
};

#endif
