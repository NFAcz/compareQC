#!/bin/bash
for i in *.gz; do echo $i ; time (zcat $i | grep 'YAVG\|UAVG\|VAVG' | awk -F '"' '{print $4}' > txt/`basename -s .xml.gz $i`.txt) ; done
