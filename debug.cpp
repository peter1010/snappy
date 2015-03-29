#include <stdlib.h>
#include <linux/videodev2.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "debug.h"

struct MaskLookup_s {
    uint32_t mask;
    const char * str;
};


struct ValLookup_s {
    int val;
    const char * str;
};


static const char * list2str(const char * items[], int num_items)
{
    static char * tmp = NULL;
    if(num_items == 0) {
        return "";
    }
    else if(num_items == 1) {
        return items[0];
    }
    else {
        int i;
        int size = 0;
        char * p;

        for(i=0; i<num_items; i++) {
            size += (2 + strlen(items[i]));
        }
        if(tmp) {
            delete [] tmp;
        }
        tmp = new char[size];
        p = tmp;
        for(i=0; i<num_items; i++) {
            strcpy(p, items[i]);
            p += strlen(p);
            p[0] = ',';
            p[1] = ' ';
            p += 2;
        }
        p[-2] = '\0';
        return const_cast<const char *>(tmp);
    }
}

static const char * do_mask_lookup(uint32_t mask,
        const struct MaskLookup_s * lookup, unsigned int num_entries)
{
    const char * items[8*sizeof(uint32_t)];
    const char ** p = items;
    for(unsigned i = 0; i < num_entries && mask; i++) {
        if(mask & lookup[i].mask) {
            mask &= ~lookup[i].mask;
            *p++ = lookup[i].str;
        }
    }
    return list2str(items, p - items);
}


const char * cap2str(__u32 caps)
{
   static const struct MaskLookup_s lookup[] = {
        { V4L2_CAP_VIDEO_CAPTURE, "video capture"},
        { V4L2_CAP_VIDEO_OUTPUT, "video output"},
        { V4L2_CAP_VIDEO_OVERLAY, "video overlay"},
        { V4L2_CAP_VBI_CAPTURE, "VBI capture"},
        { V4L2_CAP_VBI_OUTPUT, "VBI output"},
        { V4L2_CAP_SLICED_VBI_CAPTURE, "sliced VBI capture"},
        { V4L2_CAP_SLICED_VBI_OUTPUT, "sliced VBI output"},
        { V4L2_CAP_RDS_CAPTURE, "RDS data capture"},
        { V4L2_CAP_VIDEO_OUTPUT_OVERLAY, "video output overlay"},
        { V4L2_CAP_HW_FREQ_SEEK, "hardware frequency seek"},
        { V4L2_CAP_RDS_OUTPUT, "RDS encoder"},
        { V4L2_CAP_VIDEO_CAPTURE_MPLANE, "Video capture mplane"},
        { V4L2_CAP_VIDEO_OUTPUT_MPLANE, "Video output mplane"},
        { V4L2_CAP_VIDEO_M2M_MPLANE, "Video M2M mplane"},
        { V4L2_CAP_VIDEO_M2M, "Video M2M"},
        { V4L2_CAP_TUNER, "Tuner"},
        { V4L2_CAP_AUDIO, "Audio"},
        { V4L2_CAP_RADIO, "Radio"},
        { V4L2_CAP_MODULATOR, "Modulator"},
        { V4L2_CAP_READWRITE, "read/write"},
        { V4L2_CAP_ASYNCIO ,"Async I/O"},
        { V4L2_CAP_STREAMING, "streaming I/O"},
        { V4L2_CAP_DEVICE_CAPS, "sets device capabilities field"},
    };
    return do_mask_lookup(caps, lookup,
            sizeof(lookup)/sizeof(struct MaskLookup_s));
}

static const char * do_val_lookup(int val,
        const struct ValLookup_s * lookup, unsigned int num_entries)
{
    for(unsigned i = 0; i < num_entries; i++) {
        if(val == lookup[i].val) {
            return lookup[i].str;
        }
    }
    return "Unknown";
}

/**
 *  Covert input type to a string
 */
const char * inputType2str(int typ)
{
    static const struct ValLookup_s lookup[] = {
        { V4L2_INPUT_TYPE_TUNER, "Tuner"},
        { V4L2_INPUT_TYPE_CAMERA, "Camera"},
    };
    return do_val_lookup(typ, lookup,
            sizeof(lookup)/sizeof(struct ValLookup_s));
}

