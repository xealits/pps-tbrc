#!/usr/bin/env python

# Simple python script for converting binary CTPPS HPTDC data files to human-readable format. 
# Based on https://github.com/forthommel/pps-tbrc/src/FileReader.cpp C++ version by L. Forthomme

import os, string, sys, posix, tokenize, array, getopt,struct
import numpy as np
import pylab as P
import matplotlib as mpl
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt

def main(argv):
    #########################
    # General script settings
    #########################

    inputbinaryfile = 'events_board0.dat'
    outputplotfile = 'testfig1.png'
    nchannels = 64
    ngroups = 16
    verbose = 0

    ###################                                                                                                                                            
    # HPTDC definitions                                                                                                                                           
    ###################                                                                                                                                           

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
    ntriggers = [0]
    occupancy = []
    toverthreshold = []
    k=0
    while k < nchannels:
        occupancy.append(0)
        toverthreshold.append(0)
        k=k+1

    nerrors = [0]
    grouperrors = []
    readoutfifooverflowerrors = []
    l1bufferoverflowerrors = []
    k=0
    while k < ngroups:
        grouperrors.append(0)
        readoutfifooverflowerrors.append(0)
        l1bufferoverflowerrors.append(0)
        k=k+1
    eventsizelimiterrors = [0]
    triggerfifooverflowerrors = [0]
    internalchiperrors = [0]

    # Open as a readable binary file - hardcoded for testing 
    with open(inputbinaryfile, 'rb') as f:
        #######################################
        # First read and unpack the file header
        #######################################
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
        k = 0
        while k < nchannels:
            channeltimemeasurements[k,1]=0
            channeltimemeasurements[k,2]=0
            k=k+1

        ###########################                                                                                                                               
        # Main loop to read in data
        ###########################
        while(f.read(1)):
            f.seek(-1,1) # Stupid python tricks - read ahead 1 byte to check for eof

            # Read and decode the "type" of this word. The type will determine what other information is present
            decode = struct.unpack("I",f.read(4))
            type = ((decode[0]) >> 27) & (0x1F)

            if(type == GlobalHeader):
                if(verbose == 1):
                    print "Global Header"
                # Reset lookup dictionary for each new event
                channeltimemeasurements={}
                k = 0
                while k < nchannels:
                    channeltimemeasurements[k,1]=0
                    channeltimemeasurements[k,2]=0
                    k=k+1

            # This word is a global trailer - calculate summary timing information for all channels in this event
            if(type == GlobalTrailer):
                status = ((decode[0]) >> 24) & (0x7)
                geo = (decode[0]) & (0x1F)
                if(verbose == 1):
                    print "\tTiming measurements for this trigger:"
                    print"\t\tChannel\tLeading\t\tTrailing\tDifference"
                channelflag=0
                while channelflag < nchannels:
                    tleading = channeltimemeasurements[channelflag,1] * 25./1024.
                    ttrailing = channeltimemeasurements[channelflag,2] * 25./1024.
                    tdifference = (channeltimemeasurements[channelflag,2] - channeltimemeasurements[channelflag,1]) * 25./1024.
                    toverthreshold[channelflag] = toverthreshold[channelflag] + tdifference
                    if(verbose == 1):
                        print "\t\t" + str(channelflag) + ":\t" + str(tleading) + ",\t" + str(ttrailing) + ",\t" + str(tdifference)
                    channelflag = channelflag+1
                if(verbose == 1):
                    print "Global Trailer (status = " + str(status) + ", Geo = " + str(geo) + ")"

            if(type == TDCHeader):
                tdcid = ((decode[0]) >> 24) & (0x3)
                eventid = ((decode[0]) >> 12) & (0xFFF)
                bunchid = (decode[0]) & (0xFFF)

                if(verbose == 1):
                    print "\tTDC ID = " + str(tdcid)
                    print "\tEvent ID = " + str(eventid)
                    
            if(type == TDCTrailer):
                if(verbose == 1):
                    wordcount = (decode[0]) & (0xFFF)
                    eventcount = ((decode[0]) >> 5) & (0x3FFFF)
                    if(verbose == 1):
                        print "\tWord count = " + str(wordcount)
                        print "\tEvent count = " + str(eventcount)

            if(type == ETTT):
                GetETT = (decode[0]) & (0x3FFFFFF)
                if(verbose == 1):
                    print "ETT = " + str(GetETT)
                ntriggers[0] = ntriggers[0] + 1

            # This word is an actual TDC measurement - get the timing and leading/trailing edge informations
            if(type == TDCMeasurement):
                istrailing = ((decode[0]) >> 26) & (0x1)
                time = (decode[0]) & (0x7FFFF)
                if(DetectionMode == PAIR):
                    time = (decode[0]) & (0xFFF)
                width = ((decode[0]) >> 12) & (0x7F)
                channelid = ((decode[0]) >> 21) & (0x1F)

                if(istrailing == 0):
                    channeltimemeasurements[channelid,1] = time
                if(istrailing == 1):
                    channeltimemeasurements[channelid,2] = time
                    occupancy[channelid] = occupancy[channelid]+1
                if(verbose == 1):
                    print "\tTDCMeasurement (trailing = " + str(istrailing) + ", time = " + str(time) + ", width = " + str(width) + ", (channel ID = " + str(channelid) + ")"
                
            # This word is an Error - decode it and count each type of error
            if(type == TDCError):
                errorflag = (decode[0]) & (0x7FFF)
                nerrors[0] = nerrors[0] + 1
                if(((decode[0]) >> 12) & 0x1):
                    eventsizelimiterrors[0] = eventsizelimiterrors[0] + 1
                if(((decode[0]) >> 13) & 0x1):
                    triggerfifooverflowerrors[0] = triggerfifooverflowerrors[0] + 1
                if(((decode[0]) >> 14) & 0x1):
                    internalchiperrors[0] = internalchiperrors[0] +1
                j = 0
                while j < 4:
                    if(((decode[0]) >> (2+3*j)) & 0x1):
                        grouperrors[j] = grouperrors[j] + 1
                    if(((decode[0]) >> (1+3*j)) & 0x1):
                        l1bufferoverflowerrors[j] = l1bufferoverflowerrors[j] + 1
                    if(((decode[0]) >> (3*j)) & 0x1):
                        readoutfifooverflowerrors[j] = readoutfifooverflowerrors[j] + 1
                if(verbose == 1):
                    print "Error detected: " + str(errorflag)


    ####################
    # Generate DQM plots
    ####################
    i = 0
    while i < nchannels:
        if(occupancy[i]>0):
            toverthreshold[i] = toverthreshold[i]/occupancy[i] # Before normalizing counts
        if(ntriggers[0]>0):
            occupancy[i] = occupancy[i]/ntriggers[0]
        i = i + 1

    mpl.rcParams['xtick.labelsize'] = 6
    mpl.rcParams['ytick.labelsize'] = 6

    plt.subplot(3, 3, 1)
    plt.bar(range(0,1),ntriggers,width=1.0)
    plt.ylabel('Triggered events',fontsize=6)
