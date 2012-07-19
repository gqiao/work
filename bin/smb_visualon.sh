#!/bin/sh

PW=Visualon264

smbmount //10.2.64.2/media         /work/visualon/m -o user=qiao_shanshan,pass=${PW},iocharset=utf8
smbmount //10.2.64.2/protected     /work/visualon/p -o user=qiao_shanshan,pass=${PW},iocharset=utf8
smbmount //10.2.64.2/ref           /work/visualon/r -o user=qiao_shanshan,pass=${PW},iocharset=utf8
smbmount //10.2.64.2/soft          /work/visualon/s -o user=qiao_shanshan,pass=${PW},iocharset=utf8
smbmount //10.2.64.2/temp          /work/visualon/t -o user=qiao_shanshan,pass=${PW},iocharset=utf8
smbmount //10.2.64.2/upload        /work/visualon/u -o user=qiao_shanshan,pass=${PW},iocharset=utf8
smbmount //10.2.64.2/development   /work/visualon/v -o user=qiao_shanshan,pass=${PW},iocharset=utf8
smbmount //10.2.64.2/qiao_shanshan /work/visualon/w -o user=qiao_shanshan,pass=${PW},iocharset=utf8
smbmount //10.2.64.2/public        /work/visualon/x -o user=qiao_shanshan,pass=${PW},iocharset=utf8
smbmount //10.2.64.2/fromcustomer  /work/visualon/y -o user=qiao_shanshan,pass=${PW},iocharset=utf8
smbmount //10.2.64.2/release       /work/visualon/z -o user=qiao_shanshan,pass=${PW},iocharset=utf8




