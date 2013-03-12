#!/bin/sh

echo
echo "*** Running PLOT_DATA_PLANE on a sample GRIB1 gridded file  ***"
../bin/plot_data_plane \
   ../data/sample_fcst/2012040900/nam/nam_2012040900_F012.grib \
   ${TEST_OUT_DIR}/plot_data_plane/nam_2012040900_F012_GRIB1_APCP_12.ps \
   'name = "APCP"; level = "A12";' \
    -title "GRIB1 NAM 12-hour APCP" \
    -v 1


echo
echo "*** Running PLOT_DATA_PLANE on a sample netCDF file  ***"
../bin/plot_data_plane \
  ${TEST_OUT_DIR}/pcp_combine/sample_fcst_12L_2005080712V_12A.nc \
  ${TEST_OUT_DIR}/plot_data_plane/sample_fcst_12L_2005080712V_12A_APCP12_NC_MET.ps \
  'name = "APCP_12"; level = "(*,*)";' \
  -title "NC MET 12-hour APCP" \
  -v 1
