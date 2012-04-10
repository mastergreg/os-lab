#!/usr/bin/env python
# -*- coding: utf-8
#
#* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
# File Name : gatherer.py
# Creation Date : 10-04-2012
# Last Modified : Tue 10 Apr 2012 12:16:26 PM EEST
# Created By : Greg Liras <gregliras@gmail.com>
#_._._._._._._._._._._._._._._._._._._._._.*/

from time import sleep
from threading import Thread


class gatherer(Thread):
    _DATA=""
    def __init__(self,f):
        Thread.__init__(self)
        self._myfile = open(f,'r')
    def run(self):
        while True:
            try:
                i = self._myfile.readline()
                self._DATA=i
            except EOFError:
                i = ""
            sleep(1)
    def getData(self):
        print self._DATA
        return self._DATA
            
        



