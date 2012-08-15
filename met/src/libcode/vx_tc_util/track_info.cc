// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "math_constants.h"

#include "track_info.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class TrackInfo
//
////////////////////////////////////////////////////////////////////////

TrackInfo::TrackInfo() {
  
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

TrackInfo::~TrackInfo() {
  
   clear();
}

////////////////////////////////////////////////////////////////////////

TrackInfo::TrackInfo(const TrackInfo & t) {

   init_from_scratch();

   assign(t);
}

////////////////////////////////////////////////////////////////////////

TrackInfo & TrackInfo::operator=(const TrackInfo & t) {

   if(this == &t) return(*this);

   assign(t);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void TrackInfo::init_from_scratch() {

   Point = (TrackPoint *) 0;
  
   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackInfo::clear() {

   IsSet           = false;

   StormId.clear();
   Basin.clear();
   Cyclone.clear();
   StormName.clear();
   TechniqueNumber = bad_data_int;
   Technique.clear();
   InitTime        = (unixtime) 0;
   MinValidTime    = (unixtime) 0;
   MaxValidTime    = (unixtime) 0;

   clear_points();

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackInfo::clear_points() {

   if(Point) { delete [] Point; Point = (TrackPoint *) 0; }
   NPoints = NAlloc = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackInfo::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);
   int i;

   out << prefix << "StormId         = \"" << (StormId ? StormId.text() : "(nul)") << "\"\n";
   out << prefix << "Basin           = \"" << (Basin ? Basin.text() : "(nul)") << "\"\n";
   out << prefix << "Cyclone         = \"" << (Cyclone ? Cyclone.text() : "(nul)") << "\"\n";
   out << prefix << "StormName       = \"" << (StormName ? StormName.text() : "(nul)") << "\"\n";
   out << prefix << "TechniqueNumber = " << TechniqueNumber << "\n";
   out << prefix << "Technique       = \"" << (Technique ? Technique.text() : "(nul)") << "\"\n";
   out << prefix << "InitTime        = " << unix_to_yyyymmdd_hhmmss(InitTime) << "\n";
   out << prefix << "MinValidTime    = " << unix_to_yyyymmdd_hhmmss(MinValidTime) << "\n";
   out << prefix << "MaxValidTime    = " << unix_to_yyyymmdd_hhmmss(MaxValidTime) << "\n";
   out << prefix << "NPoints         = " << NPoints << "\n";
   out << prefix << "NAlloc          = " << NAlloc << "\n";
      
   for(i=0; i<NPoints; i++) {
      out << prefix << "TrackPoint[" << i+1 << "]:" << "\n";
      Point[i].dump(out, indent_depth+1);
   }

   out << flush;

   return;

}

////////////////////////////////////////////////////////////////////////

ConcatString TrackInfo::serialize() const {
   ConcatString s;

   s << "TrackInfo: "
     << "StormId = \"" << (StormId ? StormId.text() : "(nul)") << "\""
     << ", Basin = \"" << (Basin ? Basin.text() : "(nul)") << "\""
     << ", Cyclone = \"" << (Cyclone ? Cyclone.text() : "(nul)") << "\""
     << ", StormName = \"" << (StormName ? StormName.text() : "(nul)") << "\""
     << ", TechniqueNumber = " << TechniqueNumber
     << ", Technique = \"" << (Technique ? Technique.text() : "(nul)") << "\""
     << ", InitTime = " << unix_to_yyyymmdd_hhmmss(InitTime)
     << ", MinValidTime = " << unix_to_yyyymmdd_hhmmss(MinValidTime)
     << ", MaxValidTime = " << unix_to_yyyymmdd_hhmmss(MaxValidTime)
     << ", NPoints = " << NPoints
     << ", NAlloc = " << NAlloc;

   return(s);

}

////////////////////////////////////////////////////////////////////////

ConcatString TrackInfo::serialize_r(int n, int indent_depth) const {
   Indent prefix(indent_depth);
   ConcatString s;
   int i;

   s << prefix << "[" << n << "] " << serialize() << ", Points:\n";

   for(i=0; i<NPoints; i++)
      s << Point[i].serialize_r(i+1, indent_depth+1);

   return(s);

}

////////////////////////////////////////////////////////////////////////

void TrackInfo::assign(const TrackInfo &t) {
   int i;

   clear();

   IsSet           = true;

   StormId         = t.StormId;
   Basin           = t.Basin;
   Cyclone         = t.Cyclone;
   StormName       = t.StormName;
   TechniqueNumber = t.TechniqueNumber;
   Technique       = t.Technique;
   InitTime        = t.InitTime;
   MinValidTime    = t.MinValidTime;
   MaxValidTime    = t.MaxValidTime;

   if(t.NPoints == 0) return;

   extend(t.NPoints);
   
   for(i=0; i<t.NPoints; i++) Point[i] = t.Point[i];

   NPoints = t.NPoints;

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackInfo::extend(int n) {
   int j, k;
   TrackPoint *new_line = (TrackPoint *) 0;

   // Check if enough memory is already allocated
   if(NAlloc >= n) return;

   // Check how many allocations are required  
   k = n/TrackInfoAllocInc;
   if(n%TrackInfoAllocInc) k++;
   n = k*TrackInfoAllocInc;

   // Allocate a new TrackPoint array of the required length
   new_line = new TrackPoint [n];

   if(!new_line) {
      mlog << Error
           << "\nvoid TrackInfo::extend(int) -> "
           << "memory allocation error\n\n";
      exit(1);
   }

   // Copy the array contents and delete the old one
   if(Point) {
      for(j=0; j<NPoints; j++) new_line[j] = Point[j];
      delete [] Point;  Point = (TrackPoint *) 0;
   }

   // Point to the new array
   Point     = new_line;
   new_line = (TrackPoint *) 0;

   // Store the allocated length
   NAlloc = n;

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackInfo::initialize(const ATCFLine &l) {

   IsSet           = true;

   Basin           = l.basin();
   Cyclone         = l.cyclone_number();
   StormName       = l.storm_name();
   TechniqueNumber = l.technique_number();
   Technique       = l.technique();

   // For BEST tracks, keep InitTime unset
   if(Technique) {
      if(strcasecmp(Technique, BestTrackStr) == 0) InitTime = (unixtime) 0;
      else                                         set_init(l.warning_time());
   }

   // Set the valid time range
   MinValidTime = MaxValidTime = l.valid();
   
   return;
}

////////////////////////////////////////////////////////////////////////

void TrackInfo::set_point(int n, const TrackPoint &p) {

   // Check range
   if((n < 0) || (n >= NPoints)) {
      mlog << Error
           << "\nTrackInfo::set_point(int, const TrackPoint &) -> "
           << "range check error for index value " << n << "\n\n";
      exit(1);
   }

   Point[n] = p;

   return;
}

////////////////////////////////////////////////////////////////////////

int TrackInfo::lead_index(int l) const {
   int i;

   // Loop through the TrackPoints looking for a matching lead time
   for(i=0; i<NPoints; i++) {
      if(Point[i].lead() == l) break;
   }

   if(i == NPoints) i = -1;

   return(i);
}

////////////////////////////////////////////////////////////////////////

int TrackInfo::valid_index(unixtime u) const {
   int i;

   // Loop through the TrackPoints looking for a matching valid time
   for(i=0; i<NPoints; i++) {
      if(Point[i].valid() == u) break;
   }

   if(i == NPoints) i = -1;

   return(i);
}

////////////////////////////////////////////////////////////////////////

const TrackPoint & TrackInfo::operator[](int n) const {

   // Check range
   if((n < 0) || (n >= NPoints)) {
      mlog << Error
           << "\nTrackInfo::operator[](int) -> "
           << "range check error for index value " << n << "\n\n";
      exit(1);
   }

   return(Point[n]);
}

////////////////////////////////////////////////////////////////////////

const ConcatString & TrackInfo::storm_id() {
   int year, mon, day, hr, minute, sec;
   unixtime ut;

   // If already set, return it's value
   if(!StormId.empty()) return(StormId);

   // Use timing information to determine the year.
        if(InitTime > 0)     ut = InitTime;
   else if(MinValidTime > 0) ut = MinValidTime;
   else                      ut = MaxValidTime;

   // Ensure that the StormId components are valid
   if(!Basin.empty() && !Cyclone.empty() && ut > 0) {

      unix_to_mdyhms(ut, mon, day, year, hr, minute, sec);

      // Set StormId
      StormId << Basin << Cyclone << year;
   }

   return(StormId);
}

////////////////////////////////////////////////////////////////////////

int TrackInfo::valid_inc() const {
   int i;
   NumArray ut_inc;

   // Compute list of time spacing between TrackPoints
   for(i=1; i<NPoints; i++)
      ut_inc.add((int) (Point[i].valid() - Point[i-1].valid()));

   // Return the most common spacing
   return(nint(ut_inc.mode()));
}

////////////////////////////////////////////////////////////////////////

void TrackInfo::add(const TrackPoint &p) {

   extend(NPoints + 1);
   Point[NPoints++] = p;

   // Check the valid time range
   if(MinValidTime == (unixtime) 0 || p.valid() < MinValidTime)
      MinValidTime = p.valid();
   if(MaxValidTime == (unixtime) 0 || p.valid() > MaxValidTime)
      MaxValidTime = p.valid();

   return;
}

////////////////////////////////////////////////////////////////////////

bool TrackInfo::add(const ATCFLine &l) {
   bool found = false;
   bool status = false;
   int i;

   // Initialize TrackInfo with ATCFLine, if necessary
   if(!IsSet) initialize(l);

   // Check if TrackPoint doesn't match this TrackInfo
   if(!is_match(l)) return(false);
   
   // Check if the storm name needs to be set or has changed
   if(!StormName) StormName = l.storm_name();
   else if(StormName && l.storm_name() &&
           StormName != l.storm_name()) {
      mlog << Warning
           << "\nTrackInfo::add(const ATCFLine &) -> "
           << "the storm name should remain the same \"" << StormName
           << "\" != \"" << l.storm_name() << "\".\n\n";
   }

   // Check that the TrackPoint valid time is increasing
   if(NPoints > 0) {
      if(l.valid() < Point[NPoints-1].valid()) {
         mlog << Warning
              << "\nTrackInfo::add(const ATCFLine &) -> "
              << "skipping ATCFLine since the valid time is not increasing ("
              << unix_to_yyyymmdd_hhmmss(l.valid()) << " < "
              << unix_to_yyyymmdd_hhmmss(Point[NPoints-1].valid())
              << "):\n" << l.line() << "\n\n";
         return(false);
      }
   }

   // Add ATCFLine to an existing TrackPoint if possible
   for(i=NPoints-1; i>=0; i--) {
      if(Point[i].is_match(l)) {
         found = true;
         status = Point[i].set(l);
         break;
      }
   }

   // Otherwise, create a new point
   if(!found) {
      extend(NPoints + 1);
      status = Point[NPoints++].set(l);
   }

   // Check the valid time range
   if(MinValidTime == (unixtime) 0 || l.valid() < MinValidTime)
      MinValidTime = l.valid();
   if(MaxValidTime == (unixtime) 0 || l.valid() > MaxValidTime)
      MaxValidTime = l.valid();

   return(status);
}

////////////////////////////////////////////////////////////////////////

void TrackInfo::add_watch_warn(const ConcatString &ww_sid,
                               WatchWarnType ww_type, unixtime ww_ut) {
   int i;

   // Check for a matching storm id
   if(storm_id() != ww_sid) return;

   // Loop over the TrackPoints
   for(i=0; i<NPoints; i++) Point[i].set_watch_warn(ww_type, ww_ut);

   return;
}

////////////////////////////////////////////////////////////////////////

bool TrackInfo::has(const ATCFLine &l) const {
   int found = false;
   int i;

   // Check if the TrackPoint data matches
   for(i=NPoints-1; i>=0; i--) {
      if(Point[i].has(l)) {
         found = true;
         break;
      }
   }

   // Return whether the TrackPoint matches and the TrackInfo matches
   return(found && is_match(l));
}

////////////////////////////////////////////////////////////////////////

bool TrackInfo::is_match(const ATCFLine &l) const {
   bool match = true;
   int diff;

   // Make sure technique is defined
   if(!Technique) return(false);
   
   // Apply matching logic for BEST tracks
   if(strcasecmp(Technique, BestTrackStr) == 0) {

      // Check basin, cyclone, and technique
      // No need to check storm name
      if(Basin     != l.basin()          ||
         Cyclone   != l.cyclone_number() ||
         Technique != l.technique())
         match = false;

      // Subsequent track point times cannot differ by too much
      if(NPoints > 0) {
         diff = (int) (l.warning_time() - Point[NPoints-1].valid());
         if(abs(diff) >
            MaxBestTrackTimeInc)
            match = false;
      }
   }
   
   // Apply matching logic for non-BEST tracks
   else {

      // Check basin, cyclone, technique number, technique
      // and check that the init time remains constant
      if(Basin           != l.basin()            ||
         Cyclone         != l.cyclone_number()   ||
         TechniqueNumber != l.technique_number() ||
         Technique       != l.technique()        ||
         InitTime        != l.warning_time())
      match = false;
   }
   
   return(match);
}

////////////////////////////////////////////////////////////////////////

bool TrackInfo::is_match(const TrackInfo &t) const {
   bool match = true;

   // Check if track is for the same basin and storm with overlapping
   // valid times.
   if(Basin        != t.basin()     ||
      Cyclone      != t.cyclone()   ||
      MinValidTime  > t.valid_max() ||
      MaxValidTime  < t.valid_min())
      match = false;

   // Check that technique is defined
   if(!Technique || !t.technique()) return(false);
   
   // If neither is a BEST track, check that the init times matches
   if(strcasecmp(Technique,     BestTrackStr) != 0 &&
      strcasecmp(t.technique(), BestTrackStr) != 0 &&
      InitTime != t.init())
      match = false;

   return(match);
}

////////////////////////////////////////////////////////////////////////

bool TrackInfo::is_interp() const {
   const char *s = Technique;
   
   s += (strlen(Technique) - 1);

   // Return true if the last character of the model name is 'I'
   return(*s == 'I');
}

////////////////////////////////////////////////////////////////////////

bool TrackInfo::is_6hour() const {
   const char *s = Technique;

   s += (strlen(Technique) - 1);

   // Return true if the last character of the model name is '2'
   return(*s == '2');
}

////////////////////////////////////////////////////////////////////////

void TrackInfo::merge_points(const TrackInfo &t) {
   TrackInfo new_t;
   int i, j;
   
   // Initialize to the current TrackInfo
   new_t = *this;

   // Delete the TrackPoints
   new_t.clear_points();

   // Loop through the TrackPoints,
   // assuming they are in chronological order
   i = j = 0;
   while(i<NPoints || j<t.n_points()) {

      // Check range
      if(i >= NPoints)      { new_t.add(t[j]);     j++; continue; }
      if(j >= t.n_points()) { new_t.add(Point[i]); i++; continue; }
      
      // Add the TrackPoint, who's valid time is less
           if(Point[i].valid() <= t[j].valid()) { new_t.add(Point[i]); i++; }
      else if(Point[i].valid() >  t[j].valid())  { new_t.add(t[j]);    j++; }

   } // end while

   *this = new_t;

   return;
}

////////////////////////////////////////////////////////////////////////
//
//  Code for class TrackInfoArray
//
////////////////////////////////////////////////////////////////////////

TrackInfoArray::TrackInfoArray() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

TrackInfoArray::~TrackInfoArray() {

   clear();
}

////////////////////////////////////////////////////////////////////////

TrackInfoArray::TrackInfoArray(const TrackInfoArray & t) {

   init_from_scratch();

   assign(t);
}

////////////////////////////////////////////////////////////////////////

TrackInfoArray & TrackInfoArray::operator=(const TrackInfoArray & t) {

   if(this == &t)  return(*this);

   assign(t);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void TrackInfoArray::init_from_scratch() {

   Track = (TrackInfo *) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackInfoArray::clear() {

   if(Track) { delete [] Track; Track = (TrackInfo *) 0; }
   NTracks = NAlloc  = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackInfoArray::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);
   int i;

   out << prefix << "NTracks = " << NTracks << "\n";
   out << prefix << "NAlloc  = " << NAlloc << "\n";

   for(i=0; i<NTracks; i++) {
      out << prefix << "TrackInfo[" << i+1 << "]:" << "\n";
      Track[i].dump(out, indent_depth+1);
   }

   out << flush;

   return;

}

////////////////////////////////////////////////////////////////////////

ConcatString TrackInfoArray::serialize() const {
   ConcatString s;

   s << "TrackInfoArray: "
     << "NTracks = " << NTracks
     << ", NAlloc = " << NAlloc;

   return(s);

}

////////////////////////////////////////////////////////////////////////

ConcatString TrackInfoArray::serialize_r(int indent_depth) const {
   Indent prefix(indent_depth);
   ConcatString s;
   int i;

   s << prefix << serialize() << ", Tracks:\n";

   for(i=0; i<NTracks; i++)
      s << Track[i].serialize_r(i+1, indent_depth+1);

   return(s);

}

////////////////////////////////////////////////////////////////////////

void TrackInfoArray::assign(const TrackInfoArray &t) {
   int i;

   clear();

   if(t.NTracks == 0) return;

   extend(t.NTracks);

   for(i=0; i<t.NTracks; i++) Track[i] = t.Track[i];

   NTracks = t.NTracks;

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackInfoArray::extend(int n) {
   int j, k;
   TrackInfo *new_info = (TrackInfo *) 0;

   // Check if enough memory is already allocated
   if(NAlloc >= n) return;

   // Check how many allocations are required
   k = n/TrackInfoArrayAllocInc;
   if(n%TrackInfoArrayAllocInc) k++;
   n = k*TrackInfoArrayAllocInc;

   // Allocate a new TrackInfo array of the required length
   new_info = new TrackInfo [n];

   if(!new_info) {
      mlog << Error
           << "\nvoid TrackInfoArray::extend(int) -> "
           << "memory allocation error\n\n";
      exit(1);
   }

   // Copy the array contents and delete the old one
   if(Track) {
      for(j=0; j<NTracks; j++) new_info[j] = Track[j];
      delete [] Track;  Track = (TrackInfo *) 0;
   }

   // Point to the new array
   Track     = new_info;
   new_info = (TrackInfo *) 0;

   // Store the allocated length
   NAlloc = n;

   return;
}

////////////////////////////////////////////////////////////////////////

const TrackInfo & TrackInfoArray::operator[](int n) const {

   // Check range
   if((n < 0) || (n >= NTracks)) {
      mlog << Error
           << "\nTrackInfoArray::operator[](int) -> "
           << "range check error for index value " << n << "\n\n";
      exit(1);
   }

   return(Track[n]);
}

////////////////////////////////////////////////////////////////////////

void TrackInfoArray::add(const TrackInfo &t) {

   extend(NTracks + 1);
   Track[NTracks++] = t;

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackInfoArray::set(int n, const TrackInfo &t) {

   // Check range
   if((n < 0) || (n >= NTracks)) {
      mlog << Error
           << "\nTrackInfoArray::set(int, const TrackInfo &) -> "
           << "range check error for index value " << n << "\n\n";
      exit(1);
   }

   Track[n] = t;

   return;
}

////////////////////////////////////////////////////////////////////////

bool TrackInfoArray::add(const ATCFLine &l, bool check_has) {
   bool found  = false;
   bool status = false;
   int i;

   // Check if this ATCFLine already exists in the TrackInfoArray
   if(check_has) {
      if(has(l)) {
         mlog << Warning
              << "\nTrackInfoArray::add(const ATCFLine &) -> "
              << "skipping duplicate ATCFLine:\n"
              << l.line() << "\n\n";
         return(false);
      }
   }

   // Add ATCFLine to an existing track if possible
   for(i=NTracks-1; i>=0; i--) {
      if(Track[i].is_match(l)) {
         found = true;
         status = Track[i].add(l);
         break;
      }
   }

   // Otherwise, create a new track
   if(!found) {
      extend(NTracks + 1);
      status = Track[NTracks++].add(l);
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

bool TrackInfoArray::has(const ATCFLine &l) const {
   int found = false;
   int i;

   // Check if the TrackInfo data matches
   for(i=NTracks-1; i>=0; i--) {
      if(Track[i].has(l)) {
         found = true;
         break;
      }
   }

   // Return whether the TrackInfo matches
   return(found);
}

////////////////////////////////////////////////////////////////////////
//
// Code for misc functions
//
////////////////////////////////////////////////////////////////////////

TrackInfo consensus(const TrackInfoArray &tracks,
                    const ConcatString &model, int req) {
   int i, j, k, i_pnt;
   TrackInfo tavg;
   NumArray lead_list;

   // Variables for average TrackPoint
   int        pcnt;
   TrackPoint pavg, psum;
   QuadInfo   wavg;

   // Check for at least one track
   if(tracks.n_tracks() == 0) {
      mlog << Error
           << "\nTrackInfoArray::consensus() -> "
           << "cannot compute a consensus for zero tracks!\n\n";
      exit(1);
   }

   // Initialize average track to the first track
   tavg.set_basin(tracks[0].basin());
   tavg.set_cyclone(tracks[0].cyclone());
   tavg.set_storm_name(tracks[0].storm_name());
   tavg.set_technique_number(tracks[0].technique_number());
   tavg.set_technique(model);
   tavg.set_init(tracks[0].init());

   // Loop through the tracks and build a list of lead times
   for(i=0; i<tracks.n_tracks(); i++) {

      // Error out if these elements change
      if(tavg.basin()   != tracks[i].basin()   ||
         tavg.cyclone() != tracks[i].cyclone() ||
         tavg.init()    != tracks[i].init()) {
         mlog << Error
              << "\nTrackInfoArray::consensus() -> "
              << "the basin, cyclone number, and init time must "
              << "remain constant.\n\n";
         exit(1);
      }

      // Warning if the the technique number changes
      if(tavg.technique_number() != tracks[i].technique_number()) {
         mlog << Warning
              << "\nTrackInfoArray::consensus() -> "
              << "the technique number has changed ("
              << tavg.technique_number() << "!="
              << tracks[i].technique_number() << ").\n\n";
      }

      // Loop through the points for the lead times
      for(j=0; j<tracks[i].n_points(); j++) {

         // Add the lead time to the list
         if(!is_bad_data(tracks[i][j].lead())) {
            if(!lead_list.has(tracks[i][j].lead()))
               lead_list.add(tracks[i][j].lead());
         }
      } // end for j

      // Sort the lead times
      lead_list.sort_array();
   }

   // Loop through the lead times and construct a TrackPoint for each
   for(i=0; i<lead_list.n_elements(); i++) {

      // Initialize TrackPoint
      pavg.clear();
      psum.clear();
      pcnt = 0;

      // Loop through the tracks and get an average TrackPoint
      for(j=0; j<tracks.n_tracks(); j++) {

         // Get the index of the TrackPoint for this lead time
         i_pnt = tracks.Track[j].lead_index(lead_list[i]);
         if(i_pnt < 0) continue;

         // Keep track of the TrackPoint count and sums
         pcnt++;
         psum += tracks.Track[j][i_pnt];
      }

      // Check for the minimum number of points
      if(pcnt < req) continue;

      // Compute the average point
      pavg = psum;
      pavg.set_lat(psum.lat()/pcnt);
      pavg.set_lon(psum.lon()/pcnt);
      pavg.set_v_max(nint(psum.v_max()/pcnt));
      pavg.set_mslp(nint(psum.mslp()/pcnt));

      // Compute the average winds
      for(j=0; j<NWinds; j++) {

         // Initialize the wind QuadInfo sum
         wavg = pavg[j];

         // Compute the average wind
         for(k=0; k<NQuadInfoValues; k++) wavg.set_value(k, nint(wavg[k]/pcnt));

         // Store the average wind
         pavg.set_wind(j, wavg);
      }

      // Compute consensus CycloneLevel
      pavg.set_level(wind_speed_to_cyclone_level(pavg.v_max()));

      // Add the current track point
      tavg.add(pavg);
   }

   // Return the consensus track
   return(tavg);
}

////////////////////////////////////////////////////////////////////////
//
// Determine if the basin/cyclone/init of the storm match any of the
// storm id's specified in the list.  The storm id consists of:
//   2-character basin, 2-character cyclone, 4-character year
//
// To match all storms in a season, replace cyclone with "AL".
// To match multiple years, replace year with the the last two digits
// of the beginning and ending years.
//
////////////////////////////////////////////////////////////////////////

bool has_storm_id(const StringArray &storm_id,
                  const ConcatString &basin,
                  const ConcatString &cyclone,
                  unixtime ut) {
   int i, year, year_beg, year_end;
   unixtime ut_beg, ut_end;
   bool match = false;

   // Loop over the storm id entries
   for(i=0; i<storm_id.n_elements(); i++) {

      // Check that the basin matches
      if(strncasecmp(storm_id[i], basin, 2) != 0) continue;

      // Check that the cyclone number matches
      if(strncasecmp(storm_id[i]+2, cyclone, 2) != 0 &&
         strncasecmp(storm_id[i]+2,    "AL", 2) != 0) continue;

      // Parse the year
      year = atoi(storm_id[i]+4);

      // Handle a single year
      if(year >= 1900 && year < 2100) {
         year_beg = year_end = year;
      }
      // Handle a range of years
      else {
         year_beg = year/100;
         year_end = year%100;

         // Add the appropriate century
         if(year_beg > 50) year_beg += 1900;
         else              year_beg += 2000;
         if(year_end > 50) year_end += 1900;
         else              year_end += 2000;
      }

      // Check that the ut time falls in the specified time range
      ut_beg = mdyhms_to_unix(01, 01, year_beg,   0, 0, 0);
      ut_end = mdyhms_to_unix(01, 01, year_end+1, 0, 0, 0);
      if(ut < ut_beg || ut >= ut_end) continue;

      // Otherwise, it's a match
      match = true;
      break;
   }

   return(match);
}

////////////////////////////////////////////////////////////////////////
