#!/usr/bin/env python
# -*- coding: utf-8
#
#* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
# File Name : gatherer.py
# Creation Date : 10-04-2012
# Last Modified : Tue 10 Apr 2012 05:49:31 PM EEST
# Created By : Greg Liras <gregliras@gmail.com>
#_._._._._._._._._._._._._._._._._._._._._.*/

from time import sleep
from threading import Thread


class gatherer(Thread):
    _DATA=""
    _fname=""
    _cond=True
    def __init__(self,f):
        Thread.__init__(self)
        self._fname=f
        self._myfile = open(f,'r')
    def run(self):
        while self._cond:
            i = self._myfile.readline()
            self._DATA=i.strip()
        exit(0)
    def __repr__(self):
        return "%s: %s"%(self._fname,self._DATA)
    def stop(self):
        self._cond=False
        self._myfile.close()
        self._Thread__delete()
        



