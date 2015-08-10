from glob import glob
from os.path import isfile
from sys import argv
from subprocess import call

def main(argv):
  if len(argv)<3:
    print 'Usage:', argv[0], '<run id> <board id>'
    return
  files_in = [f for f in glob('/home/ppstb/timing_data/events_%d_*_board%d.dat' % (int(argv[1]), int(argv[2]))) if isfile(f)]

  i = 0
  for f in files_in:
    print 'Processing (%d/%d) %s...' % (i+1, len(files_in), f)
    call(['/home/ppstb/pps-tbrc/build/test/write_tree', f, f.replace('.dat', '.root')])
    i += 1
if __name__=='__main__':
  main(argv)
