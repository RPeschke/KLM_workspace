#!/usr/bin/env python
import sys
import time
t0 = time.time()
import os
import csv
SCRIPTPATH = os.getcwd()
sys.path.append( SCRIPTPATH+'/lib/' )
import linkEth                      ##
addr_fpga = '192.168.20.5'          ## Ethernet
addr_pc = '192.168.20.1'            ## Link
port_pc = '28672'                   ## Configuration
port_fpga = '24576'                 ##
syncwd="000000010253594e4300000000" ##
################################################################################
##         --- Take Data Using Software Trigger ---
##
##
##
##
##
##
##
##
##      Author: Chris Ketter
##      email:  cketter@hawaii.edu
##      last modified: 16 Jan. 2018
##
################################################################################

#---------------- USAGE ----------------#
#e.g./: sudo ./py/takeSoftwareTriggeredData.py KLMS_0173 74p47 0000000001 0000000000000001 13 15 50000
usageMSG="MultiASIC take data record usage:\n"+\
"sudo ./py/takeSoftwareTriggeredData.py <S/N> <HV> <ASICmask> <HVoffset> <numEvts>\n"+\
"Where:\n"+\
    "<S/N>          = KLMS_0XXX\n"+\
    "<HV>           = (e.g.) 74p47\n"+\
    "<ASICmask>     = (e.g.) 0000000001\n"+\
    "<HVoffset>     = DAC counts from HVlow\n"+\
    "<numEvts>      = (e.g.) 50000\n"

if (len(sys.argv)!=6):
    print usageMSG
    exit(-1)

interface   = "eth4"
SN          = str(sys.argv[1])
rawHV       = str(sys.argv[2])
ASICmask    = int(sys.argv[3],2)
hvOffset    = int(sys.argv[4])
NumEvts     = int(sys.argv[5])

#opMode      = 1     ## pedestal subtracted data
#opMode      = 2     ## pedestals only
opMode      = 3     ## raw data (i.e. pedSub offline)

outMode     = 0     ## entire waveforms
#outMode     = 1     ## feature extracted data only

trgTyp      = 0     ## software trigger (ext trigger)
#trgTyp      = 1     ## hardware trigger (self trigger)

#fxdWin      = 0     ## digitize triggered window (write strobe minus lookback)
fxdWin      = 1

winOffset   = 0     ## lookback parameter

'''print("Looking for calibration file . . .")
#---- LOOK FOR CALIBRATION VALUES ----#
thBase = [0 for ASIC in range(10)]
hvLow  = [0 for ASIC in range(10)]
hvHigh = [0 for ASIC in range(10)]
for ASIC in range (10):
    if (2**ASIC & ASICmask) > 0:
        calib_file = "data/%s/calib/HVandTH/%s_HV%s_ASIC%d.txt" % (SN, SN, rawHV, ASIC)
        if not (os.path.isfile(calib_file)):  #infile.mode == 'r':
            print "Could not find calibration file! Exiting . . ."
            exit(-1)
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
print "Found HV starting values and threshold base values from calibration file.\n"


#---- TRIGGER THRESHOLDS AND HV OFFSETS ----#
cmdZeroTh = cmdHVoff = cmdASIC_HV = cmdASIC_Th = syncwd
print("Unmasked ASICs/Channels:")
for ASIC in range(10):
    for CH in range (15):
        cmdHVoff  += hex( int('C',16)*(2**28) | ASIC*(2**20) | (CH)*(2**16) | 255 ).split('x')[1]
        cmdZeroTh += hex( int('B',16)*(2**28) | ASIC*(2**24) | (2*CH)*(2**16) | 0 ).split('x')[1]
        if ((2**ASIC & ASICmask) > 0) and ((2**CH & CHmask) > 0):
            print("%d/%d\t" % (ASIC,CH))
            cmdASIC_HV += hex( int('C',16)*(2**28) | ASIC*(2**20) | (CH)*(2**16)   | hvLow[ASIC][CH]-hvOffset    ).split('x')[1]
            cmdASIC_Th += hex( int('B',16)*(2**28) | ASIC*(2**24) | (2*CH)*(2**16) | thBase[ASIC][CH]-trigOffset ).split('x')[1]
        else:
            cmdASIC_HV += hex( int('C',16)*(2**28) | ASIC*(2**20) | (CH)*(2**16)   | 255  ).split('x')[1]
            cmdASIC_Th += hex( int('B',16)*(2**28) | ASIC*(2**24) | (2*CH)*(2**16) | 4095 ).split('x')[1]
'''
cmdZeroTh = cmdHVoff = syncwd
for ASIC in range(10):
    for CH in range (16):
        cmdHVoff  += hex( int('C',16)*(2**28) | ASIC*(2**20) | (CH)*(2**16) | 255 ).split('x')[1]
        cmdZeroTh += hex( int('B',16)*(2**28) | ASIC*(2**24) | (2*CH)*(2**16) | 0 ).split('x')[1]

