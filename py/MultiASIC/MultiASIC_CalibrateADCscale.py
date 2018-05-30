#!/usr/bin/env python
import sys
import time
import os
import csv
sys.path.append('/home/testbench2/root_6_08/lib')
import ROOT
ROOT.gROOT.LoadMacro("rootMacros/MakeCalibTTrees.cxx")
ROOT.gROOT.LoadMacro("rootMacros/AnalyzeWaveforms.cxx")
ROOT.gROOT.LoadMacro("rootMacros/FindSaturationPoint.cxx")
SCRIPTPATH = os.path.dirname(__file__)
sys.path.append( SCRIPTPATH+'/lib/' )
import linkEth
t0 = time.time()

################################################################################
##              ----- Multi ASIC Pre-Amp Saturation Scan -----
##      This script collects data at high MPPC gain for every ASIC, but one
##  channel at a time. The root macros will pedestal subtract the data, file it
##  by ASIC and channel using a TDirectory structure, then plot and fit the data
##  in order to find the saturation point for each channel.
##      In the future this script will:
##      Use the preamp saturation point to set the Wilkinson ramp.
##
##
##
##  Author: Chris Ketter
##          University of Hawaii at Manoa
##          cketter@hawaii.edu
##
##  Last Modified: 2 Dec. 2017
##
################################################################################


#---------------- USAGE ----------------#
usageMSG="usage:\n"+\
"sudo ./MultiASIC_CalibrateADCscale.py <SN> <HV> \n"+\
"Where:\n"+\
    "<S/N>       = KLMS_0XXX\n"+\
    "<HV>        = (e.g.) 75p52\n"
if (len(sys.argv)!=3):
    print usageMSG
    exit(-1)

SN       = str(sys.argv[1])
rawHV      = str(sys.argv[2])
floatRawHV = float(rawHV.replace("p","."))


interface   = "eth4"    #  depends on pc
ASICmask    = int("0000000001",2)
evtsPerCh   = 50000      #  must be >0
winOffset   = 3         #  integer in range [0, 508]
opMode      = 3         #  1 = Pedestal subtracted data, 2 = Pedestals only, 3 = Raw Waveform only
outMode     = 0         #  0 = Waveforms, 1 = Charge & Time, -1 = Don't touch this register
trgTyp      = 1         #  0 = Software, 1 = Hardware
trigOffset  = 600#650
hvOffset    = 25

#---- LOOK FOR CALIBRATION VALUES ----#
thBase = [0 for ASIC in range(10)]
hvLow  = [0 for ASIC in range(10)]
hvHigh = [0 for ASIC in range(10)]
for ASIC in range (10):
#    if (2**ASIC & ASICmask) > 0:
    calib_file = "data/%s/calib/HVandTH/%s_HV%s_ASIC%d.txt" % (SN, SN, rawHV, ASIC)
#       if not (os.path.isfile(calib_file)):  #infile.mode == 'r':
#           os.system("sudo ./setThresholdsAndGains.py %s %s %s %d" % (interface, SN, rawHV, ASIC))
    num_lines = int(os.popen("sed -n '$=' "+calib_file).read())
    #print "file: %s, num lines: %d" % (calib_file, num_lines)
    if (num_lines == 15):
        infile = open(calib_file, 'r')
    else:
        print "Calibration file has wrong number of lines. Exiting!"
        print "Remove faulty calibration file and rerun script."
        quit()
    csvFile = csv.reader(infile, delimiter='\t')
    fileLines= []
    for line in csvFile:
        fileLines.append(line)
    infile.close()
    thBase[ASIC] = [int(line[0]) for line in fileLines]
    hvLow[ASIC]  = [int(line[1]) for line in fileLines]
    hvHigh[ASIC] = [int(line[2]) for line in fileLines]
print "\nFound threshold base from file:"
for row in thBase:
    print(' '.join([str(elem) for elem in row]))
print "\nFound low HV values from file:"
for row in hvLow:
    print(' '.join([str(elem) for elem in row]))
print "\nFound high HV values from file:"
for row in hvHigh:
    print(' '.join([str(elem) for elem in row]))
