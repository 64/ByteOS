#include <math.h>
#include <stdint.h>

/*
----------------------------------------------------------------------
Copyright © 2005-2014 Rich Felker, et al.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
----------------------------------------------------------------------
*/

// TODO: Add errno stuff
#define FORCE_EVAL(x) do {                        \
	if (sizeof(x) == sizeof(float)) {         \
		volatile float __x;               \
		__x = (x);                        \
	} else if (sizeof(x) == sizeof(double)) { \
		volatile double __x;              \
		__x = (x);                        \
	} else {                                  \
		volatile long double __x;         \
		__x = (x);                        \
	}                                         \
} while(0)

/* Get two 32 bit ints from a double.  */
#define EXTRACT_WORDS(hi,lo,d)                    \
do {                                              \
  union {double f; uint64_t i;} __u;              \
  __u.f = (d);                                    \
  (hi) = __u.i >> 32;                             \
  (lo) = (uint32_t)__u.i;                         \
} while (0)

/* Get the more significant 32 bit int from a double.  */
#define GET_HIGH_WORD(hi,d)                       \
do {                                              \
  union {double f; uint64_t i;} __u;              \
  __u.f = (d);                                    \
  (hi) = __u.i >> 32;                             \
} while (0)

/* Get the less significant 32 bit int from a double.  */
#define GET_LOW_WORD(lo,d)                        \
do {                                              \
  union {double f; uint64_t i;} __u;              \
  __u.f = (d);                                    \
  (lo) = (uint32_t)__u.i;                         \
} while (0)

/* Set a double from two 32 bit ints.  */
#define INSERT_WORDS(d,hi,lo)                     \
do {                                              \
  union {double f; uint64_t i;} __u;              \
  __u.i = ((uint64_t)(hi)<<32) | (uint32_t)(lo);  \
  (d) = __u.f;                                    \
} while (0)

/* Set the more significant 32 bits of a double from an int.  */
#define SET_HIGH_WORD(d,hi)                       \
do {                                              \
  union {double f; uint64_t i;} __u;              \
  __u.f = (d);                                    \
  __u.i &= 0xffffffff;                            \
  __u.i |= (uint64_t)(hi) << 32;                  \
  (d) = __u.f;                                    \
} while (0)

/* Set the less significant 32 bits of a double from an int.  */
#define SET_LOW_WORD(d,lo)                        \
do {                                              \
  union {double f; uint64_t i;} __u;              \
  __u.f = (d);                                    \
  __u.i &= 0xffffffff00000000ull;                 \
  __u.i |= (uint32_t)(lo);                        \
  (d) = __u.f;                                    \
} while (0)

/* Get a 32 bit int from a float.  */
#define GET_FLOAT_WORD(w,d)                       \
do {                                              \
  union {float f; uint32_t i;} __u;               \
  __u.f = (d);                                    \
  (w) = __u.i;                                    \
} while (0)

/* Set a float from a 32 bit int.  */
#define SET_FLOAT_WORD(d,w)                       \
do {                                              \
  union {float f; uint32_t i;} __u;               \
  __u.i = (w);                                    \
  (d) = __u.f;                                    \
} while (0)

