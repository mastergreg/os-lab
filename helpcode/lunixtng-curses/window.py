#!/usr/bin/env python
# -*- coding: utf-8
#
#* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
# File Name : window.py
# Creation Date : 10-04-2012
# Last Modified : Tue 10 Apr 2012 12:27:09 PM EEST
# Created By : Greg Liras <gregliras@gmail.com>
#_._._._._._._._._._._._._._._._._._._._._.*/
import curses
from gatherer import gatherer
from time improt sleep

def main():
    bat0   = getherer("/dev/lunix0-bat")
    light0 = getherer("/dev/lunix0-light")
    temp0  = getherer("/dev/lunix0-temp")
    bat1   = getherer("/dev/lunix1-bat")
    light1 = getherer("/dev/lunix1-light")
    temp1  = getherer("/dev/lunix1-temp")
    bat0.start()
    light0.start()
    temp.start()
    bat1.start()
    light1.start()
    temp1.start()

    myscreen = curses.initscr()

    while True:
        myscreen.border(0)
        myscreen.addstr(2, 5, "/dev/lunix0-bat: %s"%bat0.getData())
        myscreen.addstr(3, 5, "/dev/lunix0-light: %s"%light0.getData())
        myscreen.addstr(4, 5, "/dev/lunix0-temp: %s"%temp0.getData())
        myscreen.addstr(5, 5, "/dev/lunix1-bat: %s"%bat1.getData())
        myscreen.addstr(6, 5, "/dev/lunix1-light: %s"%light1.getData())
        myscreen.addstr(7, 5, "/dev/lunix1-temp: %s"%temp1.getData())
        myscreen.refresh()   
        sleep(1)

    curses.endwin()

if __name__=="__main__":
    main()

