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

#ifndef BOOST_GEOMETRY_PROJECTIONS_OCEA_HPP
#define BOOST_GEOMETRY_PROJECTIONS_OCEA_HPP

#include <boost/geometry/util/math.hpp>

#include <boost/geometry/srs/projections/impl/base_static.hpp>
#include <boost/geometry/srs/projections/impl/base_dynamic.hpp>
#include <boost/geometry/srs/projections/impl/projects.hpp>
#include <boost/geometry/srs/projections/impl/factory_entry.hpp>

namespace boost { namespace geometry
{

namespace srs { namespace par4
{
    struct ocea {}; // Oblique Cylindrical Equal Area

}} //namespace srs::par4

namespace projections
{
    #ifndef DOXYGEN_NO_DETAIL
    namespace detail { namespace ocea
    {
            template <typename T>
            struct par_ocea
            {
                T    rok;
                T    rtk;
                T    sinphi;
                T    cosphi;
                T    singam;
                T    cosgam;
            };

            // template class, using CRTP to implement forward/inverse
            template <typename CalculationType, typename Parameters>
            struct base_ocea_spheroid : public base_t_fi<base_ocea_spheroid<CalculationType, Parameters>,
                     CalculationType, Parameters>
            {

                typedef CalculationType geographic_type;
                typedef CalculationType cartesian_type;

                par_ocea<CalculationType> m_proj_parm;

                inline base_ocea_spheroid(const Parameters& par)
                    : base_t_fi<base_ocea_spheroid<CalculationType, Parameters>,
                     CalculationType, Parameters>(*this, par) {}

                // FORWARD(s_forward)  spheroid
                // Project coordinates from geographic (lon, lat) to cartesian (x, y)
                inline void fwd(geographic_type& lp_lon, geographic_type& lp_lat, cartesian_type& xy_x, cartesian_type& xy_y) const
                {
                    static const CalculationType ONEPI = detail::ONEPI<CalculationType>();

                    CalculationType t;

                    xy_y = sin(lp_lon);
                    t = cos(lp_lon);
                    xy_x = atan((tan(lp_lat) * this->m_proj_parm.cosphi + this->m_proj_parm.sinphi * xy_y) / t);
                    if (t < 0.)
                        xy_x += ONEPI;
                    xy_x *= this->m_proj_parm.rtk;
                    xy_y = this->m_proj_parm.rok * (this->m_proj_parm.sinphi * sin(lp_lat) - this->m_proj_parm.cosphi * cos(lp_lat) * xy_y);
                }

                // INVERSE(s_inverse)  spheroid
                // Project coordinates from cartesian (x, y) to geographic (lon, lat)
                inline void inv(cartesian_type& xy_x, cartesian_type& xy_y, geographic_type& lp_lon, geographic_type& lp_lat) const
                {
                    CalculationType t, s;

                    xy_y /= this->m_proj_parm.rok;
                    xy_x /= this->m_proj_parm.rtk;
                    t = sqrt(1. - xy_y * xy_y);
                    lp_lat = asin(xy_y * this->m_proj_parm.sinphi + t * this->m_proj_parm.cosphi * (s = sin(xy_x)));
                    lp_lon = atan2(t * this->m_proj_parm.sinphi * s - xy_y * this->m_proj_parm.cosphi,
                        t * cos(xy_x));
                }

                static inline std::string get_name()
                {
                    return "ocea_spheroid";
                }

            };

