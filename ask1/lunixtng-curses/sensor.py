#!/usr/bin/env python
# -*- coding: utf-8
#
#* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
# File Name : sensor.py
# Creation Date : 10-04-2012
# Last Modified : Tue 10 Apr 2012 02:21:03 PM EEST
# Created By : Greg Liras <gregliras@gmail.com>
#_._._._._._._._._._._._._._._._._._._._._.*/
from gatherer import gatherer

class sensor(gatherer):
    def __init__(self,minor,stype):
        gatherer.__init__(self,"/dev/lunix%d-%s"%(minor,stype))

