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
##         ----- Roughly Set Thresholds & Gains for Single ASIC -----
##      This script first sets the threshold base value for every channel on one
##  ASIC, i.e. it sets the vertical position knob of this 160 channel oscilloscope
##  to zero. Next, it sets the trigger threshold to 15 DAC counts and
##  repeatedily counts the number of trigger scalers within a period defined by
##  the firmware while sweeping the HV over all 256 possible values (a 5 volt,
##  8 bit DAC that trims voltage from the raw inupt voltage).
##      Once plotted, a horizontal line is fitted to the flat region where the
##  APDs are saturated. The range of the fit is determined by identifying the
##  first data point to fall below %90 of the maximum data point. Next, the data
##  point which is closest %70 of the maximum is identified and the corresponding
##  HV trim DAC setting is stored to file, along with the threshold base value.
##
##  Author: Chris Ketter
##
##  Last Modified: 10/24/2017
##
################################################################################
#---------------- USAGE ----------------#
usageMSG="\nUsage:\n"+\
"\t./singleASIC_Starting_Values.py <S/N> <HV> <ASIC>\n\n"+\
"Where:\n"+\
    "\t<S/N>       = KLMS_0XXX\n"+\
    "\t<HV>        = (e.g.) 74p9\n"+\
    "\t<ASIC>      = integer in range [0, 9]\n"

if len(sys.argv)==4:
    SN          = str(sys.argv[1])
    rawHV       = str(sys.argv[2])
    asicNo      = int(sys.argv[3])
if len(sys.argv)!=4 or asicNo<0 or asicNo>9:
    print usageMSG
    exit(-1)

#--------- ETHERNET CONFIGURATION ----------#
addr_fpga = '192.168.20.5'
addr_pc = '192.168.20.1'
port_pc = '28672'
port_fpga = '24576'
syncwd="000000010253594e4300000000" # must be send before every command string
# Make UDP class for receiving/sending UDP Packets
ctrl = linkEth.UDP(addr_fpga, port_fpga, addr_pc, port_pc, "eth4")
ctrl.open()
'''
class PowerLaw:
    def __init__(self,s):
        self.s = s
    def __call__(self, x, a):
        self.s.SetParameters(a[0],a[1],a[2])
        f = (x-a[0])**a[1] +a[2]
        return f
'''
def PowerLaw(x, a):
    return (x-a[0])**a[1] + a[2]

#--- CONFIGURE SCROD(?) & SET TRIG. THRESHOLD AND HV TO NULL ---#
cmdZeroTh = cmdHVoff = syncwd
for Ich in range (0,16):
    cmdHVoff  += hex( int('C',16)*(2**28) | asicNo*(2**20) | (Ich)*(2**16) | 255 ).split('x')[1]
    cmdZeroTh += hex( int('B',16)*(2**28) | asicNo*(2**24) | (2*Ich)*(2**16) | 0 ).split('x')[1]+"AE000100"
