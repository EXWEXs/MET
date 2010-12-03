// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef GRIB_STRINGS_H
#define GRIB_STRINGS_H

////////////////////////////////////////////////////////////////////////

using namespace std;

////////////////////////////////////////////////////////////////////////

static const char missing_str[]        = "MISSING";
static const char ugrd_vgrd_abbr_str[] = "UGRD_VGRD";
static const char wdir_abbr_str[]      = "WDIR";

//
// Grib code for precipiation types
//
static const int prate_grib_code = 59;
static const int tstm_grib_code  = 60;
static const int apcp_grib_code  = 61;
static const int ncpcp_grib_code = 62;
static const int acpcp_grib_code = 63;

//
// Grib codes corresponding to P, Q, T, Z, U, V variables
//
static const int pres_grib_code = 1;  // Pressure
static const int spfh_grib_code = 51; // Specific Humidity
static const int tmp_grib_code  = 11; // Temperature
static const int hgt_grib_code  = 7;  // Geopotential Height
static const int ugrd_grib_code = 33; // U-component of wind
static const int vgrd_grib_code = 34; // V-component of wind

//
// Grib codes for quantities that can be derived from
// the P, Q, T, Z, U, V variables
//
static const int dpt_grib_code   = 17; // Dewpoint temperature
static const int wdir_grib_code  = 31; // Wind direction
static const int wind_grib_code  = 32; // Wind speed
static const int rh_grib_code    = 52; // Relative humidity
static const int mixr_grib_code  = 53; // Humidity mixing ratio
static const int prmsl_grib_code = 2;  // Pressure Reduced to MSL

////////////////////////////////////////////////////////////////////////
//
// NCEP Office Note 388 - Table 2
// Grib Code to Units and Abbreviations
//
// (1) GRIB Code
// (2) GRIB Code Name
// (3) GRIB Code Units
// (4) GRIB Code Abbreviation
//
////////////////////////////////////////////////////////////////////////

struct GribCodeData {
   int         code;
   const char *name;
   const char *unit;
   const char *abbr;
};

////////////////////////////////////////////////////////////////////////
//
// GRIB tables: http://www.nco.ncep.noaa.gov/pmb/docs/on388/table2.html
// Last Updated 05/14/2010
//
////////////////////////////////////////////////////////////////////////

static const GribCodeData grib_code_list_base[] = {
   {   1, "Pressure", "Pa", "PRES" },
   {   2, "Pressure reduced to MSL", "Pa", "PRMSL" },
   {   3, "Pressure tendency", "Pa/s", "PTEND" },
   {   4, "Potential vorticity", "km^2/kg/s", "PVORT" },
   {   5, "ICAO Standard Atmosphere Reference Height", "m", "ICAHT" },
   {   6, "Geopotential", "m^2/s^2", "GP" },
   {   7, "Geopotential height", "gpm", "HGT" },
   {   8, "Geometric height", "m", "DIST" },
   {   9, "Standard deviation of height", "m", "HSTDV" },
   {  10, "Total ozone", "Dobson", "TOZNE" },
   {  11, "Temperature", "K", "TMP" },
   {  12, "Virtual temperature", "K", "VTMP" },
   {  13, "Potential temperature", "K", "POT" },
   {  14, "Pseudo-adiabatic potential temperature or equivalent potential temperature", "K", "EPOT" },
   {  15, "Maximum temperature", "K", "TMAX" },
   {  16, "Minimum temperature", "K", "TMIN" },
   {  17, "Dew point temperature", "K", "DPT" },
   {  18, "Dew point depression (or deficit)", "K", "DEPR" },
   {  19, "Lapse rate", "K/m", "LAPR" },
   {  20, "Visibility", "m", "VIS" },
   {  21, "Radar Spectra (1)", "-", "RDSP1" },
   {  22, "Radar Spectra (2)", "-", "RDSP2" },
   {  23, "Radar Spectra (3)", "-", "RDSP3" },
   {  24, "Parcel lifted index (to 500 hPa)", "K", "PLI" },
   {  25, "Temperature anomaly", "K", "TMPA" },
   {  26, "Pressure anomaly", "Pa", "PRESA" },
   {  27, "Geopotential height anomaly", "gpm", "GPA" },
   {  28, "Wave Spectra (1)", "-", "WVSP1" },
   {  29, "Wave Spectra (2)", "-", "WVSP2" },
   {  30, "Wave Spectra (3)", "-", "WVSP3" },
   {  31, "Wind direction (from which blowing)", "deg true", "WDIR" },
   {  32, "Wind speed", "m/s", "WIND" },
   {  33, "u-component of wind", "m/s", "UGRD" },
   {  34, "v-component of wind", "m/s", "VGRD" },
   {  35, "Stream function", "m^2/s", "STRM" },
   {  36, "Velocity potential", "m^2/s", "VPOT" },
   {  37, "Montgomery stream function", "m^2/s^2", "MNTSF" },
   {  38, "Sigma coordinate vertical velocity", "/s", "SGCVV" },
   {  39, "Vertical velocity (pressure)", "Pa/s", "VVEL" },
   {  40, "Vertical velocity (geometric)", "m/s", "DZDT" },
   {  41, "Absolute vorticity", "/s", "ABSV" },
   {  42, "Absolute divergence", "/s", "ABSD" },
   {  43, "Relative vorticity", "/s", "RELV" },
   {  44, "Relative divergence", "/s", "RELD" },
   {  45, "Vertical u-component shear", "/s", "VUCSH" },
   {  46, "Vertical v-component shear", "/s", "VVCSH" },
   {  47, "Direction of current", "Degree true", "DIRC" },
   {  48, "Speed of current", "m/s", "SPC" },
   {  49, "u-component of current", "m/s", "UOGRD" },
   {  50, "v-component of current", "m/s", "VOGRD" },
   {  51, "Specific humidity", "kg/kg", "SPFH" },
   {  52, "Relative humidity", "%", "RH" },
   {  53, "Humidity mixing ratio", "kg/kg", "MIXR" },
   {  54, "Precipitable water", "kg/m^2", "PWAT" },
   {  55, "Vapor pressure", "Pa", "VAPP" },
   {  56, "Saturation deficit", "Pa", "SATD" },
   {  57, "Evaporation", "kg/m^2", "EVP" },
   {  58, "Cloud Ice", "kg/m^2", "CICE" },
   {  59, "Precipitation rate", "kg/m^2/s", "PRATE" },
   {  60, "Thunderstorm probability", "%", "TSTM" },
   {  61, "Total precipitation", "kg/m^2", "APCP" },
   {  62, "Large scale precipitation (non-conv.)", "kg/m^2", "NCPCP" },
   {  63, "Convective precipitation", "kg/m^2", "ACPCP" },
   {  64, "Snowfall rate water equivalent", "kg/m^2/s", "SRWEQ" },
   {  65, "Water equiv. of accum. snow depth", "kg/m^2", "WEASD" },
   {  66, "Snow depth", "m", "SNOD" },
   {  67, "Mixed layer depth", "m", "MIXHT" },
   {  68, "Transient thermocline depth", "m", "TTHDP" },
   {  69, "Main thermocline depth", "m", "MTHD" },
   {  70, "Main thermocline anomaly", "m", "MTHA" },
   {  71, "Total cloud cover", "%", "TCDC" },
   {  72, "Convective cloud cover", "%", "CDCON" },
   {  73, "Low cloud cover", "%", "LCDC" },
   {  74, "Medium cloud cover", "%", "MCDC" },
   {  75, "High cloud cover", "%", "HCDC" },
   {  76, "Cloud water", "kg/m^2", "CWAT" },
   {  77, "Best lifted index (to 500 hPa)", "K", "BLI" },
   {  78, "Convective snow", "kg/m^2", "SNOC" },
   {  79, "Large scale snow", "kg/m^2", "SNOL" },
   {  80, "Water Temperature", "K", "WTMP" },
   {  81, "Land cover (land=1, sea=0)", "proportion", "LAND" },
   {  82, "Deviation of sea level from mean", "m", "DSLM" },
   {  83, "Surface roughness", "m", "SFCR" },
   {  84, "Albedo", "%", "ALBDO" },
   {  85, "Soil temperature", "K", "TSOIL" },
   {  86, "Soil moisture content", "kg/m^2", "SOILM" },
   {  87, "Vegetation", "%", "VEG" },
   {  88, "Salinity", "kg/kg", "SALTY" },
   {  89, "Density", "kg/m^3", "DEN" },
   {  90, "Water runoff", "kg/m^2", "WATR" },
   {  91, "Ice cover (ice=1, no ice=0)", "proportion", "ICEC" },
   {  92, "Ice thickness", "m", "ICETK" },
   {  93, "Direction of ice drift", "deg true", "DICED" },
   {  94, "Speed of ice drift", "m/s", "SICED" },
   {  95, "u-component of ice drift", "m/s", "UICE" },
   {  96, "v-component of ice drift", "m/s", "VICE" },
   {  97, "Ice growth rate", "m/s", "ICEG" },
   {  98, "Ice divergence", "m/s", "ICED" },
   {  99, "Snow melt", "kg/m^2", "SNOM" },
   { 100, "Significant height of combined wind waves and swell", "m", "HTSGW" },
   { 101, "Direction of wind waves (from which)", "Degree true", "WVDIR" },
   { 102, "Significant height of wind waves", "m", "WVHGT" },
   { 103, "Mean period of wind waves", "s", "WVPER" },
   { 104, "Direction of swell waves", "Degree true", "SWDIR" },
   { 105, "Significant height of swell waves", "m", "SWELL" },
   { 106, "Mean period of swell waves", "s", "SWPER" },
   { 107, "Primary wave direction", "Degree true", "DIRPW" },
   { 108, "Primary wave mean period", "s", "PERPW" },
   { 109, "Secondary wave direction", "Degree true", "DIRSW" },
   { 110, "Secondary wave mean period", "s", "PERSW" },
   { 111, "Net short-wave radiation flux (surface)", "W/m^2", "NSWRS" },
   { 112, "Net long wave radiation flux (surface)", "W/m^2", "NLWRS" },
   { 113, "Net short-wave radiation flux (top of atmosphere)", "W/m^2", "NSWRT" },
   { 114, "Net long wave radiation flux (top of atmosphere)", "W/m^2", "NLWRT" },
   { 115, "Long wave radiation flux", "W/m^2", "LWAVR" },
   { 116, "Short wave radiation flux", "W/m^2", "SWAVR" },
   { 117, "Global radiation flux", "W/m^2", "GRAD" },
   { 118, "Brightness temperature", "K", "BRTMP" },
   { 119, "Radiance (with respect to wave number)", "W/m/sr", "LWRAD" },
   { 120, "Radiance (with respect to wave length)", "W/m^3/sr", "SWRAD" },
   { 121, "Latent heat net flux", "W/m^2", "LHTFL" },
   { 122, "Sensible heat net flux", "W/m^2", "SHTFL" },
   { 123, "Boundary layer dissipation", "W/m^2", "BLYDP" },
   { 124, "Momentum flux, u component", "N/m^2", "UFLX" },
   { 125, "Momentum flux, v component", "N/m^2", "VFLX" },
   { 126, "Wind mixing energy", "J", "WMIXE" },
   { 127, "Image data", "-", "IMGD" }
};
static const int n_grib_code_list_base = sizeof(grib_code_list_base)/sizeof(GribCodeData);