#    plt.title(r'$\mathrm{Number\ of\ triggered\ events}$',fontsize=6)
    plt.figtext(0.15, 0.8, "N = " + str(ntriggers[0]))
    plt.axis([0, 1, 0, max(ntriggers)*2],fontsize=6)
    frame1 = plt.gca()
    frame1.axes.get_xaxis().set_visible(False)
    plt.grid(True)

    plt.subplot(3, 3, 2)
    plt.bar(range(0,nchannels),occupancy)
    plt.xlabel('HPTDC Channel',fontsize=6)
    plt.ylabel('Occupancy',fontsize=6)
#    plt.title(r'$\mathrm{HPTDC\ Channel\ occupancy}$',fontsize=6)
    plt.axis([0, nchannels, 0, 1.2],fontsize=6)
    plt.grid(True)

    plt.subplot(3, 3, 3)
    plt.bar(range(0,nchannels),toverthreshold)
    plt.xlabel('HPTDC Channel',fontsize=6)
    plt.ylabel('Mean time over threshold [ns]',fontsize=6)
    plt.axis([0, nchannels, 0, max(toverthreshold)*2])
    plt.grid(True)

    plt.subplot(3, 3, 4)
    plt.bar(range(0,1),nerrors,width=1.0,color='r')
    plt.ylabel('Total errors',fontsize=6)
    plt.axis([0, 1, 0, 2])
    if(nerrors[0]>0):
        plt.axis([0, 1, 0, max(nerrors)*2])
    frame2 = plt.gca()
    frame2.axes.get_xaxis().set_visible(False)
    plt.grid(True)

    plt.subplot(3, 3, 5)
    plt.bar(range(0,1),eventsizelimiterrors,width=1.0,color='r')
    plt.ylabel('Event size errors',fontsize=6)
    plt.axis([0, 1, 0, 2])
    if(eventsizelimiterrors[0]>0):
        plt.axis([0, 1, 0, max(eventsizelimiterrors)*2])
    frame2 = plt.gca()
    frame2.axes.get_xaxis().set_visible(False)
    plt.grid(True)

    plt.subplot(3, 3, 6)
    plt.bar(range(0,1),triggerfifooverflowerrors,width=1.0,color='r')
    plt.ylabel('Trigger FIFO overflow errors',fontsize=6)
    plt.axis([0, 1, 0, 2])
    if(triggerfifooverflowerrors[0]>0):
        plt.axis([0, 1, 0, max(triggerfifooverflowerrors)*2])
    frame2 = plt.gca()
    frame2.axes.get_xaxis().set_visible(False)
    plt.grid(True)

    plt.subplot(3, 3, 7)
    plt.bar(range(0,1),internalchiperrors,width=1.0,color='r')
    plt.ylabel('Internal chip errors',fontsize=6)
    plt.axis([0, 1, 0, 2])
    if(internalchiperrors[0]>0):
        plt.axis([0, 1, 0, max(internalchiperrors)*2])
    frame2 = plt.gca()
    frame2.axes.get_xaxis().set_visible(False)
    plt.grid(True)

    plt.subplot(3, 3, 8)
    plt.bar(range(0,ngroups),readoutfifooverflowerrors,color='r')
    plt.ylabel('Readout FIFO overflow errors',fontsize=6)
    plt.xlabel('Group',fontsize=6)
    plt.axis([0, ngroups, 0, 2])
    if(readoutfifooverflowerrors[0]>0):
        plt.axis([0, ngroups, 0, max(readoutfifooverflowerrors)*2])
    frame2 = plt.gca()
    plt.grid(True)

    plt.subplot(3, 3, 9)
    plt.bar(range(0,ngroups),l1bufferoverflowerrors,color='r')
    plt.ylabel('L1 buffer overflow errors',fontsize=6)
    plt.xlabel('Group',fontsize=6)
    plt.axis([0, ngroups, 0, 2])
    if(l1bufferoverflowerrors[0]>0):
        plt.axis([0, ngroups, 0, max(l1bufferoverflowerrors)*2])
    frame2 = plt.gca()
    plt.grid(True)

    plt.savefig(outputplotfile)
    
if __name__ == "__main__":
    main(sys.argv[1:])
