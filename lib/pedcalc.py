#!/usr/bin/env python
'''
OVERVIEW:
Test script for sending UDP commands.

AUTHORS:
Bronson Edralin <bedralin@hawaii.edu> and  Isar Mostafanezhad
University of Hawaii at Manoa
Instrumentation Development Lab (IDLab), WAT214
'''


import sys
import os
SCRIPTPATH = os.path.dirname(__file__)
#sys.path.append( SCRIPTPATH+'/lib/' )
import linkEth
import time

usageMSG="Usage: pedcalc <interface> <ASIC no>\n This command will calculate the pedestals for the ASIC of interest- will not touch ASIC parameters"
if (len(sys.argv) != 3):
	print usageMSG
	exit(-1)

asicNo=int(sys.argv[2])

if (asicNo < 0 or asicNo > 9):
	print usageMSG
	exit(-1)


# Ethernet Configuration
addr_fpga = '192.168.20.5'
addr_pc = '192.168.20.1'
port_pc = '28672'
port_fpga = '24576'
interface = sys.argv[1]


# Make UDP class for receiving/sending UDP Packets
ctrl = linkEth.UDP(addr_fpga, port_fpga, addr_pc, port_pc, interface)

cmd_en =hex((int('AF290000',16) | 2**15 | 2**asicNo  )).split('x')[1] #enable ped calc
cmd_dis=hex((int('AF290000',16)         | 2**asicNo  )).split('x')[1] #disable ped calc

ctrl.open()
syncwd="000000010253594e4300000000"

cmd_pre=""+\
"01234567"+"AE000100"+"AF000000"+"AE000100"+"AF008000"+"AE000100"+\
"AE008000"+"AE000100"+"AE000100"+"AF00FFFF"+"AE000100"+"AE000100"+\
"AF008000"+"AE000100"+"AF050080"+"AE000100"+"AF060140"+"AE000100"


cmd_pedcalc=""+\
"AF4D0000"+"AE000100"+"AF140000"+"AE000100"+"AF1E0000"+"AE000100"+\
"AF320000"+"AE000100"+"AF2C0000"+"AE000100"+"AF2D0001"+"AE000100"+\
"AF2D0000"+"AE000100"+"AF3303FF"+"AE000100"+"AF340000"+"AE000100"+\
"AF350000"+"AE000100"+"AF360000"+"AE000100"+"AF370001"+"AE000100"+\
"AF370000"+"AE000100"+"AF380000"+"AE000100"+"AF390004"+"AE000100"+\
"AF3A0000"+"AE000100"+"AF4803FF"+"AE000100"+"AF3D0F00"+"AE000100"+\
"AF260000"+"AE000100"+"AF260800"+"AE000100"+"AF260000"+"AE000100"+\
"AF0B8001"+"AE000100"+"AF0A0000"+"AE000100"+"AF0A0000"+"AE000100"+\
"AF0A0000"+"AE000100"+"AF260000"+"AE000100"+"AF264000"+"AE000100"+\
"AF26C000"+"AE000100"+"AF264000"+"AE000100"+"AF270000"+"AE000100"+\
"AF2AFE00"+"AE000100"+ cmd_dis + "AE000100"+ cmd_en +  "AE00FFFF"+\
cmd_dis   +"AE00FFFF"+"AE00FFFF"+"AE00FFFF"+"AE00FFFF"+"AE00FFFF"+\
"AE00FFFF"+"AE00FFFF"+"AE00FFFF"+"AE00FFFF"
#note: AF264008: 2**14 to enable, 2**NAVG (3 downto 0) # DOES NOT WORK. ALL ADC COUNTS TOO HIGH (AROUND 1350 AFTER USUAL SUBTRACTION OF 3400 COUNTS!)

cmd_post=""+\
"01234567"+"AE000100"+"AF000000"+"AE000100"+"AF008000"+"AE000100"+\
"AE008000"+"AE000100"+"AE000100"+"AF00FFFF"+"AE000100"+"AE000100"+\
"AF008000"+"AE000100"+"AF050080"+"AE000100"+"AF060140"+"AE000100"+\
"AF140000"+"AE000100"+"AF1E0000"+"AE000100"+"AF1F0000"+"AE000100"+\
"AF320000"+"AE000100"+"AF2C0000"+"AE000100"+"AF2D0001"+"AE000100"+\
"AF2D0000"+"AE000100"+"AF330100"+"AE000100"+"AF340000"+"AE000100"+\
"AF350000"+"AE000100"+"AF360003"+"AE000100"+"AF370001"+"AE000100"+\
"AF370000"+"AE000100"+"AF380000"+"AE000100"+"AF390004"+"AE000100"+\
"AF3A0000"+"AE000100"+"AF4803FF"+"AE000100"+"AF3D0F00"+"AE000100"+\
"AF260000"+"AE000100"+"AF260800"+"AE000100"+"AF260000"+"AE000100"+\
"AF261080"+"AE000100"+"AF25C000"+"AE000100"+"AF4B0000"+"AE000100"+\
"AF4C0003"+"AE000100"+"AF270000"+"AE000100"+"AF0B8001"+"AE000100"+\
"AF0A0000"+"AE000100"+"AF0A0000"+"AE000100"+"AF0A0000"+"AE000100"+\
"AF3E0000"+"AE000100"+"AF460000"+"AE000100"+"AF470000"+"AE000100"+\
"AF470001"+"AE000100"+"AF470000"+"AE000100"+"AF460001"+"AE000100"+\
"AF270000"+"AE000100"+"AF4D0450"+"AE000100"+"AF4DC450"+"AE000100"+\
"AF008D0E"+"AE000100"

#set HV off, all channels, current asic
cmd_HVoff=syncwd
for Iasic in range (0,10):
    if (Iasic != asicNo): continue
    for Ich in range (0,16):
        cmd_HVoff = cmd_HVoff + hex(int('C',16)*(2**28) | Iasic*(2**20) | (Ich)*(2**16) | 255).split('x')[1]

#ctrl.KLMprint(cmd_HVoff, "HV off packet")
ctrl.send(cmd_HVoff)
time.sleep(0.1)

#ctrl.KLMprint(cmd_pre, "pre-pedcalc pkt.")
ctrl.send(syncwd+cmd_pre)
time.sleep(.1)

#ctrl.KLMprint(cmd_pedcalc, "pedcalc packet")
ctrl.send(syncwd+cmd_pedcalc)
time.sleep(.9)

#ctrl.KLMprint(cmd_post, "post-pedcalc pkt.")
ctrl.send(syncwd+cmd_post)
time.sleep(.1)

ctrl.close()
