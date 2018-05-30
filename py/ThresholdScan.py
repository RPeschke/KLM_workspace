#!/usr/bin/env python
import sys
import time
import os
import csv
sys.path.append('/home/testbench2/root_6_08/lib')
import ROOT
ROOT.gROOT.LoadMacro("rootMacros/MakePedSubtractedTTree.cxx")
ROOT.gROOT.LoadMacro("rootMacros/ThresholdScanPlots.cxx")
SCRIPTPATH = os.path.dirname(__file__)
sys.path.append( SCRIPTPATH+'/lib/' )
import linkEth
t0 = time.time()

#---------------- USAGE ----------------#
usageMSG="Threshold scan usage:\n"+\
"./ThresholdScan.py "+\
    "<S/N> "+\
    "<HV> "+\
    "<ASIC> "+\
"Where:\n"+\
    "<S/N> = KLMS_0XXX\n"+\
    "<HV> = (e.g.) 74p9\n"+\
    "<ASIC> = integer in range [0, 9]\n"

if (len(sys.argv)!=4):
    print usageMSG
    exit(-1)

SN          = str(sys.argv[1])
rawHV       = str(sys.argv[2])
asicNo      = int(sys.argv[3])


if asicNo<0 or asicNo>9:
    print usageMSG
    exit(-1)


interface   = "eth4"
winOffset   = 3





#---- LOOK FOR CALIBRATION VALUES FROM FILE ----#
calib_file_name = "calib/"+SN+"_HV"+rawHV+"_"+"ASIC"+str(asicNo)+".txt"
if not (os.path.isfile(calib_file_name)):  #infile.mode == 'r':
    os.system("sudo ./setThresholdsAndGains.py "+interface+" "+SN+" "+rawHV+" "+str(asicNo))
num_lines = int(os.popen("sed -n '$=' "+calib_file_name).read())
#print "file: %s, num lines: %d" % (calib_file_name, num_lines)
if (num_lines == 15):
    infile = open(calib_file_name, 'r')
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
hvBest = [int(line[1]) for line in fileLines]
print "\nFound threshold base from file:\n%s" % thBase
print "\nFound HV values from file:\n%s\n" % hvBest



#--------- ETHERNET CONFIGURATION ----------#
addr_fpga = '192.168.20.5'
addr_pc = '192.168.20.1'
port_pc = '28672'
port_fpga = '24576'
syncwd="000000010253594e4300000000" # must be send before every command string
ctrl = linkEth.UDP(addr_fpga, port_fpga, addr_pc, port_pc, interface)
#ctrl.open()
# Make UDP class for receiving/sending UDP Packets


