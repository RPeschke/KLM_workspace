#!/usr/bin/env python
import sys
import time
import os
import csv
sys.path.append('/home/testbench2/root_6_08/lib')
import ROOT
ROOT.gROOT.LoadMacro("rootMacros/MakePedSubtractedTTree.cxx")
ROOT.gROOT.LoadMacro("rootMacros/PlotPartialSumDistributions.cxx")
ROOT.gROOT.LoadMacro("rootMacros/AnalyzeWaveforms.cxx")
ROOT.gROOT.LoadMacro("rootMacros/FindSaturationPoint.cxx")
ROOT.gROOT.LoadMacro("rootMacros/PlotPhotoElectronPeaks_vs_HV.cxx")
SCRIPTPATH = os.path.dirname(__file__)
sys.path.append( SCRIPTPATH+'/lib/' )
import linkEth

################################################################################
##              ----- Single ASIC Pre-Amp Saturation Scan -----
##      This script collects data at high MPPC gain and looks for events which
##  saturated the preamplifiers
##
##
##
##
##  Author: Chris Ketter
##          University of Hawaii at Manoa
##          cketter@hawaii.edu
##
##  Last Modified: 11/10/2017
##
################################################################################


#---------------- USAGE ----------------#
usageMSG="HVscan.py usage:\n"+\
"./HVscan.py <HV> <ASIC> \n"+\
"Where:\n"+\
    "<HV>        = (e.g.) 75p52\n"+\
    "<ASIC>      = integer in range [0, 9]\n"
#    "<S/N>       = KLMS_0XXX\n"+\
if (len(sys.argv)!=3):
    print usageMSG
    exit(-1)

rawHV       = str(sys.argv[1])
asicNo      = int(sys.argv[2])

SN          = "KLMS_0173"
interface   = "eth4"    #  depends on pc
numEvts     = 150000      #  must be >0
winOffset   = 3         #  integer in range [0, 508]
opMode      = 3         #  1 = Pedestal subtracted data, 2 = Pedestals only, 3 = Raw Waveform only
outMode     = 0         #  0 = Waveforms, 1 = Charge & Time, -1 = Don't touch this register
trgTyp      = 1         #  0 = Software, 1 = Hardware
usrOffset   = 15

if asicNo<0 or asicNo>9:
    print usageMSG
    exit(-1)


#---- LOOK FOR CALIBRATION VALUES FROM TODAY ----#
calib_file = "data/%s/calib/HVandTH/%s_HV%s_ASIC%d.txt" % (SN, SN, rawHV, asicNo)
if not (os.path.isfile(calib_file)):  #infile.mode == 'r':
    os.system("sudo ./setThresholdsAndGains.py %s %s %s %d" % (interface, SN, rawHV, asicNo))
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
thBase = [int(line[0]) for line in fileLines]
hvStart = [int(line[1]) for line in fileLines]
hvEnd = [int(line[2]) for line in fileLines]
print "\nFound threshold base from file:\n%s" % thBase
print "\nFound HV start values from file:\n%s" % hvStart
print "\nFound HV end values from file:\n%s\n" % hvEnd
print "Raw HV = %.3f" % float(rawHV.replace("p","."))

#--------- ETHERNET CONFIGURATION ----------#
addr_fpga = '192.168.20.5'
addr_pc = '192.168.20.1'
port_pc = '28672'
port_fpga = '24576'
syncwd="000000010253594e4300000000" # must be send before every command string
ctrl = linkEth.UDP(addr_fpga, port_fpga, addr_pc, port_pc, interface)
# Make UDP class for receiving/sending UDP Packets

#--- CONFIGURE SCROD(?) & SET TRIG. THRESHOLD AND HV TO NULL ---#
cmdZeroTh = cmdHVoff = cmdASIC_HV = cmdASIC_Th = syncwd
for Ich in range (0,15):
    cmdHVoff   += hex( int('C',16)*(2**28) | asicNo*(2**20) | (Ich)*(2**16) | 255 ).split('x')[1]
    cmdZeroTh  += hex( int('B',16)*(2**28) | asicNo*(2**24) | (2*Ich)*(2**16) | 0 ).split('x')[1]+"AE000100"
    cmdASIC_HV += hex( int('C',16)*(2**28) | asicNo*(2**20) | (Ich)*(2**16)   | hvStart[Ich]-10   ).split('x')[1]+"AE000100"
    cmdASIC_Th += hex( int('B',16)*(2**28) | asicNo*(2**24) | (2*Ich)*(2**16) | (thBase[Ich]-500) ).split('x')[1]+"AE000100"