#    print "\n\nRaw HV = %.3f" % float(rawHV.replace("p","."))

#--------- ETHERNET CONFIGURATION ----------#
addr_fpga = '192.168.20.5'
addr_pc = '192.168.20.1'
port_pc = '28672'
port_fpga = '24576'
syncwd="000000010253594e4300000000" # must be send before every command string
ctrl = linkEth.UDP(addr_fpga, port_fpga, addr_pc, port_pc, interface)
# Make UDP class for receiving/sending UDP Packets

#--- CONFIGURE SCROD(?) & SET TRIG. THRESHOLD AND HV TO NULL ---#
cmdZeroTh = cmdHVoff = syncwd
for ASIC in range(10):
    if (2**ASIC & ASICmask) > 0:
        for ch in range (0,15):
            cmdHVoff   += hex( int('C',16)*(2**28) | ASIC*(2**20) | (ch)*(2**16) | 255 ).split('x')[1]+"AE000100"
            cmdZeroTh  += hex( int('B',16)*(2**28) | ASIC*(2**24) | (2*ch)*(2**16) | 0 ).split('x')[1]+"AE000100"

cmd_runConfig  = syncwd + "AF250000" + "AE000100"#disable ext. trig
cmd_runConfig += "AF3E0000" + "AE000100"#set win start---> is set to zero for internal triggering
cmd_runConfig += hex(int('AF360000',16) | 0              | winOffset       ).split('x')[1] +"AE000100"#set win offset
cmd_runConfig += hex(int('AF330000',16) | 0              | ASICmask ).split('x')[1] +"AE000100"#set asic number
cmd_runConfig += hex(int('AF260000',16) | opMode*(2**12) | 2**7            ).split('x')[1] +"AE000100"#CMDREG_PedDemuxFifoOutputSelect(13 downto 12)-->either wavfifo or pedfifo,CMDREG_PedSubCalcMode(10 downto 7)-->currently only using bit 7: 1 for peaks 2 for troughs, sample offset is 3400 o6 600 respectively
cmd_runConfig += hex(int('AF270000',16) | trgTyp*(2**15) | ASICmask ).split('x')[1] +"AE000100"#set trig mode & asic trig enable
cmd_runConfig += hex(int('AF4A0000',16) | outMode*(2**8) | 1*2**4 + 2*2**0 ).split('x')[1] +"AE000100"#set outmode and win boundaries for trigger bits: x12 scans 1 win back and two forward
cmd_runConfig += "AF4F0002" # external trigger bit format
#ctrl.send(cmd_runConfig)
#time.sleep(0.1)
#ctrl.KLMprint(cmd_runConfig,"cmd_runConfig")

#------------------------------------------------------------------------------#
#-------------------SETUP ASIC AND SCROD FOR DATA COLLECTION-------------------#
#------------------------------------------------------------------------------#
# Test one channel on 10 ASICs at a time
for testCH in range (2,3):
    t0CH = time.time()
    cmdASIC_HV = syncwd
    cmdASIC_Th = syncwd
    for ASIC in range(10):
        if (2**ASIC & ASICmask) > 0:
            for ch in range (15):
                if (ch==testCH):
                    cmdASIC_HV += hex( int('C',16)*(2**28) | ASIC*(2**20) | (ch)*(2**16)   | hvLow[ASIC][ch] - hvOffset  ).split('x')[1]+"AE000100"
                    cmdASIC_Th += hex( int('B',16)*(2**28) | ASIC*(2**24) | (2*ch)*(2**16) | (thBase[ASIC][ch]-trigOffset) ).split('x')[1]+"AE000100"
                else:
                    cmdASIC_HV += hex( int('C',16)*(2**28) | ASIC*(2**20) | (ch)*(2**16)   | 255   ).split('x')[1]+"AE000100"
                    cmdASIC_Th += hex( int('B',16)*(2**28) | ASIC*(2**24) | (2*ch)*(2**16) | 0 ).split('x')[1]+"AE000100"
