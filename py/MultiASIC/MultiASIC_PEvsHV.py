#!/usr/bin/env python
import sys
import time
import os
import csv
sys.path.append('/home/testbench2/root_6_08/lib')
import ROOT
ROOT.gROOT.LoadMacro("rootMacros/MakeCalibTTrees.cxx")
ROOT.gROOT.LoadMacro("rootMacros/PlotPartialSumDistributions.cxx")
ROOT.gROOT.LoadMacro("rootMacros/PlotPhotoElectronPeaks_vs_HV.cxx")
ROOT.gROOT.LoadMacro("rootMacros/AnalyzeWaveforms.cxx")
SCRIPTPATH = os.path.dirname(__file__)
sys.path.append( SCRIPTPATH+'/lib/' )
import linkEth
t0 = time.time()

#ALL WINDOWS: sudo ./doEverything.py eth4 KLMS0173 75p18 128 0 0 0 3 3 0 0
#SELF TRIGGERING: sudo ./doEverything.py eth4 KLMS0173 75p18 10 1 -1 18 3 3 0 0
#---------------- USAGE ----------------#
usageMSG="MultiASIC take data record usage:\n"+\
"sudo ./MultiASIC_PEvsHV.py <S/N> <HV>\n"+\
"Where:\n"+\
    "<S/N> = KLMS_0XXX\n"+\
    "<HV> = (e.g.) 74p9\n"

if (len(sys.argv)!=3):
    print usageMSG
    exit(-1)

interface   = "eth4"
SN          = str(sys.argv[1])
rawHV       = str(sys.argv[2])
floatRawHV = float(rawHV.replace("p","."))
evtsPerCh   = 150000
trgTyp      = 1
winStart    = -1
winOffset   = 3
#ASICmask      = "0000011111"
ASICmask      = int("0000000001",2)
opMode      = 3
outMode     = 0
trigOffset   = 15

#--------- ETHERNET CONFIGURATION ----------#
addr_fpga = '192.168.20.5'
addr_pc = '192.168.20.1'
port_pc = '28672'
port_fpga = '24576'
syncwd="000000010253594e4300000000" # must be send before every command string


#--- CONFIGURE SCROD(?) & SET TRIG. THRESHOLD AND HV TO NULL ---#
cmdZeroTh = cmdHVoff = syncwd
for ASIC in range(10):
    if (2**ASIC & ASICmask) > 0:
        for i in range (0,16):
            cmdHVoff  += hex( int('C',16)*(2**28) | ASIC*(2**20) | (i)*(2**16) | 255 ).split('x')[1]
            cmdZeroTh += hex( int('B',16)*(2**28) | ASIC*(2**24) | (2*i)*(2**16) | 0 ).split('x')[1]+"AE000100"

#---- LOOK FOR CALIBRATION VALUES ----#
thBase = [0 for ASIC in range(10)]
hvLow  = [0 for ASIC in range(10)]
hvHigh = [0 for ASIC in range(10)]
for ASIC in range (10):
    if (2**ASIC & ASICmask) > 0:
        calib_file = "data/%s/calib/HVandTH/%s_HV%s_ASIC%d.txt" % (SN, SN, rawHV, ASIC)
#        if not (os.path.isfile(calib_file)):  #infile.mode == 'r':
#            os.system("sudo ./setThresholdsAndGains.py %s %s %s %d" % (interface, SN, rawHV, ASIC))
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
#for row in thBase:
#    print(' '.join([str(elem) for elem in row]))
#print "\nFound low HV values from file:"
#for row in hvLow:
#    print(' '.join([str(elem) for elem in row]))
#print "\nFound high HV values from file:"
#for row in hvHigh:
#    print(' '.join([str(elem) for elem in row]))
##    print "\n\nRaw HV = %.3f" % float(rawHV.replace("p","."))


#for testCH in range(15):
for testCH in range(1):
    tCH = time.time()
