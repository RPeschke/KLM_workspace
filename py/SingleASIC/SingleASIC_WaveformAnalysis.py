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

ascii_input = "temp/waveformSamples.txt"
ped_file    = "data/"+SN+"/pedestals.root"
root_file   = "data/"+SN+"/"+SN+".root"
#floatRawHV = float(rawHV.replace("p","."))

#########################################
##                                     ##
##         LOAD ROOT MACROS            ##
##                                     ##
#########################################
ROOT.gROOT.LoadMacro("root/SingleCH_MakePedSubtractedTTree.cxx")
ROOT.gROOT.LoadMacro("root/SingleCH_PlotPhotoElectronPeaks.cxx")
ROOT.gROOT.LoadMacro("root/SingleCH_PlotSomeWaveforms.cxx")

#########################################
##                                     ##
##          RUN ROOT MACROS            ##
##                                     ##
#########################################
#####MakePedSubtractedTTree(ascii_input, ped_file, SN, root_dir, root_tree,                 argvCH, spcSvr=0, tree_desc="TargetX Data"){
ROOT.MakePedSubtractedTTree(ascii_input, ped_file, SN, "ch0",   "HV180_Trig15_100kEvts",      0,        1, "PE Distribution Data")
time.sleep(0.1)
#os.system("echo -n > temp/waveformSamples.txt") #clear ascii file

#####PlotSomeWaveforms(SN, root_dir, root_tree, numPages, argCH)

ROOT.PlotPhotoElectronPeaks(SN, "ch0", "HV180_Trig15_100kEvts", 0, 0, 71.89)
time.sleep(0.1)

#ROOT.PlotSomeWaveforms(SN, "ch0", "HVlow+100_Trig60_10kEvts", 5, 0)
