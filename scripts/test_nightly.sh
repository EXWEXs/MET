#!/bin/bash
# This script is intended to be run from a cron job nightly to
# verify the current trunk output compares with a known working
# output, utilizing the regression test script found in the MET
# software repository
# Output should be directed to the LOGFILE per cron convention

# Configure run
RUNDIR="/d3/projects/MET/MET_regression"
EMAIL_LIST="johnhg@ucar.edu bullock@ucar.edu hsoh@ucar.edu mccabe@ucar.edu fillmore@ucar.edu"
REF_BASE=6384
KEEP_STUFF_DURATION=5    #This is in days

# Variables required to build MET (pre-METv5.0)
export NETCDF=/d3/projects/MET/MET_releases/external_libs/netcdf/netcdf-4.1.3
export GSL=/d3/projects/MET/MET_releases/external_libs/gsl/gsl-1.13
export BUFRLIB=/d3/projects/MET/MET_releases/external_libs/bufrlib/bufrlib_gnu
export BUFRINC=/d3/projects/MET/MET_releases/external_libs/bufrlib/bufrlib_gnu
export HDF=/d3/projects/MET/MET_releases/external_libs/hdf/HDF4.2r3
export HDFEOS=/d3/projects/MET/MET_releases/external_libs/hdf_eos/hdfeos

# Variables required to build MET (METv5.0 and later)
export MET_DEVELOPMENT=true
export MET_NETCDF=/d3/projects/MET/MET_releases/external_libs/netcdf/netcdf-4.1.3
export MET_GSL=/d3/projects/MET/MET_releases/external_libs/gsl/gsl-1.13
export MET_GRIB2CINC=/d3/projects/MET/MET_releases/external_libs/g2clib/g2clib-1.4.0
export MET_GRIB2CLIB=/d3/projects/MET/MET_releases/external_libs/g2clib/g2clib-1.4.0
export MET_BUFRLIB=/d3/projects/MET/MET_releases/external_libs/bufrlib/bufrlib_gnu
export MET_HDF=/d3/projects/MET/MET_releases/external_libs/hdf/HDF4.2r3
export MET_HDFEOS=/d3/projects/MET/MET_releases/external_libs/hdf_eos/hdfeos
export MET_CAIROINC=/usr/include/cairo
export MET_CAIROLIB=/usr/lib
export MET_FREETYPEINC=/usr/include/freetype2
export MET_FREETYPELIB=/usr/lib

# Variables required for using Python numpy to create numpy array of data to be used by MET tools
export MET_PYTHON_CC="-I/usr/include/python2.7 -I/usr/include/x86_64-linux-gnu/python2.7"
export MET_PYTHON_LD="-L/usr/lib/python2.7/config-x86_64-linux-gnu -L/usr/lib -lpython2.7 -lpthread -ldl  -lutil -lm"

# Variables required to build MET (METv6.0 and later)
export MET_NETCDF=/usr/local/netcdf
export MET_HDF5=/usr/local/hdf5

# Variables required to run MET
export MET_FONT_DIR=/d3/projects/MET/MET_test_data/unit_test/fonts

# This is a cron script -- create the shell environment for this job
export PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:\
             /usr/bin/X11:/opt/bin:${MET_NETCDF}/bin"
export LD_LIBRARY_PATH=${MET_NETCDF}/lib:${MET_HDF5}/lib:/usr/local/lib:/usr/lib

# Exit codes
E_NOEXECSCRIPT=61
E_REGTEST_FAILED=62
E_FOUND_WARNING=63

# Keep it open
umask 0002

# Get to the run directory
cd ${RUNDIR}

# Clean the place up
find . -mtime +${KEEP_STUFF_DURATION} -name "NB*" | xargs rm -rf

# Internals - create a daily log file
today=`date +%Y%m%d`
LOGFILE=${PWD}/NB${today}.out
>${LOGFILE}

# Check we have a script to run
if [[ ! -r scripts/test_regression.sh ]]
then
  echo "$0: FAILURE scripts/test_regression.sh not found" > ${LOGFILE}
  exit $E_NOEXECSCRIPT
fi

# make a little home - always start from scratch
if [[ -e NB${today} ]];  then rm -rf NB${today}; fi
mkdir NB${today}
cd NB${today}

# copy over the test scripts directory
cp -r -p ../scripts .

# run the regression test fail if nonZero status
scripts/test_regression.sh trunk ${REF_BASE} trunk >> ${LOGFILE} 2>&1
if [[ $? -ne 0 ]]
then
  echo "$0: FAILURE the regression test failed." >> ${LOGFILE}
  echo -e "Nightly Build Log: `hostname`:${LOGFILE}\n\n`tail -10 ${LOGFILE}`" | mail -s "MET Nightly Build Failed (autogen msg)" ${EMAIL_LIST}
  exit $E_REGTEST_FAILED
fi

# Look for warnings and/or failure and report it
N_ERR=`egrep "ERROR:"   ${LOGFILE} | wc -l`
N_WRN=`egrep "WARNING:" ${LOGFILE} | wc -l`
echo "$0: Found $N_WRN WARNINGS and $N_ERR ERRORS in regtest" >> ${LOGFILE}

if [[ $N_ERR -gt 0 ]]      # check for errors
then
  echo "$0: FAILURE grep found ERRORS in regtest" >> ${LOGFILE}
  echo "Nightly Build Log: `hostname`:${LOGFILE}" | mail -s "MET Nightly Build Failed (autogen msg)" ${EMAIL_LIST}
  exit ${E_FOUNDWARNING}
fi

exit 0
