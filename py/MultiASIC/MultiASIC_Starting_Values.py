#!/usr/bin/env python
import sys
import time
import os
import csv
import powerlaw
from array import array
sys.path.append('/home/testbench2/root_6_08/lib')
from ROOT import TCanvas, TGraph
from ROOT import gROOT, TF1
SCRIPTPATH = os.path.dirname(__file__)
sys.path.append( SCRIPTPATH+'/lib/' )
import linkEth
t1=time.time()

################################################################################
##         ----- Roughly Set Thresholds & Gains for Every ASIC -----
##      This script first sets the threshold base value for every channel on each
##  ASIC, i.e. it sets the vertical position knob of this 160 channel oscilloscope
##  to zero. Next, it sets the trigger threshold to 150 DAC counts and
##  repeatedily counts the number of trigger scalers within a period defined by
##  the firmware while sweeping the HV over all 256 possible values (a 5 volt,
##  8 bit DAC that trims voltage from the raw inupt voltage). The trigger
##  scalers are printed to the terminal and meanwhile the program looks for
##  scaler frequencies closest to flow and fhigh defined below. Note, these
##  parameters will vary from one type of MPPC to another.
##      All of the values are written to file using a format that subsequent
##  data collection scritps can read and recognize.
##
##  Author: Chris Ketter
##
##  Last Modified: 27 Dec 2017
##
################################################################################

#---------------- USAGE ----------------#
usageMSG="\nUsage:\n"+\
"\tsudo ./MultiASIC_Starting_Values.py <S/N> <HV>\n\n"+\
"Where:\n"+\
    "\t<S/N>       = KLMS_0XXX\n"+\
    "\t<HV>        = (e.g.) 74p9\n"

if len(sys.argv)==3:
    SN          = str(sys.argv[1])
    rawHV       = str(sys.argv[2])
if len(sys.argv)!=3:
    print usageMSG
    exit(-1)

fHigh = 200
fLow = 10
thOffset = 150

#--------- ETHERNET CONFIGURATION ----------#
addr_fpga = '192.168.20.5'
addr_pc = '192.168.20.1'
port_pc = '28672'
port_fpga = '24576'
syncwd="000000010253594e4300000000" # must be send before every command string
# Make UDP class for receiving/sending UDP Packets
ctrl = linkEth.UDP(addr_fpga, port_fpga, addr_pc, port_pc, "eth4")
ctrl.open()

#--- BUILD COMMANDS TO SET TRIG. THRESHOLD AND HV TO NULL ---#
cmdZeroTh = cmdHVoff = syncwd
for asicNo in range(10):
    for Ich in range (0,16):
        cmdHVoff  += hex( int('C',16)*(2**28) | asicNo*(2**20) | (Ich)*(2**16) | 255 ).split('x')[1]#+"AE000100"
        cmdZeroTh += hex( int('B',16)*(2**28) | asicNo*(2**24) | (2*Ich)*(2**16) | 0 ).split('x')[1]#+"AE000100"
ctrl.send(cmdHVoff)
time.sleep(0.1)
ctrl.send(cmdZeroTh)
time.sleep(0.1)

# --- CONFIGURE SCROD REGISTERS FOR COUNTING TRIGGER SCALERS --- #
ctrl.send(syncwd+'AF4D0B00'+'AE000100'+'AF4DCB00') # for KLM SciFi -- don't know why it's here
time.sleep(0.1)
ctrl.send(syncwd+'AF2F0004'+'AF300004')# 0000 0000 0000 0100 --> (47) TRIG_SCALER_CLK_MAX, (48) TRIG_SCALER_CLK_MAX_TRIGDEC
time.sleep(0.1)
ctrl.send(syncwd+'AF4A0136')# 0011 0110 (7 downto 0) WAVE_TRIGASIC_DUMP_CFG, 0001 (11 downto 8) PEDSUB_DATAOUT_MODE
time.sleep(0.1)


#--------------------------------------------------------------------------#
#----------------------SET GAIN AND TRIGGER THRESHOLDS---------------------#
#--------------------------------------------------------------------------#
T0 = 1000*4*2**16/63.5e6 # Clock period in ms (used for counting scalerCount)
thBase   = [0 for ASIC in range(10)]
hvLow    = [0 for ASIC in range(10)]
hvHigh   = [0 for ASIC in range(10)]
TrigFreq = [0 for ASIC in range(10)]
for asicNo in range(10):
    thBase[asicNo]   = [3200 for i in range(15)]
    hvLow[asicNo]    = [255 for i in range(15)]
    hvHigh[asicNo]   = [255 for i in range(15)]
    TrigFreq[asicNo] = [0 for i in range(256)]