////////////////////////////////////////////////////////////////////////

static const GribCodeData grib_code_list_ptv2[] = {
   { 128, "Mean Sea Level Pressure (Standard Atmosphere Reduction)", "Pa", "MSLSA" },
   { 129, "Mean Sea Level Pressure (MAPS System Reduction)", "Pa", "MSLMA" },
   { 130, "Mean Sea Level Pressure (NAM Model Reduction)", "Pa", "MSLET" },
   { 131, "Surface lifted index", "K", "LFTX" },
   { 132, "Best (4 layer) lifted index", "K", "4LFTX" },
   { 133, "K index", "K", "KX" },
   { 134, "Sweat index", "K", "SX" },
   { 135, "Horizontal moisture divergence", "kg/kg/s", "MCONV" },
   { 136, "Vertical speed shear", "1/s", "VWSH" },
   { 137, "3-hr pressure tendency Std. Atmos. Reduction", "Pa/s", "TSLSA" },
   { 138, "Brunt-Vaisala frequency (squared)", "1/s^2", "BVF2" },
   { 139, "Potential vorticity (density weighted)", "1/s/m", "PVMW" },
   { 140, "Categorical rain (yes=1; no=0)", "non-dim", "CRAIN" },
   { 141, "Categorical freezing rain (yes=1; no=0)", "non-dim", "CFRZR" },
   { 142, "Categorical ice pellets (yes=1; no=0)", "non-dim", "CICEP" },
   { 143, "Categorical snow (yes=1; no=0)", "non-dim", "CSNOW" },
   { 144, "Volumetric soil moisture content", "fraction", "SOILW" },
   { 145, "Potential evaporation rate", "W/m^2", "PEVPR" },
   { 146, "Cloud work function", "J/kg", "CWORK" },
   { 147, "Zonal flux of gravity wave stress", "N/m^2", "U-GWD" },
   { 148, "Meridional flux of gravity wave stress", "N/m^2", "V-GWD" },
   { 149, "Potential vorticity", "m^2/s/kg", "PVORT" },
   { 150, "Covariance between meridional and zonal components of the wind", "m^2/s^2", "COVMZ" },
   { 151, "Covariance between temperature and zonal components of the wind", "K*m/s", "COVTZ" },
   { 152, "Covariance between temperature and meridional components of the wind", "K*m/s", "COVTM" },
   { 153, "Cloud water", "kg/kg", "CLWMR" },
   { 154, "Ozone mixing ratio", "kg/kg", "O3MR" },
   { 155, "Ground Heat Flux", "W/m^2", "GFLUX" },
   { 156, "Convective inhibition", "J/kg", "CIN" },
   { 157, "Convective Available Potential Energy", "J/kg", "CAPE" },
   { 158, "Turbulent Kinetic Energy", "J/kg", "TKE" },
   { 159, "Condensation pressure of parcel lifted from indicated surface", "Pa", "CONDP" },
   { 160, "Clear Sky Upward Solar Flux", "W/m^2", "CSUSF" },
   { 161, "Clear Sky Downward Solar Flux", "W/m^2", "CSDSF" },
   { 162, "Clear Sky upward long wave flux", "W/m^2", "CSULF" },
   { 163, "Clear Sky downward long wave flux", "W/m^2", "CSDLF" },
   { 164, "Cloud forcing net solar flux", "W/m^2", "CFNSF" },
   { 165, "Cloud forcing net long wave flux", "W/m^2", "CFNLF" },
   { 166, "Visible beam downward solar flux", "W/m^2", "VBDSF" },
   { 167, "Visible diffuse downward solar flux", "W/m^2", "VDDSF" },
   { 168, "Near IR beam downward solar flux", "W/m^2", "NBDSF" },
   { 169, "Near IR diffuse downward solar flux", "W/m^2", "NDDSF" },
   { 170, "Rain water mixing ratio", "kg/kg", "RWMR" },
   { 171, "Snow mixing ratio", "kg/kg", "SNMR" },
   { 172, "Momentum flux", "N/m^2", "MFLX" },
   { 173, "Mass point model surface", "non-dim", "LMH" },
   { 174, "Velocity point model surface", "non-dim", "LMV" },
   { 175, "Model layer number (from bottom up)", "non-dim", "MLYNO" },
   { 176, "latitude (-90 to +90)", "deg", "NLAT" },
   { 177, "east longitude (0-360)", "deg", "ELON" },
   { 178, "Ice mixing ratio", "kg/kg", "ICMR" },
   { 179, "Graupel mixing ratio", "kg/kg", "GRMR" },
   { 180, "Surface wind gust", "m/s", "GUST" },
   { 181, "x-gradient of log pressure", "1/m", "LPSX" },
   { 182, "y-gradient of log pressure", "1/m", "LPSY" },
   { 183, "x-gradient of height", "m/m", "HGTX" },
   { 184, "y-gradient of height", "m/m", "HGTY" },
   { 185, "Turbulence Potential Forecast Index", "non-dim", "TPFI" },
   { 186, "Total Icing Potential Diagnostic", "non-dim", "TIPD" },
   { 187, "Lightning", "non-dim", "LTNG" },
   { 188, "Rate of water dropping from canopy to ground", "-", "RDRIP" },
   { 189, "Virtual potential temperature", "K", "VPTMP" },
   { 190, "Storm relative helicity", "m^2/s^2", "HLCY" },
   { 191, "Probability from ensemble", "numeric", "PROB" },
   { 192, "Probability from ensemble normalized with respect to climate expectancy", "numeric", "PROBN" },
   { 193, "Probability of precipitation", "%", "POP" },
   { 194, "Percent of frozen precipitation", "%", "CPOFP" },
   { 195, "Probability of freezing precipitation", "%", "CPOZP" },
   { 196, "u-component of storm motion", "m/s", "USTM" },
   { 197, "v-component of storm motion", "m/s", "VSTM" },
   { 198, "Number concentration for ice particles", "-", "NCIP" },
   { 199, "Direct evaporation from bare soil", "W/m^2", "EVBS" },
   { 200, "Canopy water evaporation", "W/m^2", "EVCW" },
   { 201, "Ice-free water surface", "%", "ICWAT" },
   { 202, "Convective weather detection index", "non-dim", "CWDI" },
   { 203, "VAFTAD", "log10(kg/m^3)", "VAFTD" },
   { 204, "downward short wave rad. flux", "W/m^2", "DSWRF" },
   { 205, "downward long wave rad. flux", "W/m^2", "DLWRF" },
   { 206, "Ultra violet index (1 hour integration centered at solar noon)", "W/m^2", "UVI" },
   { 207, "Moisture availability", "%", "MSTAV" },
   { 208, "Exchange coefficient", "(kg/m^3)(m/s)", "SFEXC" },
   { 209, "No. of mixed layers next to surface", "integer", "MIXLY" },
   { 210, "Transpiration", "W/m^2", "TRANS" },
   { 211, "upward short wave rad. flux", "W/m^2", "USWRF" },
   { 212, "upward long wave rad. flux", "W/m^2", "ULWRF" },
   { 213, "Amount of non-convective cloud", "%", "CDLYR" },
   { 214, "Convective Precipitation rate", "kg/m^2/s", "CPRAT" },
   { 215, "Temperature tendency by all physics", "K/s", "TTDIA" },
   { 216, "Temperature tendency by all radiation", "K/s", "TTRAD" },
   { 217, "Temperature tendency by non-radiation physics", "K/s", "TTPHY" },
   { 218, "Precipitation index(0.0-1.00)", "fraction", "PREIX" },
   { 219, "Std. dev. of IR T over 1x1 deg area", "K", "TSD1D" },
   { 220, "Natural log of surface pressure", "ln(kPa)", "NLGSP" },
   { 221, "Planetary boundary layer height", "m", "HPBL" },
   { 222, "5-wave geopotential height", "gpm", "5WAVH" },
   { 223, "Plant canopy surface water", "kg/m^2", "CNWAT" },
   { 224, "Soil type (as in Zobler)", "Integer (0-9)", "SOTYP" },
   { 225, "Vegitation type (as in SiB)", "Integer (0-13)", "VGTYP" },
   { 226, "Blackadar's mixing length scale", "m", "BMIXL" },
   { 227, "Asymptotic mixing length scale", "m", "AMIXL" },
   { 228, "Potential evaporation", "kg/m^2", "PEVAP" },
   { 229, "Snow phase-change heat flux", "W/m^2", "SNOHF" },
   { 230, "5-wave geopotential height anomaly", "gpm", "5WAVA" },
   { 231, "Convective cloud mass flux", "Pa/s", "MFLUX" },
   { 232, "Downward total radiation flux", "W/m^2", "DTRF" },
   { 233, "Upward total radiation flux", "W/m^2", "UTRF" },
   { 234, "Baseflow-groundwater runoff", "kg/m^2", "BGRUN" },
   { 235, "Storm surface runoff", "kg/m^2", "SSRUN" },
   { 236, "Supercooled Large Droplet (SLD) Icing Potential Diagnostic", "Numeric", "SIPD" },
   { 237, "Total ozone", "kg/m^2", "03TOT" },
   { 238, "Snow cover", "percent", "SNOWC" },
   { 239, "Snow temperature", "K", "SNOT" },
   { 240, "Covariance between temperature and vertical component of the wind", "K*m/s", "COVTW" },
   { 241, "Large scale condensate heat rate", "K/s", "LRGHR" },
   { 242, "Deep convective heating rate", "K/s", "CNVHR" },
   { 243, "Deep convective moistening rate", "kg/kg/s", "CNVMR" },
   { 244, "Shallow convective heating rate", "K/s", "SHAHR" },
   { 245, "Shallow convective moistening rate", "kg/kg/s", "SHAMR" },
   { 246, "Vertical diffusion heating rate", "K/s", "VDFHR" },
   { 247, "Vertical diffusion zonal acceleration", "m/s^2", "VDFUA" },
   { 248, "Vertical diffusion meridional acceleration", "m/s^2", "VDFVA" },
   { 249, "Vertical diffusion moistening rate", "kg/kg/s", "VDFMR" },
   { 250, "Solar radiative heating rate", "K/s", "SWHR" },
   { 251, "Long wave radiative heating rate", "K/s", "LWHR" },
   { 252, "Drag coefficient", "non-dim", "CD" },
   { 253, "Friction velocity", "m/s", "FRICV" },
   { 254, "Richardson number", "non-dim.", "RI" },
   { 255, "Missing", "-", "-" }
};
static const int n_grib_code_list_ptv2 = sizeof(grib_code_list_ptv2)/sizeof(GribCodeData);

