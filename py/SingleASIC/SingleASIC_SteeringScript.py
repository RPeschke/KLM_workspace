#!/usr/bin/env python
import sys
import time
t0 = time.time()
import os
sys.path.append('/home/testbench2/root_6_08/lib')
import ROOT
################################################################################
##                 --- Single ASIC Waveform Analysis ---
##      This script is a steering module which has the sole function of running
##  root scripts. The input data should be waiting in temp/waveformSamples.txt.
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
#e.g./: ./py/SingleASIC_WaveformAnalysis.py KLMS_0173 74p57
usageMSG="MultiASIC take data record usage:\n"+\
"./py/SingleASIC_WaveformAnalysis.py <S/N> <HV>\n"+\
"Where:\n"+\
    "<S/N>          = KLMS_0XXX\n"+\
    "<HV>           = (e.g.) 74p57\n"

if (len(sys.argv)!=3):
    print usageMSG
    exit(-1)

SN          = str(sys.argv[1])
rawHV       = str(sys.argv[2])

trig        = 30
kEvts       = 100
numEvts     = 1000*kEvts
ASICmask    = "0000000001"
CHmask      = "0000000000000001"
ascii_input = "temp/waveformSamples.txt"
ped_file    = "data/"+SN+"/pedestals.root"
root_file   = "data/"+SN+"/"+SN+".root"
#floatRawHV = float(rawHV.replace("p","."))

#########################################
##         LOAD ROOT MACROS            ##
#########################################
ROOT.gROOT.LoadMacro("root/SingleCH_MakePedSubtractedTTree.cxx")
#ROOT.gROOT.LoadMacro("root/SingleCH_PlotPhotoElectronPeaks.cxx")
ROOT.gROOT.LoadMacro("root/MultiGaussFit.cxx")
ROOT.gROOT.LoadMacro("root/SingleCH_PlotSomeWaveforms.cxx")
ROOT.gROOT.LoadMacro("root/PlotPE_vs_HV.cxx")
ROOT.gROOT.LoadMacro("root/PlotScalerSteps.cxx")
#os.system("./setup_thisMB.sh")

for HVtrimOffset in range(20,22,2):#,50,2):
    actHV = (74.57 - 5.0*(200-HVtrimOffset)/256.0)
    strHV = ("%.2f"%actHV).replace('.','p')
    #root_tree = "HV71p44_Trig25_100kEvts"
    root_tree = "HV%s_Trig%d_%dkEvts"%(strHV,trig,kEvts)

    ##### Take Data #####
#    os.system("sudo ./py/takeSelfTriggeredData.py %s %s %s %s %d %d %d" % (SN,rawHV,ASICmask,CHmask,HVtrimOffset,trig,numEvts))
    time.sleep(0.1)

    #########################################
    ##          RUN ROOT MACROS            ##
    #########################################
    #####MakePedSubtractedTTree(ascii_input, ped_file, SN, root_dir, root_tree, argvCH, spcSvr=0, tree_desc="TargetX Data"){
#    ROOT.MakePedSubtractedTTree(ascii_input, ped_file, SN,    "ch0", root_tree,      0,        0, "PE Distribution Data")
    #time.sleep(0.1)
    os.system("echo -n > temp/waveformSamples.txt") #clear ascii file

    #####PlotPhotoElectronPeaks(SN, root_dir, root_tree, argASIC, argCH, argHV)
#    ROOT.PlotPhotoElectronPeaks(SN,    "ch0", root_tree,       0,     0, actHV)
    #time.sleep(0.1)

    #####PlotSomeWaveforms(SN, root_dir, root_tree, numPages, argCH)
#    ROOT.PlotSomeWaveforms(SN,    "ch0", root_tree,        5,     0)
    #time.sleep(0.1)

    #####PlotScalerSteps( root_file, root_dir,  root_tree, hvTrim)
    ROOT.PlotScalerSteps("data/S10362-013-050C/S10362-013-050C.root", "TrigScalers", "SFreq", 190 )

#####PlotPhotoElectronPeaks_vs_HV(SN, root_dir, argASIC, argCH)
#ROOT.PlotPhotoElectronPeaks_vs_HV(SN,    "ch0",       0,     0)
