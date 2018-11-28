# compareQC
Tool for comparing QC tools outputs in order to find duplicates

Program will compare two xml.gz files (outputs from QCTools) file A needs to be subset of file B. You can also set tolerance threshold wich means how much can one average frame luminance differ.. it compares Y value from YUV colorspace only.



# usage:

```
compareQC -a QCOutputFile1.xml.gz -b QCFile2.xml.gz -t 5.0
```

