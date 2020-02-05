from __future__ import print_function

import pandas as pd
import os
import sys

###########################################

print('Python Script:\t', sys.argv[0])

   ##
   ##  input file specified on the command line
   ##  load the data into the numpy array
   ##

if len(sys.argv) == 2:
    # Read the input file as the first argument
    input_file = os.path.expandvars(sys.argv[1])
    try:
        # Print some output to verify that this script ran
        print("Input File:\t" + repr(input_file))
        point_data = pd.read_csv(input_file, header=None, delim_whitespace=True, keep_default_na=False,
                          names=['typ', 'sid', 'vld', 'lat', 'lon', 'elv', 'var', 'lvl', 'hgt', 'qc', 'obs'],
                          dtype={'typ':'str','sid':'str','var':'str','qc':'str'}).values.tolist()
        print("Data Length:\t" + repr(len(point_data)))
        print("Data Type:\t" + repr(type(point_data)))
    except NameError:
        print("Can't find the input file")
else:
    print("ERROR: read_ascii_point.py -> Must specify exactly one input file.")
    sys.exit(1)

###########################################
