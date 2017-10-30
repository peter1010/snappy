snappy
======

Experimental code to take pictures with web cam


Color space conversions
=======================

Y' = Kr * R' + (1 - Kr - Kb) * G' + Kb * B'
Pb = 0.5 * (B' - Y')/(1 - Kb)
Pr = 0.5 * (R' - Y')/(1 - Kr)


B' = Y' + 2(1-Kb) * Pb
R' = Y' + 2(1-Kr) * Pr
G' = (Y' - Kr * R' - Kb * B')/(1 - Kr - Kb)
G' = (y' - Kr * (Y' + 2Pr(1-kr) - Kb * (Y' + 2Pb(1-Kb))/(1-Kr-Kb)
G' = y' + (-Kr * 2Pr(1-Kr) - Kb * 2Pb(1-Kb))/(1-Kr-Kb)
G' = y' +


R' = [0.0, 1.0]        Y' = [0.0, 1.0]
G' = [0.0, 1.0]  =>    Pb = [-0.5, 0.5]
B' = [0.0, 1.0]        Pr = [-0.5, 0.5]

To covert from analog to digital

Yd = y_offset + y_scale * Y'
Cb = c_offset + c_scale * Pb
Cr = c_offset + c_scale * Pr

ITU-R BT.601 conversion
=======================
Kb = 0.114
Kr = 0.299

y_offset = 16
c_offset = 128

y_scale = 219/255
c_scale = 224/255

y =  16 + (16829 * r + 33039 * g +  6416 * b) >> 16
Cb= 128 + (-9714 * r - 19070 * g + 28784 * b) >> 16
Cr= 128 + (28784 * r - 24103 * g -  4681 * b) >> 16

ITU-R BT.709 conversion
=======================
Kb = 0.0722
Kr = 0.2126

y_offset = 16
c_offset = 128

y_scale = 219/255
c_scale = 224/255

ITU-R BT.2020 conversion
========================
Kb = 0.0722
Kr = 0.2126

y_offset = 16
c_offset = 128

y_scale = 219/255
c_scale = 224/255

JPEG conversion
===============
Kb = 0.114
Kr = 0.299

y_offset = 0
c_offset = 128

y_scale = 1
c_scale = 1

IPCam
=====

Commands...
/camera_control.cgi?loginuse=xxx&loginpas=yyy&param=1&value=91
/snapshot.cgi?user=xxx&pwd=yyy

brightness=1
contrast=2

&param=&value=&user=&pwd=&
