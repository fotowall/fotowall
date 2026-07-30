/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2009 by Enrico Ros                                      *
 *         2005-2006 Enrico Ros <koral@email.it>                           *
 *         2007-2009 Enrico Ros <enrico.ros@gmail.com>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __enricomath_h__
#define __enricomath_h__

#include <cmath>

// uncomment the following to perform checks on operations (slow, safer)
#define VECTOR_CHECK_MATH

class Vector2;
class Control2;
class Vector3;
class Quaternion;
class Matrix3;

/**
 * @short 2D Vector
 *
 * This class is mainly handled by operators. Common Operators:
 *   +, -, * (scalar product), / (scalar division), [](get x,y components)
 *
 * Plus, we define those Additional ones:
 *   ! return a NORMALIZEd copy of the vector
 *       V1 = !Vector3( 0.21, 0.1, 0.3 ); // 0.564 0.261 0.783
 *
 *   & return the PROJECTION on another VECTOR (often used over a normalized)
 *       V1 & Vector3( 3.123, 0, 0 );     // 1.761 0.000 0.000
 *       V1 & !Vector3( 3.123, 0, 0 );    // 0.553 0.000 0.000
 */

class Vector2
{
    public:
        Vector2();
        Vector2( double x, double y );

        static const Vector2 X, Y;
        static const Vector2 Null;

        inline bool isNull() const;
        inline double x() const;
        inline double y() const;
        inline double module() const;
        inline double angle() const;
        inline double projectedLen( const Vector2 & v ) const;

        inline double & rx();
        inline double & ry();

        // operations
        inline Vector2 & setX( double );
        inline Vector2 & setY( double );
        inline Vector2 & reset();

        // operators
        inline Vector2 & operator+=( const Vector2 & );
        inline Vector2 & operator-=( const Vector2 & );
        inline Vector2 & operator*=( double );
        inline Vector2 & operator/=( double );
        inline Vector2 & operator&=( const Vector2 & );

        friend inline bool operator==( const Vector2 &, const Vector2 & );
        friend inline bool operator!=( const Vector2 &, const Vector2 & );
        friend inline const Vector2 operator!( const Vector2 & );
        friend inline const Vector2 operator+( const Vector2 &, const Vector2 & );
        friend inline const Vector2 operator-( const Vector2 &, const Vector2 & );
        friend inline const Vector2 operator*( const Vector2 &, double );
        friend inline const Vector2 operator*( double, const Vector2 & );
        friend inline const Vector2 operator/( const Vector2 &, double );
        friend inline const Vector2 operator&( const Vector2 &, const Vector2 & );

        inline void dump() const;

    private:
        double m_x, m_y;
};


/**
 * \brief 2D Control Point
 * ...
 */

class Control2
{
    public:
        Control2();
        Control2( double ro, double theta );

        static const Control2 X, Y;
        static const Control2 Null;
        //static fromXY( double x, double y );

        inline bool isNull() const;
        inline double ro() const;
        inline double theta() const;
        inline Vector2 toVector2() const;

        // operations
        inline Control2 & setRo( double );
        inline Control2 & setTheta( double );
        inline Control2 & addRo( double );
        inline Control2 & addTheta( double );

        // operators
        friend inline bool operator==( const Control2 &, const Control2 & );
        friend inline bool operator!=( const Control2 &, const Control2 & );

        inline void dump() const;

    private:
        double m_ro, m_theta;
};

/**
 * @short 3D Vector
 *
 * This class is mainly handled by operators. Common Operators:
 *   +, -, * (scalar product), / (scalar division), [](get x,y,z components)
 *
 * Plus, we define those Additional ones:
 *   ! return a NORMALIZEd copy of the vector
 *       V1 = !Vector3( 0.21, 0.1, 0.3 ); // 0.564 0.261 0.783
 *
 *   & return the PROJECTION on another VECTOR (often used over a normalized)
 *       V1 & Vector3( 3.123, 0, 0 );     // 1.761 0.000 0.000
 *       V1 & !Vector3( 3.123, 0, 0 );    // 0.553 0.000 0.000
 *
 *   ^ vector PRODUCT. returned vector is normal to both operands
 *       V2 = V1 ^ Vector3( 0, 3, 0 );    // 0.000 0.000 1.659
 */

class Vector3
{
    public:
        Vector3();
        Vector3( double x, double y, double z );

