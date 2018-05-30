#!/bin/bash
echo
echo
echo "If you see abnormal behaviour, please use 'ps -au'"
echo "command then kill the related python processes using"
echo "'sudo kill -9 PID' the start again."
echo
echo
echo
#echo "ENTER the interface type:"
#echo "e.g. >>>eth4"
#read -p ">>>" InterfaceType
#echo
InterfaceType=eth4
echo "ENTER ASIC mask:"
echo "e.g. >>>0100000001  to setup ASIC_0 and ASIC_8"
echo
read -p " >>>" binMask
#binMask=0000000000
ASICmask=$(bc<<<"obase=10;ibase=2;$binMask")
#echo
echo "Reprogramming FPGA..."
sudo /bin/bash lib/wrkspc_ck_progFPGA.sh
echo "Waiting 30 seconds..."
sleep 30
echo "Reading SCROD register 0"
sudo ./lib/readRegScrod.py $InterfaceType 0
sleep .1
echo
echo "Setting ASIC and DAC configs..."
sudo ./lib/setMBTXConfig.py $InterfaceType
sleep .1
echo "                               ...Set Config-done."
echo "Reading SCROD register 0."
sudo ./lib/readRegScrod.py $InterfaceType 0
sleep .1
echo
echo "Now calculating pedestals:"
for i in {0..9}
do
	if [ $(($(bc<<<"2^$i")&$ASICmask)) -gt 0 ]
	then
		echo "Calculating pedestals for ASIC $i ..."
		sudo ./lib/pedcalc.py $InterfaceType $i
		sleep .1
	else
		echo "Skipping ASIC $i....................."
		sleep .05
	fi
done
sleep 1
echo "                                    ......Pedcalc done"
echo
echo "KLM Motherboard has been configured."
echo "Proceed to set gains and thresholds before taking data."
echo
echo
