#!/usr/bin/env python
import sys
import time
import os
import csv
from array import array
sys.path.append('/home/testbench2/root_6_08/lib')
from ROOT import *
SCRIPTPATH = os.getcwd()#os.path.dirname(__file__)
sys.path.append( SCRIPTPATH+'/lib/' )
import linkEth
t1=time.time()

################################################################################
##         ----- Roughly Set Thresholds & Gains for One ASIC -----
##      This script first sets the threshold base value for every channel on one
##  ASIC, i.e. it sets the vertical position knob of this 160 channel oscilloscope
##  to zero.
##
##
##
##
##
##
##  Author: Chris Ketter
##
##  Last Modified: 10 Jan 2018
##
################################################################################

#---------------- USAGE ----------------#
usageMSG="\nUsage:\n"+\
"\tsudo ./SingleASIC_Starting_Values.py <ASIC>\n\n"+\
"Where:\n"+\
    "\t<ASIC>      = (e.g.) 0\n"

if len(sys.argv)==2:
    asicNo      = int(sys.argv[1])
if len(sys.argv)!=2:
    print usageMSG
    exit(-1)


#--------- DECLARE ROOT FILE & TREE ----------#
freq = array("f", 150*[0.])
HVtrim = array("i", [0])
approxHV = array("f", [0.])
rootfile = TFile("data/S10362-013-050C/S10362-013-050C.root", "UPDATE")
rootfile.ls()
rootfile.mkdir("TrigScalers")
rootfile.ls()
rootfile.cd("TrigScalers")
tree = TTree("SFreq", "Trig Scan. One entry per HV value, 150 TrigDAC values for each.")
tree.Branch("trigFreq", Sfreq, "trigFreq[150]/F")
tree.Branch("HVtrimDAC", HVtrim, "HVtrimDAC/I")
tree.Branch("approxHV", approxHV, "approxHV/F")


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
for Ich in range (0,16):
    cmdHVoff  += hex( int('C',16)*(2**28) | asicNo*(2**20) | (Ich)*(2**16) | 255 ).split('x')[1]#+"AE000100"
    cmdZeroTh += hex( int('B',16)*(2**28) | asicNo*(2**24) | (2*Ich)*(2**16) | 0 ).split('x')[1]#+"AE000100"
ctrl.send(cmdHVoff)
time.sleep(0.1)
ctrl.send(cmdZeroTh)
time.sleep(0.1)

# --- CONFIGURE SCROD REGISTERS FOR COUNTING TRIGGER SCALERS --- #
ctrl.send(syncwd+'AF4D0B00'+'AE000100'+'AF4DCB00') # for KLM SciFi -- kept for legacy
time.sleep(0.1)
ctrl.send(syncwd+'AF2F0004'+'AF300004')# 0000 0000 0000 0100 --> (47) TRIG_SCALER_CLK_MAX, (48) TRIG_SCALER_CLK_MAX_TRIGDEC
time.sleep(0.1)
ctrl.send(syncwd+'AF4A0136')# 0011 0110 (7 downto 0) WAVE_TRIGASIC_DUMP_CFG, 0001 (11 downto 8) PEDSUB_DATAOUT_MODE
time.sleep(0.1)


#--------------------------------------------------------------------------#
#----------------------SET GAIN AND TRIGGER THRESHOLDS---------------------#
#--------------------------------------------------------------------------#
T0 = 1000*4*2**16/63.5e6 # Clock period in ms (used for counting scalerCount)

thBase   = [3200 for i in range(15) ]

c1 = TCanvas( 'c1', '', 0, 0, 1000, 1000 )