        static const Vector3 X, Y, Z;
        static const Vector3 Null;

        inline bool isNull() const;
        inline double x() const;
        inline double y() const;
        inline double z() const;
        inline double module() const;
        inline double projectedLen( const Vector3 & v ) const;

        // operations
        inline Vector3 & setX( double );
        inline Vector3 & setY( double );
        inline Vector3 & setZ( double );
        inline Vector3 & rotate( double radians, const Vector3 & axis );
        inline Vector3 & rotate( const Quaternion & quat );

        // operators
        inline Vector3 & operator+=( const Vector3 & );
        inline Vector3 & operator-=( const Vector3 & );
        inline Vector3 & operator*=( double );
        inline Vector3 & operator*=( const Quaternion & );
        inline Vector3 & operator*=( const Matrix3 & );
        inline Vector3 & operator/=( double );
        inline Vector3 & operator&=( const Vector3 & );
        inline Vector3 & operator^=( const Vector3 & );

        friend inline bool operator==( const Vector3 &, const Vector3 & );
        friend inline bool operator!=( const Vector3 &, const Vector3 & );
        friend inline const Vector3 operator!( const Vector3 & );
        friend inline const Vector3 operator+( const Vector3 &, const Vector3 & );
        friend inline const Vector3 operator-( const Vector3 &, const Vector3 & );
        friend inline const Vector3 operator*( const Vector3 &, double );
        friend inline const Vector3 operator*( double, const Vector3 & );
        friend inline const Vector3 operator*( const Vector3 &, const Quaternion & );
        friend inline const Vector3 operator*( const Matrix3 &, const Vector3 & );
        friend inline const Vector3 operator/( const Vector3 &, double );
        friend inline const Vector3 operator&( const Vector3 &, const Vector3 & );
        friend inline const Vector3 operator^( const Vector3 &, const Vector3 & );

        inline void dump() const;

    private:
        double m_x, m_y, m_z;
};

/**
 * @short Quaternion.
 *
 * Mainly for representing rotations, this class identifies a quaternion.
 * There is only 1 implemented operator:
 *   * quaternions multiplication. adds the second rotation the the first
 *       Q1 = Quaternion( +PI/2, Vector3(1,0,0) );  // +90deg over X
 *       Q2 = Quaternion( -PI/2, Vector3(0,1,0) );  // -90deg over Y
 *       (Q1*Q2).dump();                  // +120deg over 0.577 -0.577 0.577
 */
class Quaternion
{
    public:
        Quaternion();
        Quaternion( double radians, const Vector3 & axis );
        Quaternion( double yaw, double pitch, double roll );
        Quaternion( double a, double b, double c, double d );

        static const Quaternion Null;

        inline bool isNull() const;
        inline bool isIdentity() const;
        inline double yaw() const;
        inline double pitch() const;
        inline double roll() const;
        inline double angle() const;
        inline const Vector3 axis() const;
        inline double module() const;

        // operations
        inline void normalize();

        // operators
        inline Quaternion & operator*=( const Quaternion & );
        friend inline const Quaternion operator!( const Quaternion & );
        friend inline const Quaternion operator*( const Quaternion &, const Quaternion & );

        inline void dump() const;

    private:
        friend class Vector3;
        double m_a, m_b, m_c, m_d;
};

/**
 * @short 3x3 Matrix.
 * ..used only by Vector3 for now..
 */
class Matrix3
{
    public:
        Matrix3();
        Matrix3( double a1, double a2, double a3,
                 double b1, double b2, double b3,
                 double c1, double c2, double c3 );

        static const Matrix3 Identity;
        static const Matrix3 Null;

        // operators
        inline Matrix3 & operator+=( const Matrix3 & );
        inline Matrix3 & operator-=( const Matrix3 & );
        inline Matrix3 & operator*=( double );

        friend inline bool operator==( const Matrix3 &, const Matrix3 & );
        friend inline bool operator!=( const Matrix3 &, const Matrix3 & );
        friend inline const Matrix3 operator+( const Matrix3 &, const Matrix3 & );
        friend inline const Matrix3 operator-( const Matrix3 &, const Matrix3 & );
        friend inline const Matrix3 operator*( const Matrix3 &, double );
        friend inline const Matrix3 operator*( double, const Matrix3 & );
        friend inline const Vector3 operator*( const Matrix3 &, const Vector3 & );

