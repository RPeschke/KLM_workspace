#!/usr/bin/env python

#tx_ethtrigscan1 <interface>

import sys
import time
import os
#SCRIPTPATH = os.path.dirname(__file__)
#sys.path.append( SCRIPTPATH+'/lib/' )
import linkEth

syncwd="000000010253594e4300000000" # must be send before every command string

usageMSG="Usage: readRegScrod <interface> <Reg No>";
if len(sys.argv)!=3:
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

ctrl.open()

regNo=int(sys.argv[2],10)

print "Reg= "+str(regNo)+ "	Val= "+str(ctrl.readReg(int(sys.argv[2],10)));

ctrl.close()