#--------------------------------------------------------------------------#
#----------------------SET GAIN AND TRIGGER THRESHOLDS---------------------#
#--------------------------------------------------------------------------#
calib_file_name = "calib/"+SN+"_HV"+rawHV+"_"+"ASIC"+str(asicNo)+".txt"
T0 = 1000*4*2**16/63.5e6 # Clock period in ms (used for counting scalerCount)
time.sleep(0.1)
ctrl.send(cmdHVoff)
time.sleep(0.1)
#ctrl.KLMprint('AF4D0B00'+'AE000100'+'AF4DCB00'+'AF2F0004'+'AF300004'+'AF4A0136', "what is this?")
ctrl.send(syncwd+'AF4D0B00'+'AE000100'+'AF4DCB00') # for KLM SciFi -- don't know why it's here
#ctrl.KLMprint('AF4D0B00'+'AE000100'+'AF4DCB00', "Pre Threshold Cmd 1")
time.sleep(0.1)
ctrl.send(syncwd+'AF2F0004'+'AF300004')# 0000 0000 0000 0100 --> (47) TRIG_SCALER_CLK_MAX, (48) TRIG_SCALER_CLK_MAX_TRIGDEC # I think this means 4*(some lg. const) num. of clock cycles for counting scalers
#ctrl.KLMprint('AF2F0004'+'AF300004', "Pre Threshold Cmd 2")
time.sleep(0.1)
#ctrl.send(syncwd+'AF4A100F')
#ctrl.send(syncwd+'AF4A1F00')
#ctrl.send(syncwd+'AF4A0204')
#ctrl.send(syncwd+'AF4A0104')
ctrl.send(syncwd+'AF4A0136')# 0011 0110 (7 downto 0) WAVE_TRIGASIC_DUMP_CFG, 0001 (11 downto 8) PEDSUB_DATAOUT_MODE
#ctrl.KLMprint('AF4A0136', "Pre Threshold Cmd 3")
time.sleep(0.1)
#ctrl.KLMprint(cmdZeroTh, "cmdZeroTh")
ctrl.send(cmdZeroTh)
time.sleep(0.1)
#c1 = TCanvas( 'c1', '', 0, 0, 1000, 1000 )
print "Performing threshold scan and trim DAC scan for ASIC_%d" % asicNo
thBase = [3200 for i in range(15)]
hvStart = [255 for i in range(15)]
hvEnd = [255 for i in range(15)]
TrigFreq = [0 for i in range(256)]
for chNo in range(15):
    #---------- THRESHOLD SCAN (HV OFF) ----------#
    fmax = -1
    ctrl.send(cmdHVoff)
    time.sleep(0.1)
    ctrl.send(cmdZeroTh)#set all trigs to 0
    time.sleep(0.1)
    ctrl.send(cmdHVoff)# set all HV to off: sweeping to find base TH
    print "\n*********** -Ch%d- ***********" % chNo
    print "Counting scalar frequencies at different thresholds (HV off)."
    fineTh = array('d')
    for th in range (3700,3400,-1):
        cmdTh = syncwd + hex( int('B',16)*(2**28) | asicNo*(2**24) | (2*chNo)*(2**16) | th ).split('x')[1]
        ctrl.send(cmdTh)
        time.sleep(0.01)#wait for counters to settle then read registers
        # (scalerCount is 32-bit value shared between 2 16-bit registers)
        scalerCount = ctrl.readReg(138+asicNo) + ctrl.readReg(168+asicNo)*65536
        if (scalerCount != 0):
            fineTh.append(th)
            print "Threshold: %8d\tScaler Count: %d" % (th, scalerCount)
        #--- PICK OUT MAX SCALAR FREQ. AND ASSOC. THRESHOLD VALUE ---#
        freq = (scalerCount)/T0
        if (freq > fmax):
            fmax = freq
            thBase[chNo] = th
        if (fmax>1000 and freq==0):
            break

    print "         Max Scalar Freq      Threshold Base"
    print "Ch_%-2d    %-8.3f            %-d" % (chNo,fmax,thBase[chNo])
    time.sleep(1.0)
    #----SET THRESHOLD VALUES (CHOSEN ABOVE) WITH OFFSET=40----#
    cmdTh = syncwd + hex( int('B',16)*(2**28) | asicNo*(2**24) | (2*chNo)*(2**16) | (thBase[chNo]-35) ).split('x')[1]
    time.sleep(0.1)
    ctrl.send(cmdTh)# set thresholds for all asics
    time.sleep(1.0)# wait for things to settle
    #---------- TRIM-DAC SCAN ----------#
    minDeltaF = 20000.
    print "\nPerforming HV scan for channel %d" % chNo
    x, y = array('d'), array('d')
    fdiffHigh = 100000
    fdiffLow = 100000
    fHigh = 300
    fLow = 10
    fBreak = 1000
    for hv in range (255,-1,-1): # 255 is lowest HV setting (i.e. most trim)
        cmdHV = syncwd
        cmdHV = cmdHV + hex( int('C',16)*(2**28) | asicNo*(2**20) | (chNo)*(2**16) | hv ).split('x')[1]
        ctrl.send(cmdHV)
        time.sleep(0.09)
        TrigFreq[hv] = 1/T0*(ctrl.readReg(138+asicNo) + ctrl.readReg(168+asicNo)*65536)
        print "trim DAC: %8d\tScaler count: %d" % (hv, TrigFreq[hv])
        if TrigFreq[hv]>0:
            x.append(hv)
            y.append(TrigFreq[hv])

        if abs(TrigFreq[hv]-fHigh) < fdiffHigh:
            fdiffHigh = abs(TrigFreq[hv]-2000)
        if abs(TrigFreq[hv]-fLow) < fdiffLow:
            fdiffLow = abs(TrigFreq[hv]-100)
        if TrigFreq[hv]>fBreak:
            break
    cmdHV = syncwd + hex( int('C',16)*(2**28) | asicNo*(2**20) | (chNo)*(2**16) | 255 ).split('x')[1]
    ctrl.send(cmdHV)
    maxValue = max(TrigFreq)
    if maxValue < 500:
        print "WARNING: ASIC %d ch. %d scalers too low, raw HV may be out of range" % (asicNo, chNo)
    if maxValue==0:
        print "ERROR: ASIC %d ch. %d scalers all zero." % (asicNo, chNo)
        print "likely causes:"
        print "     -->raw HV out of range"
        print "     -->loose/disconnected ribbon cable"


    hvEnd[chNo] = -fit1.GetParameter(0)/fit1.GetParameter(1)
    hvEnd[chNo] = int(round(hvEnd[chNo],0))
    hvStart[chNo] = -(fit0.GetParameter(0)-fit1.GetParameter(0))/(fit0.GetParameter(1)-fit1.GetParameter(1))
    hvStart[chNo] = int(round(hvStart[chNo],0))
    print "\nFound HV Range:\n\n\tHVStart = %d\n\tHVEnd = %d\n\tDifference = %d" % (hvStart[chNo], hvEnd[chNo],hvEnd[chNo]-hvStart[chNo])
    #fit0.SetParameters(1,1,1)