////////////////////////////////////////////////////////////////////////

static const GribCodeData grib_code_list_ptv128[] = {
   { 128, "Ocean depth - mean", "m", "AVDEPTH" },
   { 129, "Ocean depth - instantaneous", "m", "DEPTH" },
   { 130, "Ocean surface elevation relative to geoid", "m", "ELEV" },
   { 131, "Max ocean surface elevation in last 24 hours", "m", "MXEL24" },
   { 132, "Min ocean surface elevation in last 24 hours", "m", "MNEL24" },
   { 135, "Oxygen (O2 (aq))", "Mol/kg", "O2" },
   { 136, "PO4", "Mol/kg", "PO4" },
   { 137, "NO3", "Mol/kg", "NO3" },
   { 138, "SiO4", "Mol/kg", "SIO4" },
   { 139, "CO2 (aq)", "Mol/kg", "CO2AQ" },
   { 140, "HCO3", "Mol/kg", "HCO3" },
   { 141, "CO3", "Mol/kg", "CO3" },
   { 142, "TCO2", "Mol/kg", "TCO2" },
   { 143, "TALK", "Mol/kg", "TALK" },
   { 146, "S11 - 1,1 component of ice stress tensor", "-", "S11" },
   { 147, "S12 - 1,2 component of ice stress tensor", "-", "S12" },
   { 148, "S22 - 2,2 component of ice stress tensor", "-", "S22" },
   { 149, "T1 - First invariant of stress tensor", "-", "INV1" },
   { 150, "T2 - Second invariant of stress tensor", "-", "INV2" },
   { 155, "Wave roughness", "-", "WVRGH" },
   { 156, "Wave stresses", "-", "WVSTRS" },
   { 157, "Whitecap coverage", "-", "WHITE" },
   { 158, "Swell direction width", "-", "SWDIRWID" },
   { 159, "Swell frequency width", "-", "SWFREWID" },
   { 160, "Wave age", "-", "WVAGE" },
   { 161, "Physical Wave age", "-", "PWVAGE" },
   { 165, "Master length scale (turbulence)", "m", "LTURB" },
   { 170, "Net air-ice heat flux", "W/m^2", "AIHFLX" },
   { 171, "Net air-ocean heat flux", "W/m^2", "AOHFLX" },
   { 172, "Net ice-ocean heat flux", "W/m^2", "IOHFLX" },
   { 173, "Net ice-ocean salt flux", "kg/s", "IOSFLX" },
   { 175, "Ocean mixed layer temperature", "K", "OMLT" },
   { 176, "Ocean mixed layer salinity", "kg/kg", "OMLS" },
   { 177, "Ocean mixed layer potential density (Referenced to 2000m)", "kg/m^3", "P2OMLT" },
   { 178, "Ocean mixed layer u velocity", "m/s", "OMLU" },
   { 179, "Ocean mixed layer v velocity", "m/s", "OMLV" },
   { 180, "Assimilative heat flux", "W/m^2", "ASHFL" },
   { 181, "Assimilative salt flux", "mm/day", "ASSFL" },
   { 182, "Bottom layer depth", "m", "BOTLD" },
   { 183, "Barotropic U velocity", "m/s", "UBARO" },
   { 184, "Barotropic V velocity", "m/s", "VBARO" },
   { 185, "Interface depths", "m", "INTFD" },
   { 186, "3-D temperature", "deg c", "WTMPC" },
   { 187, "3-D Salinity", "psu", "SALIN" },
   { 188, "Evaporation - precipitation", "cm/day", "EMNP" },
   { 190, "Kinetic energy", "J/kg", "KENG" },
   { 191, "Barotropic Kinetic energy", "J/kg", "BKENG" },
   { 192, "Layer Thickness", "m", "LAYTH" },
   { 193, "Surface temperature trend", "deg/day", "SSTT" },
   { 194, "Surface salinity trend", "psu/day", "SSST" },
   { 195, "Ocean Vertical Heat Diffusivity", "m^2/s", "OVHD" },
   { 196, "Ocean Vertical Salt Diffusivity", "m^2/s", "OVSHD" },
   { 197, "Ocean Vertical Momentum Diffusivity", "m^2/s", "OVMD" },
   { 254, "Relative error variance", "non-dim", "REV" },
   { 255, "Missing", "-", "-" }
};
static const int n_grib_code_list_ptv128 = sizeof(grib_code_list_ptv128)/sizeof(GribCodeData);

////////////////////////////////////////////////////////////////////////