for chNo in range(1):
    #---------- SCAN FOR THRESHOLD BASE VALUE (HV OFF) ----------#
    fmax = -1
    freq = 0
    ctrl.send(cmdHVoff)#set all HV trims to 5V
    time.sleep(0.1)
    ctrl.send(cmdZeroTh)#set all trigs to 0
    time.sleep(0.1)
    print "\n*********** -Ch%d- ***********" % chNo
    print "Counting scalar frequencies at different thresholds (HV off)."
    for th in range (3700,3400,-1):
        cmdTh = syncwd + hex( int('B',16)*(2**28) | asicNo*(2**24) | (2*chNo)*(2**16) | th ).split('x')[1]#+"AE000100"
        ctrl.send(cmdTh)
        time.sleep(0.01)#wait for counters to settle then read registers
        # (scalerCount is 32-bit value shared between 2 16-bit registers)
        scalerCount = ctrl.readReg(138+asicNo) + ctrl.readReg(168+asicNo)*65536
        if scalerCount > 0:
            sys.stdout.write("\nTrigDac=%3d " % th)
        #--- PICK OUT MAX SCALAR FREQ. AND ASSOC. THRESHOLD VALUE ---#
        freq = (scalerCount)/T0
        if scalerCount > 0:
            sys.stdout.write("%6.0f " % freq)# "Threshold: %8d\tScaler Count: %d" % (th, scalerCount)
        if (freq > fmax):
            fmax = freq
            thBase[chNo] = th

    print "\n\nMax Scalar Freq:"
    sys.stdout.write("%.1f " % fmax)

    print "\n\nThreshold Base:"
    sys.stdout.write("%d " % thBase[chNo])
    time.sleep(0.01)


    #---------- SCAN TRIG. SCALERS FOR EACH HV SETTING ----------#
    print "\nPerforming HV scan for channel %d" % chNo
    gr2d = TH2F( "gr2d","S10362-013-050C Trigger Scalers",150,0,150, 256,0,256)
    gr2d.Draw("contz")
    gStyle.SetOptStat(0)
    gr2d.GetYaxis().SetTitle("HV trim value (DAC counts)")
    gr2d.GetYaxis().SetTitleOffset(1.4)
    gr2d.GetXaxis().SetTitle("Trig. Level (DAC counts)")
    c1.Update()
    #gr.GetYaxis().SetTitle( 'HV Trim (DAC counts)' )
    for hv in range (256):#(180,179,-1): # 255 is lowest HV setting (i.e. most trim)
    #for hv in range (180,179,-1): # 255 is lowest HV setting (i.e. most trim)
        print "\x1b[6;30;42m\nHV trim: %d\x1b[0m" % hv
        HVtrim[0] = hv
        approxHV[0] = (74.57 - 5.0*hv/256.0)
        cmdHV = syncwd
        cmdHV += hex( int('C',16)*(2**28) | asicNo*(2**20) | (chNo)*(2**16) | hv ).split('x')[1]#+"AE000100"
        ctrl.send(cmdHV)
        time.sleep(0.01)#wait for counters to settle then read registers
        freq = 0
        for thOffset in range (150):
            cmdTh = syncwd + hex( int('B',16)*(2**28) | asicNo*(2**24) | (2*chNo)*(2**16) | thBase[chNo]-thOffset ).split('x')[1]#+"AE000100"
            ctrl.send(cmdTh)
            time.sleep(0.01)#wait for counters to settle then read registers
            scalerCount=0
            for i in range(100):#average scalers 100 times
                scalerCount += ctrl.readReg(138+asicNo) + ctrl.readReg(168+asicNo)*65536
                #time.sleep(0.01)
            freq = scalerCount/T0/100
            Sfreq[thOffset] = freq
            sys.stdout.write("\nTrig Level: %3d   Freq: %6.0f " % (thOffset,freq))
            bin = gr2d.GetBin(thOffset,hv);
            gr2d.SetBinContent(bin,freq)
            ### END TRIG LEVEL LOOP ###
        tree.Fill()#one tree entry per HV value
        #mg = TMultiGraph()
        gr2d.Draw("contz")
        #gr1 = TGraph( 145, x, y )
        #gr2 = TGraph( 145, x, dy )
        #gr1.SetMarkerStyle(20)
        #gr2.SetMarkerStyle(20)
        #gr2.SetMarkerColor(2)
        #mg.Add(gr1)
        #mg.Add(gr2)
        #mg.Draw('ap')
        #mg.GetXaxis().SetTitle( 'Trigger Level (trig DAC counts)' )
        #mg.SetMaximum(1000)
        #mg.SetMinimum(-200)
        #mg.GetYaxis().SetTitle( 'Trigger Scaler Frequency (kHz)' )
        #mg.GetYaxis().SetTitleOffset(1.5)
        #grTitle = "Channel " + str(chNo) + ' Trigger Scalers'
        #mg.SetTitle( grTitle )
        c1.Update()
        #sigma1 = 4.697
        #sigma2 = 5.068
        #sigma3 = 5.347
        #sigma4 = 5.902
        #sigma5 = 6.2
        #eqnMG = "TMath::Cos([0]*(x-[1]))+TMath::Power([2],x/[3])"
        #eqnMG = "[0]*TMath::Power([1],x/[2])"
        #eqnMG = "[0]*(TMath::Power([1],1)*TMath::Exp(   -.5*TMath::Power( (x-(1*[2]+[3]))/%f,2 )   ) + TMath::Power([1],2)*TMath::Exp(   -.5*TMath::Power( (x-(2*[2]+[3]))/%f,2 )   ) + TMath::Power([1],3)*TMath::Exp(   -.5*TMath::Power( (x-(3*[2]+[3]))/%f,2 )   ) + TMath::Power([1],4)*TMath::Exp(   -.5*TMath::Power( (x-(4*[2]+[3]))/%f,2 )   )  )" % (sigma1,sigma2,sigma3,sigma4)
        #MultiGauss = TF1("MultiGauss", eqnMG, 15, 150);
        #MultiGauss.SetParameters(24000,.2,17,0);
        #gr1.Fit("MultiGauss", "RS");
        #c1.Update()
        #time.sleep(3)
        ### END HV LOOP ###
    #c1.Print("ScalerSteps_HVtrim180_FIT.pdf")
    #c1.Write("ScalerSteps_HVtrim%d"%hv)
    #gr2d.Draw("contz")
    #c1.Print("ScalerFreq_vs_Trig&HV.pdf")
    c1.Write("ScalerFreq_TempPlot")
    tree.Write()
    #rootfile.Write()
    rootfile.Close()
    ctrl.send(cmdHVoff)
    time.sleep(0.01)
    ### END CHANNEL LOOP ###
ctrl.close()

deltaTime = (time.time()-t1)/60
print "Process completed in %.2f min." % deltaTime