for chNo in range(15):
    #---------- THRESHOLD SCAN (HV OFF) ----------#
    fmax = [-1 for Asic in range(10)]
    freq = [0  for Asic in range(10)]
    ctrl.send(cmdHVoff)
    time.sleep(0.1)
    ctrl.send(cmdZeroTh)#set all trigs to 0
    time.sleep(0.1)
    print "\n*********** -Ch%d- ***********" % chNo
    print "Counting scalar frequencies at different thresholds (HV off)."
    for th in range (3700,3400,-1):
        cmdTh = syncwd
        for asicNo in range(10):
            cmdTh += hex( int('B',16)*(2**28) | asicNo*(2**24) | (2*chNo)*(2**16) | th ).split('x')[1]#+"AE000100"
        ctrl.send(cmdTh)
        time.sleep(0.01)#wait for counters to settle then read registers
        # (scalerCount is 32-bit value shared between 2 16-bit registers)
        scalerCount = ctrl.readRegs(138, 148)# + ctrl.readRegs(168, 178)*65536#+asicNo) + ctrl.readRegs(168+asicNo)*65536
        #print "test"
        if max(scalerCount) > 0:
            sys.stdout.write("\nTrigDac=%3d " % th)
        #--- PICK OUT MAX SCALAR FREQ. AND ASSOC. THRESHOLD VALUE ---#
        for asicNo in range(10):
            freq[asicNo] = (scalerCount[asicNo])/T0
            if max(scalerCount) > 0:
                sys.stdout.write("%6.0f " % freq[asicNo])# "Threshold: %8d\tScaler Count: %d" % (th, scalerCount)
            if (freq[asicNo] > fmax[asicNo]):
                fmax[asicNo] = freq[asicNo]
                thBase[asicNo][chNo] = th

    print "\n\nMax Scalar Freq:"
    for asicNo in range(10):
        sys.stdout.write("%.1f " % fmax[asicNo])

    print "\n\nThreshold Base:"
    for asicNo in range(10):
        sys.stdout.write("%d " % thBase[asicNo][chNo])
    time.sleep(1.0)
    #---------- TRIM-DAC SCAN ----------#
    cmdTh = syncwd
    for asicNo in range(10):
        cmdTh += hex( int('B',16)*(2**28) | asicNo*(2**24) | (2*chNo)*(2**16) | thBase[asicNo][chNo]-thOffset ).split('x')[1]+"AE000100"
    ctrl.send(cmdTh)
    time.sleep(0.01)

    minDeltaF = 20000.
    print "\nPerforming HV scan for channel %d" % chNo
    x = [array('d') for Asic in range(10)]
    y = [array('d') for Asic in range(10)]
    fdiffHigh = [100000 for Asic in range(10)]
    fdiffLow  = [100000 for Asic in range(10)]
    for hv in range (255,-1,-1): # 255 is lowest HV setting (i.e. most trim)
        cmdHV = syncwd
        for asicNo in range(10):
            cmdHV = cmdHV + hex( int('C',16)*(2**28) | asicNo*(2**20) | (chNo)*(2**16) | hv ).split('x')[1]#+"AE000100"
        ctrl.send(cmdHV)
        time.sleep(0.01)#wait for counters to settle then read registers
        scalerCount = ctrl.readRegs(138, 148)# + ctrl.readRegs(168, 178)*65536
        if max(scalerCount) > 0:
            sys.stdout.write("\nhvTrim=%3d " % hv)
        for asicNo in range(10):
            TrigFreq[asicNo][hv] = scalerCount[asicNo]/T0
            if max(scalerCount) > 0:
                sys.stdout.write("%6.0f " % TrigFreq[asicNo][hv])
            if scalerCount[asicNo]>0:
                x[asicNo].append(hv)
                y[asicNo].append(TrigFreq[asicNo][hv])
            if abs(TrigFreq[asicNo][hv]-fHigh) < fdiffHigh[asicNo]:
                fdiffHigh[asicNo] = abs(TrigFreq[asicNo][hv]-fHigh)
                hvHigh[asicNo][chNo] = hv
            if abs(TrigFreq[asicNo][hv]-fLow) < fdiffLow[asicNo]:
                fdiffLow[asicNo] = abs(TrigFreq[asicNo][hv]-fLow)
                hvLow[asicNo][chNo] = hv
    ctrl.send(cmdHVoff)
    time.sleep(0.01)#wait for counters to settle then read registers
    print "\n\nFound starting HV values.\nhvLow:"
    for asicNo in range(10):
        sys.stdout.write("%d " % hvLow[asicNo][chNo])
    print "\nhvHigh:"
    for asicNo in range(10):
        sys.stdout.write("%d " % hvHigh[asicNo][chNo])
    for asicNo in range(10):
        maxValue = max(TrigFreq[asicNo])
        if maxValue < fHigh:
            print "\nWARNING: ASIC %d ch. %d scalers too low, raw HV may be out of range" % (asicNo, chNo)
        if maxValue==0:
            print "\nERROR: ASIC %d ch. %d scalers all zero." % (asicNo, chNo)
            print "likely causes:"
            print "     -->raw HV out of range"
            print "     -->loose/disconnected ribbon cable"


#----WRITE CALIBRATION DATA TO FILE ----#
calib_file_name = ["data/"+SN+"/calib/HVandTH/"+SN+"_HV"+rawHV+"_"+"ASIC"+str(asicNo)+".txt" for asicNo in range(10)]
for asicNo in range(10):
    outfile = open(calib_file_name[asicNo], 'w')
    for i in range(15):
        outfile.write("%d\t%d\t%d\n" % (thBase[asicNo][i], hvLow[asicNo][i], hvHigh[asicNo][i]))
    outfile.close()
ctrl.close()
deltaTime = (time.time()-t1)/60
print "Calibration Completed in %.2f min." % deltaTime
print "\nResults:"
print "\nThreshold base:"
for row in thBase:
    print(' '.join([str(elem) for elem in row]))
print "\n\nHV low values:"
for row in hvLow:
    print(' '.join([str(elem) for elem in row]))
print "\n\nHV high values:"
for row in hvHigh:
    print(' '.join([str(elem) for elem in row]))
