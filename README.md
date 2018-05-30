# KLM_workspace


Workspace for the KLM readout software.


In this directory, a KLM motherboard can be configured, calibrated, and set
to take data with the use of two scripts.

1st, the low-voltage power supply should be switched on and voltages should
be measured to verify that they are consistent with the set points indicated
on the silkscreen near each test point.

Now the HV can be switched on. It should be set to 74.9 volts.

Open a terminal and navigate to this directory: wrkspc1_ck

Set up the environment for Impact by executing the line "source impact_bashrc"

Launch Impact by typing "impact" into the command line.

Configure the motherboard by executing the line "source setup_this_MB.sh"

Upon successful completion of setup_this_MB.sh, the MB is ready for collecting
data.


The following is a heirarchy of the "wrkspc" directory structure:
```
wrkspc/
    bin/                ~binary executable of C++ parsing script
    Bitfiles/           ~bit files to load onto FPGA
    data/
        KLMS_0xxx/
            calib/
                HVandTH/
                    KLMS_0xxx_HVxxpxx_ASICx.txt
            plots/
        pedestals.root
        data.root
    lib/
        linkEth.py
        setMBTXConfig.py
        pedcalc.py
        readRegScrod.py
    old/                ~outdated shell scripts
    py/                 ~python take-data-and-run-root-macros scripts
        MultiASIC/
        SingleASIC/
            Starting_Values.py    ~set TH base and coarse set HV trim DAC
        old/
    root/               ~root macros for storing, analyzing, and plotting data
    src/                ~source code for C++ parsing script
    tmp/                ~temporary files used during data collection and parsing
    README
    Makefile            ~for compiling the C++ parsing script
```