        inline void dump() const;

    private:
        friend class Vector3;
        double m_a1, m_a2, m_a3, m_b1, m_b2, m_b3, m_c1, m_c2, m_c3;
};


/****************************************************************************
 inline functions
 ****************************************************************************/
#include <math.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.141592653589793238462643
#endif // !M_PI

/****************************************************************************
 Vector2 - inline functions
 ****************************************************************************/
inline Vector2::Vector2()
    : m_x( 0.0 ), m_y( 0.0 ) {}

inline Vector2::Vector2( double x, double y )
    : m_x( x ), m_y( y ) {}

inline bool Vector2::isNull() const
{ return m_x == 0.0 && m_y == 0.0; }

inline double Vector2::x() const
{ return m_x; }

inline double Vector2::y() const
{ return m_y; }

inline double Vector2::module() const
{ return sqrt( m_x*m_x + m_y*m_y ); }

inline double Vector2::angle() const
{ return atan2( m_y, m_x ); }

inline double Vector2::projectedLen( const Vector2 & v2 ) const
{ return m_x*v2.m_x + m_y*v2.m_y; }

inline double & Vector2::rx()
{ return m_x; }

inline double & Vector2::ry()
{ return m_y; }

inline Vector2 & Vector2::setX( double x )
{ m_x = x; return *this; }

inline Vector2 & Vector2::setY( double y )
{ m_y = y; return *this; }

inline Vector2 & Vector2::reset()
{ m_x = 0; m_y = 0; return *this; }

inline Vector2 & Vector2::operator+=( const Vector2 &v )
{ m_x+=v.m_x; m_y+=v.m_y; return *this; }

inline Vector2 & Vector2::operator-=( const Vector2 &v )
{ m_x-=v.m_x; m_y-=v.m_y; return *this; }

inline Vector2 & Vector2::operator*=( double a )
{ m_x *= a; m_y *= a; return *this; }

inline Vector2 & Vector2::operator/=( double a )
{
#ifdef VECTOR_CHECK_MATH
    if ( a == 0.0 )
        fprintf(stderr,"Vector2: scalar division by zero.\n");
#endif
    m_x /= a; m_y /= a;
    return *this;
}

inline Vector2 & Vector2::operator&=( const Vector2 & v )
{
    if ( isNull() || v.isNull() )
        m_x = m_y = 0.0;
    else {
        double mod = m_x*v.m_x + m_y*v.m_y; // |this| * |v| * cos(angle)
        mod /= v.module(); // 'prenormalize' v before applying 'len'
        m_x = mod * v.m_x; m_y = mod * v.m_y;
    }
    return *this;
}

inline bool operator==( const Vector2 &v1, const Vector2 &v2 )
{ return v1.m_x == v2.m_x && v1.m_y == v2.m_y; }

inline bool operator!=( const Vector2 &v1, const Vector2 &v2 )
{ return v1.m_x != v2.m_x || v1.m_y != v2.m_y; }

inline const Vector2 operator!( const Vector2 &v )
{
#ifdef VECTOR_CHECK_MATH
    if ( v.isNull() ) {
        fprintf(stderr,"Vector2: normalizing a null vector.\n");
        return Vector2( 0.0, 0.0 );
    }
#endif
    double mod = v.module();
    return Vector2( v.m_x / mod, v.m_y / mod );
}

inline const Vector2 operator+( const Vector2 & v1, const Vector2 & v2 )
{ return Vector2( v1.m_x+v2.m_x, v1.m_y+v2.m_y ); }

inline const Vector2 operator-( const Vector2 & v1, const Vector2 & v2 )
{ return Vector2( v1.m_x-v2.m_x, v1.m_y-v2.m_y ); }

inline const Vector2 operator*( const Vector2 & v, double a )
{ return Vector2( a*v.m_x, a*v.m_y ); }

inline const Vector2 operator*( double a, const Vector2 &v )
{ return Vector2( a*v.m_x, a*v.m_y ); }

inline const Vector2 operator/( const Vector2 &v, double a )
{
#ifdef VECTOR_CHECK_MATH
    if ( a == 0.0 )
        fprintf(stderr,"Vector2: scalar division by zero.\n");
#endif
    return Vector2( v.m_x/a, v.m_y/a );
}

