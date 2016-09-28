#!/usr/bin/perl

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#
# tcst_conv.pl - a utility to convert TCST text output files
# to the STAT format consistent with the output of other MET
# tools
#
# last revised: 2016-09-27 for MET version 6.0
#
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

use warnings;
use strict;

sub usage(){
  return "usage: $0 {tcst_file}\n" .
         "  The lines of the input TCST output file are reformatted as a MET\n" .
         "  stat file.\n\n";
}

my @fld_hdrs  = qw(VERSION MODEL DESC FCST_LEAD FCST_VALID_BEG FCST_VALID_END OBS_LEAD OBS_VALID_BEG
                   OBS_VALID_END FCST_VAR FCST_LEV OBS_VAR OBS_LEV OBTYPE VX_MASK INTERP_MTHD
                   INTERP_PNTS FCST_THRESH OBS_THRESH COV_THRESH ALPHA LINE_TYPE);

my @fld_tcst  = qw(AMODEL BMODEL DESC STORM_ID BASIN CYCLONE STORM_NAME INIT_MASK VALID_MASK
                   TOTAL INDEX LEVEL WATCH_WARN INITIALS ALAT ALON BLAT BLON
                   TK_ERR X_ERR Y_ERR ALTK_ERR CRTK_ERR ADLAND BDLAND AMSLP BMSLP AMAX_WIND BMAX_WIND
                   AAL_WIND_34 BAL_WIND_34 ANE_WIND_34 BNE_WIND_34 ASE_WIND_34 BSE_WIND_34 ASW_WIND_34 BSW_WIND_34 ANW_WIND_34 BNW_WIND_34
                   AAL_WIND_50 BAL_WIND_50 ANE_WIND_50 BNE_WIND_50 ASE_WIND_50 BSE_WIND_50 ASW_WIND_50 BSW_WIND_50 ANW_WIND_50 BNW_WIND_50
                   AAL_WIND_64 BAL_WIND_64 ANE_WIND_64 BNE_WIND_64 ASE_WIND_64 BSE_WIND_64 ASW_WIND_64 BSW_WIND_64 ANW_WIND_64 BNW_WIND_64
                   ARADP BRADP ARRP BRRP AMRD BMRD AGUSTS BGUSTS AEYE BEYE ADIR BDIR ASPEED BSPEED ADEPTH BDEPTH);

my $fmt_hdr =
      "%-8s"  . # VERSION
      "%-12s" . # MODEL
      "%-12s" . # DESC
      "%-10s" . # FCST_LEAD
      "%-16s" . # FCST_VALID_BEG
      "%-16s" . # FCST_VALID_END
      "%-9s"  . # OBS_LEAD
      "%-16s" . # OBS_VALID_BEG
      "%-16s" . # OBS_VALID_END
      "%-12s" . # FCST_VAR
      "%-9s"  . # FCST_LEV
      "%-12s" . # OBS_VAR
      "%-9s"  . # OBS_LEV
      "%-10s" . # OBTYPE
      "%-8s"  . # VX_MASK
      "%-12s" . # INTERP_MTHD
      "%-12s" . # INTERP_PNTS
      "%-12s" . # FCST_THRESH
      "%-11s" . # OBS_THRESH
      "%-11s" . # COV_THRESH
      "%-6s"  . # ALPHA
      "%-10s";  # LINE_TYPE

my $fmt_tcst =
      "%15s"  . # AMODEL
      "%15s"  . # BMODEL
      "%15s"  . # DESC
      "%15s"  . # STORM_ID
      "%15s"  . # BASIN
      "%15s"  . # CYCLONE
      "%15s"  . # STORM_NAME
      "%15s"  . # INIT_MASK
      "%15s"  . # VALID_MASK
      "%15s"  . # TOTAL
      "%15s"  . # INDEX
      "%15s " . # LEVEL
      "%15s"  . # WATCH_WARN
      "%15s"  . # INITIALS
      "%15s"  . # ALAT
      "%15s"  . # ALON
      "%15s"  . # BLAT
      "%15s"  . # BLON
      "%15s"  . # TK_ERR
      "%15s"  . # X_ERR
      "%15s"  . # Y_ERR
      "%15s"  . # ALTK_ERR
      "%15s"  . # CRTK_ERR
      "%15s"  . # ADLAND
      "%15s"  . # BDLAND
      "%15s"  . # AMSLP
      "%15s"  . # BMSLP
      "%15s"  . # AMAX_WIND
      "%15s"  . # BMAX_WIND
      "%15s"  . # AAL_WIND_34
      "%15s"  . # BAL_WIND_34
      "%15s"  . # ANE_WIND_34
      "%15s"  . # BNE_WIND_34
      "%15s"  . # ASE_WIND_34
      "%15s"  . # BSE_WIND_34
      "%15s"  . # ASW_WIND_34
      "%15s"  . # BSW_WIND_34
      "%15s"  . # ANW_WIND_34
      "%15s"  . # BNW_WIND_34
      "%15s"  . # AAL_WIND_50
      "%15s"  . # BAL_WIND_50
      "%15s"  . # ANE_WIND_50
      "%15s"  . # BNE_WIND_50
      "%15s"  . # ASE_WIND_50
      "%15s"  . # BSE_WIND_50
      "%15s"  . # ASW_WIND_50
      "%15s"  . # BSW_WIND_50
      "%15s"  . # ANW_WIND_50
      "%15s"  . # BNW_WIND_50
      "%15s"  . # AAL_WIND_64
      "%15s"  . # BAL_WIND_64
      "%15s"  . # ANE_WIND_64
      "%15s"  . # BNE_WIND_64
      "%15s"  . # ASE_WIND_64
      "%15s"  . # BSE_WIND_64
      "%15s"  . # ASW_WIND_64
      "%15s"  . # BSW_WIND_64
      "%15s"  . # ANW_WIND_64
      "%15s"  . # BNW_WIND_64
      "%15s"  . # ARADP
      "%15s"  . # BRADP
      "%15s"  . # ARRP
      "%15s"  . # BRRP
      "%15s"  . # AMRD
      "%15s"  . # BMRD
      "%15s"  . # AGUSTS
      "%15s"  . # BGUSTS
      "%15s"  . # AEYE
      "%15s"  . # BEYE
      "%15s"  . # ADIR
      "%15s"  . # BDIR
      "%15s"  . # ASPEED
      "%15s"  . # BSPEED
      "%15s"  . # ADEPTH
      "%15s";   # BDEPTH