#---- DATA-COLLECTION PARAMETERS FOR SCROD REGISTERS ----#
cmd_runConfig  = syncwd + "AF3E8000"+"AE000100"+"AF250000"+"AE000100"#disable ext. trig
#cmd_runConfig += "AF3E0000" + "AE000100"#set win start---> is set to zero for internal triggering
cmd_runConfig += hex(int('AF3E0000',16) | fxdWin*(2**15) | fxdWin*0        ).split('x')[1] +"AE000100"#set win start---> is set to zero for internal triggering
cmd_runConfig += hex(int('AF360000',16) | 0              | winOffset       ).split('x')[1] +"AE000100"#set win offset
cmd_runConfig += hex(int('AF330000',16) | 2**25           | ASICmask        ).split('x')[1] +"AE000100"#set asic number
cmd_runConfig += hex(int('AF260000',16) | opMode*(2**12) | 2**7            ).split('x')[1] +"AE000100"#CMDREG_PedDemuxFifoOutputSelect(13 downto 12)-->either wavfifo or pedfifo,CMDREG_PedSubCalcMode(10 downto 7)-->currently only using bit 7: 1 for peaks 2 for troughs, sample offset is 3400 o6 600 respectively
cmd_runConfig += hex(int('AF270000',16) | trgTyp*(2**15) | ASICmask        ).split('x')[1] +"AE000100"#set trig mode & asic trig enable
cmd_runConfig += hex(int('AF4A0000',16) | outMode*(2**8) | 1*2**4 + 2*2**0 ).split('x')[1] +"AE000100"#set outmode and win boundaries for trigger bits: x12 scans 1 win back and two forward
cmd_runConfig += "AF4F0002" # external trigger bit format

trig_cmd = syncwd + "AF00FFFF"+"AF00FFFF"+"AF370001"+"AE000001"+"AF370000"+"AE000001"+"AF320001"+"AE000001"+"AF320000" # modified original from AF00FFF+AF00FFFFF / CK

#---- OPEN ETHERNET LINK AND PROGRAM SCROD REGISTERS FOR DATA COLLECTION ----#
ctrl = linkEth.UDP(addr_fpga, port_fpga, addr_pc, port_pc, interface)
ctrl.open()
time.sleep(0.1)
#ctrl.KLMprint(cmdASIC_HV, "cmdASIC_HV")
#ctrl.send(cmdASIC_HV)
ctrl.send(cmdHVoff)
time.sleep(0.2)
#ctrl.KLMprint(cmdASIC_Th, "cmdASIC_Th")
#ctrl.send(cmdASIC_Th)
ctrl.send(cmdZeroTh)
time.sleep(0.2)
#ctrl.KLMprint(cmd_runConfig, "run config")
ctrl.send(cmd_runConfig)
time.sleep(0.2)


#------------------------------------------------------------------------------#
#-------------------------------DATA COLLECTION--------------------------------#
#------------------------------------------------------------------------------#
print "Taking %s events . . ." % (NumEvts)
os.system("echo -n > temp/data.dat") #clean file without removing (prevents ownership change to root in this script)
f = open('temp/data.dat','ab') #a=append, b=binary
time.sleep(0.1)
t1 = time.time()
for i in range(1, NumEvts+1):
    ctrl.send(trig_cmd)
    rcv = ctrl.receive(20000)# rcv is string of Hex
    time.sleep(0.01)
    if ((i%1)==0):
        sys.stdout.write('.')
        sys.stdout.flush()
    if (i%80)==0 or i==(NumEvts):
        sys.stdout.write("<--%d\n" % i)
        sys.stdout.flush()
    rcv = linkEth.hexToBin(rcv)
    f.write(rcv) # write received binary data into file
t2 = time.time()
EvtRate = NumEvts/float(t2-t1)
print "\nOverall hit rate was %.2f Hz" % EvtRate
ctrl.send(syncwd + "AF270000" + "AE000100")
time.sleep(0.2)
f.close()
#ctrl.send(cmdHVoff)  # No sense in leaving it cranked up anymore
time.sleep(0.2)
#ctrl.send(cmdZeroTh)
time.sleep(0.2)
ctrl.close()
time.sleep(0.1)


#---- RUN DATA-PARSING EXECUTABLE ----#
print "\nParsing %s Data" % SN
os.system("echo -n > temp/waveformSamples.txt") #clean file without removing (prevents ownership change to root in this script)
os.system("./bin/tx_ethparse1_ck temp/data.dat temp/waveformSamples.txt temp/triggerBits.txt " + str(outMode))
os.system("echo -n > temp/data.dat") #clean binary file again to save disk space!
time.sleep(0.1)
print "\nData parsed."

totTime = (time.time() - t0)/60.
print "Overall process took %.2f min" % totTime
print "Data written in temp/waveformSamples.txt"