inline const Vector2 operator&( const Vector2 & v1, const Vector2 & v2 )
{
    if ( v1.isNull() || v2.isNull() )
        return Vector2( 0.0, 0.0 );
    double mod = v1.m_x*v2.m_x + v1.m_y*v2.m_y; // |v1| * |v2| * cos(angle)
    mod /= v2.module(); // 'prenormalize' v2 before applying 'len'
    return Vector2( mod * v2.m_x, mod * v2.m_y );
}

inline void Vector2::dump() const
{ printf( "%f %f [%f]\n", m_x, m_y, module() ); }



/****************************************************************************
 Control2 - inline functions
 ****************************************************************************/
inline Control2::Control2()
    : m_ro( 0.0 ), m_theta( 0.0 ) {}

inline Control2::Control2( double ro, double theta )
    : m_ro( ro ), m_theta( theta ) {}

inline bool Control2::isNull() const
{ return m_ro == 0.0 && m_theta == 0.0; }

inline double Control2::ro() const
{ return m_ro; }

inline double Control2::theta() const
{ return m_theta; }

Vector2 Control2::toVector2() const
{ return Vector2( m_ro * cos(m_theta), m_ro * sin(m_theta) ); }

inline Control2 & Control2::setRo( double ro )
{ m_ro = ro; return *this; }

inline Control2 & Control2::setTheta( double theta )
{ m_theta = theta; return *this; }

inline Control2 & Control2::addRo( double ro )
{ m_ro += ro; return *this; }

inline Control2 & Control2::addTheta( double theta )
{ m_theta += theta; return *this; }

inline bool operator==( const Control2 &c1, const Control2 &c2 )
{ return c1.m_ro == c2.m_ro && c1.m_theta == c1.m_theta; }

inline bool operator!=( const Control2 &c1, const Control2 &c2 )
{ return c1.m_ro != c2.m_ro || c1.m_theta != c2.m_theta; }

void Control2::dump() const
{ printf( "%f %f deg\n", m_ro, 180.0 * m_theta / M_PI ); }



/****************************************************************************
 Vector3 - inline functions
 ****************************************************************************/
inline Vector3::Vector3()
    : m_x( 0.0 ), m_y( 0.0 ), m_z( 0.0 ) {}

inline Vector3::Vector3( double x, double y, double z )
    : m_x( x ), m_y( y ), m_z( z ) {}

inline bool Vector3::isNull() const
{ return m_x == 0.0 && m_y == 0.0 && m_z == 0.0; }

inline double Vector3::x() const
{ return m_x; }

inline double Vector3::y() const
{ return m_y; }

inline double Vector3::z() const
{ return m_z; }

inline double Vector3::module() const
{ return sqrt( m_x*m_x + m_y*m_y + m_z*m_z ); }

inline double Vector3::projectedLen( const Vector3 & v2 ) const
{ return m_x*v2.m_x + m_y*v2.m_y + m_z*v2.m_z; }

inline Vector3 & Vector3::setX( double x )
{ m_x = x; return *this; }

inline Vector3 & Vector3::setY( double y )
{ m_y = y; return *this; }

inline Vector3 & Vector3::setZ( double z )
{ m_z = z; return *this; }

inline Vector3 & Vector3::rotate( double radians, const Vector3 & axis )
/// rotazione con Quaternione! [2pg di polinomi..3h di conti.. :-]
/// versione superfast da 0.24 microsecondi a rotazione ;-))
{
#ifdef VECTOR_CHECK_MATH
    if ( axis.isNull() ) {
        printf("Vector3: rotating around a null axis!\n");
        return *this;
    }
#endif
    // definisce costanti del quaternione (re: rot, imm: axis)
    auto h_radians = radians / 2.0;
    auto mSin = std::sin(h_radians);
    auto mCos = std::cos(h_radians);
    mSin /= axis.module();
    double a = mCos, b = mSin*axis.m_x, c = mSin*axis.m_y, d = mSin*axis.m_z;
    // precalcola 10 costanti di rotazione
    double a2 = a*a,   b2 = b*b,   c2 = c*c,   d2 = d*d,
           ab = 2*a*b, ac = 2*a*c, ad = 2*a*d,
           bc = 2*b*c, bd = 2*b*d, cd = 2*c*d;
    // calcola le nuove componenti ruotate
    double i = (a2+b2-c2-d2) * m_x + (bc-ad) * m_y + (ac+bd) * m_z,
           j = (ad+bc) * m_x + (a2-b2+c2-d2) * m_y + (cd-ab) * m_z,
           k = (bd-ac) * m_x + (ab+cd) * m_y + (a2-b2-c2+d2) * m_z;
    // ed ecco il vettore ruotato!
    m_x = i; m_y = j; m_z = k;
    return *this;
}

