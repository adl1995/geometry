// Boost.Geometry

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_KARNEY_DIRECT_HPP
#define BOOST_GEOMETRY_FORMULAS_KARNEY_DIRECT_HPP


#include <boost/math/constants/constants.hpp>

#include <boost/geometry/util/math.hpp>

#include <boost/geometry/formulas/flattening.hpp>
#include <boost/geometry/formulas/result_direct.hpp>


namespace boost { namespace geometry { namespace formula
{


/*!
\brief The solution of the direct problem of geodesics on latlong coordinates,
       after Karney (2011).
\author See
- Charles F.F Karney, Algorithms for geodesics, 2011
https://arxiv.org/pdf/1109.4448.pdf
*/
template <
    typename CT,
    std::size_t SeriesOrder = 8,
    bool EnableCoordinates = true,
    bool EnableReverseAzimuth = false,
    bool EnableReducedLength = false,
    bool EnableGeodesicScale = false
>
class karney_direct
{
    static const bool CalcQuantities = EnableReducedLength || EnableGeodesicScale;
    static const bool CalcCoordinates = EnableCoordinates || CalcQuantities;
    static const bool CalcRevAzimuth = EnableReverseAzimuth || CalcCoordinates || CalcQuantities;

public:
    typedef result_direct<CT> result_type;

    /*
     Generate and evaluate the series expansion of the following integral

     I1 = integrate( sqrt(1+k2*sin(sigma1)^2), sigma1, 0, sigma )

     which is valid for k2 small. We substitute k2 = 4 * eps / (1 - eps)^2
     and expand (1 - eps) * I1 retaining terms up to order eps^maxpow
     in A1 and C1[l].

     The resulting series is of the form

     A1 * ( sigma + sum(C1[l] * sin(2*l*sigma), l, 1, maxpow) ).

     The scale factor A1-1 = mean value of (d/dsigma)I1 - 1

     The expansion above is performed in Maxima, a Computer Algebra System.
     The C++ code (that yields the function evaluate_series_A1 below) is
     generated by the following Maxima script and is based on script:
     http://geographiclib.sourceforge.net/html/geod.mac

        // Maxima script begin
        codeA1(maxpow):=block([tab2:"    ",tab3:"        "],
        print("// The scale factor A1-1 = mean value of (d/dsigma)I1 - 1
        static inline CT evaluate_series_A1(CT eps) {
            CT eps2 = math::sqr(eps);
            CT t;
            switch (SeriesOrder/2) {"),
          for n:0 thru entier(maxpow/2) do block([
            q:horner(ataylor(subst([eps=sqrt(eps2)],A1*(1-eps)-1),eps2,n)),
            linel:1200],
            print(concat(tab2,"case ",string(n),":")),
            print(concat(tab3,"t = ",string(q),";")),
            print(concat(tab3,"break;"))),
          print("    }
            return (t + eps) / (1 - eps);
        }"),
        'done)$
        codeA1(8)$        
        // Maxima script end
     */
    static inline CT evaluate_series_A1(CT eps) {
        CT eps2 = math::sqr(eps);
        CT t;
        switch (SeriesOrder/2) {
        case 0:
            t = CT(0);
            break;
        case 1:
            t = eps2/CT(4);
            break;
        case 2:
            t = eps2*(eps2+CT(16))/CT(64);
            break;
        case 3:
            t = eps2*(eps2*(eps2+CT(4))+CT(64))/CT(256);
            break;
        case 4:
            t = eps2*(eps2*(eps2*(CT(25)*eps2+CT(64))+CT(256))+CT(4096))/CT(16384);
            break;
        }
        return (t + eps) / (CT(1) - eps);
    }

    template <typename T, typename Dist, typename Azi, typename Spheroid>
    static inline result_type apply(T const& lo1,
                                    T const& la1,
                                    Dist const& distance,
                                    Azi const& azimuth12,
                                    Spheroid const& spheroid)
    {
        result_type result;

        CT const lon1 = lo1;
        CT const lat1 = la1;

        if (math::equals(distance, Dist(0)) || distance < Dist(0))
        {
            result.lon2 = lon1;
            result.lat2 = lat1;
            return result;
        }

        CT const c1 = 1;
        CT const c2 = 2;

        CT const a = CT(get_radius<0>(spheroid));
        CT const b = CT(get_radius<2>(spheroid));
        CT const f = formula::flattening<CT>(spheroid);
        CT const one_minus_f = c1 - f;
        CT const two_minus_f = c2 - f;

        CT const e2 = f * two_minus_f;
        CT const ep2 = e2 / math::sqr(one_minus_f);

        CT sin_alpha1, cos_alpha1;
        math::sin_cos_degrees<CT>(azimuth12, sin_alpha1, cos_alpha1);

        // The reduced latitude.
        CT sin_beta1, cos_beta1;
        math::sin_cos_degrees<CT>(lat1, sin_beta1, cos_beta1);
        sin_beta1 *= one_minus_f;

        CT cos_alpha0 = boost::math::hypot(cos_alpha1, sin_alpha1 * sin_beta1);

        CT k2 = math::sqr(cos_alpha0) * ep2;

        CT epsilon = k2 / (c2 * (c1 + std::sqrt(c1 + k2)) + k2);

        CT expansion_A1 = evaluate_series_A1(epsilon);
        CT tau12 = distance / (b + (c1 + expansion_A1));
    }

};

}}} // namespace boost::geometry::formula


#endif // BOOST_GEOMETRY_FORMULAS_KARNEY_DIRECT_HPP