#    for i in range(hv):
#        if TrigFreq[i]<=0.9*maxValue:
#            gr.Fit("fit0", "", "", 0.0, float(hv))
#            target = 0.70*fit0.GetParameter(0)
#            break
#    for i in range(hv):
#        if (abs(TrigFreq[i]-target) < minDeltaF): #finding closeset frequency to f0
#            hvBest[chNo] = i
#            minDeltaF = abs(TrigFreq[i]-target)
    #c1.Update()
#    if chNo==0: plotTitle = "calib/"+SN+"_HV"+rawHV+"_"+"ASIC"+str(asicNo)+"_plateau.pdf("
#    elif chNo==14: plotTitle = "calib/"+SN+"_HV"+rawHV+"_"+"ASIC"+str(asicNo)+"_plateau.pdf)"
#    else: plotTitle = "calib/"+SN+"_HV"+rawHV+"_"+"ASIC"+str(asicNo)+"_plateau.pdf"
    #c1.Print("test.pdf")
#c1.Close()
#----WRITE CALIBRATION DATA TO FILE ----#
outfile = open(calib_file_name, 'w')
print "\n\nASIC_%d results for all channels:" % (asicNo)
print "Ch: Threshold base: TrimDAC:"
for i in range(15):
    print "%-3d %-15d %-8d %-8d" % (i, thBase[i], hvStart[i], hvEnd[i])
    outfile.write("%d\t%d\t%d\n" % (thBase[i], hvStart[i], hvEnd[i]))
outfile.close()
ctrl.close()
deltaTime = (time.time()-t1)/60
print "Calibration Completed in %.2f min." % deltaTime
