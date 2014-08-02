#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <sys/mman.h>

#include "logging.h"
#include "debug.h"

SET_LOG_CATEGORY;


/**
 * Check the capabilities of the device
 *
 * returns those capabilities
 */
static __u32 check_capabilities(int fd)
{
    struct v4l2_capability cap;
    memset(&cap, 0, sizeof(cap));
    int retVal = ioctl(fd, VIDIOC_QUERYCAP, &cap);
    if(retVal == -1)
    {
        LOG_WARN("Couldnt get Capabilities");
        return 0;
    }

    LOG_INFO("%.16s, Card: %.32s, ver: 0x%x, bus: %.32s",
            cap.driver, cap.card, cap.version, cap.bus_info);

    __u32 caps = cap.capabilities;
    LOG_INFO("Card capabilites are 0x%X (%s)",
            caps, cap2str(caps));

    if(caps & V4L2_CAP_DEVICE_CAPS )
    {
        caps = cap.device_caps;
        LOG_INFO("Device capabilites are 0x%X (%s)",
            caps, cap2str(caps));
    }

    if((caps & (V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_VIDEO_CAPTURE_MPLANE)) == 0)
    {
        LOG_ERROR("Cant do video capture");
        return 0;
    }
    if((caps & (V4L2_CAP_STREAMING)) == 0)
    {
        LOG_ERROR("Cant do video capture, wrong type of memory");
        return 0;
    }
    return caps;
}

/*
 * Only applicable to tuner me thinks
 */

static void check_standards(int fd)
{
    struct v4l2_standard standard;
    int i;
    for(i = 0; i < 100; i++)
    {
        memset(&standard, 0, sizeof(standard));
        standard.index = i;
        int retVal = ioctl(fd, VIDIOC_ENUMSTD, &standard);
        if(retVal == -1)
        {
            perror("VIDIOC_ENUMSTD");
            break;
        }
        LOG_INFO("Video standard %u (%.24s)", standard.index, standard.name);
//        LOG_INFO("Input type: %s", inputType2str(input.type));
//        LOG_INFO("Input status: 0x%x", input.status);
//        LOG_INFO("Input cap: 0x%x", input.capabilities);
    }
}

static void check_controls(int fd)
{
    struct v4l2_queryctrl control;
    int i;
    __u32 id = 0;
    for(i = 0; i < 100; i++)
    {
        memset(&control, 0, sizeof(control));
        control.id = id | V4L2_CTRL_FLAG_NEXT_CTRL;
        int retVal = ioctl(fd, VIDIOC_QUERYCTRL, &control);
        if(retVal == -1)
        {
            perror("VIDIOC_QUERYCTRL");
            break;
        }
        id = control.id;
        LOG_INFO("Control %u (%.24s)", id, control.name);
        LOG_INFO("Control type: %s", ctrlType2str(control.type));

	struct v4l2_control setting;
        memset(&setting, 0, sizeof(setting));
        setting.id = id;
        retVal = ioctl(fd, VIDIOC_G_CTRL, &setting);
        if(retVal == -1)
        {
            perror("VIDIOC_G_CTRL");
            return;
        }
        LOG_INFO("Control min=%i, max=%i, step=%i, default=%i, actual=%i",
                control.minimum, control.maximum, control.step, control.default_value,
                setting.value);

        if(control.type == V4L2_CTRL_TYPE_MENU)
        {
            int j;
            for(j = control.minimum; j <= control.maximum; j+= control.step)
            {
                struct v4l2_querymenu menu;
                memset(&menu, 0, sizeof(menu));
                menu.id = id;
                menu.index = j;
                retVal = ioctl(fd, VIDIOC_QUERYMENU, &menu);
                if(retVal == -1)
                {
                    perror("VIDIOC_QUERYMENU");
                    break;
                }
                LOG_INFO("%s %.32s", j == setting.value ? "*" : " ", menu.name);
            }
        }
        LOG_INFO("Control flags=0x%X, %s", control.flags, ctrlFlag2str(control.flags));
    }
}