inline Vector3 & Vector3::rotate( const Quaternion & quat )
{
    // prende i coefficienti dal quaternione
    double a = quat.m_a, b = quat.m_b, c = quat.m_c, d = quat.m_d;
    // precalcola 10 costanti di rotazione
    double a2 = a*a,   b2 = b*b,   c2 = c*c,   d2 = d*d,
           ab = 2*a*b, ac = 2*a*c, ad = 2*a*d,
           bc = 2*b*c, bd = 2*b*d, cd = 2*c*d;
    // calcola le nuove componenti ruotate
    double i = (a2+b2-c2-d2) * m_x + (bc-ad) * m_y + (ac+bd) * m_z,
           j = (ad+bc) * m_x + (a2-b2+c2-d2) * m_y + (cd-ab) * m_z,
           k = (bd-ac) * m_x + (ab+cd) * m_y + (a2-b2-c2+d2) * m_z;
    // ed ecco il vettore ruotato!
    m_x = i; m_y = j; m_z = k;
    return *this;
}

inline Vector3 & Vector3::operator+=( const Vector3 &v )
{ m_x+=v.m_x; m_y+=v.m_y; m_z+=v.m_z; return *this; }

inline Vector3 & Vector3::operator-=( const Vector3 &v )
{ m_x-=v.m_x; m_y-=v.m_y; m_z-=v.m_z; return *this; }

inline Vector3 & Vector3::operator*=( double a )
{ m_x *= a; m_y *= a; m_z *= a; return *this; }

inline Vector3 & Vector3::operator*=( const Quaternion & quat )
{ return rotate( quat ); }

inline Vector3 & Vector3::operator*=( const Matrix3 & m )
{
    double a = m.m_a1 * m_x + m.m_a2 * m_y + m.m_a3 * m_z,
           b = m.m_b1 * m_x + m.m_b2 * m_y + m.m_b3 * m_z,
           c = m.m_c1 * m_x + m.m_c2 * m_y + m.m_c3 * m_z;
    m_x = a, m_y = b, m_z = c; return *this;
}

inline Vector3 & Vector3::operator/=( double a )
{
#ifdef VECTOR_CHECK_MATH
    if ( a == 0.0 )
        fprintf(stderr,"Vector3: scalar division by zero.\n");
#endif
    m_x /= a; m_y /= a; m_z /= a;
    return *this;
}

inline Vector3 & Vector3::operator&=( const Vector3 & v )
{
    if ( isNull() || v.isNull() )
        m_x = m_y = m_z = 0.0;
    else {
        double mod = m_x*v.m_x + m_y*v.m_y + m_z*v.m_z; // |this| * |v| * cos(angle)
        mod /= v.module(); // 'prenormalize' v before applying 'len'
        m_x = mod * v.m_x; m_y = mod * v.m_y; m_z = mod * v.m_z;
    }
    return *this;
}

inline Vector3 & Vector3::operator^=( const Vector3 & v )
{
    double i = m_y*v.m_z - m_z*v.m_y, j = m_z*v.m_x - m_x*v.m_z, k = m_x*v.m_y - m_y*v.m_x;
    m_x = i; m_y = j; m_z = k;
    return * this;
}

inline bool operator==( const Vector3 &v1, const Vector3 &v2 )
{ return v1.m_x == v2.m_x && v1.m_y == v2.m_y && v1.m_z == v2.m_z; }

inline bool operator!=( const Vector3 &v1, const Vector3 &v2 )
{ return v1.m_x != v2.m_x || v1.m_y != v2.m_y || v1.m_z != v2.m_z; }

inline const Vector3 operator!( const Vector3 &v )
{
#ifdef VECTOR_CHECK_MATH
    if ( v.isNull() ) {
        fprintf(stderr,"Vector3: normalizing a null vector.\n");
        return Vector3( 0.0, 0.0, 0.0 );
    }
#endif
    double mod = v.module();
    return Vector3( v.m_x / mod, v.m_y / mod, v.m_z / mod );
}

