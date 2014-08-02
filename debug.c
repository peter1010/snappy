#include <stdlib.h>
#include <linux/videodev2.h>
#include <string.h>
#include <stdio.h>

#include "debug.h"

static const char * list2str(const char * items[], int num_items)
{
    static char * tmp = 0;
    if(num_items == 0)
    {
        return "";
    }
    else if(num_items == 1)
    {
        return items[0];
    }
    else
    {
        int i;
        int size = 0;
        for(i=0; i<num_items; i++)
        {
            size += (2 + strlen(items[i]));
        }
        free(tmp);
        tmp = (char *) malloc(size);
        char * p = tmp;
        for(i=0; i<num_items; i++)
        {
            strcpy(p, items[i]);
            p += strlen(p);
            p[0] = ',';
            p[1] = ' ';
            p += 2;
        }
        p[-2] = '\0';
        return (const char *)tmp;
    }
}

const char * cap2str(__u32 caps)
{
    const char * items[8*sizeof(caps)];
    const char ** p = items;

    if(caps & V4L2_CAP_VIDEO_CAPTURE)
        *p++ = "video capture";
    if(caps & V4L2_CAP_VIDEO_OUTPUT)
        *p++ = "video output";
    if(caps & V4L2_CAP_VIDEO_OVERLAY)
        *p++ = "video overlay";
    if(caps & V4L2_CAP_VBI_CAPTURE)
        *p++ = "VBI capture";
    if(caps & V4L2_CAP_VBI_OUTPUT)
        *p++ = "VBI output";
    if(caps & V4L2_CAP_SLICED_VBI_CAPTURE)
        *p++ = "sliced VBI capture";
    if(caps & V4L2_CAP_SLICED_VBI_OUTPUT)
        *p++ = "sliced VBI output";
    if(caps & V4L2_CAP_RDS_CAPTURE)
        *p++ = "RDS data capture";
    if(caps & V4L2_CAP_VIDEO_OUTPUT_OVERLAY)
        *p++ = "video output overlay";
    if(caps & V4L2_CAP_HW_FREQ_SEEK)
        *p++ = "hardware frequency seek";
    if(caps & V4L2_CAP_RDS_OUTPUT)
        *p++ = "RDS encoder";
    if(caps & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
        *p++ = "Video capture mplane";
    if(caps & V4L2_CAP_VIDEO_OUTPUT_MPLANE)
        *p++ = "Video output mplane";
    if(caps & V4L2_CAP_VIDEO_M2M_MPLANE)
        *p++ = "Video M2M mplane";
    if(caps & V4L2_CAP_VIDEO_M2M)
        *p++ = "Video M2M";
    if(caps & V4L2_CAP_TUNER)
        *p++ = "Tuner";
    if(caps & V4L2_CAP_AUDIO)
        *p++ = "Audio";
    if(caps & V4L2_CAP_RADIO)
        *p++ = "Radio";
    if(caps & V4L2_CAP_MODULATOR)
        *p++ = "Modulator";
    if(caps & V4L2_CAP_READWRITE)
        *p++ = "read/write";
    if(caps & V4L2_CAP_ASYNCIO)
        *p++ = "Async I/O";
    if(caps & V4L2_CAP_STREAMING)
        *p++ = "streaming I/O";
    if(caps & V4L2_CAP_DEVICE_CAPS )
        *p++ = "sets device capabilities field";
    return list2str(items, p - items);
}

/**
 *  Covert input type to a string
 */
const char * inputType2str(int typ)
{
    const char * p;
    switch(typ)
    {
        case V4L2_INPUT_TYPE_TUNER:
            p = "Tuner";
            break;
        case V4L2_INPUT_TYPE_CAMERA:
            p = "Camera";
            break;
        default:
            p = "Unknown";
            break;
    }
    return p;
}

const char * bufType2str(int typ)
{
    const char *p;
    switch(typ)
    {
        case V4L2_BUF_TYPE_VIDEO_CAPTURE:
            p = "Capture";
            break;
        case V4L2_BUF_TYPE_VIDEO_OUTPUT:
            p = "Output";
            break;
        case V4L2_BUF_TYPE_VIDEO_OVERLAY:
            p = "Overlay";
            break;
        case V4L2_BUF_TYPE_VBI_CAPTURE:
            p = "VBI-Capture";
            break;
        case V4L2_BUF_TYPE_VBI_OUTPUT:
            p = "VBI-Output";
            break;
        case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
            p = "Sliced-VBI-Capture";
            break;
        case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
            p = "Sliced-VBI-Output";
            break;
        case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY:
            p = "Output-Overlay";
            break;
        case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
            p = "Capture-Mplane";
            break;
        case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
            p = "Output-Mplane";
            break;
//        case V4L2_BUF_TYPE_SDR_CAPTURE: p = "SDR-Capture"; break;
        default:
            p = "Unkown";
            break;
    }
    return p;
}

const char * ctrlType2str(int typ)
{
    const char * p;
    switch(typ)
    {
        case V4L2_CTRL_TYPE_INTEGER:
            p = "Integer";
            break;
        case V4L2_CTRL_TYPE_BOOLEAN:
            p = "Boolean";
            break;
        case V4L2_CTRL_TYPE_MENU:
            p = "Menu";
            break;
        case V4L2_CTRL_TYPE_BUTTON:
            p = "button";
            break;
        case V4L2_CTRL_TYPE_INTEGER64:
            p = "Integer64";
            break;
        case V4L2_CTRL_TYPE_CTRL_CLASS:
            p = "CtrlClass";
            break;
        case V4L2_CTRL_TYPE_STRING:
            p = "String";
            break;
        case V4L2_CTRL_TYPE_BITMASK:
            p = "Bitmask";
            break;
        case V4L2_CTRL_TYPE_INTEGER_MENU:
            p = "Integer_menu";
            break;
        default:
            p = "Unknown";
            break;
    }
    return p;
}

const char * ctrlFlag2str(__u32 flags)
{
    const char * items[8*sizeof(flags)];
    const char ** p = items;

    /*  Control flags  */
    if(flags & V4L2_CTRL_FLAG_DISABLED)
        *p++ = "Disabled";
    if(flags & V4L2_CTRL_FLAG_GRABBED)
        *p++ = "Grabbed";
    if(flags & V4L2_CTRL_FLAG_READ_ONLY)
        *p++ = "Read Only";
    if(flags & V4L2_CTRL_FLAG_UPDATE)
        *p++ = "Update";
    if(flags & V4L2_CTRL_FLAG_INACTIVE)
        *p++ = "Inactive";
    if(flags & V4L2_CTRL_FLAG_SLIDER)
        *p++ = "Slider";
    if(flags & V4L2_CTRL_FLAG_WRITE_ONLY)
        *p++ = "Write Only";
    if(flags & V4L2_CTRL_FLAG_VOLATILE)
        *p++ = "Volatile";
    return list2str(items, p - items);
}

const char * fmtdescflag2str(__u32 flags)
{
    const char * p;
    if(flags & V4L2_FMT_FLAG_COMPRESSED)
    {
        if(flags & V4L2_FMT_FLAG_EMULATED)
        {
            p = "Compressed & Emulated";
        }
        else
        {
            p = "Compressed";
        }
    }
    else if(flags & V4L2_FMT_FLAG_EMULATED)
    {
        p = "Emulated";
    }
    else
    {
        p = "";
    }
    return p;
}

const char * colorspace2str(int space)
{
    const char * p;
    switch(space)
    {
        case V4L2_COLORSPACE_SMPTE170M:
            p = "SMPTE170M";
            break;

        case V4L2_COLORSPACE_SMPTE240M:
            p = "SMPTE240M";
            break;

        case V4L2_COLORSPACE_REC709:
            p = "REC709";
            break;

        case V4L2_COLORSPACE_BT878:
            p = "BT878";
            break;

        case V4L2_COLORSPACE_470_SYSTEM_M:
            p = "470_SYSTEM_M";
            break;

        case V4L2_COLORSPACE_470_SYSTEM_BG:
            p = "470_SYSTEM_BG";
            break;

        case V4L2_COLORSPACE_JPEG:
            p = "JPEG";
            break;

        case V4L2_COLORSPACE_SRGB:
            p = "SRGB";
            break;

        default:
            p = "unknown";
            break;
    }
    return p;
}


const char * pixelfmt2str(__u32 px)
{
    static char tmp[100];
    sprintf(tmp, "(%8x) %c%c%c%c",
        px, px & 0xff, (px >> 8) & 0xff, (px >> 16) & 0xff, (px >> 24) & 0xff);
    return tmp;
}

const char * capcap2str(__u32 mode)
{
    const char * items[8*sizeof(mode)];
    const char ** p = items;

    /*  Control flags  */
    if(mode & V4L2_MODE_HIGHQUALITY)
        *p++ = "HighQuality";
    if(mode & V4L2_CAP_TIMEPERFRAME)
        *p++ = "TimePerFrame";
    return list2str(items, p - items);
}

