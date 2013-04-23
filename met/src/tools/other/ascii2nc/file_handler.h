

   // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
   // ** Copyright UCAR (c) 1992 - 2013
   // ** University Corporation for Atmospheric Research (UCAR)
   // ** National Center for Atmospheric Research (NCAR)
   // ** Research Applications Lab (RAL)
   // ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
   // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __FILEHANDLER_H__
#define  __FILEHANDLER_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <map>
#include <time.h>
#include <vector>

#include "netcdf.hh"

#include "vx_config.h"
#include "vx_util.h"

#include "observation.h"
#include "summary_calc.h"
#include "summary_key.h"
#include "time_summary_interval.h"

////////////////////////////////////////////////////////////////////////


class FileHandler
{

public:

  FileHandler(const string &program_name);
  virtual ~FileHandler();

  virtual bool isFileType(LineDataFile &ascii_file) const = 0;
  
  bool readAsciiFiles(const vector< ConcatString > &ascii_filename_list);
  bool writeNetcdfFile(const string &nc_filename);
  
  bool summarizeObs(const TimeSummaryInfo &summary_info);
  

protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  static const long HDR_ARRAY_LEN;
  static const long OBS_ARRAY_LEN;
  static const long MAX_STRING_LEN;

  static const float FILL_VALUE;
  

  ///////////////////////
  // Protected members //
  ///////////////////////

  string _programName;
  
  // Variables for writing output NetCDF file

  NcFile *_ncFile;
  NcVar  *_hdrTypeVar;
  NcVar  *_hdrStationIdVar;
  NcVar  *_hdrValidTimeVar;
  NcVar  *_hdrArrayVar;
  NcVar  *_obsQualityVar;
  NcVar  *_obsArrayVar;

  long _nhdr;

  int _hdrNum;
  int _obsNum;

  bool _dataSummarized;
  TimeSummaryInfo _summaryInfo;
  
  // List of observations read from the ascii files

  vector< Observation > _observations;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  // Count the number of headers needed for the netCDF file.  All of the
  // observations must be loaded into the _observations vector before calling
  // this method.

  void _countHeaders();
  
  time_t _getValidTime(const string &time_string) const;
  
  // Read the observations from the given file and add them to the
  // _observations vector.

  virtual bool _readObservations(LineDataFile &ascii_file) = 0;
 
  // Write the observations in the _observations vector into the current
  // netCDF file.

  bool _writeObservations();

  // Use the configuration information to generate the list of summary
  // calculators needed.

  vector< SummaryCalc* > _getSummaryCalculators(const TimeSummaryInfo &info) const;
  
  // Use the configuration file time summary information to figure out the
  // time intervals for our summaries

  vector< TimeSummaryInterval > _getTimeIntervals(const time_t first_data_time,
						  const time_t last_data_time,
						  const TimeSummaryInfo &info) const;
  
  // Check to see if the given time is within the defined time interval

  bool _isInTimeInterval(const time_t curr_time,
			 const int begin_secs, const int end_secs) const;

  // Generate the summary header type string for the netCDF file

  string _getSummaryHeaderType(const string &header_type,
			       const string &summary_type,
			       const int summary_width_secs) const;
  
  // Convert the unix time to number of seconds since the beginning of
  // the day

  static int _unixtimeToSecs(const time_t unix_time)
  {
    struct tm *time_struct = gmtime(&unix_time);
    
    return (time_struct->tm_hour * 3600) +
      (time_struct->tm_min * 60) + time_struct->tm_sec;
  }
  
  // Convert the number of seconds from the beginning of the day to a string

  static string _secsToTimeString(const int secs)
  {
    // Get the different fields from the number of seconds

    int remaining_secs = secs;
    int hour = remaining_secs / 3600;
    remaining_secs -= hour * 3600;
    int minute = remaining_secs / 60;
    remaining_secs -= minute * 60;
    int second = remaining_secs;
  
    // Create the string

    char string_buffer[20];
    
    sprintf(string_buffer, "%02d%02d%02d", hour, minute, second);
    
    return string(string_buffer);
  }
  
  // Get the first possible interval time after 0:00Z

  static time_t _getFirstIntervalOfDay(const time_t test_time,
				       const int begin_secs, const int end_secs,
				       const int step)
  {
    struct tm *time_struct = gmtime(&test_time);

    int start_of_day_secs = 0;
    
    if (begin_secs < end_secs)
    {
      // This is the "normal" case.  We can just take the begin time and
      // start there.

      start_of_day_secs = begin_secs;
      
    }
    else
    {
      // If we get here, we have one of two cases.  If the start time and end
      // time are equal, we assume that the user wants to process the entire
      // day but base the time intervals off of the specified time.  If the
      // start time is greater than the end time, then the time period spans
      // midnight.  Either way, we want to take the specified start time and
      // the step and step back to the first time on the specified day.

      start_of_day_secs = begin_secs % step;
    }
    
    time_struct->tm_hour = start_of_day_secs / 3600;
    start_of_day_secs -= time_struct->tm_hour * 3600;
    
    time_struct->tm_min = start_of_day_secs / 60;
    start_of_day_secs -= time_struct->tm_min * 60;
      
    time_struct->tm_sec = start_of_day_secs;

    return timegm(time_struct);
  }
  
  // Get the interval time of the interval that contains the given data
  // time.

  static time_t _getIntervalTime(const time_t test_time,
				 const int begin_secs, const int end_secs,
				 const int step, const int width)
  {
    // Get the first interval time for the day

    time_t test_interval = _getFirstIntervalOfDay(test_time,
						  begin_secs, end_secs,
						  step);

    // Loop through the day until we find the first interval the includes the
    // given time or is later than the given time (because the user could define
    // the intervals so that there are unprocessed times between them).
    
    while (test_time > test_interval + ((width / 2) + 1))
    {
      test_interval += step;
    }
    
    return test_interval;
  }
  
  static time_t _getEndOfDay(const time_t unix_time)
  {
    struct tm *time_struct = gmtime(&unix_time);

    time_struct->tm_hour = 23;
    time_struct->tm_min = 59;
    time_struct->tm_sec = 59;
    
    return timegm(time_struct);
  }
  
  static string _timeToString(const time_t unix_time)
  {
    struct tm *time_struct = gmtime(&unix_time);
    
    char time_string[80];
    
    sprintf(time_string, "%04d%02d%02d_%02d%02d%02d",
	    time_struct->tm_year + 1900, time_struct->tm_mon + 1,
	    time_struct->tm_mday,
	    time_struct->tm_hour, time_struct->tm_min, time_struct->tm_sec);
    
    return time_string;
  }
  
  void _closeNetcdf();
  bool _openNetcdf(const string &nc_filename);
  bool _writeHdrInfo(const ConcatString &hdr_typ,
		     const ConcatString &hdr_sid,
		     const ConcatString &hdr_vld,
		     double lat, double lon, double elv);
  bool _writeObsInfo(int gc, float prs, float hgt, float obs,
		     const ConcatString &qty);
  

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __FILEHANDLER_H__  */


////////////////////////////////////////////////////////////////////////