if( 1 > @ARGV && 2 < @ARGV ){ die "ERROR: unexpected number of arguments\n\n" . usage() }

my $file = pop @ARGV;

# open the input file
open(my $fh_tcst_in, "<", $file) or die "ERROR: cannot open input file $file\n\n" . usage();

# print the column headers
printf( "${fmt_hdr}${fmt_tcst}\n", (@fld_hdrs, @fld_tcst) );

# handle each input file line
while(<$fh_tcst_in>){
  chomp();

  # print out the header line
  next if( /^VERSION/ );

  # parse the data line, and build the header
  my @vals = split /\s+/;
  my @outs = (
     $vals[0],  # VERSION
     "NA",      # MODEL
     $vals[3],  # DESC
     $vals[9],  # FCST_LEAD
     $vals[10], # FCST_VALID_BEG
     $vals[10], # FCST_VALID_END
     "NA",      # OBS_LEAD
     $vals[10], # OBS_VALID_BEG
     $vals[10], # OBS_VALID_END
     "NA",      # FCST_VAR
     "NA",      # FCST_LEV
     "NA",      # OBS_VAR
     "NA",      # OBS_LEV
     "NA",      # OBTYPE
     "NA",      # VX_MASK
     "NA",      # INTERP_MTHD
     "NA",      # INTERP_PNTS
     "NA",      # FCST_THRESH
     "NA",      # OBS_THRESH
     "NA",      # COV_THRESH
     "NA",      # ALPHA
     "TCST"     # LINE_TYPE
  );

  # write the output line
  push @outs, (@vals[1 .. 7,11,12,14 .. 79]);

  # print the line
  printf("${fmt_hdr}${fmt_tcst}\n", @outs);
}

close($fh_tcst_in);

# TCST Header Line
#     0 VERSION
#     1 AMODEL
#     2 BMODEL
#     3 DESC
#     4 STORM_ID
#     5 BASIN
#     6 CYCLONE
#     7 STORM_NAME
#     8 INIT
#     9 LEAD
#    10 VALID
#    11 INIT_MASK
#    12 VALID_MASK
#    13 LINE_TYPE
#    14 TOTAL
#    15 INDEX
#    16 LEVEL
#    17 WATCH_WARN
#    18 INITIALS
#    19 ALAT
#    20 ALON
#    21 BLAT
#    22 BLON
#    23 TK_ERR
#    24 X_ERR
#    25 Y_ERR
#    26 ALTK_ERR
#    27 CRTK_ERR
#    28 ADLAND
#    29 BDLAND
#    30 AMSLP
#    31 BMSLP
#    32 AMAX_WIND
#    33 BMAX_WIND
#    34 AAL_WIND_34
#    35 BAL_WIND_34
#    36 ANE_WIND_34
#    37 BNE_WIND_34
#    38 ASE_WIND_34
#    39 BSE_WIND_34
#    40 ASW_WIND_34
#    41 BSW_WIND_34
#    42 ANW_WIND_34
#    43 BNW_WIND_34
#    44 AAL_WIND_50
#    45 BAL_WIND_50
#    46 ANE_WIND_50
#    47 BNE_WIND_50
#    48 ASE_WIND_50
#    49 BSE_WIND_50
#    50 ASW_WIND_50
#    51 BSW_WIND_50
#    52 ANW_WIND_50
#    53 BNW_WIND_50
#    54 AAL_WIND_64
#    55 BAL_WIND_64
#    56 ANE_WIND_64
#    57 BNE_WIND_64
#    58 ASE_WIND_64
#    59 BSE_WIND_64
#    60 ASW_WIND_64
#    61 BSW_WIND_64
#    62 ANW_WIND_64
#    63 BNW_WIND_64
#    64  ARADP
#    65  BRADP
#    66  ARRP
#    67  BRRP
#    68  AMRD
#    69  BMRD
#    70  AGUSTS
#    71  BGUSTS
#    72  AEYE
#    73  BEYE
#    74  ADIR
#    75  BDIR
#    76  ASPEED
#    77  BSPEED
#    78  ADEPTH
#    79  BDEPTH
