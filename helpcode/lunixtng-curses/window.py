#!/usr/bin/env python
# -*- coding: utf-8
#
#* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
# File Name : window.py
# Creation Date : 10-04-2012
# Last Modified : Tue 10 Apr 2012 07:09:12 PM EEST
# Created By : Greg Liras <gregliras@gmail.com>
#_._._._._._._._._._._._._._._._._._._._._.*/
import curses
from sensor import sensor
from time import sleep

threads=[]
def main():
    global threads
    nsensors = 2
    for i in xrange(nsensors):
        threads.append(sensor(i,'batt'))
        threads.append(sensor(i,'light'))
        threads.append(sensor(i,'temp'))
    map( lambda x : x.start(),threads )

    myscreen = curses.initscr()
    myscreen.border(0)

    while True:
        myscreen.refresh()   
        for i,j in enumerate(threads):
            myscreen.addstr(i+2, 2, str(j))
        sleep(1)

    curses.endwin()

if __name__=="__main__":
    global threads
    try:
        main()
    except KeyboardInterrupt:
        curses.endwin()
        exit(0)

