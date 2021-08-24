@ECHO OFF
xps -nw -scr system.tcl
call %XILINX%/../settings64.bat
make -f system.make exporttosdk
