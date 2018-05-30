#!/usr/bin/env python
import sys
import time
t0 = time.time()
import os
sys.path.append('/home/testbench2/root_6_08/lib')
import ROOT
################################################################################
##                      KLM Hawaii Steering Script
##
##      This script is meant to simplify data collection for the KLM scintillator
##  testbench in Hawaii. Below are sections for Data Collection Parameters,
##  loading of ROOT macros, running targetX ASIC calibration procedures,
##  collecting targetX data, saving the data to a ROOT tree, and analyzing data.
##  The user can uncomment or add in the scripts they want to run, comment out
##  the scripts they don't need, then run everything by launching this script.
##
##      Author: Chris Ketter
##      email:  cketter@hawaii.edu
##      last modified: 25 May 2018
##
################################################################################

#---------------- USAGE ----------------#
#e.g./: ./KLM_HI_SteeringScript.py KLMS_0173 74p52
usageMSG="Usage:\n"+\
"./KLM_HI_SteeringScript.py <S/N> <HV>\n"+\
"Where:\n"+\
    "<S/N>          = KLMS_0XXX\n"+\
    "<HV>           = (e.g.) 74p52\n"

if (len(sys.argv)!=3):
    print usageMSG
    exit(-1)

SN          = str(sys.argv[1])
strRawHV    = str(sys.argv[2])

#########################################
##     DATA COLLECTION PARAMETERS      ##
#########################################
numEvts      = 150000
trigOffset   = 30
HVtrimOffset = 5
ASICmask     = "0000000001"
Window       = 0
CHmask       = "0000000000000001"
ascii_input  = "temp/waveformSamples.txt"
ped_file     = "data/"+SN+"/pedestals.root"
root_file    = "data/"+SN+"/"+SN+".root"
if not (os.path.isdir("data/"+SN)):
    os.system("mkdir -p data/" + SN)
    os.system("chown testbench2:testbench2 data/"+SN)
floatrawHV = float(strRawHV.replace("p","."))
approxHV = floatrawHV - 5.*169./256. ## fix this later. Should be rawHV - 5*(trimDACvalue)/(256.)
#os.system("./setup_thisMB.sh") ## REPROGRAM EVERYTHING COMMAND

#########################################
##         LOAD ROOT MACROS            ##
#########################################
ROOT.gROOT.LoadMacro("root/MakeMBeventTTree.cxx")
ROOT.gROOT.LoadMacro("root/PlotSomeWaveforms.cxx")
ROOT.gROOT.LoadMacro("root/MultiGaussFit.cxx")
time.sleep(0.1)

#########################################
##         TAKE CALIB DATA             ##
#########################################
####/py/CalibChThresholdScan.py <S/N> <HV> <ASICmask>
os.system("sudo ./py/SingleASIC/SingleASIC_Starting_Values.py KLMS_0173 %s" % strRawHV)
time.sleep(0.1)

#########################################
#####         Take Data             #####
#########################################
#for i in range(128):
#    #os.system("sudo ./py/takeSoftwareTriggeredData.py %s %s %s %d %d" % (SN,strRawHV,ASICmask,HVtrimOffset,numEvts))
#os.system("sudo ./py/takeSoftwareTriggeredDataFixedWin.py %s %s %s %d %d" % (SN,strRawHV,ASICmask,8,numEvts))
os.system("sudo ./py/takeSelfTriggeredData.py %s %s %s %s %d %d %d" % (SN,strRawHV,ASICmask,CHmask,HVtrimOffset,trigOffset,numEvts))
time.sleep(0.1)

#########################################
##          Make ROOT TTree            ##
#########################################
####Constructor: MakeMBeventTTree(const char* ascii_input, const char* root_output, const char* TFile_option)
print "writring in %s" % root_file
ROOT.MakeMBeventTTree("temp/waveformSamples.txt", root_file, "RECREATE")
time.sleep(0.1)
os.system("echo -n > temp/waveformSamples.txt") #clear ascii file
os.system("chown testbench2:testbench2 " + root_file + " && chmod g+w " + root_file)


#########################################
###           Analyze Data            ###
#########################################

#####PlotPhotoElectronPeaks(char* root_file, int ASIC, int CH, float HV)
ROOT.MultiGaussFit(root_file, 0, 0, approxHV)

#####PlotSomeWaveforms(const char* root_file, const int argCH)
#ROOT.PlotSomeWaveforms(root_file,15)
#time.sleep(0.1)
#ROOT.PlotSomeWaveforms(root_file,0)

#####PlotPhotoElectronPeaks_vs_HV(SN, root_dir, argASIC, argCH)
#ROOT.PlotPhotoElectronPeaks_vs_HV(SN,    "ch0",       0,     0)
