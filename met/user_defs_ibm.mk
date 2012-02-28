

###############################################################################

   ##
   ##  Begin Variables to be modified before building
   ##  Default settings for IBM compilers
   ##

###############################################################################

# Path to GNU Make command
MAKE         = /usr/local/bin/gmake

# Architecture flags
ARCH_FLAGS   = -DIBM -DBIGENDIAN -DBLOCK4

# Path to the C++ Compiler
# C++ compiler flags
# Any additional required libraries
CXX          = /set/this/path/to/xlC
CXX_FLAGS    = # -g -qfullpath
CXX_LIBS     =

# Path to the Fortran Compiler
# Fortran compiler flags
# Any additional required libraries
FC           = /set/this/path/to/xlf
FC_FLAGS     = -qextname # -g -qfullpath
FC_LIBS      = -lxlf90

# Make print options
PRINT_OPTS   =

# Top level directory for the NetCDF library
# NetCDF include directory specified as: -I/your/include/path
# NetCDF library directory specified as: -L/your/library/path
NETCDF_BASE  = /set/this/path/to/netcdf
NETCDF_INCS  = -I$(NETCDF_BASE)/include
NETCDF_LIBS  = -L$(NETCDF_BASE)/lib

# Top level directory for BUFRLIB
# BUFRLIB include directory specified as: -I/your/include/path
# BUFRLIB library directory specified as: -L/your/library/path
BUFR_BASE    = /set/this/path/to/bufrlib
BUFR_INCS    = -I$(BUFR_BASE)
BUFR_LIBS    = -L$(BUFR_BASE)

# Top level directory for the GNU Scientific Library (GSL) if it's not
# installed in a standard location.
# GSL include directory specified as: -I/your/include/path
# GSL library directory specified as: -L/your/library/path
GSL_BASE     = /set/this/path/to/gsl
GSL_INCS     = -I$(GSL_BASE)/include
GSL_LIBS     = -L$(GSL_BASE)/lib

# Top level directory for the F2C or G2C Library if it's not installed in a
# standard location.
# F2C include directory specified as: -I/your/include/path
# F2C library directory containing libf2c.a or libg2c.a and specified as:
# -L/your/library/path
# Name of the library to be used: -lf2c or -lg2c
# NOTE: Typically required for the IBM xlf Fortran compiler
F2C_BASE     = /set/this/path/to/libf2c
F2C_INCS     = -I$(F2C_BASE)
F2C_LIBS     = -L$(F2C_BASE)
F2C_LIBNAME  = -lf2c

# Top level directory for the GRIB2 C Library if it's not
# installed in a standard location.
# grib2c include directory specified as: -I/your/include/path
# grib2c library directory specified as: -L/your/library/path
GRIB2C_BASE  =
GRIB2C_INCS  =
GRIB2C_LIBS  =

# Optional flags to disable the compilation of MET tools
# Specify a non-zero value to enable the compilation of the tool
ENABLE_ASCII2NC        = 1
ENABLE_ENSEMBLE_STAT   = 1
ENABLE_GEN_POLY_MASK   = 1
ENABLE_GRID_STAT       = 1
ENABLE_MADIS2NC        = 1
ENABLE_MODE            = 1
ENABLE_MODE_ANALYSIS   = 1
ENABLE_PB2NC           = 1
ENABLE_PCP_COMBINE     = 1
ENABLE_PLOT_DATA_PLANE = 1
ENABLE_PLOT_POINT_OBS  = 1
ENABLE_POINT_STAT      = 1
ENABLE_STAT_ANALYSIS   = 1
ENABLE_WAVELET_STAT    = 1
ENABLE_WWMCA           = 1

# Flags to compile support for additional file formats
WITH_GRIB2             = 0

###############################################################################

   ##
   ##  End Variables to be modified before building
   ##

###############################################################################