#    for hvOffset in range(30, 0, -5):
    for hvOffset in range(15,16):
        tHV = time.time()
        #------------------------------------------------------------------------------#
        #-------------------SETUP ASIC AND SCROD FOR DATA COLLECTION-------------------#
        #------------------------------------------------------------------------------#
        #----SET THRESHOLD OFFSET TO USER SPECIFIED VALUE----#

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
                        cmdASIC_Th += hex( int('B',16)*(2**28) | ASIC*(2**24) | (2*ch)*(2**16) | 4095 ).split('x')[1]+"AE000100"

        ctrl = linkEth.UDP(addr_fpga, port_fpga, addr_pc, port_pc, interface)
        ctrl.open()
        #ctrl.KLMprint(cmdASIC_Th, "cmdASIC_Th")
        #ctrl.KLMprint(cmdASIC_HV, "cmdASIC_HV")
        time.sleep(0.05)
        ctrl.send(cmdHVoff)
        time.sleep(0.1)
        ctrl.send(cmdZeroTh)
        time.sleep(0.1)
        ctrl.send(cmdASIC_HV)
        time.sleep(0.2)
        ctrl.send(cmdASIC_Th)
        time.sleep(0.2)

        #----DATA-COLLECTION PARAMETERS FOR SCROD REGISTERS----#
        cmd_runConfig  = syncwd + "AF250000" + "AE000100"#disable ext. trig
        cmd_runConfig += "AF3E0000" + "AE000100"#set win start---> is set to zero for internal triggering
        cmd_runConfig += hex(int('AF360000',16) | 0              | winOffset       ).split('x')[1] +"AE000100"#set win offset
        cmd_runConfig += hex(int('AF330000',16) | 0              | ASICmask        ).split('x')[1] +"AE000100"#set asic number
        cmd_runConfig += hex(int('AF260000',16) | opMode*(2**12) | 2**7            ).split('x')[1] +"AE000100"#CMDREG_PedDemuxFifoOutputSelect(13 downto 12)-->either wavfifo or pedfifo,CMDREG_PedSubCalcMode(10 downto 7)-->currently only using bit 7: 1 for peaks 2 for troughs, sample offset is 3400 o6 600 respectively
        cmd_runConfig += hex(int('AF270000',16) | trgTyp*(2**15) | ASICmask        ).split('x')[1] +"AE000100"#set trig mode & asic trig enable
        cmd_runConfig += hex(int('AF4A0000',16) | outMode*(2**8) | 1*2**4 + 2*2**0 ).split('x')[1] +"AE000100"#set outmode and win boundaries for trigger bits: x12 scans 1 win back and two forward
        cmd_runConfig += "AF4F0002" # external trigger bit format
        #ctrl.KLMprint(cmd_runConfig, "run config")

        #------------------------------------------------------------------------------#
        #-------------------------------DATA COLLECTION--------------------------------#
        #------------------------------------------------------------------------------#
        print "\n\nTaking %s events for channel %d. . ." % (evtsPerCh, testCH)
        buffSize = 20000
        os.system("echo -n > temp/data.dat") #clean file without removing (prevents ownership change to root in this script)
        f = open('temp/data.dat','ab') #a=append, b=binary
        time.sleep(0.1)
        ctrl.send(cmd_runConfig)
        #ctrl.KLMprint(cmd_runConfig, "run_Config")
        time.sleep(0.1)
        t1 = time.time()
        for i in range(1, evtsPerCh+1):
            rcv = ctrl.receive(buffSize)# rcv is string of Hex
            time.sleep(0.001)
            if ((i%100)==0):
                sys.stdout.write('.')
                sys.stdout.flush()
            if (i%8000)==0 or i==(evtsPerCh):
                sys.stdout.write("<--%d\n" % i)
                sys.stdout.flush()
            rcv = linkEth.hexToBin(rcv)
            f.write(rcv) # write received binary data into file
        t2 = time.time()
        EvtRate = evtsPerCh/float(t2-t1)
        print "\nOverall hit rate was %.2f Hz" % EvtRate

        f.close()
        ctrl.close()
        time.sleep(0.1)


        #------------------------------------------------------------------------------#
        #---------------------CALL ON PARSING AND PLOTTING SCRIPTS---------------------#
        #------------------------------------------------------------------------------#

        #---- RUN DATA-PARSING EXECUTABLE ----#
        print "\nParsing %s Data" % SN
        os.system("./bin/tx_ethparse1_ck temp/data.dat temp/waveformSamples.txt temp/triggerBits.txt " + str(outMode))
        time.sleep(0.1)
        print "Data parsed."

        #---- RUN ROOT MACROS ----#

        # TTree
        root_file = "data/%s/%s.root" % (SN,SN)
        #print("\nROOT macro MakeCalibTTrees('temp/waveformSamples.txt', %s, 'PEdata', %d, 1):" % (root_file, testCH))
        #ROOT.MakeCalibTTrees("temp/waveformSamples.txt", root_file, "PEdata", testCH, 1)#ascii,file,dir,CH,(opt bool)spcSvr=0
        print("\nROOT macro MakeCalibTTrees('temp/waveformSamples.txt', %s, 'PEdata', %d):" % (root_file, testCH))
        ROOT.MakeCalibTTrees("temp/waveformSamples.txt", root_file, "PEdata", testCH)#, 1)#ascii,file,dir,CH,(opt bool)spcSvr=0
        os.system("chmod a+w " + root_file)
        os.system("chown -R testbench2:testbench2 " + root_file)
        os.system("rm temp/waveformSamples.txt")

        # Photon Distributions
        for ASIC in range(10):
            if (2**ASIC & ASICmask) > 0:
                actHV =  floatRawHV  -  5*(hvLow[ASIC][testCH] - hvOffset)/255.
                dirName = "PEdata/ASIC%d/ch%d" % (ASIC, testCH)
                rootTree = "ch%d_PEdata" % testCH
                print("\nROOT macro PlotPartialSumDistributions(%s, %s, %s, %d, %d, %f):" % (root_file, dirName, rootTree, ASIC, testCH, actHV))
                ROOT.PlotPartialSumDistributions(root_file, dirName, rootTree, ASIC, testCH, actHV)# file,dir,tree,asic,ch,(float)HV
                #print("\nROOT macro AnalyzeWaveforms(%s, %s, %s, %d, %d, %f):" % (root_file, dirName, rootTree, ASIC, testCH, actHV))
                #ROOT.AnalyzeWaveforms(root_file, dirName, rootTree, ASIC, testCH, actHV)# file,dir,tree,asic,ch,(float)HV
                time.sleep(0.5)
        timeHV = (time.time() - tHV)/60.
        print "Test at HV offset %d (all ASICs) finished in %.2f minutes" % (hvOffset, timeHV)
        # <~~~ END HV LOOP

    for ASIC in range(10):
       if (2**ASIC & ASICmask) > 0:
           actHV =  floatRawHV  -  (hvLow[ASIC][testCH] - hvOffset)/256.
           dirName = "PEdata/ASIC%d/ch%d" % (ASIC, testCH)
           print("\nROOT macro PlotPhotoElectronPeaks_vs_HV(%s, %s, %d, %d)" % (root_file, dirName, ASIC, testCH))
           ROOT.PlotPhotoElectronPeaks_vs_HV(root_file, dirName, ASIC, testCH)# file,dir,asic,ch
           time.sleep(0.5)
    timeCH = (time.time() - tCH)/60.
    print "Channel %d (all ASICs) finished in %.2f minutes" % (testCH, timeCH)
    # <~~~ END CH LOOP
totTime = (time.time() - t0)/60.
print "Overall process took %.2f min" % totTime
