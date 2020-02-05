################################################
#
#    Adapted from a script provided by George McCabe
#    Adapted by Randy Bullock
#
#    usage:  /path/to/python point_write_pickle.py pickle_output_filename <user_python_script>.py <args>
#
################################################

import os
import sys
import pickle

print('Python Script:\t', sys.argv[0])
print('User Command:\t',  sys.argv[2:])
print('Write Pickle:\t',  sys.argv[1])

pickle_filename = sys.argv[1];

pyembed_module_dir  = os.path.dirname(sys.argv[2])
pyembed_module_name = os.path.basename(sys.argv[2]).replace('.py','')
sys.argv = sys.argv[2:]

sys.path.append(pyembed_module_dir)
met_in = __import__(pyembed_module_name)

pickle.dump( met_in.point_data, open( pickle_filename, "wb" ) )
