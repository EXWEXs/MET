////////////////////////////////////////////////////////////////////////

#ifndef  __GRID_DIAG_CONF_INFO_H__
#define  __GRID_DIAG_CONF_INFO_H__

using namespace std;

#include <iostream>

#include "vx_config.h"
#include "vx_data2d_factory.h"
#include "vx_data2d.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

class GridDiagVxOpt {

    private:

        void init_from_scratch();

    public:

        GridDiagVxOpt();
        ~GridDiagVxOpt();

};

////////////////////////////////////////////////////////////////////////

class GridDiagConfInfo {

    private:

        void init_from_scratch();

        // Number of data fields
        int n_data;

    public:

        // GridDiag configuration object
        MetConfig Conf;

        // Config file version
        ConcatString Version;

        // Variable information
        VarInfo** data_info;

        GridDiagConfInfo();
        ~GridDiagConfInfo();

        void clear();

        void read_config(const char*, const char*);
        void process_config(GrdFileType);

        int get_n_data() const;
};

////////////////////////////////////////////////////////////////////////

inline int GridDiagConfInfo::get_n_data() const {
    return n_data;
}

////////////////////////////////////////////////////////////////////////

#endif  /*  __GRID_DIAG_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