static const GribCodeData grib_code_list_ptv129[] = {
   { 128, "Probability anomaly of temperature", "%", "PAOT" },
   { 129, "Probability anomaly of precipitation", "%", "PAOP" },
   { 130, "Probability of Wetting Rain, exceeding 0.10 inches in a given time period", "%", "CWR" },
   { 131, "Rain fraction of total liquid water", "non-dim", "FRAIN" },
   { 132, "Ice fraction of total condensate", "non-dim", "FICE" },
   { 133, "Rime Factor", "non-dim", "FRIME" },
   { 134, "Convective cloud efficiency", "non-dim", "CUEFI" },
   { 135, "Total condensate", "kg/kg", "TCOND" },
   { 136, "Total column-integrated cloud water", "kg/m^2", "TCOLW" },
   { 137, "Total column-integrated cloud ice", "kg/m^2", "TCOLI" },
   { 138, "Total column-integrated rain", "kg/m^2", "TCOLR" },
   { 139, "Total column-integrated snow", "kg/m^2", "TCOLS" },
   { 140, "Total column-integrated condensate", "kg/m^2", "TCOLC" },
   { 141, "Pressure of level from which parcel was lifted", "Pa", "PLPL" },
   { 142, "Height of level from which parcel was lifted", "m", "HLPL" },
   { 143, "Cloud Emissivity", "Fraction 0-1", "CEMS" },
   { 144, "Cloud Optical Depth", "non-dim", "COPD" },
   { 145, "Effective Particle size", "Microns", "PSIZ" },
   { 146, "Total Water Cloud", "%", "TCWAT" },
   { 147, "Total Ice Cloud", "%", "TCICE" },
   { 148, "Wind difference", "m/s", "WDIF" },
   { 149, "Wave Steepness", "non-dim", "WSTP" },
   { 150, "Probability of Temperature being above normal", "%", "PTAN" },
   { 151, "Probability of Temperature being near normal", "%", "PTNN" },
   { 152, "Probability of Temperature being below normal", "%", "PTBN" },
   { 153, "Probability of Precipitation being above normal", "%", "PPAN" },
   { 154, "Probability of Precipitation being near normal", "%", "PPNN" },
   { 155, "Probability of Precipitation being below normal", "%", "PPBN" },
   { 156, "Particulate matter (coarse)", "�g/m^3", "PMTC" },
   { 157, "Particulate matter (fine)", "�g/m^3", "PMTF" },
   { 158, "Analysis error of temperature", "K", "AETMP" },
   { 159, "Analysis error of dew point", "%", "AEDPT" },
   { 160, "Analysis error of specific humidity", "kg/kg", "AESPH" },
   { 161, "Analysis error of u-wind", "m/s", "AEUWD" },
   { 162, "Analysis error of v-wind", "m/s", "AEVWD" },
   { 163, "Particulate matter (fine)", "log10(�g/m^3)", "LPMTF" },
   { 164, "Integrated column particulate matter (fine)", "log10(�g/m^3)", "LIPMF" },
   { 165, "Derived radar reflectivity backscatter from rain", "mm^6/m^3", "REFZR" },
   { 166, "Derived radar reflectivity backscatter from ice", "mm^6/m^3", "REFZI" },
   { 167, "Derived radar reflectivity backscatter from parameterized convection", "mm^6/m^3", "REFZC" },
   { 168, "Total column-integrated supercooled liquid water", "kg/m^2", "TCLSW" },
   { 169, "Total column-integrated melting ice", "kg/m^2", "TCOLM" },
   { 170, "Ellrod Index", "non-dim", "ELRDI" },
   { 171, "Seconds prior to initial reference time (defined in octects 18-20)", "sec", "TSEC" },
   { 172, "Seconds after initial reference time (defined in octets 18-20)", "sec", "TSECA" },
   { 173, "Number of samples/observations", "non-dim", "NUM" },
   { 174, "Analysis error of pressure", "Pa", "AEPRS" },
   { 175, "Icing severity", "non-dim", "ICSEV" },
   { 176, "Icing probability", "non-dim", "ICPRB" },
   { 177, "Low-level aviation interest", "non-dim", "LAVNI" },
   { 178, "High-level aviation interest", "non-dim", "HAVNI" },
   { 179, "Flight Category", "non-dim", "FLGHT" },
   { 180, "Ozone concentration", "PPB", "OZCON" },
   { 181, "Categorical ozone concentration", "non-dim", "OZCAT" },
   { 182, "Vertical eddy diffusivity heat exchange (Kh)", "m^2/s", "VEDH" },
   { 183, "Sigma level value", "non-dim", "SIGV" },
   { 184, "Ensemble Weight", "non-dim", "EWGT" },
   { 185, "Confidence indicator - Ceiling", "non-dim", "CICEL" },
   { 186, "Confidence indicator - Visibility", "non-dim", "CIVIS" },
   { 187, "Confidence indicator - Flight Category", "non-dim", "CIFLT" },
   { 188, "Latitude of V wind component of velocity", "deg", "LAVV" },
   { 189, "Longitude of V wind component of velocity", "deg", "LOVV" },
   { 190, "Scatterometer estimated U wind component", "m/s", "USCT" },
   { 191, "Scatterometer estimated V wind component", "m/s", "VSCT" },
   { 192, "Latitude of U wind component of velocity", "deg", "LAUV" },
   { 193, "Longitude of U wind component of velocity", "deg", "LOUV" },
   { 194, "Tropical Cyclone Heat Potential", "J/m^2", "TCHP" },
   { 195, "Geometric Depth Below Sea Surface", "m", "DBSS" },
   { 196, "Ocean Dynamic Height Anomaly", "dynamic m", "ODHA" },
   { 197, "Ocean Heat Content", "J/m^2", "OHC" },
   { 198, "Sea Surface Height Relative to Geoid", "m", "SSHG" },
   { 199, "Salt Flux", "kg/m^2s", "SLTFL" },
   { 200, "UV-B downward solar flux", "W/m^2", "DUVB" },
   { 201, "Clear sky UV-B downward solar flux", "W/m^2", "CDUVB" },
   { 202, "Total downward heat flux at surface (downward is positive)", "W/m^2", "THFLX" },
   { 203, "U velocity variance", "m^2/s^2", "UVAR" },
   { 204, "V velocity variance", "m^2/s^2", "VVAR" },
   { 205, "UV Velocity Cross Correlation", "m^2/s^2", "UVVCC" },
   { 206, "Meteorological Correlation Length Scale", "m", "MCLS" },
   { 207, "Latitude of pressure point", "deg", "LAPP" },
   { 208, "Longitude of pressure point", "deg", "LOPP" },
   { 210, "Observed radar reflectivity", "dbZ", "REFO" },
   { 211, "Derived radar reflectivity", "dbZ", "REFD" },
   { 212, "Maximum/Composite radar reflectivity", "dbZ", "REFC" },
   { 213, "Simulated Brightness Temperature for GOES12, Channel 2", "K", "SBT122" },
   { 214, "Simulated Brightness Temperature for GOES12, Channel 3", "K", "SBT123" },
   { 215, "Simulated Brightness Temperature for GOES12, Channel 4", "K", "SBT124" },
   { 216, "Simulated Brightness Temperature for GOES12, Channel 5", "K", "SBT125" },
   { 217, "Minimum Relative Humidity", "%", "MINRH" },
   { 218, "Maximum Relative Humidity", "%", "MAXRH" },
   { 219, "Ceiling", "m", "CEIL" },
   { 220, "Planetary boundary layer Regime", "-", "PBLREG" },
   { 221, "Simulated Brightness Counts for GOES12, Channel 3", "Byte", "SBC123" },
   { 222, "Simulated Brightness Counts for GOES12, Channel 4", "Byte", "SBC124" },
   { 223, "Rain Precipitation Rate", "kg/m^2/s", "RPRATE" },
   { 224, "Snow Precipitation Rate", "kg/m^2/s", "SPRATE" },
   { 225, "Freezing Rain Precipitation Rate", "kg/m^2/s", "FPRATE" },
   { 226, "Ice Pellets Precipitation Rate", "kg/m^2/s", "IPRATE" },
   { 227, "Updraft Helicity", "m^2/s^2", "UPHL" },
   { 228, "Storm Surge", "m", "SURGE" },
   { 229, "Extra Tropical Storm Surge", "m", "ETSRG" },
   { 230, "Relative Humidity with Respect to Precipitable Water", "%", "RHPW" },
   { 231, "Ozone Daily Max from 1-hour Average", "ppbV", "OZMAX1" },
   { 232, "Ozone Daily Max from 8-hour Average", "ppbV", "OZMAX8" },
   { 233, "PM 2.5 Daily Max from 1-hour Average", "nanog/m^3", "PDMAX1" },
   { 234, "PM 2.5 Daily Max from 24-hour Average", "nanog/m^3", "PDMAX24" },
   { 240, "Radar Echo Top (18.3 DBZ)", "m", "RETOP" },
   { 242, "20% Tropical Cyclone Storm Surge Exceedance", "m", "TCSRG20" },
   { 243, "30% Tropical Cyclone Storm Surge Exceedance", "m", "TCSRG30" },
   { 244, "40% Tropical Cyclone Storm Surge Exceedance", "m", "TCSRG40" },
   { 245, "50% Tropical Cyclone Storm Surge Exceedance", "m", "TCSRG50" },
   { 246, "60% Tropical Cyclone Storm Surge Exceedance", "m", "TCSRG60" },
   { 247, "70% Tropical Cyclone Storm Surge Exceedance", "m", "TCSRG70" },
   { 248, "80% Tropical Cyclone Storm Surge Exceedance", "m", "TCSRG80" },
   { 249, "90% Tropical Cyclone Storm Surge Exceedance", "m", "TCSRG90" },
   { 251, "Difference Between 2 States In Total Energy Norm", "J/kg", "DIFTEN" },
   { 252, "Pseudo-Precipitation", "kg/m^2", "PSPCP" },
   { 255, "Missing", "-", "-" }
};
static const int n_grib_code_list_ptv129 = sizeof(grib_code_list_ptv129)/sizeof(GribCodeData);

