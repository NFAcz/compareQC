#!/bin/sh
qcli -i $1 -y -o /tmp/tmp.xml.gz
zcat /tmp/tmp.xml.gz | grep 'YAVG\|UAVG\|VAVG' | awk -F '"' '{print $4}' > /tmp/tmp.txt
for i in /mnt/ramdisk/*; do compareQC -a /tmp/tmp.txt -b "$i" -t 5 ; done
