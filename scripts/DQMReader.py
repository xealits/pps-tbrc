#!/usr/bin/env python

# Simple python script for converting binary CTPPS HPTDC data files to human-readable format. 
# Based on https://github.com/forthommel/pps-tbrc/src/FileReader.cpp C++ version by L. Forthomme

import os, string, sys, posix, tokenize, array, getopt,struct
import numpy as np
import pylab as P
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt

def main(argv):


    # AcquisitionMode
    CONT_STORAGE=0
    TRIG_MATCH=1

    # DetectionMode
    PAIR=0
    OTRAILING=1
    OLEADING=2
    TRAILEAD=3

    # EventTypes - for each TDC event in the file
    TDCMeasurement = 0x0
    TDCHeader = 0x1
    TDCTrailer = 0x3
    TDCError = 0x4
    GlobalHeader = 0x8
    GlobalTrailer = 0x10
    ETTT = 0x11
    Filler = 0x18

    # Define counters and arrays for DQM plots
    occupancy = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
    toverthreshold = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
    nerrors = [0]
    ntriggers = [0]
    nchannels = 16
    verbose = 0

    # Open as a readable binary file - hardcoded for testing 
    with open('events_board0.dat', 'rb') as f:
        # First read and unpack the file header 
        byte = f.read(24)
        decode = struct.unpack("IIIBII",byte[:24])
        magic = decode[0]
        run_id = decode[1]
        spill_id = decode[2]
        num_hptdc = decode[3]
        AcquisitionMode = decode[4]
        DetectionMode = decode[5]

        if(verbose == 1):
            print "File header"
            print "\tmagic = " + str(magic) + " = " + str(hex(magic)) + " = " + str(hex(magic).split("0x")[1].decode("hex"))
            print "\trun_id = " + str(run_id)
            print "\tspill_id = " + str(spill_id)
            print "\tnum_hptdc = " + str(num_hptdc)
            print "\tAcquisitionMode = " + str(hex(AcquisitionMode))
            print "\tDetectionMode = " + str(hex(DetectionMode))

        # Lookup dictionary of time measurements: time = {Channel ID, Leading/Trailing edge}
        channeltimemeasurements={}

        # Now loop over the data
        while(f.read(1)):
            f.seek(-1,1) # Stupid python tricks - read ahead 1 byte to check for eof

            # Read and decode the "type" of this word. The type will determine what other information is present
            decode = struct.unpack("I",f.read(4))
            type = ((decode[0])>>27)&(0x1F)

            if(type == GlobalHeader):
                if(verbose == 1):
                    print "Global Header"
                # Reset lookup dictionary for each new event
                channeltimemeasurements={}

            if(type == GlobalTrailer):
                status = ((decode[0])>>24)&(0x7)
                if(verbose == 1):
                    print "\tTiming measurements for this trigger:"
                    print"\t\tChannel\tLeading\t\tTrailing\tDifference"
                channelflag=0
                while channelflag < nchannels:
                    tleading = channeltimemeasurements[channelflag,1]*25./1024.
                    ttrailing = channeltimemeasurements[channelflag,2]*25./1024.
                    tdifference = (channeltimemeasurements[channelflag,2]-channeltimemeasurements[channelflag,1])*25./1024.
                    toverthreshold[channelflag] = toverthreshold[channelflag]+tdifference
                    if(verbose == 1):
                        print "\t\t" + str(channelflag) + ":\t" + str(tleading) + ",\t" + str(ttrailing) + ",\t" + str(tdifference)
                        print "Global Trailer (status = " + str(status) +")"
                    channelflag = channelflag+1

            if(type == TDCHeader):
                if(verbose == 1):
                    print "HEY!!!!!!!!!!! TDCHeader!!!!!!!!!!"

            if(type == TDCTrailer):
                if(verbose == 1):
                    print "HEY!!!!!!!!!!! TDCTrailer!!!!!!!!!!"

            if(type == ETTT):
                GetETT = (decode[0])&(0x3FFFFFF)
                if(verbose == 1):
                    print "ETT = " + str(GetETT)
                ntriggers[0] = ntriggers[0]+1

            if(type == TDCMeasurement):
                istrailing = ((decode[0])>>26)&(0x1)
                time = (decode[0])&(0x7FFFF)
                if(DetectionMode == PAIR):
                    time = (decode[0])&(0xFFF)
                width = ((decode[0])>>12)&(0x7F)
                channelid = ((decode[0])>>21)&(0x1F)

                if(istrailing == 0):
                    channeltimemeasurements[channelid,1] = time
                if(istrailing == 1):
                    channeltimemeasurements[channelid,2] = time
                    occupancy[channelid] = occupancy[channelid]+1
                if(verbose == 1):
                    print "\tTDCMeasurement (trailing = " + str(istrailing) + ", time = " + str(time) + ", width = " + str(width) + ", (channel ID = " + str(channelid) + ")"
                
            if(type == TDCError):
                errorflag = (decode[0])&(0x7FFF)
                nerrors[0] = nerrors[0]+1
                if(verbose == 1):
                    print "Error detected: " + str(errorflag)


    ####################
    # Generate DQM plots
    ####################
    i = 0
    while i < nchannels:
        toverthreshold[i] = toverthreshold[i]/occupancy[i] # Before normalizing counts
        occupancy[i] = occupancy[i]/ntriggers[0]
        i = i + 1

    plt.subplot(2, 2, 1)
    plt.bar(range(0,1),ntriggers,width=1.0)
    plt.ylabel('Number of triggered events')
    plt.title(r'$\mathrm{Number\ of\ triggered\ events}$')
    plt.axis([0, 1, 0, max(ntriggers)*2])
    frame1 = plt.gca()
    frame1.axes.get_xaxis().set_visible(False)
    plt.grid(True)

    plt.subplot(2, 2, 2)
    plt.bar(range(0,nchannels),occupancy)
    plt.xlabel('HPTDC Channel')
    plt.ylabel('Occupancy per trigger')
    plt.title(r'$\mathrm{HPTDC\ Channel\ occupancy}$')
    plt.axis([0, nchannels, 0, 1.2])
    plt.grid(True)

    plt.subplot(2, 2, 3)
    plt.bar(range(0,nchannels),toverthreshold)
    plt.xlabel('HPTDC Channel')
    plt.ylabel('Mean time over threshold [ns]')
    plt.title(r'$\mathrm{Mean\ time\ over\ threshold\ per\ channel}$')
    plt.axis([0, nchannels, 0, max(toverthreshold)*2])
    plt.grid(True)

    plt.subplot(2, 2, 4)
    plt.bar(range(0,1),nerrors,width=1.0)
    plt.ylabel('Number of errors')
    plt.title(r'$\mathrm{Number\ of\ errors}$')
    plt.axis([0, 1, 0, 2])
    if(nerrors[0]>0):
        plt.axis([0, 1, 0, max(nerrors)*2])
    frame2 = plt.gca()
    frame2.axes.get_xaxis().set_visible(False)
    plt.grid(True)

    plt.tight_layout() # Or equivalently,  "plt.tight_layout()"                                        
#    plt.show()
    plt.savefig('testfig1.png')

if __name__ == "__main__":
    main(sys.argv[1:])