////////////////////////////////////////////////////////////////////////

static const GribCodeData grib_code_list_ptv130[] = {
   { 144, "Volumetric soil moisture (frozen + liquid)", "fraction", "SOILW" },
   { 145, "Potential latent heat flux (potential evaporation)", "W/m^2", "PEVPR" },
   { 146, "Vegetation canopy temperature", "K", "VEGT" },
   { 147, "Bare soil surface skin temperature", "K", "BARET" },
   { 148, "Average surface skin temperature", "K", "AVSFT" },
   { 149, "Effective radiative skin temperature", "K", "RADT" },
   { 150, "Surface water storage", "kg/m^2", "SSTOR" },
   { 151, "Liquid soil moisture content (non-frozen)", "kg/m^2", "LSOIL" },
   { 152, "Open water evaporation (standing water)", "W/m^2", "EWATR" },
   { 154, "Land Surface Precipitation Accumulation (LSPA)", "kg/m^2", "LSPA" },
   { 155, "Ground Heat Flux", "W/m^2", "GFLUX" },
   { 156, "Convective inhibition (CIN)", "J/kg", "CIN" },
   { 157, "Convective available potential energy (CAPE)", "J/kg", "CAPE" },
   { 158, "Turbulent Kinetic Energy", "J/kg", "TKE" },
   { 159, "Maximum snow albedo", "%", "MXSALB" },
   { 160, "Liquid volumetric soil moisture (non-frozen)", "fraction", "SOILL" },
   { 161, "Frozen precipitation (e.g. snowfall)", "kg/m^2", "ASNOW" },
   { 162, "Liquid precipitation (rainfall)", "kg/m^2", "ARAIN" },
   { 163, "Groundwater recharge", "kg/m^2", "GWREC" },
   { 164, "Flood plain recharge", "kg/m^2", "QREC" },
   { 165, "Snow temperature, depth-avg", "K", "SNOWT" },
   { 166, "Visible beam downward solar flux", "W/m^2", "VBDSF" },
   { 167, "Visible diffuse downward solar flux", "W/m^2", "VDDSF" },
   { 168, "Near IR beam downward solar flux", "W/m^2", "NBDSF" },
   { 169, "Near IR diffuse downward solar flux", "W/m^2", "NDDSF" },
   { 170, "Snow-free albedo", "%", "SNFALB" },
   { 171, "Number of soil layers in root zone", "non-dim", "RLYRS" },
   { 172, "Momentum flux", "N/m^2", "MFLX" },
   { 176, "Latitude (-90 to +90)", "deg", "NLAT" },
   { 177, "East longitude (0-360)", "deg", "ELON" },
   { 178, "Field Capacity (soil moisture)", "fraction", "FLDCAP" },
   { 179, "Aerodynamic conductance", "m/s", "ACOND" },
   { 180, "Snow age", "s", "SNOAG" },
   { 181, "Canopy conductance", "m/s", "CCOND" },
   { 182, "Leaf area index (0-9)", "non-dim", "LAI" },
   { 183, "Roughness length for heat", "m", "SFCRH" },
   { 184, "Snow albedo (over snow cover area only)", "%", "SALBD" },
   { 187, "Normalized Difference Vegetation Index (NDVI)", "non-dim", "NDVI" },
   { 188, "Canopy drip", "kg/m^2", "DRIP" },
   { 189, "Visible, Black Sky Albedo", "%", "VBSALB" },
   { 190, "Visible, White Sky Albedo", "%", "VWSALB" },
   { 191, "Near IR, Black Sky Albedo", "%", "NBSALB" },
   { 192, "Near IR, White Sky Albedo", "%", "NWSALB" },
   { 198, "Sublimation (evaporation from snow)", "W/m^2", "SBSNO" },
   { 199, "Direct evaporation from bare soil", "W/m^2", "EVBS" },
   { 200, "Canopy water evaporation", "W/m^2", "EVCW" },
   { 203, "Minimal stomatal resistance", "s/m", "RSMIN" },
   { 204, "Downward shortwave radiation flux", "W/m^2", "DSWRF" },
   { 205, "Downward longwave radiation flux", "W/m^2", "DLWRF" },
   { 207, "Moisture availability", "%", "MSTAV" },
   { 208, "Exchange coefficient", "(kg/m^3)(m/s)", "SFEXC" },
   { 210, "Transpiration", "W/m^2", "TRANS" },
   { 211, "Upward short wave radiation flux", "W/m^2", "USWRF" },
   { 212, "Upward long wave radiation flux", "W/m^2", "ULWRF" },
   { 219, "Wilting point", "fraction", "WILT" },
   { 220, "Field Capacity", "fraction", "FLDCP" },
   { 221, "Planetary boundary layer height", "m", "HPBL" },
   { 222, "Surface slope type", "Index", "SLTYP" },
   { 223, "Plant canopy surface water", "kg/m^2", "CNWAT" },
   { 224, "Soil type", "Index (0-9)", "SOTYP" },
   { 225, "Vegetation type", "Index (0-13)", "VGTYP" },
   { 226, "Blackadar's mixing length scale", "m", "BMIXL" },
   { 227, "Asymptotic mixing length scale", "m", "AMIXL" },
   { 228, "Potential evaporation", "kg/m^2", "PEVAP" },
   { 229, "Snow phase-change heat flux", "W/m^2", "SNOHF" },
   { 230, "Transpiration stress-onset (soil moisture)", "fraction", "SMREF" },
   { 231, "Direct evaporation cease (soil moisture)", "fraction", "SMDRY" },
   { 234, "Subsurface runoff (baseflow)", "kg/m^2", "BGRUN" },
   { 235, "Surface runoff (non-infiltrating)", "kg/m^2", "SSRUN" },
   { 238, "Snow cover", "%", "SNOWC" },
   { 239, "Snow temperature", "K", "SNOT" },
   { 240, "Soil porosity", "fraction", "POROS" },
   { 246, "Solar parameter in canopy conductance", "fraction", "RCS" },
   { 247, "Temperature parameter in canopy conductance", "fraction", "RCT" },
   { 248, "Humidity parameter in canopy conductance", "fraction", "RCQ" },
   { 249, "Soil moisture parameter in canopy conductance", "fraction", "RCSOL" },
   { 252, "Surface drag coefficient", "non-dim", "CD" },
   { 253, "Surface friction velocity", "m/s", "FRICV" },
   { 254, "Richardson number", "non-dim", "RI" },
   { 255, "Missing", "-", "-" }
};
static const int n_grib_code_list_ptv130 = sizeof(grib_code_list_ptv130)/sizeof(GribCodeData);

////////////////////////////////////////////////////////////////////////