static const double
bp[]   = {1.0, 1.5,},
dp_h[] = { 0.0, 5.84962487220764160156e-01,}, /* 0x3FE2B803, 0x40000000 */
dp_l[] = { 0.0, 1.35003920212974897128e-08,}, /* 0x3E4CFDEB, 0x43CFD006 */
two53  =  9007199254740992.0, /* 0x43400000, 0x00000000 */
huge   =  1.0e300,
tiny   =  1.0e-300,
/* poly coefs for (3/2)*(log(x)-2s-2/3*s**3 */
L1 =  5.99999999999994648725e-01, /* 0x3FE33333, 0x33333303 */
L2 =  4.28571428578550184252e-01, /* 0x3FDB6DB6, 0xDB6FABFF */
L3 =  3.33333329818377432918e-01, /* 0x3FD55555, 0x518F264D */
L4 =  2.72728123808534006489e-01, /* 0x3FD17460, 0xA91D4101 */
L5 =  2.30660745775561754067e-01, /* 0x3FCD864A, 0x93C9DB65 */
L6 =  2.06975017800338417784e-01, /* 0x3FCA7E28, 0x4A454EEF */
P1 =  1.66666666666666019037e-01, /* 0x3FC55555, 0x5555553E */
P2 = -2.77777777770155933842e-03, /* 0xBF66C16C, 0x16BEBD93 */
P3 =  6.61375632143793436117e-05, /* 0x3F11566A, 0xAF25DE2C */
P4 = -1.65339022054652515390e-06, /* 0xBEBBBD41, 0xC5D26BF1 */
P5 =  4.13813679705723846039e-08, /* 0x3E663769, 0x72BEA4D0 */
lg2     =  6.93147180559945286227e-01, /* 0x3FE62E42, 0xFEFA39EF */
lg2_h   =  6.93147182464599609375e-01, /* 0x3FE62E43, 0x00000000 */
lg2_l   = -1.90465429995776804525e-09, /* 0xBE205C61, 0x0CA86C39 */
ovt     =  8.0085662595372944372e-017, /* -(1024-log2(ovfl+.5ulp)) */
cp      =  9.61796693925975554329e-01, /* 0x3FEEC709, 0xDC3A03FD =2/(3ln2) */
cp_h    =  9.61796700954437255859e-01, /* 0x3FEEC709, 0xE0000000 =(float)cp */
cp_l    = -7.02846165095275826516e-09, /* 0xBE3E2FE0, 0x145B01F5 =tail of cp_h*/
ivln2   =  1.44269504088896338700e+00, /* 0x3FF71547, 0x652B82FE =1/ln2 */
ivln2_h =  1.44269502162933349609e+00, /* 0x3FF71547, 0x60000000 =24b 1/ln2*/
ivln2_l =  1.92596299112661746887e-08; /* 0x3E54AE0B, 0xF85DDF44 =1/ln2 tail*/

