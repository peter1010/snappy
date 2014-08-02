#ifndef _DEBUG_H_
#define _DEBUG_H_

extern const char * cap2str(__u32 caps);

extern const char * inputType2str(int typ);

extern const char * bufType2str(int typ);

extern const char * ctrlType2str(int typ);

extern const char * ctrlFlag2str(__u32 flags);

extern const char * colorspace2str(int space);

extern const char * fmtdescflag2str(__u32 flags);

extern const char * pixelfmt2str(__u32 px);

extern const char * capcap2str(__u32 cap);
#endif
