#!/usr/bin/env python
import sys
import time
t0 = time.time()
import os
sys.path.append('/home/testbench2/root_6_08/lib')
import ROOT
################################################################################
##      MASS - Multiple Asic Steering Script
##      This script will collect and analyze data taken using the calibration
##  channel (dec ch 15) of each ASIC.
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
##      last modified: 17 Jan. 2018
##
################################################################################

#---------------- USAGE ----------------#
#e.g./: ./MASS_UseCalibBoard.py KLMS_0173 74p57
usageMSG="Usage:\n"+\
"./MASS_UseCalibBoard.py <S/N> <HV>\n"+\
"Where:\n"+\
    "<S/N>          = KLMS_0XXX\n"+\
    "<HV>           = (e.g.) 74p57\n"

if (len(sys.argv)!=3):
    print usageMSG
    exit(-1)

SN          = str(sys.argv[1])
rawHV       = str(sys.argv[2])

trig        = 100
HVtrimOffset = 20
numEvts     = 16
ASICmask    = "0000000001"
Window      = 0
CHmask      = "0000000000000001"
ascii_input = "temp/waveformSamples.txt"
ped_file    = "data/"+SN+"/pedestals.root"
root_file   = "data/"+SN+"/"+SN+".root"
if not (os.path.isdir("data/"+SN)):
    os.system("mkdir -p data/" + SN)
    os.system("chown testbench2:testbench2 data/"+SN)

#floatRawHV = float(rawHV.replace("p","."))

#os.system("./setup_thisMB.sh") ## REPROGRAM EVERYTHING COMMAND

#########################################
##         LOAD ROOT MACROS            ##
#########################################
ROOT.gROOT.LoadMacro("root/MakeMBeventTTree.cxx")
ROOT.gROOT.LoadMacro("root/PlotPedestalStatisticsOneASIC.cxx")
ROOT.gROOT.LoadMacro("root/PlotSomeWaveforms.cxx")
time.sleep(0.1)

#########################################
##         TAKE CALIB DATA             ##
#########################################
####/py/CalibChThresholdScan.py <S/N> <HV> <ASICmask>
#os.system("sudo ./py/CalibChThresholdScan.py KLMS_0173 74p47 0000000001")


#########################################
#####         Take Data             #####
#########################################
#os.system("echo -n > temp/waveformSamples.txt") #clean file without removing (prevents ownership change to root in this script)
#for i in range(128):
#    #os.system("sudo ./py/takeSoftwareTriggeredData.py %s %s %s %d %d" % (SN,rawHV,ASICmask,HVtrimOffset,numEvts))
os.system("sudo ./py/takeSoftwareTriggeredDataFixedWin.py %s %s %s %d %d" % (SN,rawHV,ASICmask,8,numEvts))
#os.system("sudo ./py/takeSelfTriggeredData.py %s %s %s %s %d %d %d" % (SN,rawHV,ASICmask,CHmask,HVtrimOffset,trig,numEvts))
#    time.sleep(0.1)

#########################################
##          Make ROOT TTree            ##
#########################################
####Constructor: MakeMBeventTTree(const char* ascii_input, const char* root_output, const char* TFile_option)
print "writring in %s" % root_file
ROOT.MakeMBeventTTree("temp/waveformSamples.txt", root_file, "RECREATE")
time.sleep(0.1)
##os.system("echo -n > temp/waveformSamples.txt") #clear ascii file
os.system("chown testbench2:testbench2 " + root_file + " && chmod g+w " + root_file)


#########################################
###           Analyze Data            ###
#########################################
#####PlotPedestalStatisticsOneASIC(const char* root_file, const char* out_pdf)
#ROOT.PlotPedestalStatisticsOneASIC(root_file, "data/PedAnalysis/ASIC0_AllWinCh_10kEvt_pedAnalysis.pdf")
#####PlotPhotoElectronPeaks(SN, root_dir, root_tree, argASIC, argCH, argHV)
    #ROOT.PlotPhotoElectronPeaks(SN,    "ch0", root_tree,       0,     0, actHV)
#time.sleep(0.1)
#####PlotSomeWaveforms(const char* root_file, const int argCH)
ROOT.PlotSomeWaveforms(root_file,15)
time.sleep(0.1)
ROOT.PlotSomeWaveforms(root_file,0)
#####PlotScalerSteps( root_file, root_dir,  root_tree, hvTrim)
    #ROOT.PlotScalerSteps("data/S10362-013-050C/S10362-013-050C.root", "TrigScalers", "SFreq", 190 )

#####PlotPhotoElectronPeaks_vs_HV(SN, root_dir, argASIC, argCH)
#ROOT.PlotPhotoElectronPeaks_vs_HV(SN,    "ch0",       0,     0)