double pow(double x, double y)
{
	double z,ax,z_h,z_l,p_h,p_l;
	double y1,t1,t2,r,s,t,u,v,w;
	int32_t i,j,k,yisint,n;
	int32_t hx,hy,ix,iy;
	uint32_t lx,ly;

	EXTRACT_WORDS(hx, lx, x);
	EXTRACT_WORDS(hy, ly, y);
	ix = hx & 0x7fffffff;
	iy = hy & 0x7fffffff;

	/* x**0 = 1, even if x is NaN */
	if ((iy|ly) == 0)
		return 1.0;
	/* 1**y = 1, even if y is NaN */
	if (hx == 0x3ff00000 && lx == 0)
		return 1.0;
	/* NaN if either arg is NaN */
	if (ix > 0x7ff00000 || (ix == 0x7ff00000 && lx != 0) ||
	    iy > 0x7ff00000 || (iy == 0x7ff00000 && ly != 0))
		return x + y;

	/* determine if y is an odd int when x < 0
	 * yisint = 0       ... y is not an integer
	 * yisint = 1       ... y is an odd int
	 * yisint = 2       ... y is an even int
	 */
	yisint = 0;
	if (hx < 0) {
		if (iy >= 0x43400000)
			yisint = 2; /* even integer y */
		else if (iy >= 0x3ff00000) {
			k = (iy>>20) - 0x3ff;  /* exponent */
			if (k > 20) {
				j = ly>>(52-k);
				if ((j<<(52-k)) == ly)
					yisint = 2 - (j&1);
			} else if (ly == 0) {
				j = iy>>(20-k);
				if ((j<<(20-k)) == iy)
					yisint = 2 - (j&1);
			}
		}
	}

	/* special value of y */
	if (ly == 0) {
		if (iy == 0x7ff00000) {  /* y is +-inf */
			if (((ix-0x3ff00000)|lx) == 0)  /* (-1)**+-inf is 1 */
				return 1.0;
			else if (ix >= 0x3ff00000) /* (|x|>1)**+-inf = inf,0 */
				return hy >= 0 ? y : 0.0;
			else                       /* (|x|<1)**+-inf = 0,inf */
				return hy >= 0 ? 0.0 : -y;
		}
		if (iy == 0x3ff00000) {    /* y is +-1 */
			if (hy >= 0)
				return x;
			y = 1/x;
#if FLT_EVAL_METHOD!=0
			{
				union {double f; uint64_t i;} u = {y};
				uint64_t i = u.i & -1ULL/2;
				if (i>>52 == 0 && (i&(i-1)))
					FORCE_EVAL((float)y);
			}
#endif
			return y;
		}
		if (hy == 0x40000000)    /* y is 2 */
			return x*x;
		if (hy == 0x3fe00000) {  /* y is 0.5 */
			if (hx >= 0)     /* x >= +0 */
				return sqrt(x);
		}
	}

	ax = fabs(x);
	/* special value of x */
	if (lx == 0) {
		if (ix == 0x7ff00000 || ix == 0 || ix == 0x3ff00000) { /* x is +-0,+-inf,+-1 */
			z = ax;
			if (hy < 0)   /* z = (1/|x|) */
				z = 1.0/z;
			if (hx < 0) {
				if (((ix-0x3ff00000)|yisint) == 0) {
					z = (z-z)/(z-z); /* (-1)**non-int is NaN */
				} else if (yisint == 1)
					z = -z;          /* (x<0)**odd = -(|x|**odd) */
			}
			return z;
		}
	}

	s = 1.0; /* sign of result */
	if (hx < 0) {
		if (yisint == 0) /* (x<0)**(non-int) is NaN */
			return (x-x)/(x-x);
		if (yisint == 1) /* (x<0)**(odd int) */
			s = -1.0;
	}

	/* |y| is huge */
	if (iy > 0x41e00000) { /* if |y| > 2**31 */
		if (iy > 0x43f00000) {  /* if |y| > 2**64, must o/uflow */
			if (ix <= 0x3fefffff)
				return hy < 0 ? huge*huge : tiny*tiny;
			if (ix >= 0x3ff00000)
				return hy > 0 ? huge*huge : tiny*tiny;
		}
		/* over/underflow if x is not close to one */
		if (ix < 0x3fefffff)
			return hy < 0 ? s*huge*huge : s*tiny*tiny;
		if (ix > 0x3ff00000)
			return hy > 0 ? s*huge*huge : s*tiny*tiny;
		/* now |1-x| is tiny <= 2**-20, suffice to compute
		   log(x) by x-x^2/2+x^3/3-x^4/4 */
		t = ax - 1.0;       /* t has 20 trailing zeros */
		w = (t*t)*(0.5 - t*(0.3333333333333333333333-t*0.25));
		u = ivln2_h*t;      /* ivln2_h has 21 sig. bits */
		v = t*ivln2_l - w*ivln2;
		t1 = u + v;
		SET_LOW_WORD(t1, 0);
		t2 = v - (t1-u);
	} else {
		double ss,s2,s_h,s_l,t_h,t_l;
		n = 0;
		/* take care subnormal number */
		if (ix < 0x00100000) {
			ax *= two53;
			n -= 53;
			GET_HIGH_WORD(ix,ax);
		}
		n += ((ix)>>20) - 0x3ff;
		j = ix & 0x000fffff;
		/* determine interval */
		ix = j | 0x3ff00000;   /* normalize ix */
		if (j <= 0x3988E)      /* |x|<sqrt(3/2) */
			k = 0;
		else if (j < 0xBB67A)  /* |x|<sqrt(3)   */
			k = 1;
		else {
			k = 0;
			n += 1;
			ix -= 0x00100000;
		}
		SET_HIGH_WORD(ax, ix);

		/* compute ss = s_h+s_l = (x-1)/(x+1) or (x-1.5)/(x+1.5) */
		u = ax - bp[k];        /* bp[0]=1.0, bp[1]=1.5 */
		v = 1.0/(ax+bp[k]);
		ss = u*v;
		s_h = ss;
		SET_LOW_WORD(s_h, 0);
		/* t_h=ax+bp[k] High */
		t_h = 0.0;
		SET_HIGH_WORD(t_h, ((ix>>1)|0x20000000) + 0x00080000 + (k<<18));
		t_l = ax - (t_h-bp[k]);
		s_l = v*((u-s_h*t_h)-s_h*t_l);
		/* compute log(ax) */
		s2 = ss*ss;
		r = s2*s2*(L1+s2*(L2+s2*(L3+s2*(L4+s2*(L5+s2*L6)))));
		r += s_l*(s_h+ss);
		s2 = s_h*s_h;
		t_h = 3.0 + s2 + r;
		SET_LOW_WORD(t_h, 0);
		t_l = r - ((t_h-3.0)-s2);
		/* u+v = ss*(1+...) */
		u = s_h*t_h;
		v = s_l*t_h + t_l*ss;
		/* 2/(3log2)*(ss+...) */
		p_h = u + v;
		SET_LOW_WORD(p_h, 0);
		p_l = v - (p_h-u);
		z_h = cp_h*p_h;        /* cp_h+cp_l = 2/(3*log2) */
		z_l = cp_l*p_h+p_l*cp + dp_l[k];
		/* log2(ax) = (ss+..)*2/(3*log2) = n + dp_h + z_h + z_l */
		t = (double)n;
		t1 = ((z_h + z_l) + dp_h[k]) + t;
		SET_LOW_WORD(t1, 0);
		t2 = z_l - (((t1 - t) - dp_h[k]) - z_h);
	}

	/* split up y into y1+y2 and compute (y1+y2)*(t1+t2) */
	y1 = y;
	SET_LOW_WORD(y1, 0);
	p_l = (y-y1)*t1 + y*t2;
	p_h = y1*t1;
	z = p_l + p_h;
	EXTRACT_WORDS(j, i, z);
	if (j >= 0x40900000) {                      /* z >= 1024 */
		if (((j-0x40900000)|i) != 0)        /* if z > 1024 */
			return s*huge*huge;         /* overflow */
		if (p_l + ovt > z - p_h)
			return s*huge*huge;         /* overflow */
	} else if ((j&0x7fffffff) >= 0x4090cc00) {  /* z <= -1075 */  // FIXME: instead of abs(j) use unsigned j
		if (((j-0xc090cc00)|i) != 0)        /* z < -1075 */
			return s*tiny*tiny;         /* underflow */
		if (p_l <= z - p_h)
			return s*tiny*tiny;         /* underflow */
	}
	/*
	 * compute 2**(p_h+p_l)
	 */
	i = j & 0x7fffffff;
	k = (i>>20) - 0x3ff;
	n = 0;
	if (i > 0x3fe00000) {  /* if |z| > 0.5, set n = [z+0.5] */
		n = j + (0x00100000>>(k+1));
		k = ((n&0x7fffffff)>>20) - 0x3ff;  /* new k for n */
		t = 0.0;
		SET_HIGH_WORD(t, n & ~(0x000fffff>>k));
		n = ((n&0x000fffff)|0x00100000)>>(20-k);
		if (j < 0)
			n = -n;
		p_h -= t;
	}
	t = p_l + p_h;
	SET_LOW_WORD(t, 0);
	u = t*lg2_h;
	v = (p_l-(t-p_h))*lg2 + t*lg2_l;
	z = u + v;
	w = v - (z-u);
	t = z*z;
	t1 = z - t*(P1+t*(P2+t*(P3+t*(P4+t*P5))));
	r = (z*t1)/(t1-2.0) - (w + z*w);
	z = 1.0 - (r-z);
	GET_HIGH_WORD(j, z);
	j += n<<20;
	if ((j>>20) <= 0)  /* subnormal output */
		z = scalbn(z,n);
	else
		SET_HIGH_WORD(z, j);
	return s*z;
}

