#!/usr/bin/env python

# Simple python script for converting binary CTPPS HPTDC data files to human-readable format. 
# Based on https://github.com/forthommel/pps-tbrc/src/FileReader.cpp C++ version by L. Forthomme

import os, string, sys, posix, tokenize, array, getopt,struct

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

            # Read and unpack the "type" of this word. The type will determine what other information is present
            decode = struct.unpack("I",f.read(4))
            type = ((decode[0])>>27)&(0x1F)

            if(type == GlobalHeader):
                print "Global Header"
                channeltimemeasurements={}

            if(type == GlobalTrailer):
                status = ((decode[0])>>24)&(0x7)
                print "\tTiming measurements for this trigger:"
                print"\t\tChannel\tLeading\t\tTrailing\tDifference"
                channelflag=0
                while channelflag < 16:
                    print "\t\t" + str(channelflag) + ":\t" + str(channeltimemeasurements[channelflag,1]*25./1024.) + ",\t" + str(channeltimemeasurements[channelflag,2]*25./1024.) + ",\t" + str((channeltimemeasurements[channelflag,2]-channeltimemeasurements[channelflag,1])*25./1024.)
                    channelflag = channelflag+1
                print "Global Trailer (status = " + str(status) +")"

            if(type == TDCHeader):
                print "HEY!!!!!!!!!!! TDCHeader!!!!!!!!!!"

            if(type == TDCTrailer):
                print "HEY!!!!!!!!!!! TDCTrailer!!!!!!!!!!"

            if(type == ETTT):
                GetETT = (decode[0])&(0x3FFFFFF)
                print "ETT = " + str(GetETT)

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

                #                print "\tTDCMeasurement (trailing = " + str(istrailing) + ", time = " + str(time) + ", width = " + str(width) + ", (channel ID = " + str(channelid) + ")"

            if(type == TDCError):
                print "Error!!! We really should do something about this..."

if __name__ == "__main__":
    main(sys.argv[1:])
