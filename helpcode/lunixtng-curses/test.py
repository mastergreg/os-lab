#!/usr/bin/env python
# -*- coding: utf-8
#
#* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
# File Name : test.py
# Creation Date : 10-04-2012
# Last Modified : Tue 10 Apr 2012 12:13:01 PM EEST
# Created By : Greg Liras <gregliras@gmail.com>
#_._._._._._._._._._._._._._._._._._._._._.*/

import gatherer
from time import sleep

def main():
    a = gatherer.gatherer("./gatherer.py")
    a.start()
    while True:
        print a.getData()
        sleep(1)




if __name__=="__main__":
    main()

