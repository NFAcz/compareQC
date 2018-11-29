# compareQC
Tool for comparing QC tools outputs in order to find duplicates

Program will compare two xml.gz files (outputs from QCTools) file A needs to be subset of file B. You can also set tolerance threshold wich means how much can one average frame luminance differ.. it compares Y value from YUV colorspace only.



# usage:

Starting with two video files where A is a fragment or shorter video you need to find in B

```
A.mov
B.mov
```

first you need to compute QC logs of both files ```https://mediaarea.net/QCTools```

```
qcli -i A.mov -o A.xml.gz
qcli -i B.mov -o B.xml.gz
```

then you can compare gzipped files direcly like this 


```
compareQC -a A.xml.gz -b B.xml.gz -t 5.0
```

Values of -t (threshold should not be larger than 20 to give some meaningful results) 
