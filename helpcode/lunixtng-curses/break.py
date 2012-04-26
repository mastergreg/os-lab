#!/usr/bin/env python
# -*- coding: utf-8
#
#* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
# File Name : break.py
# Creation Date : 26-04-2012
# Last Modified : Thu 26 Apr 2012 06:20:57 PM EEST
# Created By : Greg Liras <gregliras@gmail.com>
#_._._._._._._._._._._._._._._._._._._._._.*/

import os

def main():
    f = open("/dev/lunix0-batt","r")
    p = os.fork()
    print "pi ",p
    while 1:
        print "pid",p ,"data:",f.read(4)



if __name__=="__main__":
    main()

