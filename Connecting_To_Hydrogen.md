#connecting Drumroll to Hydrogen via alsa midi

# Using Drumroll to send midi to Hydrogen #

Drumroll can be configured to send MIDI events to the Hydrogen advanced drum machine.

# Steps #

Start Hydrogen with alsa audio driver

  * hydrogen -d alsa

Run drumroll with --autoconnect-hydrogen and --nosound options

  * drumroll --autoconnect-hydrogen --nosound