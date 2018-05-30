#!/usr/bin/env python
import sys
import time
import os
sys.path.append('/home/testbench2/root_6_08/lib')
import ROOT
ROOT.gROOT.LoadMacro("root/SingleASIC_MakePedestalTTree.cxx")
SCRIPTPATH = os.getcwd()#os.path.dirname(__file__)
sys.path.append( SCRIPTPATH+'/lib/' )
import linkEth
################################################################################
##         --- Single ASIC take and average pedestal data ---
##      This script takes pedestals for selected TargetX ASIC N times (user
##  specified). The data file is parsed then stored into a root tree with N
##  entries for selected ASIC. Finally, this temoprary root tree is read back
##  and averaged into a single tree with one entry containing all averaged
##  pedestal values as an array.
##      To read back pedestal data, load the pedestal tree, then use the
##  TTree::GetEntry(0) method, e.g. "pedTree->GetEntry(0)"
##
##      Author: Chris Ketter
##      email:  cketter@hawaii.edu
##      last modified: 16 Jan. 2018
##
################################################################################



#---------------- USAGE ----------------#
#e.g./: sudo ./py/SingleASIC/SingleASIC_takePedsOffline.py KLMS_0173 100
usageMSG="SingleASIC_takePedsOffline.py usage:\n"+\
"./py/SingleASIC/SingleASIC_takePedsOffline.py <S/N> <ASIC> <NumAvgs>\n"+\
"Where:\n"+\
    "\t<S/N>          = KLMS_0XXX\n"+\
    "\t<ASIC>         = 0\n"+\
    "\t<NumAvgs>      = (e.g.) 64"

if (len(sys.argv)!=4):
    print usageMSG
    exit(-1)

SN          = str(sys.argv[1])
ASIC        = int(sys.argv[2])
NumAvgs     = int(sys.argv[3])
interface   = "eth4"


#--------- ETHERNET CONFIGURATION ----------#
addr_fpga = '192.168.20.5'
addr_pc = '192.168.20.1'
port_pc = '28672'
port_fpga = '24576'
syncwd="000000010253594e4300000000" # must be send before every command string
ctrl = linkEth.UDP(addr_fpga, port_fpga, addr_pc, port_pc, interface)
ctrl.open()


#--- BUILD CONFIGURATION COMMANDS ---#
cmdZeroTh = cmdHVoff = syncwd
for asicNo in range (10):
    for Ich in range (16):
        cmdHVoff  += hex( int('C',16)*(2**28) | asicNo*(2**20) | (Ich)*(2**16) | 255 ).split('x')[1]+"AE000100"
        cmdZeroTh += hex( int('B',16)*(2**28) | asicNo*(2**24) | (2*Ich)*(2**16) | 0 ).split('x')[1]+"AE000100"

cmd_FIFO_reset = syncwd + "AF260000"+"AE000100"+"AF260800"+"AE000100"+"AF260000"+"AE000100"+"AF261080"

trig_cmd = syncwd + "AF00FFFF"+"AF00FFFF"+"AF370001"+"AE000001"+"AF370000"+"AE000001"+"AF320001"+"AE000001"+"AF320000" # modified original from AF00FFF+AF00FFFFF / CK


#--- SEND CONFIGURATION COMMANDS ---#
ctrl.send(cmdHVoff)
time.sleep(0.1)
ctrl.send(cmdZeroTh)
time.sleep(0.1)

ped_file = "data/%s/pedestals.root" % (SN)
if not (os.path.isdir("data/%s/" % SN)):
    os.system("mkdir -p data/%s/calib/HVandTH" % SN) # make deepest subdir with parents
    os.system("sudo chown -R testbench2:testbench2 data/") # recursively change ownership
print("Removing pedestal file if it already exists:")
os.system("rm -i " + ped_file)

f = open('temp/ASICdata.dat','ab') #a=append, b=binary
for n in range(NumAvgs):
    print "\nCollecting Data [%d/%d]" % (n+1, NumAvgs)
    cmd_pedConfig = syncwd
    cmd_pedConfig += hex(int('AF330000',16) | 2**ASIC).split('x')[1] +"AE000100"#set asic number
    cmd_pedConfig += "AF3E8000"+"AE000100"+"AF250000"+"AE000100"#disable ext. trig
    cmd_pedConfig += hex(int('AF270000',16) | 0*(2**15) | 2**ASIC ).split('x')[1] +"AE000100"
    cmd_pedConfig += "AF360000"+"AE000100"#  set win offset to 0
    cmd_pedConfig += hex(int('AF260000',16) | 3*(2**12) | 0*2**7 ).split('x')[1]+"AE000100"#pedsub mode: (13 downto 12)="11" is waveform only (no subtraction), bit (7)="0" is peak mode
    cmd_pedConfig += "AF4A0000"+"AE000100"+"AF4F0000"#set outmode to "waveforms" & trigBit search range to +/- 0 windows & ext. trig. bit format to zero.
    ctrl.send(cmd_pedConfig)
    infostr = "ASIC "+str(ASIC)+": "
    sys.stdout.write(infostr)
    time.sleep(0.1)
    #ctrl.send(cmd_FIFO_reset)
    #time.sleep(0.1)
    for i in range(0, 128):
        ctrl.send(syncwd+hex(int('AF3E0000',16) | 2**15 | (i*4)%512 ).split('x')[1]) #set win start
        time.sleep(0.01)
        ctrl.send(trig_cmd)
        rcv = ctrl.receive(20000)# rcv is string of Hex
        time.sleep(0.001)
        if (i%10==0): sys.stdout.write('.')
        sys.stdout.flush()
        rcv = linkEth.hexToBin(rcv)
        f.write(rcv)
    print
    time.sleep(0.1)
f.close()

#---- RUN DATA-PARSING EXECUTABLE ----#
print("\nPARSING EXECUTABLE")
os.system("./bin/tx_ethparse1_ck temp/ASICdata.dat temp/ASICsamps.txt temp/ASICtrigbits.txt 0")
time.sleep(0.1)

#---- WRITE DATA INTO TEMP ROOT FILE ----#
print("\nROOT MACRO")
ROOT.MakePedestalTTree("temp/ASICsamps.txt")
time.sleep(0.1)
os.system("chown -R testbench2:testbench2 temp/pedTemp.root")
os.system("chmod g+w temp/pedTemp.root")
os.system("sudo rm temp/ASIC*")

#---- READ TEMP ROOT FILE, AVERAGE PED VALUES, & WRITE TO FINAL ROOT FILE ----#
ROOT.AveragePedestalTTree(ped_file)
time.sleep(0.1)
os.system("sudo rm temp/pedTemp.root")
os.system("chown -R testbench2:testbench2 " + ped_file)
os.system("chmod g+w " + ped_file)
#next two lines are for sanity check
#ROOT.MakePedSubtractedTTree(waveformOutfile, pedfile, ped_file)
#ROOT.PlotAllWaveforms(ped_file)
os.system("mpg123 lib/R2D2-yeah.mp3")
print "\nPedestal data written in:   %s\n\n\n" % ped_file
ctrl.close()
