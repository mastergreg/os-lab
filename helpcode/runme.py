#!/usr/bin/env python
# -*- coding: utf-8
#
#* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
# File Name : runme.py
# Creation Date : 05-04-2012
# Last Modified : Thu 05 Apr 2012 05:16:32 PM EEST
# Created By : Greg Liras <gregliras@gmail.com>
#_._._._._._._._._._._._._._._._._._._._._.*/

from time import sleep
def main():
    f = open("data","rb")
    while True:
        f.seek(0)
        for i in f:
            print i
            sleep(1)
    f.close()


if __name__=="__main__":
    main()

