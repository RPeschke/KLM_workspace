#!/usr/bin/env python
import sys
import time
import os
import csv
from array import array
#sys.path.append('/home/testbench2/root_6_08/lib')
#from ROOT import TCanvas, TGraph
#from ROOT import gROOT, TF1
SCRIPTPATH = os.getcwd()#os.path.dirname(__file__)
sys.path.append( SCRIPTPATH+'/lib/' )
import linkEth
t1=time.time()
###########################################
# Potential issue:
#   Works once, then have to reprogram
#   motherboard before it will work again
#
#
#---------------- USAGE ----------------#
#---------------- USAGE ----------------#
#e.g./: sudo ./py/CalibChThresholdScan.py KLMS_0173 74p47 0000000001
usageMSG="Usage:\n"+\
"sudo ./py/CalibChThresholdScan.py <S/N> <HV> <ASICmask>\n"+\
"Where:\n"+\
    "<S/N>          = KLMS_0XXX\n"+\
    "<HV>           = (e.g.) 74p47\n"+\
    "<ASICmask>     = (e.g.) 0000000001\n"

if (len(sys.argv)!=4):
    print usageMSG
    exit(-1)

interface   = "eth4"
SN          = str(sys.argv[1])
rawHV       = str(sys.argv[2])
ASICmask    = int(sys.argv[3],2)



#--------- ETHERNET CONFIGURATION ----------#
addr_fpga = '192.168.20.5'
addr_pc = '192.168.20.1'
port_pc = '28672'
port_fpga = '24576'
syncwd="000000010253594e4300000000" # must be send before every command string
# Make UDP class for receiving/sending UDP Packets
ctrl = linkEth.UDP(addr_fpga, port_fpga, addr_pc, port_pc, "eth4")
ctrl.open()

thBase = [0 for i in range(10)]

for ASIC in range(10):
    cmdZeroTh = cmdHVoff = syncwd
    for Ich in range (0,16):
        cmdHVoff  += hex( int('C',16)*(2**28) | ASIC*(2**20) | (Ich)*(2**16) | 255 ).split('x')[1]
        cmdZeroTh += hex( int('B',16)*(2**28) | ASIC*(2**24) | (2*Ich)*(2**16) | 0 ).split('x')[1]+"AE000100"

    if 2**ASIC & ASICmask:
        #--- CONFIGURE SCROD(?) & SET TRIG. THRESHOLD AND HV TO NULL ---#

        #--------------------------------------------------------------------------#
        #----------------------SET GAIN AND TRIGGER THRESHOLDS---------------------#
        #--------------------------------------------------------------------------#
        T0 = 1000*4*65536/63.5e6 # Clock period in ms (used for counting scalerCount)
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

        print "Performing threshold scan and trim DAC scan for ASIC_%d" % ASIC
        #for chNo in range(15):
        chNo = 15
        #---------- THRESHOLD SCAN (HV OFF) ----------#
        fmax = -1
        ctrl.send(cmdHVoff)
        time.sleep(0.1)
        ctrl.send(cmdZeroTh)#set all trigs to 0
        time.sleep(0.1)
        ctrl.send(cmdHVoff)# set all HV to off: sweeping to find base TH
        print "\n*********** -Ch%d- ***********" % chNo
        print "Counting scalar frequencies at different thresholds (HV off)."
        for th in range (4096):
            cmdTh = syncwd + hex( int('B',16)*(2**28) | ASIC*(2**24) | (2*chNo)*(2**16) | th ).split('x')[1]
            ctrl.send(cmdTh)
            time.sleep(0.01)#wait for counters to settle then read registers
            # (scalerCount is 32-bit value shared between 2 16-bit registers)
            scalerCount = ctrl.readReg(138+ASIC) + ctrl.readReg(168+ASIC)*65536
            if (scalerCount != 0):
                print "Threshold: %8d\tScaler Count: %d" % (th, scalerCount)
            #--- PICK OUT MAX SCALAR FREQ. AND ASSOC. THRESHOLD VALUE ---#
            freq = (scalerCount)/T0
            if (freq > fmax):
                fmax = freq
                thBase[ASIC] = th
        print "         Max Scalar Freq      Threshold Base"
        print "Ch_%-2d    %-8.3f            %-d" % (chNo,fmax,thBase[ASIC])
        #----SET THRESHOLD VALUES (CHOSEN ABOVE) WITH OFFSET=40----#
        ctrl.send(cmdZeroTh)# set thresholds for all asics
        time.sleep(0.1)# wait for things to settle
ctrl.close()
deltaTime = (time.time()-t1)/60


calib_file_name = "data/"+SN+"/calib/ch15ThVals.txt"
#----WRITE CALIBRATION DATA TO FILE ----#
outfile = open(calib_file_name, 'w')
print "\n\nResults for all ASICs:"
print "ASIC: Threshold base:"
for i in range(10):
    print "%-4d %-15d" % (i, thBase[i])
    outfile.write("%d\n" % (thBase[i]))
outfile.close()
print "Calibration Completed in %.2f min." % deltaTime