inline const Vector3 operator+( const Vector3 & v1, const Vector3 & v2 )
{ return Vector3( v1.m_x+v2.m_x, v1.m_y+v2.m_y, v1.m_z+v2.m_z ); }

inline const Vector3 operator-( const Vector3 & v1, const Vector3 & v2 )
{ return Vector3( v1.m_x-v2.m_x, v1.m_y-v2.m_y, v1.m_z-v2.m_z ); }

inline const Vector3 operator*( const Vector3 & v, double a )
{ return Vector3( a*v.m_x, a*v.m_y, a*v.m_z ); }

inline const Vector3 operator*( double a, const Vector3 &v )
{ return Vector3( a*v.m_x, a*v.m_y, a*v.m_z ); }

inline const Vector3 operator*( const Vector3 & v, const Quaternion & quat )
{ return Vector3( v ).rotate( quat ); }

inline const Vector3 operator*( const Matrix3 & m, const Vector3 & v )
{ return Vector3( m.m_a1 * v.m_x + m.m_a2 * v.m_y + m.m_a3 * v.m_z,
                  m.m_b1 * v.m_x + m.m_b2 * v.m_y + m.m_b3 * v.m_z,
                  m.m_c1 * v.m_x + m.m_c2 * v.m_y + m.m_c3 * v.m_z ); }

inline const Vector3 operator/( const Vector3 &v, double a )
{
#ifdef VECTOR_CHECK_MATH
    if ( a == 0.0 )
        fprintf(stderr,"Vector3: scalar division by zero.\n");
#endif
    return Vector3( v.m_x/a, v.m_y/a, v.m_z/a );
}

inline const Vector3 operator&( const Vector3 & v1, const Vector3 & v2 )
{
    if ( v1.isNull() || v2.isNull() )
        return Vector3( 0.0, 0.0, 0.0 );
    double mod = v1.m_x*v2.m_x + v1.m_y*v2.m_y + v1.m_z*v2.m_z; // |v1| * |v2| * cos(angle)
    mod /= v2.module(); // 'prenormalize' v2 before applying 'len'
    return Vector3( mod * v2.m_x, mod * v2.m_y, mod * v2.m_z );
}

inline const Vector3 operator^( const Vector3 & v1, const Vector3 & v2 )
{ return Vector3( v1.m_y*v2.m_z - v1.m_z*v2.m_y, v1.m_z*v2.m_x - v1.m_x*v2.m_z,
                  v1.m_x*v2.m_y - v1.m_y*v2.m_x ); }

inline void Vector3::dump() const
{ printf( "%f %f %f [%f]\n", m_x, m_y, m_z, module() ); }



/****************************************************************************
 Quaternion - inline functions
 ****************************************************************************/
inline Quaternion::Quaternion()
    : m_a( 1.0 ), m_b( 0.0 ), m_c( 0.0 ), m_d( 0.0 ) {}

inline Quaternion::Quaternion( double radians, const Vector3 & axis )
    : m_a( 1.0 ), m_b( 0.0 ), m_c( 0.0 ), m_d( 0.0 )
{
    double r = radians / 2.0;
    auto mSin = std::sin(r);
    auto mCos = std::cos(r);
#ifdef VECTOR_CHECK_MATH
    if ( axis.isNull() ) {
        printf("Quaternion: constructing one with a null axis!\n");
        return;
    }
#endif
    mSin /= axis.module();
    m_a = mCos, m_b = mSin*axis.x(), m_c = mSin*axis.y(), m_d = mSin*axis.z();
}

inline Quaternion::Quaternion( double yaw, double pitch, double roll )
{
    double h_yaw = yaw / 2.0;
    double h_pitch = pitch / 2.0;
    double h_roll = roll / 2.0;
    auto s_yaw = std::sin(h_yaw);
    auto c_yaw = std::cos(h_yaw);
    auto s_pitch = std::sin(h_pitch);
    auto c_pitch = std::cos(h_pitch);
    auto s_roll = std::sin(h_roll);
    auto c_roll = std::cos(h_roll);
    m_a = c_roll * c_pitch * c_yaw + s_roll * s_pitch * s_yaw;
    m_b = s_roll * c_pitch * c_yaw - c_roll * s_pitch * s_yaw;
    m_c = c_roll * s_pitch * c_yaw + s_roll * c_pitch * s_yaw;
    m_d = c_roll * c_pitch * s_yaw - s_roll * s_pitch * c_yaw;
}

