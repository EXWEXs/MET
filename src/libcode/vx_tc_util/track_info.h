// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __VX_TRACK_INFO_H__
#define  __VX_TRACK_INFO_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "vx_cal.h"
#include "vx_math.h"
#include "vx_util.h"

#include "track_line.h"
#include "track_point.h"

////////////////////////////////////////////////////////////////////////

static const int TrackInfoAllocInc      = 100;
static const int TrackInfoArrayAllocInc = 10;
static const int MaxBestTrackTimeInc    = 24 * sec_per_hour;

////////////////////////////////////////////////////////////////////////
//
// TrackInfo class stores multiple ATCF lines that are grouped together
// into a tropical cyclone track.
//
////////////////////////////////////////////////////////////////////////

class TrackInfo {

   private:

      void init_from_scratch();
      void assign(const TrackInfo &);
      void extend(int);
      
      bool         IsSet;

      // Storm and model identification
      ConcatString Basin;
      ConcatString Cyclone;
      ConcatString StormName;
      int          TechniqueNumber;
      ConcatString Technique;

      // Timing information
      unixtime       InitTime;
      unixtime       MinValidTime;
      unixtime       MaxValidTime;

      // TrackPoints                    
      TrackPoint    *Point;
      int            NPoints;
      int            NAlloc;

   public:

      TrackInfo();
     ~TrackInfo();
      TrackInfo(const TrackInfo &);
      TrackInfo & operator=(const TrackInfo &);

      void clear();
      void clear_points();

      void         dump(ostream &, int = 0)  const;
      ConcatString serialize()               const;
      ConcatString serialize_r(int, int = 0) const;

         //
         //  set stuff
         //

      void initialize(const TrackLine &);

      void set_basin(const ConcatString &);
      void set_cyclone(const ConcatString &);
      void set_storm_name(const ConcatString &);
      void set_technique_number(int);
      void set_technique(const ConcatString &);
      void set_init(const unixtime);
      
         //
         //  get stuff
         //

      int lead_index(int)       const;
      int valid_index(unixtime) const;

      const TrackPoint & operator[](int)    const;
      ConcatString       basin()            const;
      ConcatString       cyclone()          const;
      ConcatString       storm_name()       const;
      int                technique_number() const;
      ConcatString       technique()        const;
      unixtime           init()             const;
      int                init_hh()          const;
      unixtime           valid_min()        const;
      unixtime           valid_max()        const;
      int                valid_inc()        const;
      int                n_points()         const;

         //
         //  do stuff
         //

      void add(const TrackPoint &);
      bool add(const TrackLine &);
      
      bool has(const TrackLine &) const;
      
      bool is_match(const TrackLine &) const;
      bool is_match(const TrackInfo &) const;
      
      bool is_interp() const;
      bool is_6hour() const;

      void merge_points(const TrackInfo &);

};

////////////////////////////////////////////////////////////////////////

inline void TrackInfo::set_basin(const ConcatString &s)      { Basin = s;           };
inline void TrackInfo::set_cyclone(const ConcatString &s)    { Cyclone = s;         };
inline void TrackInfo::set_storm_name(const ConcatString &s) { StormName = s;       };
inline void TrackInfo::set_technique_number(int i)           { TechniqueNumber = i; };
inline void TrackInfo::set_technique(const ConcatString &s)  { Technique = s;       };
inline void TrackInfo::set_init(const unixtime u)            { InitTime = u;        };

inline ConcatString TrackInfo::basin()            const { return(Basin);                   }
inline ConcatString TrackInfo::cyclone()          const { return(Cyclone);                 }
inline ConcatString TrackInfo::storm_name()       const { return(StormName);               }
inline int          TrackInfo::technique_number() const { return(TechniqueNumber);         }
inline ConcatString TrackInfo::technique()        const { return(Technique);               }
inline unixtime     TrackInfo::init()             const { return(InitTime);                }
inline int          TrackInfo::init_hh()          const { return(InitTime % sec_per_hour); }
inline unixtime     TrackInfo::valid_min()        const { return(MinValidTime);            }
inline unixtime     TrackInfo::valid_max()        const { return(MaxValidTime);            }
inline int          TrackInfo::n_points()         const { return(NPoints);                 }

////////////////////////////////////////////////////////////////////////
//
// TrackInfoArray class stores an array of TrackInfo objects.
//
////////////////////////////////////////////////////////////////////////

class TrackInfoArray {

   friend TrackInfo consensus(const TrackInfoArray &, const ConcatString &, int);

   private:

      void init_from_scratch();
      void assign(const TrackInfoArray &);
      void extend(int);

      TrackInfo     *Track;
      int            NTracks;
      int            NAlloc;

   public:

      TrackInfoArray();
     ~TrackInfoArray();
      TrackInfoArray(const TrackInfoArray &);
      TrackInfoArray & operator=(const TrackInfoArray &);

      void clear();

      void         dump(ostream &, int = 0) const;
      ConcatString serialize()              const;
      ConcatString serialize_r(int = 0)     const;

         //
         //  set stuff
         //

         //
         //  get stuff
         //

      const TrackInfo & operator[](int) const;
      int n_tracks() const;

         //
         //  do stuff
         //

      void add(const TrackInfo &);
      void set(int, const TrackInfo &);
      bool add(const TrackLine &, bool = true);
      bool has(const TrackLine &) const;

};

////////////////////////////////////////////////////////////////////////

inline int TrackInfoArray::n_tracks() const { return(NTracks); }

////////////////////////////////////////////////////////////////////////

extern TrackInfo consensus(const TrackInfoArray &, const ConcatString &, int);

////////////////////////////////////////////////////////////////////////

#endif   /*  __VX_TRACK_INFO_H__  */

////////////////////////////////////////////////////////////////////////
