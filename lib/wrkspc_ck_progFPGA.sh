#!/bin/bash
LM_LICENSE_FILE=2100@hepflexlm.phys.hawaii.edu
export LM_LICENSE_FILE
source /opt/Xilinx/14.7/ISE_DS/settings64.sh

impact -batch lib/wrkspc_ck_impact.cmd