double sqrt(double x)
{
	double z;
	int32_t sign = (int)0x80000000;
	int32_t ix0,s0,q,m,t,i;
	uint32_t r,t1,s1,ix1,q1;

	EXTRACT_WORDS(ix0, ix1, x);

	/* take care of Inf and NaN */
	if ((ix0&0x7ff00000) == 0x7ff00000) {
		return x*x + x;  /* sqrt(NaN)=NaN, sqrt(+inf)=+inf, sqrt(-inf)=sNaN */
	}
	/* take care of zero */
	if (ix0 <= 0) {
		if (((ix0&~sign)|ix1) == 0)
			return x;  /* sqrt(+-0) = +-0 */
		if (ix0 < 0)
			return (x-x)/(x-x);  /* sqrt(-ve) = sNaN */
	}
	/* normalize x */
	m = ix0>>20;
	if (m == 0) {  /* subnormal x */
		while (ix0 == 0) {
			m -= 21;
			ix0 |= (ix1>>11);
			ix1 <<= 21;
		}
		for (i=0; (ix0&0x00100000) == 0; i++)
			ix0<<=1;
		m -= i - 1;
		ix0 |= ix1>>(32-i);
		ix1 <<= i;
	}
	m -= 1023;    /* unbias exponent */
	ix0 = (ix0&0x000fffff)|0x00100000;
	if (m & 1) {  /* odd m, double x to make it even */
		ix0 += ix0 + ((ix1&sign)>>31);
		ix1 += ix1;
	}
	m >>= 1;      /* m = [m/2] */

	/* generate sqrt(x) bit by bit */
	ix0 += ix0 + ((ix1&sign)>>31);
	ix1 += ix1;
	q = q1 = s0 = s1 = 0;  /* [q,q1] = sqrt(x) */
	r = 0x00200000;        /* r = moving bit from right to left */

	while (r != 0) {
		t = s0 + r;
		if (t <= ix0) {
			s0   = t + r;
			ix0 -= t;
			q   += r;
		}
		ix0 += ix0 + ((ix1&sign)>>31);
		ix1 += ix1;
		r >>= 1;
	}

	r = sign;
	while (r != 0) {
		t1 = s1 + r;
		t  = s0;
		if (t < ix0 || (t == ix0 && t1 <= ix1)) {
			s1 = t1 + r;
			if ((t1&sign) == sign && (s1&sign) == 0)
				s0++;
			ix0 -= t;
			if (ix1 < t1)
				ix0--;
			ix1 -= t1;
			q1 += r;
		}
		ix0 += ix0 + ((ix1&sign)>>31);
		ix1 += ix1;
		r >>= 1;
	}

	/* use floating add to find out rounding direction */
	if ((ix0|ix1) != 0) {
		z = 1.0 - tiny; /* raise inexact flag */
		if (z >= 1.0) {
			z = 1.0 + tiny;
			if (q1 == (uint32_t)0xffffffff) {
				q1 = 0;
				q++;
			} else if (z > 1.0) {
				if (q1 == (uint32_t)0xfffffffe)
					q++;
				q1 += 2;
			} else
				q1 += q1 & 1;
		}
	}
	ix0 = (q>>1) + 0x3fe00000;
	ix1 = q1>>1;
	if (q&1)
		ix1 |= sign;
	ix0 += m << 20;
	INSERT_WORDS(z, ix0, ix1);
	return z;
}