#    ctrl.KLMprint(cmdASIC_HV, 'cmdASIC_HV')
#    ctrl.KLMprint(cmdASIC_Th, 'cmdASIC_Th')

    #----SET THRESHOLD OFFSET TO USER SPECIFIED VALUE----#
    ctrl.open()
    time.sleep(1.0)
#    ctrl.send(cmdHVoff)
#    time.sleep(0.1)
    ctrl.send(cmdASIC_Th)
    time.sleep(0.1)
    ctrl.send(cmdASIC_HV)
    time.sleep(0.1)




    #------------------------------------------------------------------------------#
    #-------------------------------DATA COLLECTION--------------------------------#
    #------------------------------------------------------------------------------#
    print "\n\nTaking %s events . . ." % evtsPerCh
    buffSize = 20000
    os.system("echo -n > temp/data.dat") #clean file without removing (prevents ownership change to root in this script)
    f = open('temp/data.dat','ab') #a=append, b=binary
    time.sleep(0.1)
    ctrl.send(cmd_runConfig)
    time.sleep(0.1)
    t1 = time.time()
    for i in range(0, evtsPerCh):
        rcv = ctrl.receive(buffSize)# rcv is string of Hex
        time.sleep(0.001)
        if i!=0 and (i%800)==0:
            sys.stdout.write("<--%d\n" % i)
            sys.stdout.flush()
        if ((i%10)==0):
            sys.stdout.write('.')
            sys.stdout.flush()
        rcv = linkEth.hexToBin(rcv)
        f.write(rcv) # write received binary data into file
    EvtRate = evtsPerCh/float(time.time()-t1)
    print "\nOverall hit rate was %.2f Hz" % EvtRate
    f.close()
    ctrl.send(cmdHVoff)
    time.sleep(0.1)
    ctrl.close()


    #------------------------------------------------------------------------------#
    #---------------------CALL ON PARSING AND PLOTTING SCRIPTS---------------------#
    #------------------------------------------------------------------------------#

    #---- RUN DATA-PARSING EXECUTABLE ----#
    print "\nParsing %s Data" % SN
    os.system("./bin/tx_ethparse1_ck temp/data.dat temp/waveformSamples.txt temp/triggerBits.txt " + str(outMode))
    time.sleep(0.1)
    print "Data parsed."

    #---- RUN ROOT MACROS ----#
    print("\nWriting data into a TTree . . .\n")
    root_file = "data/%s/%s.root" % (SN,SN)
    #ROOT.MakePedSubtractedTTree("temp/waveformSamples.txt", root_file
    ROOT.MakeCalibTTrees("temp/waveformSamples.txt", root_file, "satData", testCH)#ascii_input,file,dir,CH,opt bool spcSvr=0
    print "Data written in:   %s" % root_file
    os.system("chmod a+w " + root_file)
    os.system("chown -R testbench2:testbench2 " + root_file)
    os.system("rm temp/waveformSamples.txt")
    for ASIC in range(10):
        if (2**ASIC & ASICmask) > 0:
            actHV =  floatRawHV  -  5*(hvLow[ASIC][testCH] - hvOffset)/255.
            dirName = "satData/ASIC%d/ch%d" % (ASIC, testCH)
            rootTree = "ch%d_satData" % testCH

            print "FindSatPoint(%s, %s, %s, %d, %d, %d)" % (root_file, dirName, rootTree, ASIC, testCH, trigOffset)
            ROOT.FindSatPoint(root_file, dirName, rootTree, ASIC, testCH, trigOffset)# file,dir,tree,asic,ch,thresh
            time.sleep(0.5)

            print "AnalyzeBigWaveforms(%s, %s, %s, %d, %d, %.2f)" % (root_file, dirName, rootTree, ASIC, testCH, actHV)
            ROOT.AnalyzeBigWaveforms(root_file, dirName, rootTree, ASIC, testCH, actHV)#file,dir,tree,ASIC,ch,(float)argHV
            time.sleep(0.5)

    #Now need to modify Wilkinson ramp and retest

    timeCH = (time.time() - t0CH)/60.
    print "Channel %d finished in %.2f minutes" % (testCH, timeCH)

totTime = (time.time() - t0)/60.
print "Overall process took %.2f min" % totTime