const char * bufType2str(int typ)
{
    static const struct ValLookup_s lookup[] = {
        { V4L2_BUF_TYPE_VIDEO_CAPTURE, "Capture"},
        { V4L2_BUF_TYPE_VIDEO_OUTPUT, "Output"},
        { V4L2_BUF_TYPE_VIDEO_OVERLAY, "Overlay"},
        { V4L2_BUF_TYPE_VBI_CAPTURE, "VBI-Capture"},
        { V4L2_BUF_TYPE_VBI_OUTPUT, "VBI-Output"},
        { V4L2_BUF_TYPE_SLICED_VBI_CAPTURE, "Sliced-VBI-Capture"},
        { V4L2_BUF_TYPE_SLICED_VBI_OUTPUT, "Sliced-VBI-Output"},
        { V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY, "Output-Overlay"},
        { V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE, "Capture-Mplane"},
        { V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE, "Output-Mplane" },
//      { V4L2_BUF_TYPE_SDR_CAPTURE, "SDR-Capture"},
    };
    return do_val_lookup(typ, lookup,
            sizeof(lookup)/sizeof(struct ValLookup_s));
}

const char * ctrlType2str(int typ)
{
    static const struct ValLookup_s lookup[] = {
        { V4L2_CTRL_TYPE_INTEGER, "Integer"},
        { V4L2_CTRL_TYPE_BOOLEAN, "Boolean"},
        { V4L2_CTRL_TYPE_MENU, "Menu"},
        { V4L2_CTRL_TYPE_BUTTON, "button"},
        { V4L2_CTRL_TYPE_INTEGER64, "Integer64"},
        { V4L2_CTRL_TYPE_CTRL_CLASS, "CtrlClass"},
        { V4L2_CTRL_TYPE_STRING, "String"},
        { V4L2_CTRL_TYPE_BITMASK, "Bitmask"},
        { V4L2_CTRL_TYPE_INTEGER_MENU, "Integer_menu"},
    };
    return do_val_lookup(typ, lookup,
            sizeof(lookup)/sizeof(struct ValLookup_s));
}

const char * ctrlFlag2str(__u32 flags)
{
    /*  Control flags  */
    static const struct MaskLookup_s lookup[] = {
        { V4L2_CTRL_FLAG_DISABLED, "Disabled"},
        { V4L2_CTRL_FLAG_GRABBED, "Grabbed"},
        { V4L2_CTRL_FLAG_READ_ONLY, "Read Only"},
        { V4L2_CTRL_FLAG_UPDATE, "Update"},
        { V4L2_CTRL_FLAG_INACTIVE, "Inactive"},
        { V4L2_CTRL_FLAG_SLIDER, "Slider"},
        { V4L2_CTRL_FLAG_WRITE_ONLY, "Write Only"},
        { V4L2_CTRL_FLAG_VOLATILE, "Volatile"},
    };
    return do_mask_lookup(flags, lookup,
            sizeof(lookup)/sizeof(struct MaskLookup_s));
}

const char * fmtdescflag2str(__u32 flags)
{
    static const struct MaskLookup_s lookup[] = {
        { V4L2_FMT_FLAG_COMPRESSED, "Compressed"},
        { V4L2_FMT_FLAG_EMULATED, "Emulated"},
    };
    return do_mask_lookup(flags, lookup,
            sizeof(lookup)/sizeof(struct MaskLookup_s));
}


const char * colorspace2str(int space)
{
    static const struct ValLookup_s lookup[] = {
        { V4L2_COLORSPACE_SMPTE170M, "SMPTE170M"},
        { V4L2_COLORSPACE_SMPTE240M, "SMPTE240M"},
        { V4L2_COLORSPACE_REC709, "REC709"},
        { V4L2_COLORSPACE_BT878, "BT878"},
        { V4L2_COLORSPACE_470_SYSTEM_M, "470_SYSTEM_M"},
        { V4L2_COLORSPACE_470_SYSTEM_BG, "470_SYSTEM_BG"},
        { V4L2_COLORSPACE_JPEG, "JPEG"},
        { V4L2_COLORSPACE_SRGB, "SRGB"},
    };
    return do_val_lookup(space, lookup,
            sizeof(lookup)/sizeof(struct ValLookup_s));
}


const char * pixelfmt2str(__u32 px)
{
    static char tmp[50];
    snprintf(tmp, sizeof(tmp), "(%8x) %c%c%c%c",
        px, px & 0xff, (px >> 8) & 0xff, (px >> 16) & 0xff, (px >> 24) & 0xff);
    return tmp;
}

const char * capcap2str(__u32 mode)
{
    /*  Control flags  */
    static const struct MaskLookup_s lookup[] = {
        { V4L2_MODE_HIGHQUALITY, "HighQuality"},
        { V4L2_CAP_TIMEPERFRAME, "TimePerFrame"},
    };
    return do_mask_lookup(mode, lookup,
            sizeof(lookup)/sizeof(struct MaskLookup_s));
}