            // Oblique Cylindrical Equal Area
            template <typename Parameters, typename T>
            inline void setup_ocea(Parameters& par, par_ocea<T>& proj_parm)
            {
                static const T HALFPI = detail::HALFPI<T>();

                T phi_0=0.0, phi_1, phi_2, lam_1, lam_2, lonz, alpha;

                proj_parm.rok = 1. / par.k0;
                proj_parm.rtk = par.k0;
                /*If the keyword "alpha" is found in the sentence then use 1point+1azimuth*/
                if ( pj_param(par.params, "talpha").i) {
                    /*Define Pole of oblique transformation from 1 point & 1 azimuth*/
                    alpha    = pj_param(par.params, "ralpha").f;
                    lonz = pj_param(par.params, "rlonc").f;
                    /*Equation 9-8 page 80 (http://pubs.usgs.gov/pp/1395/report.pdf)*/
                    proj_parm.singam = atan(-cos(alpha)/(-sin(phi_0) * sin(alpha))) + lonz;
                    /*Equation 9-7 page 80 (http://pubs.usgs.gov/pp/1395/report.pdf)*/
                    proj_parm.sinphi = asin(cos(phi_0) * sin(alpha));
                /*If the keyword "alpha" is NOT found in the sentence then use 2points*/
                } else {
                    /*Define Pole of oblique transformation from 2 points*/
                    phi_1 = pj_param(par.params, "rlat_1").f;
                    phi_2 = pj_param(par.params, "rlat_2").f;
                    lam_1 = pj_param(par.params, "rlon_1").f;
                    lam_2 = pj_param(par.params, "rlon_2").f;
                    /*Equation 9-1 page 80 (http://pubs.usgs.gov/pp/1395/report.pdf)*/
                    proj_parm.singam = atan2(cos(phi_1) * sin(phi_2) * cos(lam_1) -
                        sin(phi_1) * cos(phi_2) * cos(lam_2),
                        sin(phi_1) * cos(phi_2) * sin(lam_2) -
                        cos(phi_1) * sin(phi_2) * sin(lam_1) );

                    /* take care of P->lam0 wrap-around when +lam_1=-90*/
                    if (lam_1 == -HALFPI)
                        proj_parm.singam = -proj_parm.singam;

                    /*Equation 9-2 page 80 (http://pubs.usgs.gov/pp/1395/report.pdf)*/
                    proj_parm.sinphi = atan(-cos(proj_parm.singam - lam_1) / tan(phi_1));
                }
                par.lam0 = proj_parm.singam + HALFPI;
                proj_parm.cosphi = cos(proj_parm.sinphi);
                proj_parm.sinphi = sin(proj_parm.sinphi);
                proj_parm.cosgam = cos(proj_parm.singam);
                proj_parm.singam = sin(proj_parm.singam);
                par.es = 0.;
            }

    }} // namespace detail::ocea
    #endif // doxygen

    /*!
        \brief Oblique Cylindrical Equal Area projection
        \ingroup projections
        \tparam Geographic latlong point type
        \tparam Cartesian xy point type
        \tparam Parameters parameter type
        \par Projection characteristics
         - Cylindrical
         - Spheroid
        \par Projection parameters
         - lonc: Longitude (only used if alpha (or gamma) is specified) (degrees)
         - alpha: Alpha (degrees)
         - lat_1: Latitude of first standard parallel (degrees)
         - lat_2: Latitude of second standard parallel (degrees)
         - lon_1 (degrees)
         - lon_2 (degrees)
        \par Example
        \image html ex_ocea.gif
    */
    template <typename CalculationType, typename Parameters>
    struct ocea_spheroid : public detail::ocea::base_ocea_spheroid<CalculationType, Parameters>
    {
        inline ocea_spheroid(const Parameters& par) : detail::ocea::base_ocea_spheroid<CalculationType, Parameters>(par)
        {
            detail::ocea::setup_ocea(this->m_par, this->m_proj_parm);
        }
    };

    #ifndef DOXYGEN_NO_DETAIL
    namespace detail
    {

        // Static projection
        BOOST_GEOMETRY_PROJECTIONS_DETAIL_STATIC_PROJECTION(srs::par4::ocea, ocea_spheroid, ocea_spheroid)

        // Factory entry(s)
        template <typename CalculationType, typename Parameters>
        class ocea_entry : public detail::factory_entry<CalculationType, Parameters>
        {
            public :
                virtual base_v<CalculationType, Parameters>* create_new(const Parameters& par) const
                {
                    return new base_v_fi<ocea_spheroid<CalculationType, Parameters>, CalculationType, Parameters>(par);
                }
        };

        template <typename CalculationType, typename Parameters>
        inline void ocea_init(detail::base_factory<CalculationType, Parameters>& factory)
        {
            factory.add_to_factory("ocea", new ocea_entry<CalculationType, Parameters>);
        }

    } // namespace detail
    #endif // doxygen

} // namespace projections

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_PROJECTIONS_OCEA_HPP