double scalbn(double x, int n)
{
	union {double f; uint64_t i;} u;
	double_t y = x;

	if (n > 1023) {
		y *= 0x1p1023;
		n -= 1023;
		if (n > 1023) {
			y *= 0x1p1023;
			n -= 1023;
			if (n > 1023)
				n = 1023;
		}
	} else if (n < -1022) {
		y *= 0x1p-1022;
		n += 1022;
		if (n < -1022) {
			y *= 0x1p-1022;
			n += 1022;
			if (n < -1022)
				n = -1022;
		}
	}
	u.i = (uint64_t)(0x3ff+n)<<52;
	x = y * u.f;
	return x;
}

double floor(double x) {
	double dummy;
	return x - modf(x, &dummy);
}


double modf(double x, double *iptr) {
	union {double f; uint64_t i;} u = {x};
	uint64_t mask;
	int e = (int)(u.i>>52 & 0x7ff) - 0x3ff;

	/* no fractional part */
	if (e >= 52) {
		*iptr = x;
		if (e == 0x400 && u.i<<12 != 0) /* nan */
			return x;
		u.i &= 1ULL<<63;
		return u.f;
	}

	/* no integral part*/
	if (e < 0) {
		u.i &= 1ULL<<63;
		*iptr = u.f;
		return x;
	}

	mask = -1ULL>>12>>e;
	if ((u.i & mask) == 0) {
		*iptr = x;
		u.i &= 1ULL<<63;
		return u.f;
	}
	u.i &= ~mask;
	*iptr = u.f;
	return x - u.f;
}

