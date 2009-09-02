#!/bin/sh

echo
echo "*** Testing the Model Evaluation Tools Project ***"
echo

echo
echo "*** Testing Gen-Poly-Mask application ***"
echo
./test_gen_poly_mask.sh

echo
echo "*** Testing PCP-Combine application ***"
echo
./test_pcp_combine.sh

echo
echo "*** Testing MODE application ***"
echo
./test_mode.sh

echo
echo "*** Testing Grid-Stat application ***"
echo
./test_grid_stat.sh

echo
echo "*** Testing PB2NC application ***"
echo
./test_pb2nc.sh

echo
echo "*** Testing ASCII2NC application ***"
echo
./test_ascii2nc.sh

echo
echo "*** Testing Point-Stat application ***"
echo
./test_point_stat.sh

echo
echo "*** Testing Wavelet-Stat application ***"
echo
./test_wavelet_stat.sh

echo
echo "*** Testing STAT-Analysis application ***"
echo
./test_stat_analysis.sh

echo
echo "*** Testing MODE-Analysis application ***"
echo
./test_mode_analysis.sh

echo
echo "*** Finished Testing the Model Evaluation Tools Project ***"
echo