static void check_input(int fd)
{
    struct v4l2_input input;
    int i;
    for(i = 0; i < 100; i++)
    {
        memset(&input, 0, sizeof(input));
        input.index = i;
        int retVal = ioctl(fd, VIDIOC_ENUMINPUT, &input);
        if(retVal == -1)
        {
            break;
        }
        LOG_INFO("Input %u (%.32s)", input.index, input.name);
        LOG_INFO("Input type: %s", inputType2str(input.type));
        if(input.type == V4L2_INPUT_TYPE_TUNER)
        {
            LOG_INFO("TODO - display tuner params");
        }
        LOG_INFO("Input status: 0x%x", input.status);
        LOG_INFO("Input cap: 0x%x", input.capabilities);
    }
    {
        int port;
        int retVal = ioctl(fd, VIDIOC_G_INPUT, &port);
        if(retVal == -1)
        {
            return;
        }
        LOG_INFO("Input selected is: %i", port);
    }
}

static void check_format(int fd)
{
    struct v4l2_fmtdesc desc;
    int i, j;
    LOG_INFO("Check Formats");
    /* There are 12 buffer types */
    for(j = 1; j < 12; j++)
    {
        for(i = 0; i < 10; i++)
        {
//            printf("i=%i, j=%i", i, j);
            memset(&desc, 0, sizeof(desc));
            desc.index = i;
            desc.type = j;

            int retVal = ioctl(fd, VIDIOC_ENUM_FMT, &desc);
            if(retVal == -1)
            {
                break;
            }
            LOG_INFO("Fmt %u (%.32s)", desc.index, desc.description);
            LOG_INFO("Buffer type: %s", bufType2str(desc.type));
            LOG_INFO("Format %s", pixelfmt2str(desc.pixelformat));
            LOG_INFO("Fmt flags: 0x%x (%s)", desc.flags, fmtdescflag2str(desc.flags));
        }
    }
}

static void set_capture_params(int fd)
{
    struct v4l2_streamparm params;
    struct v4l2_captureparm * capture = &params.parm.capture;
    memset(&params, 0, sizeof(params));
    params.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int retVal = ioctl(fd, VIDIOC_G_PARM, &params);
    if(retVal == -1)
    {
        perror("VIDIOC_G_PARM");
        return;
    }

    LOG_INFO("Capture capability=0x%X (%s)",
            capture->capability, capcap2str(capture->capability));
    LOG_INFO("Capture mode=0x%X (%s)",
            capture->capturemode, capcap2str(capture->capturemode));
    LOG_INFO("Read buffers=%i", capture->readbuffers);
// struct v4l2_fract  timeperframe;  /*  Time per frame in seconds */
}



static void set_format(int fd)
{
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int retVal = ioctl(fd, VIDIOC_G_FMT, &fmt);
    if(retVal == -1)
    {
        return;
    }
    struct v4l2_pix_format * pix = &fmt.fmt.pix;
    LOG_INFO("Res = %u x %u", pix->width, pix->height);
    LOG_INFO("Colorspace = %s", colorspace2str(pix->colorspace));
    LOG_INFO("Format %s", pixelfmt2str(pix->pixelformat));
    LOG_INFO("Field %u", pix->field);
    LOG_INFO("Bytes per line %u", pix->bytesperline);
    LOG_INFO("Image size %u", pix->sizeimage);

    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    retVal = ioctl(fd, VIDIOC_G_FMT, &fmt);
    if(retVal == -1)
    {
        return;
    }
    LOG_INFO("Res = %u x %u", fmt.fmt.pix_mp.width, fmt.fmt.pix_mp.height);
    __u32 px = fmt.fmt.pix_mp.pixelformat;
    LOG_INFO("Format %s", pixelfmt2str(px));
    LOG_INFO("Field %u", fmt.fmt.pix_mp.field);
}

static int request_buffers(int fd, void ** starts, size_t * lengths)
{
    int i;
    struct v4l2_requestbuffers reqbuf;

    memset(&reqbuf, 0, sizeof(reqbuf));
    reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbuf.memory = V4L2_MEMORY_MMAP;
    reqbuf.count = 10;

    int status = ioctl(fd, VIDIOC_REQBUFS, &reqbuf);
    if(status == -1)
    {
        LOG_ERROR("Failed to create buffers");
        exit(EXIT_FAILURE);
    }
    for(i = 0; i < reqbuf.count; i++)
    {
        struct v4l2_buffer buffer;

        memset(&buffer, 0, sizeof(buffer));
        buffer.type = reqbuf.type;
        buffer.memory = reqbuf.memory;
        buffer.index = i;

        status = ioctl(fd, VIDIOC_QUERYBUF, &buffer);
        if(status == -1)
        {
            LOG_ERROR("Failed to get buffer details");
            exit(EXIT_FAILURE);
        }
        void * start = mmap(NULL, buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buffer.m.offset);
        if(start == MAP_FAILED)
        {
            LOG_ERROR("Failed to map buffer");
            exit(EXIT_FAILURE);
        }
        LOG_INFO("MMAP, %p, len=%u", start, buffer.length);
        *starts++ = start;
        *lengths++ = buffer.length;
    }
    return reqbuf.count;
}

