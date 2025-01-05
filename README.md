Linux E-STIM 2B Command Line Interface
=================

E-STIM 2B Command line interface for Linux written in C.

Note: This is the initial alpha code release and bugs are expected and planned enhancements are coming soon.

Authors
-------

This program is written and maintained by AlexRope <https://fetlife.com/users/4887759>

Bugs
--------

I welcome bug reports, fixes, and other improvements.

Please report bugs via the [github issue tracker](https://github.com/AlexRope/linux_e-stim_2b_cli/issues).

Master [git repository](https://github.com/AlexRope/linux_e-stim_2b_cli):

   `git clone https://github.com/AlexRope/linux_e-stim_2b_cli.git`

Tested E-STIM 2B Firmware Versions
--------
E-STIM 2B v2.105

Build From Source
=================

Linux
--------

1. `git clone https://github.com/AlexRope/linux_e-stim_2b_cli.git`
2. `cd linux_e-stim_2b_cli`
3. `make`

Usage
=================

`usage: ./estim {-h} {-v} {-k} {-r} -a CHANNEL_A_INTENSITY -b CHANNEL_B_INTENSITY -f FEELING_LEVEL -d DUTY_CYCLE -m MODE -p POWER -j JOINED`

 `-h Print this help message`

 `-v Debug logging`

 `-k Kill Channel A and B output.`

 `-r Reset E-Stim to default startup settings.`

 `-a CHANNEL_A_INTENSITY Set channel A intensity (values: 0 - 100)`

 `-b CHANNEL_B_INTENSITY Set channel B intensity (values: 0 - 100)`

 `-f FEELING_LEVEL       Set feeling level (values: 2 - 100)`

 `-d DUTY_CYCLE          Set duty cycle (values: 2 - 100)`

 `-m MODE                Set Mode`

 `-p POWER               Set Power (values: 0 or 1)`

 `-j JOINED              Set Joined (values: 0 or 1)`
