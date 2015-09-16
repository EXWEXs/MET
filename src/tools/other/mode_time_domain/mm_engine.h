

////////////////////////////////////////////////////////////////////////


#ifndef  __MTD_MATCH_MERGE_ENGINE_H__
#define  __MTD_MATCH_MERGE_ENGINE_H__


////////////////////////////////////////////////////////////////////////


#include "mtd_partition.h"
#include "interest_calc.h"
#include "fo_graph.h"

#include "int_array.h"


////////////////////////////////////////////////////////////////////////


class MM_Engine {

   protected:

      void init_from_scratch();

      void assign(const MM_Engine &);

      // int N_Fcst_Composites;
      // int N_Obs_Composites;


   public:

      MM_Engine();
     ~MM_Engine();
      MM_Engine(const MM_Engine &);
      MM_Engine & operator=(const MM_Engine &);

      void clear();

      void partition_dump(ostream &) const;

      InterestCalculator calc;

      Mtd_Partition part;

      FO_Graph graph;

         //
         //  set stuff
         //

      void set_size(const int _n_fcst, const int _n_obs);

         //
         //  get stuff
         //

      int n_fcst_simples()    const;
      int n_obs_simples()     const;

      // int n_fcst_composites() const;
      // int n_obs_composites()  const;

          //
          //  return the index of the composite (ie, equivalence class) that
          //    has the given fcst or obs object number
          //

      int composite_with_fcst (const int) const;   //  zero-based (both input and output)
      int composite_with_obs  (const int) const;   //  zero-based (both input and output)

         //
         //  do stuff
         //

         //  Note: all object numbers are zero-based

      void do_match_merge();

      int n_composites() const;

      int map_fcst_id_to_composite (const int id) const;      //  zero-based
      int map_obs_id_to_composite  (const int id) const;      //  zero-based

      IntArray fcst_composite(int _composite_number) const;   //  zero-based
      IntArray  obs_composite(int _composite_number) const;   //  zero-based

};


////////////////////////////////////////////////////////////////////////


inline int MM_Engine::n_fcst_simples () const { return ( graph.n_fcst () ); }
inline int MM_Engine::n_obs_simples  () const { return ( graph.n_obs  () ); }

inline int MM_Engine::n_composites   () const { return ( part.n_elements() ); }

// inline int MM_Engine::n_fcst_composites  () const { return ( N_Fcst_Composites ); }
// inline int MM_Engine::n_obs_composites   () const { return ( N_Obs_Composites ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MTD_MATCH_MERGE_ENGINE_H__  */


////////////////////////////////////////////////////////////////////////