#------------------------------------------------------------------------------#
#-------------------SETUP ASIC AND SCROD FOR DATA COLLECTION-------------------#
#------------------------------------------------------------------------------#

#----SET THRESHOLD OFFSET TO USER SPECIFIED VALUE----#
ctrl.open()
time.sleep(0.05)
ctrl.send(cmdHVoff)
time.sleep(0.1)
ctrl.send(cmdASIC_Th)
time.sleep(0.1)
ctrl.send(cmdASIC_HV)
time.sleep(0.1)

#----DATA-COLLECTION PARAMETERS FOR SCROD REGISTERS----#
cmd_runConfig = syncwd + "AF250000" + "AE000100" + "AF3E0000" +"AE000100" #disable ext. trig
cmd_runConfig += hex(int('AF360000',16) | 0              | winOffset       ).split('x')[1] +"AE000100"#set win offset
cmd_runConfig += hex(int('AF330000',16) | 0              | 2**asicNo       ).split('x')[1] +"AE000100"#set asic number
cmd_runConfig += hex(int('AF260000',16) | opMode*(2**12) | 2**7            ).split('x')[1] +"AE000100"#CMDREG_PedDemuxFifoOutputSelect(13 downto 12)-->either wavfifo or pedfifo,CMDREG_PedSubCalcMode(10 downto 7)-->currently only using bit 7: 1 for peaks 2 for troughs, sample offset is 3400 o6 600 respectively
cmd_runConfig += hex(int('AF270000',16) | trgTyp*(2**15) | 2**asicNo       ).split('x')[1] +"AE000100"#set trig mode & asic trig enable
cmd_runConfig += hex(int('AF4A0000',16) | outMode*(2**8) | 1*2**4 + 2*2**0 ).split('x')[1] +"AE000100"#set outmode and win boundaries for trigger bits: x12 scans 1 win back and two forward
cmd_runConfig += "AF4F0002" # external trigger bit format


#------------------------------------------------------------------------------#
#-------------------------------DATA COLLECTION--------------------------------#
#------------------------------------------------------------------------------#
print("Taking "+str(numEvts)+" events for ASIC_"+str(asicNo))
buffSize = 20000
os.system("echo -n > temp/data.dat") #clean file without removing (prevents ownership change to root in this script)
f = open('temp/data.dat','ab') #a=append, b=binary
t1 = time.time()
time.sleep(0.1)
ctrl.send(cmd_runConfig)
#ctrl.KLMprint(cmd_runConfig, "run_Config")
time.sleep(0.1)
for i in range(0, numEvts):
    rcv = ctrl.receive(buffSize)# rcv is string of Hex
    time.sleep(0.001)
    if i!=0 and (i%8000)==0:
        sys.stdout.write("<--%d\n" % i)
        sys.stdout.flush()
    if ((i%100)==0):
        sys.stdout.write('.')
        sys.stdout.flush()
    rcv = linkEth.hexToBin(rcv)
    f.write(rcv) # write received binary data into file
EvtRate = numEvts/float(time.time()-t1)
print "\nOverall hit rate was %.2f Hz" % EvtRate
f.close()
ctrl.close()
time.sleep(0.1)


#------------------------------------------------------------------------------#
#---------------------CALL ON PARSING AND PLOTTING SCRIPTS---------------------#
#------------------------------------------------------------------------------#

#---- RUN DATA-PARSING EXECUTABLE ----#
print("\nParsing ASIC_" + str(asicNo) + " data . . .\n")
os.system("./bin/tx_ethparse1_ck temp/data.dat temp/waveformSamples.txt temp/triggerBits.txt " + str(outMode))
time.sleep(0.1)
print "Data parsed."

#---- RUN ROOT MACROS ----#
print("\nWriting data into a TTree . . .\n")
root_filename = "data/%s/ASIC%d/%s.root" % (SN, asicNo, SN)
ROOT.MakePedSubtractedTTree("temp/waveformSamples.txt", root_filename, "satData", "satData", "Pre amp saturation data.")
print "Data written in:   %s" % root_filename
os.system("chmod a+w " + root_filename)
os.system("chown -R testbench2:testbench2 " + root_filename)
time.sleep(0.1)
os.system("rm temp/waveformSamples.txt")
#root_filename = "data/%s/ASIC%d/saturationData.root" % (SN, asicNo)
#plot_name = "data/%s/ASIC%d/plots/ch%dSaturationPlot.pdf" % (SN, asicNo, testCh)
ROOT.FindSaturationPoint(root_filename, "satData", "satData")
ROOT.AnalyzeWaveforms(root_filename, testCh)

#Now need to modify Wilkinson ramp and retest

totTime = (time.time() - t1)/60.
print "Overall time was %.2f min" % totTime
