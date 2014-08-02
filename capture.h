#ifndef _CAPTURE_H_
#define _CAPTURE_H_

struct Control_s
{
    __u32 id;
    __s32 minimum;	/* Note signedness */
    __s32 maximum;
    __s32 step;
    __s32 value;
};

typedef struct Control_s Control_t;

struct Camera_s
{
    int fd;
    int buf_type; /* see enum v4l2_buf_type */
    
    unsigned width;
    unsigned height;
    __u32 pix_fmt; /* The pixel format */

    void ** buf_starts;
    size_t * buf_lengths;
    int num_bufs;
    
    __u32 brightness;
    __u32 contrast;
};

typedef struct Camera_s Camera_t;

#endif
