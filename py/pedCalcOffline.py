#!/usr/bin/env python
import sys
import time
import os
sys.path.append('/home/testbench2/root_6_08/lib')
import ROOT
ROOT.gROOT.LoadMacro("rootMacros/MakePedestalTTree.cxx")
SCRIPTPATH = os.path.dirname(__file__)
sys.path.append( SCRIPTPATH+'/lib/' )
import linkEth
#sudo ./pedCalcOffline.py eth4 KLMS_0173 74p43 100
#---------------- USAGE ----------------#
usageMSG="MultiASIC take data record usage:\n"+\
"./pedCalcOffilne.py "+\
    "<Interface> "+\
    "<S/N> "+\
    "<HV> "+\
    "<ASIC> "+\
    "<NoAvg>\n"+\
"Where:\n"+\
    "\t<Interface> = (e.g.) eth4\n"+\
    "\t<S/N> = KLMS_0XXX\n"+\
    "\t<HV> = (e.g.) 74p9\n"+\
    "\t<ASIC> = integer in range [0, 9]\n"

if (len(sys.argv)!=6):
    print usageMSG
    exit(-1)

interface   = str(sys.argv[1])
SN          = str(sys.argv[2])
rawHV       = str(sys.argv[3])
asicNo      = int(sys.argv[4])
NoAvg       = int(sys.argv[5])

if asicNo<0 or asicNo>9:
    print usageMSG
    exit(-1)


#--------- ETHERNET CONFIGURATION ----------#
addr_fpga = '192.168.20.5'
addr_pc = '192.168.20.1'
port_pc = '28672'
port_fpga = '24576'
syncwd="000000010253594e4300000000" # must be send before every command string
ctrl = linkEth.UDP(addr_fpga, port_fpga, addr_pc, port_pc, interface)
ctrl.open()


#--- CONFIGURE SCROD(?) & SET TRIG. THRESHOLD AND HV TO NULL ---#
cmdZeroTh = cmdHVoff = syncwd
for Ich in range (0,16):
    cmdHVoff  += hex( int('C',16)*(2**28) | asicNo*(2**20) | (Ich)*(2**16) | 255 ).split('x')[1]
    cmdZeroTh += hex( int('B',16)*(2**28) | asicNo*(2**24) | (2*Ich)*(2**16) | 0 ).split('x')[1]+"AE000100"


cmd_pedConfig = syncwd + "AE001000"+"AF250000"+"AE000100"#disable ext. trig
cmd_pedConfig += "AF360003"+"AE000100"#  set win offset to 3
cmd_pedConfig += hex(int('AF330000',16) | 2**asicNo).split('x')[1] +"AE000100"#set asic number
cmd_pedConfig += hex(int('AF260000',16) | 3*(2**12) | 0*2**7 ).split('x')[1]+"AE000100"#pedsub mode: (13 downto 12)="11" is waveform only (no subtraction), bit (7)="0" is peak mode
cmd_pedConfig += "AF4A0000"+"AE000100"+"AF4F0000"#set outmode to "waveforms" & trigBit search range to +/- 0 windows & ext. trig. bit format to zero.

trig_cmd = syncwd + "AF00FFFF"+"AF00FFFF"+"AF370001"+"AE000001"+"AF370000"+"AE000001"+"AF320001"+"AE000001"+"AF320000" # modified original from AF00FFF+AF00FFFFF / CK

ctrl.send(cmdHVoff)
time.sleep(0.1)
ctrl.send(cmdZeroTh)
time.sleep(0.1)
ctrl.send(cmd_pedConfig)
time.sleep(0.1)


buffSize = 20000

for n in range(NoAvg):
    print("\nDATA COLLECTION")
    time.sleep(0.1)
    f = open('tempASICdata.dat','ab') #a=append, b=binary

    for i in range(0, 128):
        ctrl.send(syncwd+hex(int('AF3E0000',16) | 2**15 | (i*4+3)%512 ).split('x')[1]) #set win start
        time.sleep(0.01)
        ctrl.send(trig_cmd)
        rcv = ctrl.receive(buffSize)# rcv is string of Hex
        time.sleep(0.001)
        if ((i%80)==0 and i!=0): print
        sys.stdout.write('.')
        sys.stdout.flush()
        rcv = linkEth.hexToBin(rcv)
        f.write(rcv)
    print
    f.close()
    time.sleep(0.1)

    #---- RUN DATA-PARSING EXECUTABLE ----#
    print("\nPARSING EXECUTABLE")
    os.system("./bin/tx_ethparse1_ck tempASICdata.dat tempASICsamps.txt tempASICtrigbits.txt 0")
    time.sleep(0.1)

    pedfile = "calib/" + SN + "_ASIC"+str(asicNo)+"_pedfile.root"
    print("\nROOT MACRO")
    if (n==0):
        ROOT.MakeFirstPedestalTTree("tempASICsamps.txt", pedfile, float(NoAvg))
        time.sleep(0.1)
    else:
        ROOT.MakeSubsequentPedestalTTree("tempASICsamps.txt", pedfile, float(NoAvg))
        time.sleep(0.1)
    os.system("chown -R testbench2:testbench2 " + pedfile)
    os.system("sudo rm tempASIC*")
    #next two lines are for sanity check
    #ROOT.MakePedSubtractedTTree(waveformOutfile, pedfile, root_filename)
    #ROOT.PlotAllWaveforms(root_filename)

print "\nPedestal data written in:   %s\n\n\n" % pedfile
ctrl.close()
