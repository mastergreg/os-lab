#!/usr/bin/env python
# -*- coding: utf-8
#
#* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
# File Name : runme.py
# Creation Date : 05-04-2012
# Last Modified : Fri 13 Apr 2012 10:43:21 PM EEST
# Created By : Greg Liras <gregliras@gmail.com>
#_._._._._._._._._._._._._._._._._._._._._.*/

from time import sleep
def main():
    f = open("data","rb")
    while True:
        i = f.read(8)
        if i == '':
            f.seek(0)
            i = f.read(4)
        print i
        sleep(1)
    f.close()


if __name__=="__main__":
    main()