static void enable_capture(fd)
{
    const int arg = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    const int status = ioctl(fd, VIDIOC_STREAMON, &arg);
    if(status == -1)
    {
        LOG_ERROR("Failed to get buffer details");
        exit(EXIT_FAILURE);
    }

}

static void disable_capture(fd)
{
    const int arg = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    const int status = ioctl(fd, VIDIOC_STREAMOFF, &arg);
    if(status == -1)
    {
        LOG_ERROR("Failed to get buffer details");
        exit(EXIT_FAILURE);
    }
}


static void queue_buffer(int fd, int i)
{
    struct v4l2_buffer buffer;

    memset(&buffer, 0, sizeof(buffer));
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.index = i;

    const int status = ioctl(fd, VIDIOC_QBUF, &buffer);
    if(status == -1)
    {
        LOG_ERROR("Failed to get buffer details");
        exit(EXIT_FAILURE);
    }
}

/**
 * Get buffer status
 *
 * @retutn the flags
 */
static __u32 query_buffer(int fd, int i)
{
    struct v4l2_buffer buffer;

    memset(&buffer, 0, sizeof(buffer));
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.index = i;

    const int status = ioctl(fd, VIDIOC_QBUF, &buffer);
    if(status == -1)
    {
        LOG_ERROR("Failed to get buffer details");
        exit(EXIT_FAILURE);
    }
    return buffer.flags;
}

/*
 * Wait for a buffer to be ready
 */
static int wait_buffer_ready(int fd, __u32 * bytes_avail)
{
    struct v4l2_buffer buffer;

    memset(&buffer, 0, sizeof(buffer));
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = V4L2_MEMORY_MMAP;

    const int status = ioctl(fd, VIDIOC_DQBUF, &buffer);
    if(status == -1)
    {
        LOG_ERROR("Failed to get buffer details");
        exit(EXIT_FAILURE);
    }
    *bytes_avail = buffer.bytesused;
    return buffer.index;

}

int main(int argc, char * argv[])
{
    set_logging_level(5);

    int i;
    int fd = open("/dev/video0", O_RDWR);
    if(fd < 0)
    {
        LOG_ERROR("Open failed");
        exit(EXIT_FAILURE);
    }

    __u32 capability = check_capabilities(fd);
    if(capability == 0)
    {
        return 1;
    }

    check_standards(fd);

    check_controls(fd);

    check_input(fd);

    check_format(fd);

    set_format(fd);


    void * starts[10];
    size_t lengths[10];

    int n = request_buffers(fd, starts, lengths);
    for(i = 0; i < n; i++)
    {
        queue_buffer(fd, i);
    }

    set_capture_params(fd);
    enable_capture(fd);
    for(i = 0; i < 10; i++)
    {
        __u32 bytes_avail;
        char fname[30];
        __u8 frame[640*480];
        int n = wait_buffer_ready(fd, &bytes_avail);
        int max_val = 0;
        snprintf(fname, sizeof(fname), "image_%i.pgm", i);
        LOG_INFO("%i %u", n, bytes_avail);
        FILE * f = fopen(fname, "wb");
        if(f)
        {
            __u8 * src = starts[n];
            __u8 * dst = frame;
            int j;
            for(j = 0; j < bytes_avail/2; j++)
            {
                __u8 val = *src++;
                *dst++ = val;
                src++;
                if(val > max_val)
                    max_val = val;
            }
            fprintf(f, "P5\n640 480\n%i\n", max_val);

            fwrite(frame, bytes_avail/2, 1, f);
            fclose(f);
        }
        queue_buffer(fd, n);
    }
    check_controls(fd);
    disable_capture(fd);

    close(fd);
    return 0;
}
