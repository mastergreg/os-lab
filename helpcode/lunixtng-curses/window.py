#!/usr/bin/env python
# -*- coding: utf-8
#
#* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
# File Name : window.py
# Creation Date : 10-04-2012
# Last Modified : Tue 10 Apr 2012 01:10:36 PM EEST
# Created By : Greg Liras <gregliras@gmail.com>
#_._._._._._._._._._._._._._._._._._._._._.*/
import curses
from gatherer import gatherer
from time import sleep
from signal import signal,SIGINT

def main():
    bat0   = gatherer("/dev/lunix0-bat")
    light0 = gatherer("/dev/lunix0-light")
    temp0  = gatherer("/dev/lunix0-temp")
    bat1   = gatherer("/dev/lunix1-bat")
    light1 = gatherer("/dev/lunix1-light")
    temp1  = gatherer("/dev/lunix1-temp")
    threads=[ bat0, light0, temp0, bat1, light1, temp1 ]
    map( lambda x : x.start,threads )

    myscreen = curses.initscr()
    myscreen.border(0)

    while True:
        for i,j in enumerate(threads):
            myscreen.addstr(i+2, 5, str(j))
        myscreen.refresh()   
        sleep(1)

    curses.endwin()

if __name__=="__main__":
    main()