double round(double x) {
	double dummy;
	return (modf(x, &dummy) >= 0.5) ? floor(x + 1) : floor(x);
}

double fmod(double x, double y) {
	return (x - (round(x / y) * y));
}

double fabs(double x) {
	if (x < 0)
		return -x;
	return x;
}

static const double
Lg1 = 6.666666666666735130e-01, /* 3FE55555 55555593 */
Lg2 = 3.999999999940941908e-01, /* 3FD99999 9997FA04 */
Lg3 = 2.857142874366239149e-01, /* 3FD24924 94229359 */
Lg4 = 2.222219843214978396e-01, /* 3FCC71C5 1D8E78AF */
Lg5 = 1.818357216161805012e-01, /* 3FC74664 96CB03DE */
Lg6 = 1.531383769920937332e-01, /* 3FC39A09 D078C69F */
Lg7 = 1.479819860511658591e-01; /* 3FC2F112 DF3E5244 */

/*
 * We always inline __log1p(), since doing so produces a
 * substantial performance improvement (~40% on amd64).
 */
static inline double __log1p(double f)
{
	double hfsq,s,z,R,w,t1,t2;

	s = f/(2.0+f);
	z = s*s;
	w = z*z;
	t1= w*(Lg2+w*(Lg4+w*Lg6));
	t2= z*(Lg1+w*(Lg3+w*(Lg5+w*Lg7)));
	R = t2+t1;
	hfsq = 0.5*f*f;
	return s*(hfsq+R);
}

static const double
two54     = 1.80143985094819840000e+16, /* 0x43500000, 0x00000000 */
ivln10hi  = 4.34294481878168880939e-01, /* 0x3fdbcb7b, 0x15200000 */
ivln10lo  = 2.50829467116452752298e-11, /* 0x3dbb9438, 0xca9aadd5 */
log10_2hi = 3.01029995663611771306e-01, /* 0x3FD34413, 0x509F6000 */
log10_2lo = 3.69423907715893078616e-13; /* 0x3D59FEF3, 0x11F12B36 */

double log10(double x)
{
	double f,hfsq,hi,lo,r,val_hi,val_lo,w,y,y2;
	int32_t i,k,hx;
	uint32_t lx;

	EXTRACT_WORDS(hx, lx, x);

	k = 0;
	if (hx < 0x00100000) {  /* x < 2**-1022  */
		if (((hx&0x7fffffff)|lx) == 0)
			return -two54/0.0;  /* log(+-0)=-inf */
		if (hx<0)
			return (x-x)/0.0;   /* log(-#) = NaN */
		/* subnormal number, scale up x */
		k -= 54;
		x *= two54;
		GET_HIGH_WORD(hx, x);
	}
	if (hx >= 0x7ff00000)
		return x+x;
	if (hx == 0x3ff00000 && lx == 0)
		return 0.0;  /* log(1) = +0 */
	k += (hx>>20) - 1023;
	hx &= 0x000fffff;
	i = (hx+0x95f64)&0x100000;
	SET_HIGH_WORD(x, hx|(i^0x3ff00000));  /* normalize x or x/2 */
	k += i>>20;
	y = (double)k;
	f = x - 1.0;
	hfsq = 0.5*f*f;
	r = __log1p(f);

	/* See log2.c for details. */
	hi = f - hfsq;
	SET_LOW_WORD(hi, 0);
	lo = (f - hi) - hfsq + r;
	val_hi = hi*ivln10hi;
	y2 = y*log10_2hi;
	val_lo = y*log10_2lo + (lo+hi)*ivln10lo + lo*ivln10hi;

	/*
	 * Extra precision in for adding y*log10_2hi is not strictly needed
	 * since there is no very large cancellation near x = sqrt(2) or
	 * x = 1/sqrt(2), but we do it anyway since it costs little on CPUs
	 * with some parallelism and it reduces the error for many args.
	 */
	w = y2 + val_hi;
	val_lo += (y2 - w) + val_hi;
	val_hi = w;

	return val_lo + val_hi;
}
