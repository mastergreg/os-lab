#!/usr/bin/env python
# -*- coding: utf-8
#
#* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
# File Name : test.py
# Creation Date : 10-04-2012
# Last Modified : Tue 10 Apr 2012 01:26:56 PM EEST
# Created By : Greg Liras <gregliras@gmail.com>
#_._._._._._._._._._._._._._._._._._._._._.*/

import gatherer
from time import sleep

def main():
    a = gatherer.gatherer("/dev/lunix0-batt")
    a.start()
    while True:
        print a
        sleep(1)




if __name__=="__main__":
    main()

