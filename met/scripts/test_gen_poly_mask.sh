#!/bin/sh

echo
echo "*** Running Gen-Poly-Mask to generate a polyline mask file for the Continental United States ***"
../bin/gen_poly_mask \
   ../data/sample_fcst/2005080700/wrfprs_ruc13_24.tm00_G212 \
   ../data/poly/CONUS.poly \
   ../out/gen_poly_mask/CONUS_poly.nc -v 2