static const GribCodeData grib_code_list_ptv131[] = {
   { 128, "Mean sea level pressure (Std Atm)", "Pa", "MSLSA" },
   { 130, "Mean sea level pressure (ETA model)", "Pa", "MSLET" },
   { 131, "Surface lifted index", "K", "LFTX" },
   { 132, "Best (4-layer) lifted index", "K", "4LFTX" },
   { 134, "Pressure (nearest grid point)", "Pa", "PRESN" },
   { 135, "Horizontal moisture divergence", "kg/kg/s", "MCONV" },
   { 136, "Vertical speed shear", "l/s", "VWSH" },
   { 139, "Potential vorticity (mass-weighted)", "1/s/m", "PVMWW" },
   { 140, "Categorical rain [yes=1; no=0]", "-", "CRAIN" },
   { 141, "Categorical freezing rain [yes=1; no=0]", "-", "CFRZR" },
   { 142, "Categorical ice pellets [yes=1; no=0]", "-", "CICEP" },
   { 143, "Categorical snow [yes=1; no=0]", "-", "CSNOW" },
   { 144, "Volumetric soil moisture (frozen + liquid)", "fraction", "SOILW" },
   { 145, "Potential evaporation rate", "W/m^2", "PEVPR" },
   { 146, "Vegetation canopy temperature", "K", "VEGT" },
   { 147, "Bare soil surface skin temperature", "K", "BARET" },
   { 148, "Average surface skin temperature", "K", "AVSFT" },
   { 149, "Effective radiative skin temperature", "K", "RADT" },
   { 150, "Surface water storage", "kg/m^2", "SSTOR" },
   { 151, "Liquid soil moisture content (non-frozen)", "kg/m^2", "LSOIL" },
   { 152, "Open water evaporation (standing water)", "W/m^2", "EWATR" },
   { 153, "Cloud water", "kg/kg", "CLWMR" },
   { 155, "Ground Heat Flux", "W/m^2", "GFLUX" },
   { 156, "Convective inhibition", "J/kg", "CIN" },
   { 157, "Convective available potential energy", "J/kg", "CAPE" },
   { 158, "Turbulent Kinetic Energy", "J/kg", "TKE" },
   { 159, "Maximum snow albedo", "%", "MXSALB" },
   { 160, "Liquid volumetric soil moisture (non-frozen)", "fraction", "SOILL" },
   { 161, "Frozen precipitation (e.g. snowfall)", "kg/m^2", "ASNOW" },
   { 162, "Liquid precipitation (rainfall)", "kg/m^2", "ARAIN" },
   { 163, "Groundwater recharge", "kg/m^2", "GWREC" },
   { 164, "Flood plain recharge", "kg/m^2", "QREC" },
   { 165, "Snow temperature, depth-avg", "K", "SNOWT" },
   { 166, "Visible beam downward solar flux", "W/m^2", "VBDSF" },
   { 167, "Visible diffuse downward solar flux", "W/m^2", "VDDSF" },
   { 168, "Near IR beam downward solar flux", "W/m^2", "NBDSF" },
   { 169, "Near IR diffuse downward solar flux", "W/m^2", "NDDSF" },
   { 170, "Snow-free albedo", "%", "SNFALB" },
   { 171, "Number of soil layers in root zone", "non-dim", "RLYRS" },
   { 172, "Momentum flux", "N/m^2", "MFLX" },
   { 173, "Mass point model surface", "non-dim", "LMH" },
   { 174, "Velocity point model surface", "non-dim", "LMV" },
   { 175, "Model layer number (from bottom up)", "non-dim", "MLYNO" },
   { 176, "Latitude (-90 to +90)", "deg", "NLAT" },
   { 177, "East longitude (0-360)", "deg", "ELON" },
   { 178, "Ice mixing ratio", "kg/kg", "ICMR" },
   { 179, "Aerodynamic conductance", "m/s", "ACOND" },
   { 180, "Snow age", "s", "SNOAG" },
   { 181, "Canopy conductance", "m/s", "CCOND" },
   { 182, "Leaf area index (0-9)", "non-dim", "LAI" },
   { 183, "Roughness length for heat", "m", "SFCRH" },
   { 184, "Snow albedo (over snow cover area only)", "%", "SALBD" },
   { 187, "Normalized Difference Vegetation Index", "non-dim", "NDVI" },
   { 188, "Rate of water dropping from canopy to gnd", "kg/m^2", "DRIP" },
   { 189, "Land-sea coverage (nearest neighbor) [land=1, sea=0]", "non-dim", "LANDN" },
   { 190, "Storm relative helicity", "m^2/s^2", "HLCY" },
   { 191, "Latitude (nearest neighbor) (-90 to +90)", "deg", "NLATN" },
   { 192, "East longitude (nearest neighbor) (0-360)", "deg", "ELONN" },
   { 194, "Prob. of frozen precipitation", "%", "CPOFP" },
   { 196, "u-component of storm motion", "m/s", "USTM" },
   { 197, "v-component of storm motion", "m/s", "VSTM" },
   { 198, "Sublimation (evaporation from snow)", "W/m^2", "SBSNO" },
   { 199, "Direct evaporation from bare soil", "W/m^2", "EVBS" },
   { 200, "Canopy water evaporation", "W/m^2", "EVCW" },
   { 202, "Total precipitation (nearest grid point)", "kg/m^2", "APCPN" },
   { 203, "Minimal stomatal resistance", "s/m", "RSMIN" },
   { 204, "Downward shortwave radiation flux", "W/m^2", "DSWRF" },
   { 205, "Downward longwave radiation flux", "W/m^2", "DLWRF" },
   { 206, "Convective precipitation (nearest grid point)", "kg/m^2", "ACPCPN" },
   { 207, "Moisture availability", "%", "MSTAV" },
   { 208, "Exchange coefficient", "(kg/m^3)(m/s)", "SFEXC" },
   { 210, "Transpiration", "W/m^2", "TRANS" },
   { 211, "Upward short wave radiation flux", "W/m^2", "USWRF" },
   { 212, "Upward long wave radiation flux", "W/m^2", "ULWRF" },
   { 213, "Non -convective cloud", "%", "CDLYR" },
   { 214, "Convective precip. rate", "kg/m^2/s", "CPRAT" },
   { 216, "Temperature tendency by all radiation", "K/s", "TTRAD" },
   { 218, "Geopotential Height (nearest grid point)", "gpm", "HGTN" },
   { 219, "Wilting point", "fraction", "WILT" },
   { 220, "Field Capacity", "fraction", "FLDCP" },
   { 221, "Planetary boundary layer height", "m", "HPBL" },
   { 222, "Surface slope type", "index", "SLTYP" },
   { 223, "Plant canopy surface water", "kg/m^2", "CNWAT" },
   { 224, "Soil type", "index", "SOTYP" },
   { 225, "Vegetation type", "index", "VGTYP" },
   { 226, "Blackadars mixing length scale", "m", "BMIXL" },
   { 227, "Asymptotic mixing length scale", "m", "AMIXL" },
   { 228, "Potential evaporation", "kg/m^2", "PEVAP" },
   { 229, "Snow phase-change heat flux", "W/m^2", "SNOHF" },
   { 230, "Transpiration stress-onset (soil moisture)", "fraction", "SMREF" },
   { 231, "Direct evaporation cease (soil moisture)", "fraction", "SMDRY" },
   { 232, "water vapor added by precip assimilation", "kg/m^2", "WVINC" },
   { 233, "water condensate added by precip assimilation", "kg/m^2", "WCINC" },
   { 234, "Subsurface runoff (baseflow)", "kg/m^2", "BGRUN" },
   { 235, "Surface runoff (non-infiltrating)", "kg/m^2", "SSRUN" },
   { 237, "Water vapor flux convergence (vertical int)", "kg/m^2", "WVCONV" },
   { 238, "Snow cover", "%", "SNOWC" },
   { 239, "Snow temperature", "K", "SNOT" },
   { 240, "Soil porosity", "fraction", "POROS" },
   { 241, "Water condensate flux convergence (vertical int)", "kg/m^2", "WCCONV" },
   { 242, "Water vapor zonal flux (vertical int)", "kg/m", "WVUFLX" },
   { 243, "Water vapor meridional flux (vertical int)", "kg/m", "WVVFLX" },
   { 244, "Water condensate zonal flux (vertical int)", "kg/m", "WCUFLX" },
   { 245, "Water condensate meridional flux (vertical int)", "kg/m", "WCVFLX" },
   { 246, "Solar parameter in canopy conductance", "fraction", "RCS" },
   { 247, "Temperature parameter in canopy conductance", "fraction", "RCT" },
   { 248, "Humidity parameter in canopy conductance", "fraction", "RCQ" },
   { 249, "Soil moisture parameter in canopy conductance", "fraction", "RCSOL" },
   { 250, "Solar radiative heating rate", "K/s", "SWHR" },
   { 251, "Longwave radiative heating rate", "K/s", "LWHR" },
   { 252, "Surface drag coefficient", "non-dim", "CD" },
   { 253, "Surface friction velocity", "m/s", "FRICV" },
   { 254, "Richardson number", "non-dim", "RI" },
   { 255, "Missing", "-", "-" }
};
static const int n_grib_code_list_ptv131 = sizeof(grib_code_list_ptv131)/sizeof(GribCodeData);

////////////////////////////////////////////////////////////////////////

static const GribCodeData grib_code_list_ptv133[] = {
   { 139, "Ozone production from temperature term", "kg/kg/s", "POZT" },
   { 154, "Omega (Dp/Dt) divide by density", "K", "OMGALF" },
   { 164, "Covariance between zonal and zonal components of the wind", "m^2/s^2", "COVZZ" },
   { 165, "Covariance between meridional and meridional components of the wind", "m^2/s^2", "COVMM" },
   { 166, "Covariance between specific humidity and zonal components of the wind", "kg/kg*m/s", "COVQZ" },
   { 167, "Covariance between specific humidity and meridional components of the wind", "kg/kg*m/s", "COVQM" },
   { 168, "Covariance between temperature and vertical components of the wind", "K*Pa/s", "COVTVV" },
   { 169, "Covariance between specific humidity and vertical components of the wind", "kg/kg*Pa/s", "COVQVV" },
   { 173, "Large scale moistening rate", "kg/kg/s", "LRGMR" },
   { 174, "Ozone vertical diffusion", "kg/kg/s", "VDFOZ" },
   { 175, "Ozone production", "kg/kg/s", "POZ" },
   { 181, "Gravity wave drag zonal acceleration", "m/s^2", "GWDU" },
   { 182, "Gravity wave drag meridional acceleration", "m/s^2", "GWDV" },
   { 183, "Convective zonal momentum mixing acceleration", "m/s^2", "CNVU" },
   { 184, "Convective meridional momentum mixing acceleration", "m/s^2", "CNVV" },
   { 185, "Surface exchange coefficients for T and Q divided by delta z", "m/s", "AKHS" },
   { 186, "Surface exchange coefficients for U and V divided by delta z", "m/s", "AKMS" },
   { 188, "Ozone tendency", "kg/kg/s", "TOZ" },
   { 191, "Sunshine Duration", "s", "SUNSD" },
   { 192, "Meridional Overturning Stream Function", "10^6*m^3/s", "MOSF" },
   { 193, "Radiative emissivity", "-", "EPSR" },
   { 195, "Specific humidity at top of viscous sublayer", "kg/kg", "QZ0" },
   { 196, "Convective Gravity wave drag zonal acceleration", "m/s^2", "CNGWDU" },
   { 197, "Convective Gravity wave drag meridional acceleration", "m/s^2", "CNGWDV" },
   { 201, "Potential temperature at top of viscous sublayer", "K", "THZ0" },
   { 202, "Convective updraft mass flux", "kg/m^2/s", "CNVUMF" },
   { 203, "Covariance between surface pressure and surface pressure", "Pa*Pa", "COVPSPS" },
   { 204, "Maximum specific humidity at 2m", "kg/kg", "QMAX" },
   { 205, "Minimum specific humidity at 2m", "kg/kg", "QMIN" },
   { 206, "Covariance between specific humidity and specific humidy", "kg/kg*kg/kg", "COVQQ" },
   { 209, "Convective downdraft mass flux", "kg/m^2/s", "CNVDMF" },
   { 219, "Convective detrainment mass flux", "kg/m^2/s", "CNVDEMF" },
   { 220, "Covariance between vertical and vertical components of the wind", "Pa^2/s^2", "COVVVVV" },
   { 234, "Covariance between temperature and temperature", "K*K", "COVTT" },
   { 236, "Tendency of vertical velocity", "m/s^2", "WTEND" },
   { 239, "Ozone production from col ozone term", "kg/kg/s", "POZO" },
   { 255, "Missing", "-", "-" }
};
static const int n_grib_code_list_ptv133 = sizeof(grib_code_list_ptv133)/sizeof(GribCodeData);

