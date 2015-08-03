#!/usr/bin/env python

# Simple python script for converting binary CTPPS HPTDC data files to human-readable format. 
# Based on https://github.com/forthommel/pps-tbrc/src/FileReader.cpp C++ version by L. Forthomme

import os, string, sys, posix, tokenize, array, getopt, struct, ast
import matplotlib
matplotlib.use("Agg")
import numpy as np
import pylab as P
import matplotlib as mpl
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt

class DQMReader:
    def __init__(self,input):
        #########################
        # General script settings
        #########################

        # Input filename obtined from the server communication. Output file has the same name except for extension
        if(input.find(".dat") == -1):
            print "File sent to DQM is not in .dat format - exiting"
            return
        self.inputbinaryfile = input
        self.outputplotfile = self.inputbinaryfile.split('.dat')[0] + '_dqm_report.svg'
        self.outputtextfile = self.inputbinaryfile.split('.dat')[0] + '_dqm_report.txt'
        self.nchannels = 32
        self.ngroups = 4
        self.verbose = 0
        
        ###################                                                                                                                                            
        # HPTDC definitions                                                                                                                                           
        ###################                                                                                                                                           

        # AcquisitionMode
        self.CONT_STORAGE=0
        self.TRIG_MATCH=1

        # DetectionMode
        self.PAIR=0
        self.OTRAILING=1
        self.OLEADING=2
        self.TRAILEAD=3

        # EventTypes - for each TDC event in the file
        self.TDCMeasurement = 0x0
        self.TDCHeader = 0x1
        self.TDCTrailer = 0x3
        self.TDCError = 0x4
        self.GlobalHeader = 0x8
        self.GlobalTrailer = 0x10
        self.ETTT = 0x11
        self.Filler = 0x18

        # Define counters and arrays for DQM plots
        self.ntriggers = [0]
        self.occupancy = []
        self.toverthreshold = []
        k=0
        while k < self.nchannels:
            self.occupancy.append(0)
            self.toverthreshold.append(0)
            k=k+1

        self.nerrors = [0]
        self.grouperrors = []
        self.readoutfifooverflowerrors = []
        self.l1bufferoverflowerrors = []
        k=0
        while k < self.ngroups:
            self.grouperrors.append(0)
            self.readoutfifooverflowerrors.append(0)
            self.l1bufferoverflowerrors.append(0)
            k=k+1
        self.eventsizelimiterrors = [0]
        self.triggerfifooverflowerrors = [0]
        self.internalchiperrors = [0]

    def SetNChannelsToPlot(self,nchan):
        self.nchannels = nchan

    def SetNGroupsToPlot(self,ngrp):
        self.ngroups = ngrp

    def SetVerbosity(self,level):
        self.verbose = level

    def SetOutputFileFormat(self,extension):
        self.outputplotfile = self.outputplotfile.split('.')[0] + '.' + str(extension)

    def SetInputFilename(self,infname):
        self.inputbinaryfile = infname

    def SetOutputFilename(self,outfname):
        self.outputplotfile = outfname

    def GetOutputFilename(self):
        return self.outputplotfile

    def ProcessBinaryFile(self):
        self.ReadFile()
        self.ProducePlots()

    #############################################################
    # Main method for analyzing a single binary HPTDC output file
    #############################################################                                                                                                                                  
    def ReadFile(self):

        with open(self.inputbinaryfile, 'rb') as f:
            # First read and unpack the file header
            byte = f.read(24)
            decode = struct.unpack("IIIBII",byte[:24])
            magic = decode[0]
            run_id = decode[1]
            spill_id = decode[2]
            num_hptdc = decode[3]
            AcquisitionMode = decode[4]
            DetectionMode = decode[5]

            if(self.verbose == 1):
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
            while k < self.nchannels:
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

                if(type == self.GlobalHeader):
                    if(self.verbose == 1):
                        print "Global Header"

                    # Reset lookup dictionary for each new event
                    channeltimemeasurements={}

                    k = 0
                    while k < self.nchannels:
                        channeltimemeasurements[k,1]=0
                        channeltimemeasurements[k,2]=0
                        k=k+1

                # This word is a global trailer - calculate summary timing information for all channels in this event
                if(type == self.GlobalTrailer):
                    status = ((decode[0]) >> 24) & (0x7)
                    geo = (decode[0]) & (0x1F)

                    if(self.verbose == 1):
                        print "\tTiming measurements for this trigger:"
                        print"\t\tChannel\tLeading\t\tTrailing\tDifference"

                    channelflag=0
                    while channelflag < self.nchannels:
                        tleading = channeltimemeasurements[channelflag,1] * 25./1024.
                        ttrailing = channeltimemeasurements[channelflag,2] * 25./1024.
                        tdifference = (channeltimemeasurements[channelflag,2] - channeltimemeasurements[channelflag,1]) * 25./1024.
                        self.toverthreshold[channelflag] = self.toverthreshold[channelflag] + tdifference

                        if(self.verbose == 1):
                            print "\t\t" + str(channelflag) + ":\t" + str(tleading) + ",\t" + str(ttrailing) + ",\t" + str(tdifference)

                        channelflag = channelflag+1

                    if(self.verbose == 1):
                        print "Global Trailer (status = " + str(status) + ", Geo = " + str(geo) + ")"

                if(type == self.TDCHeader):
                    tdcid = ((decode[0]) >> 24) & (0x3)
                    eventid = ((decode[0]) >> 12) & (0xFFF)
                    bunchid = (decode[0]) & (0xFFF)

                    if(self.verbose == 1):
                        print "\tTDC ID = " + str(tdcid)
                        print "\tEvent ID = " + str(eventid)
                    
                if(type == self.TDCTrailer):
                    wordcount = (decode[0]) & (0xFFF)
                    eventcount = ((decode[0]) >> 5) & (0x3FFFF)

                    if(self.verbose == 1):
                        print "\tWord count = " + str(wordcount)
                        print "\tEvent count = " + str(eventcount)

                if(type == self.ETTT):
                    GetETT = (decode[0]) & (0x3FFFFFF)

                    if(self.verbose == 1):
                        print "ETT = " + str(GetETT)

                    self.ntriggers[0] = self.ntriggers[0] + 1

                # This word is an actual TDC measurement - get the timing and leading/trailing edge informations
                if(type == self.TDCMeasurement):
                    istrailing = ((decode[0]) >> 26) & (0x1)
                    time = (decode[0]) & (0x7FFFF)

                    if(DetectionMode == self.PAIR):
                        time = (decode[0]) & (0xFFF)

                    width = ((decode[0]) >> 12) & (0x7F)
                    channelid = ((decode[0]) >> 21) & (0x1F)

                    if(istrailing == 0):
                        channeltimemeasurements[channelid,1] = time
                    if(istrailing == 1):
                        channeltimemeasurements[channelid,2] = time
                        self.occupancy[channelid] = self.occupancy[channelid]+1

                    if(self.verbose == 1):
                        print "\tTDCMeasurement (trailing = " + str(istrailing) + ", time = " + str(time) + ", width = " + str(width) + ", (channel ID = " + str(channelid) + ")"
                
                # This word is an Error - decode it and count each type of error
                if(type == self.TDCError):
                    errorflag = (decode[0]) & (0x7FFF)
                    self.nerrors[0] = self.nerrors[0] + 1

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

                    if(self.verbose == 1):
                        print "Error detected: " + str(errorflag)


    ####################
    # Generate DQM plots
    ####################
    def ProducePlots(self):
        i = 0
        while i < self.nchannels:
            if(self.occupancy[i]>0):
                self.toverthreshold[i] = self.toverthreshold[i]/self.occupancy[i] # Before normalizing counts
            if(self.ntriggers[0]>0):
                self.occupancy[i] = self.occupancy[i]/self.ntriggers[0]
            i = i + 1

        mpl.rcParams['xtick.labelsize'] = 6
        mpl.rcParams['ytick.labelsize'] = 6

        plt.subplot(3, 3, 1)
        plt.bar(range(0,1),self.ntriggers,width=1.0)
        plt.ylabel('Triggered events',fontsize=6)
        plt.figtext(0.15, 0.8, "N = " + str(self.ntriggers[0]))
        plt.axis([0, 1, 0, max(self.ntriggers)*2],fontsize=6)
        frame1 = plt.gca()
        frame1.axes.get_xaxis().set_visible(False)
        plt.grid(True)

        plt.subplot(3, 3, 2)
        plt.bar(range(0,self.nchannels),self.occupancy)
        plt.xlabel('HPTDC Channel',fontsize=6)
        plt.ylabel('Occupancy',fontsize=6)
        plt.axis([0, self.nchannels, 0, 1.2],fontsize=6)
        plt.grid(True)

        plt.subplot(3, 3, 3)
        plt.bar(range(0,self.nchannels),self.toverthreshold)
        plt.xlabel('HPTDC Channel',fontsize=6)
        plt.ylabel('Mean time over threshold [ns]',fontsize=6)
        plt.axis([0, self.nchannels, 0, max(self.toverthreshold)*2])
        plt.grid(True)

        plt.subplot(3, 3, 4)
        plt.bar(range(0,1),self.nerrors,width=1.0,color='r')
        plt.ylabel('Total errors',fontsize=6)
        plt.axis([0, 1, 0, 2])
        if(self.nerrors[0]>0):
            plt.axis([0, 1, 0, max(self.nerrors)*2])
        frame2 = plt.gca()
        frame2.axes.get_xaxis().set_visible(False)
        plt.grid(True)

        plt.subplot(3, 3, 5)
        plt.bar(range(0,1),self.eventsizelimiterrors,width=1.0,color='r')
        plt.ylabel('Event size errors',fontsize=6)
        plt.axis([0, 1, 0, 2])
        if(self.eventsizelimiterrors[0]>0):
            plt.axis([0, 1, 0, max(self.eventsizelimiterrors)*2])
        frame2 = plt.gca()
        frame2.axes.get_xaxis().set_visible(False)
        plt.grid(True)

        plt.subplot(3, 3, 6)
        plt.bar(range(0,1),self.triggerfifooverflowerrors,width=1.0,color='r')
        plt.ylabel('Trigger FIFO overflow errors',fontsize=6)
        plt.axis([0, 1, 0, 2])
        if(self.triggerfifooverflowerrors[0]>0):
            plt.axis([0, 1, 0, max(self.triggerfifooverflowerrors)*2])
        frame2 = plt.gca()
        frame2.axes.get_xaxis().set_visible(False)
        plt.grid(True)

        plt.subplot(3, 3, 7)
        plt.bar(range(0,1),self.internalchiperrors,width=1.0,color='r')
        plt.ylabel('Internal chip errors',fontsize=6)
        plt.axis([0, 1, 0, 2])
        if(self.internalchiperrors[0]>0):
            plt.axis([0, 1, 0, max(self.internalchiperrors)*2])
        frame2 = plt.gca()
        frame2.axes.get_xaxis().set_visible(False)
        plt.grid(True)

        plt.subplot(3, 3, 8)
        plt.bar(range(0,self.ngroups),self.readoutfifooverflowerrors,color='r')
        plt.ylabel('Readout FIFO overflow errors',fontsize=6)
        plt.xlabel('Group',fontsize=6)
        plt.axis([0, self.ngroups, 0, 2])
        if(max(self.readoutfifooverflowerrors)>0):
            plt.axis([0, self.ngroups, 0, max(self.readoutfifooverflowerrors)*2])
        frame2 = plt.gca()
        plt.grid(True)

        plt.subplot(3, 3, 9)
        plt.bar(range(0,self.ngroups),self.l1bufferoverflowerrors,color='r')
        plt.ylabel('L1 buffer overflow errors',fontsize=6)
        plt.xlabel('Group',fontsize=6)
        plt.axis([0, self.ngroups, 0, 2])
        if(max(self.l1bufferoverflowerrors)>0):
            plt.axis([0, self.ngroups, 0, max(self.l1bufferoverflowerrors)*2])
        frame2 = plt.gca()
        plt.grid(True)

        plt.savefig(self.outputplotfile)
    
        # Persistently store DQM results as simple text file
        outputtextfilehandle = open(self.outputtextfile, 'w')
        outputtextfilehandle.write(str(self.ntriggers)+'\n')
        outputtextfilehandle.write(str(self.occupancy)+'\n')
        outputtextfilehandle.write(str(self.toverthreshold)+'\n')
        outputtextfilehandle.write(str(self.nerrors)+'\n')
        outputtextfilehandle.write(str(self.eventsizelimiterrors)+'\n')
        outputtextfilehandle.write(str(self.triggerfifooverflowerrors)+'\n')
        outputtextfilehandle.write(str(self.internalchiperrors)+'\n')
        outputtextfilehandle.write(str(self.readoutfifooverflowerrors)+'\n')
        outputtextfilehandle.write(str(self.l1bufferoverflowerrors)+'\n')
        outputtextfilehandle.close()
