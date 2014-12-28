iBuddy
======

Command line program to control ibuddy USB device.
The IBuddy is a small angel like figure that has moveable body, wings and 4 LEDs. It was once conceived to 
signal the presence of buddies on MSN (What??) around 2008. I found an abandoned one and wanted it to move. 
The original software was of no use. But there was some (old) code on-line. 
I wanted a small stand-alone command line tool. This is what came out of that.

Based upon IbuddyLib C# software, PyBuddyLib and LibHID.
Tested on Windows using MingW

Use:

    ibuddy code1 code2 code3 ... code-n

Or read from a file via stdin: 

    ibuddy --interactive < codefile.txt

Each code (on the command line or on a line in the text file) has the following syntax:
 
    [time in msec]RGBHUDCA
 
[time in msec] Waiting time before the code is sent
R   Red LED ON
G   Green LED ON
B   Blue LED ON
H   Heart LED ON
U   Wings UP
D   Wings DOWN
C   Rotate body clockwise
A   Rotate body anti-clockwise
 
Note: all upper-case letters