////////////////////////////////////////////////////////////////////////

static const GribCodeData grib_code_list_ptv140[] = {
   { 168, "Mean Icing Potential", "-", "MEIP" },
   { 169, "Maximum Icing Potential", "-", "MAIP" },
   { 170, "Mean in-Cloud Turbulence Potential", "-", "MECTP" },
   { 171, "Max in-Cloud Turbulence Potential", "-", "MACTP" },
   { 172, "Mean Cloud Air Turbulence Potential", "-", "MECAT" },
   { 173, "Maximum Cloud Air Turbulence Potential", "-", "MACAT" },
   { 174, "Cumulonimbus Horizontal Extent", "%", "CBHE" },
   { 175, "Pressure at Cumulonimbus Base", "Pa", "PCBB" },
   { 176, "Pressure at Cumulonimbus Top", "Pa", "PCBT" },
   { 177, "Pressure at Embedded Cumulonimbus Base", "Pa", "PECBB" },
   { 178, "Pressure at Embedded Cumulonimbus Top", "Pa", "PECBT" },
   { 179, "ICAO Height at Cumulonimbus Base", "m", "HCBB" },
   { 180, "ICAO Height at Cumulonimbus Top", "m", "HCBT" },
   { 181, "ICAO Height at Embedded Cumulonimbus Base", "m", "HECBB" },
   { 182, "ICAO Height at Embedded Cumulonimbus Top", "m", "HECBT" },
   { 255, "Missing", "-", "-" }
};
static const int n_grib_code_list_ptv140 = sizeof(grib_code_list_ptv140)/sizeof(GribCodeData);

////////////////////////////////////////////////////////////////////////

static const GribCodeData grib_code_list_ptv141[] = {
   { 128, "Aerosol Extinction Coefficient", "km-1", "EXTNC" },
   { 129, "Aerosol Optical Depth", "-", "AOD" },
   { 130, "Aerosol Asymmetry Factor", "-", "ASFTR" },
   { 131, "Aerosol Single Scatter Albedo", "-", "SSALBD" },
   { 132, "Aerosol Back Scattering", "k/m*s/r", "BSCTRS" },
   { 140, "Total Inorganic and Organic Nitrates", "ppbV", "NOy" },
   { 141, "Nitrogen Oxide", "ppbV", "NO" },
   { 142, "Nitrogen Dioxide", "ppbV", "NO2" },
   { 143, "Nitrogen Pentoxide", "ppbV", "N2O5" },
   { 144, "Nitric Acid", "ppbV", "HNO3" },
   { 145, "Nitrogen Trioxide", "ppbV", "NO3" },
   { 146, "Peroxynitric Acid", "ppbV", "PNA" },
   { 147, "Nitrous Acid", "ppbV", "HONO" },
   { 148, "Carbon Monoxide", "ppbV", "CO" },
   { 149, "Ammonia", "ppbV", "NH3" },
   { 150, "Hydrogen Chloride", "ppbV", "HCL" },
   { 159, "Lumped Single-Bond Carbon Specie", "ppbV", "PAR" },
   { 160, "Ethene", "ppbV", "ETHE" },
   { 161, "Lumped Double-Bond Carbon Species Less Ethene", "ppbV", "OLE" },
   { 162, "Toluene", "ppbV", "TOL" },
   { 163, "Xylene", "ppbV", "XYL" },
   { 164, "Isoprene", "ppbV", "ISOP" },
   { 166, "Formaldehyde", "ppbV", "FORM" },
   { 167, "Acetaldehyde & Higher Aldehydes", "ppbV", "ALD2" },
   { 168, "Methyl Glyoxal", "ppbV", "MGLY" },
   { 169, "Cresol and Higher Molecular Weight Phenols", "ppbV", "CRES" },
   { 172, "Peroxyacyl Nitrate", "ppbV", "PAN" },
   { 173, "Lumped Gaseous Organic Nitrate", "ppbV", "NTR" },
   { 177, "Esters", "ppbV", "ROOH" },
   { 178, "Ethanol", "ppbV", "ETHOH" },
   { 179, "Methanol", "ppbV", "METHOH" },
   { 186, "Hydrogen Peroxide", "ppbV", "H2O2" },
   { 187, "Hydroxyl Radical", "ppbV", "OH" },
   { 188, "Hydroperoxyl Radical", "ppbV", "HO2" },
   { 200, "Sulfate (SO4) Particulates <= 2.5 nanom Diameter", "nanog/m^3", "ASO4" },
   { 201, "Ammonia (NH4) Particulates <= 2.5 nanom Diameter", "nanog/m^3", "ANH4" },
   { 202, "Nitrate (NO3) Particulates <= 2.5 nanom Diameter", "nanog/m^3", "ANO3" },
   { 203, "Organic Particulates <= 2.5 nanom Diameter", "nanog/m^3", "AORGA" },
   { 204, "Primarily Organic Particulates <= 2.5 nanom Diameter", "nanog/m^3", "AORGPA" },
   { 205, "Biogenically Originated Particulates <= 2.5 nanom Diameter", "nanog/m^3", "AORGB" },
   { 206, "Elemental Carbon Particulates <= 2.5 nanom Diameter", "nanog/m^3", "AEC" },
   { 207, "Unspecified Anthropogenic Particulates <= 2.5 nanom Diameter", "nano/m^3", "A25" },
   { 208, "Water Particulates <= 2.5 nanom Diameter", "nano/m^3", "AH2O" },
   { 209, "Sodium Particulates <= 2.5 nanom Diameter", "nano/m^3", "ANA" },
   { 210, "Chloride Particulates <= 2.5 nanom Diameter", "nano/m^3", "ACL" },
   { 216, "Sulfate (SO4) Particulates between 2.5 and 10 nanom Diameter", "nano/m^3", "ASO4K" },
   { 217, "Sodium (NA) Particulates between 2.5 and 10 nanom Diameter", "nano/m^3", "ANAK" },
   { 218, "Chloride (CL) Particulates between 2.5 and 10 nanom Diameter", "nano/m^3", "ACLK" },
   { 219, "Sea Salt Originated Particulates between 2.5 and 10 nanom Diameter", "nano/m^3", "ASEAS" },
   { 220, "Crustal Material Originated Particulates between 2.5 and 10 nanom Diameter", "nano/m^3", "ASOIL" },
   { 221, "Particulates between 2.5 and 10 nanom Diameter", "nano/m^3", "ACORS" },
   { 222, "Number Concentration Particulates between 2.5 and 0.1 nanom Diameter", "number/m^3", "NUMATKN" },
   { 223, "Number Concentration Particulates between 2.5 and 2.5 nanom Diameter", "number/m^3", "NUMACC" },
   { 224, "Number Concentration Particulates between 2.5 and 10 nanom Diameter", "number/m^3", "NUMCOR" },
   { 228, "Surface Area Contributed by Particulates <= 0.1 nanom Diameter", "m^2/m^3", "SRFATKN" },
   { 229, "Surface Area Contributed by Particulates between 0.1 and 2.5 nanom Diameter", "m^2/m^3", "SRFACC" },
   { 232, "Sulfur Dioxide", "ppbV", "SO2" },
   { 233, "Methanesulfonic Acid", "kg/kg", "MSA" },
   { 234, "Total Sulfate Particulates (Fine and Coarse)", "nano/m^3", "TSO4" },
   { 235, "Dimethylsulfide", "kg/kg", "DMS" },
   { 240, "Dust Particulates between 0.2 - 2.0 nanom Diameter", "kg/kg", "DU1" },
   { 241, "Dust Particulates between 2.0 - 3.6 nanom Diameter", "kg/kg", "DU2" },
   { 242, "Dust Particulates between 3.6 - 6.0 nanom Diameter", "kg/kg", "DU3" },
   { 243, "Dust Particulates between 6.0 - 12.0 nanom Diameter", "kg/kg", "DU4" },
   { 244, "Dust Particulates between 12.0 - 20.0 nanom Diameter", "kg/kg", "DU5" },
   { 245, "Sea Salt Particulates between 0.2 - 1.0 nanom Diameter", "kg/kg", "SS1" },
   { 246, "Sea Salt Particulates between 1.0 - 3.0 nanom Diameter", "kg/kg", "SS2" },
   { 247, "Sea Salt Particulates between 3.0 - 10.0 nanom Diameter", "kg/kg", "SS3" },
   { 248, "Sea Salt Particulates between 10.0 - 20.0 nanom Diameter", "kg/kg", "SS4" },
   { 249, "Hydrophobic Organic Carbon", "kg/kg", "OCDRY" },
   { 250, "Hydrophilic Organic Carbon", "kg/kg", "OCWET" },
   { 251, "Hydrophobic Black Carbon", "kg/kg", "BCDRY" },
   { 252, "Hydrophilic Black Carbon", "kg/kg", "BCWET" },
   { 255, "Missing", "-", "-" }
};
static const int n_grib_code_list_ptv141 = sizeof(grib_code_list_ptv141)/sizeof(GribCodeData);