inline Quaternion::Quaternion( double a, double b, double c, double d )
    : m_a( a ), m_b( b ), m_c( c ), m_d( d ) {}

inline bool Quaternion::isNull() const
{ return /*m_a == 0.0 &&*/ m_b == 0.0 && m_c == 0.0 && m_d == 0.0; }

inline bool Quaternion::isIdentity() const
{ return m_a == 1.0 && m_b == 0.0 && m_c == 0.0 && m_d == 0.0; }

inline double Quaternion::yaw() const
{ return atan2( 2*(m_b*m_c + m_a*m_d), m_a*m_a + m_b*m_b - m_c*m_c - m_d*m_d ); }

inline double Quaternion::pitch() const
{ return asin( -2*(m_b*m_d - m_a*m_c) ); }

inline double Quaternion::roll() const
{ return atan2( 2*(m_c*m_d + m_a*m_b), m_a*m_a - m_b*m_b - m_c*m_c + m_d*m_d ); }

inline double Quaternion::angle() const
{ return 2.0 * acos( m_a ); }

inline const Vector3 Quaternion::axis() const
{
    // if not rotating, axis is undefined
    if ( m_a >= 1.0 || m_a <= -1.0 )
        return Vector3();
    // divide m_x by module to get back vector dimensions
    double mod = sqrt( 1 - m_a*m_a );
    return Vector3( m_b/mod, m_c/mod, m_d/mod );
}

inline double Quaternion::module() const
{ return sqrt( m_a*m_a + m_b*m_b + m_c*m_c + m_d*m_d ); }

inline void Quaternion::normalize()
{
    if ( m_a == 0.0 && m_b == 0.0 && m_c == 0.0 && m_d == 0.0 ) {
#ifdef VECTOR_CHECK_MATH
        fprintf(stderr,"Quaternion: Normalizing a null quaternion!\n");
#endif
        return;
    }
    double mod = 1/sqrt( m_a*m_a + m_b*m_b + m_c*m_c + m_d*m_d );
    m_a *= mod; m_b *= mod; m_c *= mod; m_d *= mod;
}

inline Quaternion & Quaternion::operator*=( const Quaternion & q2 )
{
    double a = q2.m_a*m_a - q2.m_b*m_b - q2.m_c*m_c - q2.m_d*m_d,
           b = q2.m_a*m_b + q2.m_b*m_a + q2.m_c*m_d - q2.m_d*m_c,
           c = q2.m_a*m_c + q2.m_c*m_a + q2.m_d*m_b - q2.m_b*m_d,
           d = q2.m_a*m_d + q2.m_d*m_a + q2.m_b*m_c - q2.m_c*m_b;
    m_a = a; m_b = b; m_c = c; m_d = d;
    return *this;
}

inline const Quaternion operator!( const Quaternion & q )
{ return Quaternion( q.m_a, -q.m_b, -q.m_c, -q.m_d ); }

inline const Quaternion operator*( const Quaternion & q2, const Quaternion & q1 )
{
    double a = q1.m_a*q2.m_a - q1.m_b*q2.m_b - q1.m_c*q2.m_c - q1.m_d*q2.m_d,
           b = q1.m_a*q2.m_b + q1.m_b*q2.m_a + q1.m_c*q2.m_d - q1.m_d*q2.m_c,
           c = q1.m_a*q2.m_c + q1.m_c*q2.m_a + q1.m_d*q2.m_b - q1.m_b*q2.m_d,
           d = q1.m_a*q2.m_d + q1.m_d*q2.m_a + q1.m_b*q2.m_c - q1.m_c*q2.m_b;
    return Quaternion( a, b, c, d );
}

inline void Quaternion::dump() const
{
    printf("a:%f b:%f c:%f d:%f -- angle: %f vec:",m_a,m_b,m_c,m_d,angle());
    axis().dump();
}



/****************************************************************************
 Matrix3 - inline functions
 ****************************************************************************/
inline Matrix3::Matrix3()
    : m_a1( 0.0 ), m_a2( 0.0 ), m_a3( 0.0 ),
    m_b1( 0.0 ), m_b2( 0.0 ), m_b3( 0.0 ),
    m_c1( 0.0 ), m_c2( 0.0 ), m_c3( 0.0 ) {}

