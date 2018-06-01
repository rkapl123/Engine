#!/usr/bin/env python
import sys
import datetime

print ("# Swaption LN volatilities converted from QRE format on " + datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))

for line in sys.stdin:
    if line:
        data = line.split(' ')
        if len(data) == 3:
            data2 = data[1].split('/')
            if data2[1] == "RATE_GVOL":
                data2[1] = "RATE_LNVOL"
            data[1] = "/".join(data2)
        data = map(str.strip, data)
        print (" ".join(data))

print ("# End");
