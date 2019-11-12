// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "series_pdf.h"

////////////////////////////////////////////////////////////////////////

void init_pdf(
    int n,
    vector<int>& pdf) {

    for(int k = 0; k < n; k++) {
        pdf.push_back(0);
    }
}

////////////////////////////////////////////////////////////////////////

void init_pdf(
    double min,
    double max,
    double delta,
    vector<int>& pdf) {

    int n = (max - min) / delta;
    for(int k = 0; k < n; k++) {
        pdf.push_back(0);
    }
}

////////////////////////////////////////////////////////////////////////

void init_joint_pdf(
    int n_A,
    int n_B,
    vector<int>& pdf) {

    for(int k = 0; k < n_A * n_B; k++) {
        pdf.push_back(0);
    }
}

////////////////////////////////////////////////////////////////////////

void update_pdf(
    double min,
    double delta,
    vector<int>& pdf,
    const DataPlane& dp,
    const MaskPlane& mp) {

    for(int i = 0; i < dp.nx(); i++) {
        for(int j = 0; j < dp.ny(); j++) {
            if(mp.s_is_on(i, j)) {
                double value = dp.get(i, j);
                int k = floor((value - min) / delta);
                if(k < 0) k = 0;
                if(k >= pdf.size()) k = pdf.size() - 1;
                pdf[k]++;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////

void print_pdf(
    double min,
    double delta,
    const vector<int>& pdf) {

    for(int k = 0; k < pdf.size(); k++) {
        double bin_min = min + k * delta;
        double bin_max = min + (k + 1) * delta;
        cout << "[" << bin_min << ", " << bin_max << "] "
             << pdf[k] << "\n";
    }
}

////////////////////////////////////////////////////////////////////////

void write_nc_pdf(
    NcFile* nc_out,
    const VarInfo& info,
    double min,
    double delta,
    const vector<int>& pdf) {

    vector<double> bin_min;
    vector<double> bin_max;
    vector<double> bin_mid;

    for(int k = 0; k < pdf.size(); k++) {
        bin_min.push_back(min + delta * k);
        bin_max.push_back(min + delta * (k + 1));
        bin_mid.push_back(min + delta * (k + 0.5));
    }

    ConcatString var_name = info.name();
    ConcatString var_min_name = var_name;
    ConcatString var_max_name = var_name;
    ConcatString var_mid_name = var_name;

    var_min_name.add("_min");
    var_max_name.add("_max");
    var_mid_name.add("_mid");

    ConcatString var_pdf_name("hist_");
    var_pdf_name.add(var_name);

    NcDim var_dim = nc_out->addDim(info.name(), pdf.size());
    NcVar var_min = nc_out->addVar(
        var_min_name, ncFloat, var_dim);
    NcVar var_max = nc_out->addVar(
        var_max_name, ncFloat, var_dim);
    NcVar var_mid = nc_out->addVar(
        var_mid_name, ncFloat, var_dim);
    NcVar var_pdf = nc_out->addVar(
        var_pdf_name, ncUint, var_dim);
    var_min.putAtt("units", info.units());
    var_max.putAtt("units", info.units());
    var_mid.putAtt("units", info.units());
    var_min.putVar(bin_min.data());
    var_max.putVar(bin_max.data());
    var_mid.putVar(bin_mid.data());
    var_pdf.putVar(pdf.data());
}

////////////////////////////////////////////////////////////////////////
