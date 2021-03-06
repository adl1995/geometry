// Boost.Geometry - gis-projections (based on PROJ4)

// Copyright (c) 2008-2015 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2017, 2018.
// Modifications copyright (c) 2017-2018, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// This file is converted from PROJ4, http://trac.osgeo.org/proj
// PROJ4 is originally written by Gerald Evenden (then of the USGS)
// PROJ4 is maintained by Frank Warmerdam
// PROJ4 is converted to Boost.Geometry by Barend Gehrels

// Last updated version of proj: 5.0.0

// Original copyright notice:

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef BOOST_GEOMETRY_PROJECTIONS_WINK2_HPP
#define BOOST_GEOMETRY_PROJECTIONS_WINK2_HPP

#include <boost/geometry/util/math.hpp>

#include <boost/geometry/srs/projections/impl/base_static.hpp>
#include <boost/geometry/srs/projections/impl/base_dynamic.hpp>
#include <boost/geometry/srs/projections/impl/projects.hpp>
#include <boost/geometry/srs/projections/impl/factory_entry.hpp>

namespace boost { namespace geometry
{

namespace srs { namespace par4
{
    struct wink2 {}; // Winkel II

}} //namespace srs::par4

namespace projections
{
    #ifndef DOXYGEN_NO_DETAIL
    namespace detail { namespace wink2
    {

            static const int MAX_ITER = 10;
            static const double LOOP_TOL = 1e-7;

            template <typename T>
            struct par_wink2
            {
                T    cosphi1;
            };

            // template class, using CRTP to implement forward/inverse
            template <typename CalculationType, typename Parameters>
            struct base_wink2_spheroid : public base_t_f<base_wink2_spheroid<CalculationType, Parameters>,
                     CalculationType, Parameters>
            {

                typedef CalculationType geographic_type;
                typedef CalculationType cartesian_type;

                par_wink2<CalculationType> m_proj_parm;

                inline base_wink2_spheroid(const Parameters& par)
                    : base_t_f<base_wink2_spheroid<CalculationType, Parameters>,
                     CalculationType, Parameters>(*this, par) {}

                // FORWARD(s_forward)  spheroid
                // Project coordinates from geographic (lon, lat) to cartesian (x, y)
                inline void fwd(geographic_type& lp_lon, geographic_type& lp_lat, cartesian_type& xy_x, cartesian_type& xy_y) const
                {
                    static const CalculationType ONEPI = detail::ONEPI<CalculationType>();
                    static const CalculationType HALFPI = detail::HALFPI<CalculationType>();
                    static const CalculationType FORTPI = detail::FORTPI<CalculationType>();
                    static const CalculationType TWO_D_PI = detail::TWO_D_PI<CalculationType>();

                    CalculationType k, V;
                    int i;

                    xy_y = lp_lat * TWO_D_PI;
                    k = ONEPI * sin(lp_lat);
                    lp_lat *= 1.8;
                    for (i = MAX_ITER; i ; --i) {
                        lp_lat -= V = (lp_lat + sin(lp_lat) - k) /
                            (1. + cos(lp_lat));
                        if (fabs(V) < LOOP_TOL)
                            break;
                    }
                    if (!i)
                        lp_lat = (lp_lat < 0.) ? -HALFPI : HALFPI;
                    else
                        lp_lat *= 0.5;
                    xy_x = 0.5 * lp_lon * (cos(lp_lat) + this->m_proj_parm.cosphi1);
                    xy_y = FORTPI * (sin(lp_lat) + xy_y);
                }

                static inline std::string get_name()
                {
                    return "wink2_spheroid";
                }

            };

            // Winkel II
            template <typename Parameters, typename T>
            inline void setup_wink2(Parameters& par, par_wink2<T>& proj_parm)
            {
                proj_parm.cosphi1 = cos(pj_param(par.params, "rlat_1").f);
                par.es = 0.;
            }

    }} // namespace detail::wink2
    #endif // doxygen

    /*!
        \brief Winkel II projection
        \ingroup projections
        \tparam Geographic latlong point type
        \tparam Cartesian xy point type
        \tparam Parameters parameter type
        \par Projection characteristics
         - Pseudocylindrical
         - Spheroid
         - no inverse
        \par Projection parameters
         - lat_1: Latitude of first standard parallel (degrees)
        \par Example
        \image html ex_wink2.gif
    */
    template <typename CalculationType, typename Parameters>
    struct wink2_spheroid : public detail::wink2::base_wink2_spheroid<CalculationType, Parameters>
    {
        inline wink2_spheroid(const Parameters& par) : detail::wink2::base_wink2_spheroid<CalculationType, Parameters>(par)
        {
            detail::wink2::setup_wink2(this->m_par, this->m_proj_parm);
        }
    };

    #ifndef DOXYGEN_NO_DETAIL
    namespace detail
    {

        // Static projection
        BOOST_GEOMETRY_PROJECTIONS_DETAIL_STATIC_PROJECTION(srs::par4::wink2, wink2_spheroid, wink2_spheroid)

        // Factory entry(s)
        template <typename CalculationType, typename Parameters>
        class wink2_entry : public detail::factory_entry<CalculationType, Parameters>
        {
            public :
                virtual base_v<CalculationType, Parameters>* create_new(const Parameters& par) const
                {
                    return new base_v_f<wink2_spheroid<CalculationType, Parameters>, CalculationType, Parameters>(par);
                }
        };

        template <typename CalculationType, typename Parameters>
        inline void wink2_init(detail::base_factory<CalculationType, Parameters>& factory)
        {
            factory.add_to_factory("wink2", new wink2_entry<CalculationType, Parameters>);
        }

    } // namespace detail
    #endif // doxygen

} // namespace projections

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_PROJECTIONS_WINK2_HPP