inline Matrix3::Matrix3( double a1, double a2, double a3,
    double b1, double b2, double b3, double c1, double c2, double c3 )
    : m_a1( a1 ), m_a2( a2 ), m_a3( a3 ),
    m_b1( b1 ), m_b2( b2 ), m_b3( b3 ),
    m_c1( c1 ), m_c2( c2 ), m_c3( c3 ) {}

inline void Matrix3::dump() const
{
    printf("< %.2f\t%.2f\t%.2f\t|\n",m_a1, m_a2, m_a3);
    printf("| %.2f\t%.2f\t%.2f\t|\n",m_b1, m_b2, m_b3);
    printf("| %.2f\t%.2f\t%.2f\t>\n",m_c1, m_c2, m_c3);
}

inline Matrix3 & Matrix3::operator+=( const Matrix3 &b )
{   m_a1 += b.m_a1; m_a2 += b.m_a2; m_a3 += b.m_a3;
    m_b1 += b.m_b1; m_b2 += b.m_b2; m_b3 += b.m_b3;
    m_c1 += b.m_c1; m_c2 += b.m_c2; m_c3 += b.m_c3;
    return *this; }

inline Matrix3 & Matrix3::operator-=( const Matrix3 &b )
{   m_a1 -= b.m_a1; m_a2 -= b.m_a2; m_a3 -= b.m_a3;
    m_b1 -= b.m_b1; m_b2 -= b.m_b2; m_b3 -= b.m_b3;
    m_c1 -= b.m_c1; m_c2 -= b.m_c2; m_c3 -= b.m_c3;
    return *this; }

inline Matrix3 & Matrix3::operator*=( double k )
{   m_a1 *= k; m_a2 *= k; m_a3 *= k;
    m_b1 *= k; m_b2 *= k; m_b3 *= k;
    m_c1 *= k; m_c2 *= k; m_c3 *= k;
    return *this; }

inline bool operator==( const Matrix3 &a, const Matrix3 &b )
{ return a.m_a1 == b.m_a1 && a.m_a2 == b.m_a2 && a.m_a3 == b.m_a3 &&
         a.m_b1 == b.m_b1 && a.m_b2 == b.m_b2 && a.m_b3 == b.m_b3 &&
         a.m_c1 == b.m_c1 && a.m_c2 == b.m_c2 && a.m_c3 == b.m_c3; }

inline bool operator!=( const Matrix3 &a, const Matrix3 &b )
{ return a.m_a1 != b.m_a1 || a.m_a2 != b.m_a2 || a.m_a3 != b.m_a3 ||
         a.m_b1 != b.m_b1 || a.m_b2 != b.m_b2 || a.m_b3 != b.m_b3 ||
         a.m_c1 != b.m_c1 || a.m_c2 != b.m_c2 || a.m_c3 != b.m_c3; }

inline const Matrix3 operator+( const Matrix3 &a, const Matrix3 &b )
{ return Matrix3( a.m_a1 + b.m_a1, a.m_a2 + b.m_a2, a.m_a3 + b.m_a3,
                  a.m_b1 + b.m_b1, a.m_b2 + b.m_b2, a.m_b3 + b.m_b3,
                  a.m_c1 + b.m_c1, a.m_c2 + b.m_c2, a.m_c3 + b.m_c3 ); }

inline const Matrix3 operator-( const Matrix3 &a, const Matrix3 &b )
{ return Matrix3( a.m_a1 - b.m_a1, a.m_a2 - b.m_a2, a.m_a3 - b.m_a3,
                  a.m_b1 - b.m_b1, a.m_b2 - b.m_b2, a.m_b3 - b.m_b3,
                  a.m_c1 - b.m_c1, a.m_c2 - b.m_c2, a.m_c3 - b.m_c3 ); }

inline const Matrix3 operator*( const Matrix3 & a, double k )
{ return Matrix3( a.m_a1 * k, a.m_a2 * k, a.m_a3 * k,
                  a.m_b1 * k, a.m_b2 * k, a.m_b3 * k,
                  a.m_c1 * k, a.m_c2 * k, a.m_c3 * k ); }

inline const Matrix3 operator*( double k, const Matrix3 & a )
{ return a*k; }

#endif
