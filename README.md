# KLM_workspace


Workspace for the Hawaii KLM testbench.


In this directory, a KLM motherboard can be configured, calibrated, and set
to take data with the use of two scripts.

1st, the low-voltage power supply should be switched on and voltages should
be measured to verify that they are consistent with the set points indicated
on the silkscreen near each test point.

Now the HV can be switched on. It should be set to 74.9 volts. DON'T TURN ON
HV WITHOUT THE LOW VOLTAGE ON.

Open a terminal and navigate to this directory.

Configure the motherboard by executing the line "source setup_this_MB.sh"
This script will program the FPGA with the current 'bit file,' wait for things
to settle a bit, configure FPGA and TargetX registers, and finally take
pedestal data (in firmware). Upon executing this line, the script will ask for
an ASIC mask (or one can be provided as an argument while launching the script).
Each bit of the mask enables/disables pedestal calculation for true/false
states respectively.  

Upon successful completion of setup_this_MB.sh, the MB is ready for collecting
data.

Data collection, management, analysis, and sensor calibration are all handled
by a steering script, e.g. 'KLM_HI_SteeringScript.py' 


The following is a heirarchy of the "workspace" directory structure:
```
wrkspc/
    bin/                               ~binary executable of C++ parsing script
    Bitfiles/                          ~bit files to load onto FPGA
    data/
        KLMS_0xxx/                     ~Motherboard ID
            calib/                     
                HVandTH/               
                    KLMS_0xxx_HVxxpxx_ASICx.txt
            plots/
        pedestals.root                 ~for offline pedestal subtraction (optional)
        data.root                      ~main data file
    lib/
        linkEth.py                     ~functions for sending/receiving packets to FPGA
        setMBTXConfig.py               
        pedcalc.py                     ~take pedestals with firmware
        readRegScrod.py                ~reads FPGA registers. Used mainly to verify connection.
    old/                               ~outdated shell scripts
    py/                                ~python scripts
        MultiASIC/
        SingleASIC/
            Starting_Values.py         ~set TH base and coarse set HV trim DAC
        old/
        takeSelfTriggeredData.py       ~take TargetX-triggered data
        takeSoftwareTriggeredData.py   ~readout fixed windows
    root/                              ~root macros for storing, analyzing, and plotting data
        MakeMBeventTTree.cxx           ~converts raw (ascii) data to root file and performs
                                        feature  extraction.
    src/                               ~source code for C++ parsing script
    tmp/                               ~temporary files used during data collection and parsing
    README               
    Makefile                           ~for compiling the C++ parsing script
```
