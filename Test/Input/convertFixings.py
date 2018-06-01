#!/usr/bin/env python
import sys
import datetime

# converter for QL -> QRE names
converter = {
    'GBPLIBOR1W ACTUAL/365 (FIXED)' : 'GBP-LIBOR-1W',
    'GBPLIBOR2W ACTUAL/365 (FIXED)' : 'GBP-LIBOR-2W',
    'GBPLIBOR1M ACTUAL/365 (FIXED)' : 'GBP-LIBOR-1M',
    'GBPLIBOR3M ACTUAL/365 (FIXED)' : 'GBP-LIBOR-3M',
    'GBPLIBOR6M ACTUAL/365 (FIXED)' : 'GBP-LIBOR-6M',
    'GBPLIBOR12M ACTUAL/365 (FIXED)' : 'GBP-LIBOR-12M',

    'EONIAON ACTUAL/360' : 'EUR-EONIA',
    'EURIBOR1W ACTUAL/360' : 'EUR-EURIBOR-1W',
    'EURIBOR2W ACTUAL/360' : 'EUR-EURIBOR-2W',
    'EURIBOR1M ACTUAL/360' : 'EUR-EURIBOR-1M',
    'EURIBOR3M ACTUAL/360' : 'EUR-EURIBOR-3M',
    'EURIBOR6M ACTUAL/360' : 'EUR-EURIBOR-6M',
    'EURIBOR12M ACTUAL/360' : 'EUR-EURIBOR-6M',

    'USDLIBOR1W ACTUAL/360' : 'USD-LIBOR-1W',
    'USDLIBOR2W ACTUAL/360' : 'USD-LIBOR-2W',
    'USDLIBOR1M ACTUAL/360' : 'USD-LIBOR-1M',
    'USDLIBOR3M ACTUAL/360' : 'USD-LIBOR-3M',
    'USDLIBOR6M ACTUAL/360' : 'USD-LIBOR-6M',
    'USDLIBOR12M ACTUAL/360' : 'USD-LIBOR-12M',

    'JPYLIBOR1W ACTUAL/360' : 'JPY-LIBOR-1W',
    'JPYLIBOR2W ACTUAL/360' : 'JPY-LIBOR-2W',
    'JPYLIBOR1M ACTUAL/360' : 'JPY-LIBOR-1M',
    'JPYLIBOR3M ACTUAL/360' : 'JPY-LIBOR-3M',
    'JPYLIBOR6M ACTUAL/360' : 'JPY-LIBOR-6M',
    'JPYLIBOR12M ACTUAL/360' : 'JPY-LIBOR-12M'
}

print ("# Fixings converted from QRE format on " + datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))

for line in sys.stdin:
    if line and len(line) > 3 and line[0] != '#':
        data = line.split(';')
        # here we check to see if the QRE name is in the dict
        # if not, we ignore it, maybe we should allow it in untranslated.
        if len(data) == 3 and data[0] in converter.keys():
            index = converter[data[0].strip()]
            date = data[1].strip()
            fixing = data[2].strip()
            print (date + " " + index + " " + fixing)

print ("# End");