#buffSize = 20000
numEvts = 5000;
testThList = [40,43,46,49,52, 55,58,61,64,67, 70];
for testCh in range(0,15):
    testHV = hvBest[testCh] + 28
    actHV = 74.43-5*(testHV/256.)
    prntHV = "%.3F" % actHV
    print "HV: %s" % prntHV
    tch0 = time.time()
    ThIndex = 0
    for THoffset in testThList:
        ThIndex += 1
        print "Setting channel %d threshold to %d" % (testCh, THoffset)
        ctrl.open()
        #------------------------------------------------------------------------------#
        #-------------------SETUP ASIC AND SCROD FOR DATA COLLECTION-------------------#
        #------------------------------------------------------------------------------#
        #----SET THRESHOLD OFFSET TO USER SPECIFIED VALUE----#
        #--- CONFIGURE SCROD(?) & SET TRIG. THRESHOLD AND HV TO NULL ---#
        cmdZeroTh = cmdHVoff = syncwd
        for Ich in range (0,16):
            cmdHVoff  += hex( int('C',16)*(2**28) | asicNo*(2**20) | (Ich)*(2**16) | 255 ).split('x')[1]
            cmdZeroTh += hex( int('B',16)*(2**28) | asicNo*(2**24) | (2*Ich)*(2**16) | 0 ).split('x')[1]+"AE000100"

        cmdASIC_Th = cmdASIC_HV = syncwd
        for chNo in range(15):
            if (chNo == testCh):
                cmdASIC_Th += hex( int('B',16)*(2**28) | asicNo*(2**24) | (2*chNo)*(2**16) | (thBase[chNo] - THoffset) ).split('x')[1]#+"AE000100"
                cmdASIC_HV += hex( int('C',16)*(2**28) | asicNo*(2**20) | (chNo)*(2**16)   | testHV ).split('x')[1]#+"AE000100"
            else:
                cmdASIC_Th += hex( int('B',16)*(2**28) | asicNo*(2**24) | (2*chNo)*(2**16) | 0).split('x')[1]#+"AE000100"
                cmdASIC_HV += hex( int('C',16)*(2**28) | asicNo*(2**20) | (chNo)*(2**16)   | 255 ).split('x')[1]#+"AE000100"
        #ctrl.KLMprint(cmdASIC_Th, "cmdASIC_Th")
        #ctrl.KLMprint(cmdASIC_HV, "cmdASIC_HV")
        time.sleep(0.05)
        ctrl.send(cmdHVoff)
        time.sleep(0.1)
        ctrl.send(cmdASIC_Th)
        time.sleep(0.1)
        ctrl.send(cmdASIC_HV)
        time.sleep(0.1)

        #----DATA-COLLECTION PARAMETERS FOR SCROD REGISTERS----#
        cmd_runConfig = syncwd + "AF250000" + "AE000100"#disable ext. trig
        cmd_runConfig += "AF3E0000" + "AE000100"#set win start---> is set to zero for internal triggering
        cmd_runConfig += hex(int('AF360000',16) | 0              | winOffset       ).split('x')[1] +"AE000100"#set win offset
        cmd_runConfig += hex(int('AF330000',16) | 0              | 2**asicNo       ).split('x')[1] +"AE000100"#set asic number
        cmd_runConfig += hex(int('AF260000',16) | 3*(2**12) | 0*2**7            ).split('x')[1] +"AE000100"#CMDREG_PedDemuxFifoOutputSelect(13 downto 12)-->either wavfifo or pedfifo,CMDREG_PedSubCalcMode(10 downto 7)-->currently only using bit 7: 1 for peaks 2 for troughs, sample offset is 3400 o6 600 respectively
        cmd_runConfig += hex(int('AF270000',16) | 1*(2**15) | 2**asicNo       ).split('x')[1] +"AE000100"#set trig mode & asic trig enable
        cmd_runConfig += hex(int('AF4A0000',16) | 0*(2**8) | 1*2**4 + 2*2**0 ).split('x')[1] +"AE000100"#set outmode and win boundaries for trigger bits: x12 scans 1 win back and two forward
        cmd_runConfig += "AF4F0002" # external trigger bit format


        #------------------------------------------------------------------------------#
        #-------------------------------DATA COLLECTION--------------------------------#
        #------------------------------------------------------------------------------#
        os.system("echo -n > outdir/data.dat") #clean file without removing (prevents ownership change to root in this script)
        f = open('outdir/data.dat','ab') #a=append, b=binary
        time.sleep(0.1)
        ctrl.send(cmd_runConfig)
        #ctrl.KLMprint(cmd_runConfig, "run_Config")
        time.sleep(0.1)

        print "Taking ASIC_%d data . . ." % asicNo
        print "each dot represents 10 event(s)"
        tdata0 = time.time() # for monitoring hit rate
        for i in range(0, numEvts):
            rcv = ctrl.receive(20000)# rcv is string of Hex, buffSize is 20000
            if (i!=0 and i%(800)==0):
                sys.stdout.write("<--%d\n" % i)
                sys.stdout.flush()
            if ((i%10)==0):
                sys.stdout.write('.')
                sys.stdout.flush()
            rcv = linkEth.hexToBin(rcv)
            f.write(rcv) # write received binary data into file
        EvtRate = numEvts/float(time.time()-tdata0)
        print "\nOverall hit rate was %.2f Hz\n" % EvtRate

        f.close()
        ctrl.close()
        time.sleep(0.1)


        #------------------------------------------------------------------------------#
        #---------------------CALL ON PARSING AND PLOTTING SCRIPTS---------------------#
        #------------------------------------------------------------------------------#

        #---- SETUP FILES FOR READING/WRITING ----#
        uniqueID = SN + "_thScan_HV" + rawHV
        plotTitle = SN + "_ch" + str(testCh) + "_th" + str(THoffset) + "_HV" + prntHV
        working_dir = "outdir/thScan/"
        #os.system("mkdir -p " + working_dir + " && chmod g+w " + working_dir)
        #os.system("chown -R testbench2:testbench2 " + working_dir)
        root_filename = working_dir + uniqueID + ".root"
        waveformOutfile = working_dir + "waveformSamples.txt"
        trigBitOutfile = working_dir + "triggerBits.txt"
        pedfile = "calib/"+SN+"_ASIC"+str(asicNo)+"_pedfile.root"


        #---- RUN DATA-PARSING EXECUTABLE ----#
        print("\nParsing ASIC_" + str(asicNo) + " data . . .")
        os.system("./bin/tx_ethparse1_ck outdir/data.dat " + waveformOutfile + " " + trigBitOutfile + " 0")
        time.sleep(0.1)
        print "Data written in:   %s\n" % waveformOutfile


        #---- CONVERT DATA TO ROOT TFILE ----#
        print("Writing data into a TTree . . .")
        ROOT.MakePedSubtractedTTree(waveformOutfile, pedfile, root_filename)
        print "Data written in: " + root_filename
    #    print "Data written in:  %s\n" % root_filename
    #    os.system("chmod a+w " + root_filename)
    #    os.system("chown -R testbench2:testbench2 " + root_filename)
        time.sleep(0.1)


        #---- ANALYZE DATA & CREATE PLOTS ----#
        ROOT.Find_xIntercept(root_filename, plotTitle, THoffset, testCh, EvtRate, ThIndex)
        time.sleep(0.1)


        #---- PRINT TIME & CLEAN UP FILES ----#
        runtime = (time.time() - tdata0)/60.
        print "\nTime to take, parse, and plot data was %.2f minutes\n" % runtime
        os.system("rm " + waveformOutfile)
        os.system("rm " + root_filename)

    chTime = float(time.time()-tch0)/60
    print "\nChannel %d took %.2f min\n" % (testCh, chTime)

ROOT.PlotThresholdScanResults()
overallTime = float(time.time()-t0)/60.
print "\nProcess took %.2f min\n" % overallTime