////////////////////////////////////////////////////////////////////////
//
// NCEP Office Note 388 - Table 3
// Level Types and Values
//
// (1) Level Value
// (2) Level Type:
//     0 -> no specific level
//     2 -> vertical level type
//     3 -> pressure level type
// (3) Level Flag:
//     Octets 11 and 12 in the PDS:
//     0 -> ignore them
//     1 -> combine them to form one number
//     2 -> octet 11 contains the beginning of a range and
//          octet 12 contains the end of a range
// (4) Level Name
// (5) Level Abbreviation
//
////////////////////////////////////////////////////////////////////////

struct GribLevelData {
   int         level;
   int         type;
   int         flag;
   const char *name;
   const char *abbr;
};

////////////////////////////////////////////////////////////////////////
//
// GRIB tables: http://www.nco.ncep.noaa.gov/pmb/docs/on388/table3.html
// Last Updated 05/11/2009
//
////////////////////////////////////////////////////////////////////////

static const GribLevelData grib_level_list[] = {
   {   1, 2, 0, "Ground or water surface", "SFC" },
   {   2, 0, 0, "Cloud base level", "CBL" },
   {   3, 0, 0, "Cloud top level", "CTL" },
   {   4, 0, 0, "Level of 0 deg (C) isotherm", "0DEG" },
   {   5, 0, 0, "Level of adiabatic condensation lifted from the surface", "ADCL" },
   {   6, 0, 0, "Maximum wind level", "MWSL" },
   {   7, 0, 0, "Tropopause", "TRO" },
   {   8, 0, 0, "Nominal top of atmosphere", "NTAT" },
   {   9, 0, 0, "Sea bottom", "SEAB" },
   {  20, 0, 2, "Isothermal level (temperature in 1/100 K in octets 11 and 12)", "TMPL" },
   { 100, 3, 1, "isobaric level", "ISBL" },
   { 101, 3, 2, "layer between two isobaric levels", "ISBY" },
   { 102, 2, 0, "mean sea level", "MSL" },
   { 103, 2, 1, "Specified altitude above MSL", "GPML" },
   { 104, 2, 2, "layer between two specified altitudes above MSL", "GPMY" },
   { 105, 2, 1, "specified height level above ground", "HTGL" },
   { 106, 2, 2, "layer between two specified height levels above ground", "HTGY" },
   { 107, 0, 1, "sigma level", "SIGL" },
   { 108, 0, 2, "layer between two sigma levels", "SIGY" },
   { 109, 0, 1, "Hybrid level", "HYBL" },
   { 110, 0, 2, "layer between two hybrid levels", "HYBY" },
   { 111, 2, 1, "depth below land surface", "DBLL" },
   { 112, 2, 2, "layer between two depths below land surface", "DBLY" },
   { 113, 0, 1, "isentropic (theta) level", "THEL" },
   { 114, 0, 2, "layer between two isentropic levels", "THEY" },
   { 115, 0, 1, "level at specified pressure difference from ground to level", "SPDL" },
   { 116, 0, 2, "layer between two levels at specified pressure difference from ground to level", "SPDY" },
   { 117, 0, 1, "potential vorticity(pv) surface", "PVL" },
   { 119, 0, 1, "NAM level", "NAML" },
   { 120, 0, 2, "layer between two NAM levels", "NAMY" },
   { 121, 0, 2, "layer between two isobaric surfaces (high precision)", "IBYH" },
   { 125, 2, 1, "specified height level above ground (high precision)", "HGLH" },
   { 126, 3, 1, "isobaric level", "ISBP" },
   { 128, 0, 2, "layer between two sigma levels (high precision)", "SGYH" },
   { 141, 0, 2, "layer between two isobaric surfaces (mixed precision)", "IBYM" },
   { 160, 2, 1, "depth below sea level", "DBSL" },
   { 200, 0, 0, "entire atmosphere (considered as a single layer)", "EATM" },
   { 201, 0, 0, "entire ocean (considered as a single layer)", "EOCN" },
   { 204, 0, 0, "Highest tropospheric freezing level", "HTFL" },
   { 206, 0, 0, "Grid scale cloud bottom level", "GCBL" },
   { 207, 0, 0, "Grid scale cloud top level", "GCTL" },
   { 209, 0, 0, "Boundary layer cloud bottom level", "BCBL" },
   { 210, 0, 0, "Boundary layer cloud top level", "BCTL" },
   { 211, 0, 0, "Boundary layer cloud layer", "BCY" },
   { 212, 0, 0, "Low cloud bottom level", "LCBL" },
   { 213, 0, 0, "Low cloud top level", "LCTL" },
   { 214, 0, 0, "Low cloud layer", "LCY" },
   { 215, 0, 0, "Cloud ceiling", "CEIL" },
   { 216, 0, 0, "Cumulonimbus Base (m)", "CBB" },
   { 217, 0, 0, "Cumulonimbus Top (m)", "CBT" },
   { 220, 0, 0, "Planetary Boundary Layer (derived from Richardson number)", "PBLRI" },
   { 222, 0, 0, "Middle cloud bottom level", "MCBL" },
   { 223, 0, 0, "Middle cloud top level", "MCTL" },
   { 224, 0, 0, "Middle cloud layer", "MCY" },
   { 232, 0, 0, "High cloud bottom level", "HCBL" },
   { 233, 0, 0, "High cloud top level", "HCTL" },
   { 234, 0, 0, "High cloud layer", "HCY" },
   { 235, 0, 0, "Ocean Isotherm Level (1/10 deg C)", "OITL" },
   { 236, 0, 2, "Layer between two depths below ocean surface", "OLYR" },
   { 237, 0, 0, "Bottom of Ocean Mixed Layer (m)", "OBML" },
   { 238, 0, 0, "Bottom of Ocean Isothermal Layer (m)", "OBIL" },
   { 239, 0, 0, "Layer Ocean Surface and 26C Ocean Isothermal Level", "S26CY" },
   { 240, 0, 0, "Ocean Mixed Layer", "OMXL" },
   { 241, 0, 0, "Ordered Sequence of Data", "OSEQD" },
   { 242, 0, 0, "Convective cloud bottom level", "CCBL" },
   { 243, 0, 0, "Convective cloud top level", "CCTL" },
   { 244, 0, 0, "Convective cloud layer", "CCY" },
   { 245, 0, 0, "Lowest level of the wet bulb zero", "LLTW" },
   { 246, 0, 0, "Maximum equivalent potential temperature level", "MTHE" },
   { 247, 0, 0, "Equilibrium level", "EHLT" },
   { 248, 0, 0, "Shallow convective cloud bottom level", "SCBL" },
   { 249, 0, 0, "Shallow convective cloud top level", "SCTL" },
   { 251, 0, 0, "Deep convective cloud bottom level", "DCBL" },
   { 252, 0, 0, "Deep convective cloud top level", "DCTL" },
   { 253, 0, 0, "Lowest bottom level of supercooled liquid water layer", "LBLSW" },
   { 254, 0, 0, "Highest top level of supercooled liquid water layer", "HTLSW" },
   { 255, 0, 0, "Missing", "NA" }
};
static const int n_grib_level_list = sizeof(grib_level_list)/sizeof(GribLevelData);

////////////////////////////////////////////////////////////////////////

void get_grib_code_name(int, int, char *);
void get_grib_code_unit(int, int, char *);
void get_grib_code_abbr(int, int, char *);

void get_grib_level_name(int, char *);
void get_grib_level_abbr(int, char *);
void get_grib_level_str(int, unsigned char *, char *);

int str_to_grib_code(const char *);
int str_to_grib_code(const char *, int);

int str_to_grib_code(const char *, int &, double &, double &);
int str_to_grib_code(const char *, int &, double &, double &, int);

int str_to_prob_info(const char *, double &, double &, int);


////////////////////////////////////////////////////////////////////////

#endif   //  GRIB_STRINGS_H

////////////////////////////////////////////////////////////////////////

