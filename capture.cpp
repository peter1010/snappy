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
#include "capture.h"

#define MAX_INPUTS (100)
#define MAX_STANDARDS (100)
#define MAX_FORMATS (100)


Camera::Camera()
    : m_fd(0), m_buf_type(0), m_width(0), m_height(0), m_pix_fmt(0),
      m_buf_starts(0), m_buf_lengths(0), m_num_bufs(0), m_brightness(0),
      m_contrast(0)
{
    const int fd = open("/dev/video0", O_RDWR);
    if(fd < 0)
    {
        LOG_ERROR("Open failed");
        exit(EXIT_FAILURE);
    }
    m_fd = fd;
}

/**
 * Check the capabilities of the device
 *
 * @param[in,out] info
 *
 * returns those capabilities
 */
uint32_t Camera::check_capabilities()
{
    struct v4l2_capability cap;
    uint32_t caps;
    int retVal;

    memset(&cap, 0, sizeof(cap));
    retVal = ioctl(m_fd, VIDIOC_QUERYCAP, &cap);
    if(retVal == -1) {
        LOG_WARN("Couldnt get Capabilities");
        return 0;
    }

    LOG_INFO("%.16s, Card: %.32s, ver: 0x%x, bus: %.32s",
            cap.driver, cap.card, cap.version, cap.bus_info);

    caps = cap.capabilities;
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
void Camera::check_standards()
{
    struct v4l2_standard standard;
    int i;
    for(i = 0; i < MAX_STANDARDS; i++)
    {
        int retVal;

        memset(&standard, 0, sizeof(standard));
        standard.index = i;
        retVal = ioctl(m_fd, VIDIOC_ENUMSTD, &standard);
        if(retVal == -1)
        {
            LOG_ERRNO_AS_ERROR("VIDIOC_ENUMSTD");
            break;
        }
        LOG_INFO("Video standard %u (%.24s)", standard.index, standard.name);
//        LOG_INFO("Input type: %s", inputType2str(input.type));
//        LOG_INFO("Input status: 0x%x", input.status);
//        LOG_INFO("Input cap: 0x%x", input.capabilities);
    }
}

/**
 * Set a control value to 'value'
 */
void Camera::set_control_value(int id, __s32 value)
{
    int retVal;
    if(V4L2_CTRL_ID2CLASS(id) == V4L2_CTRL_CLASS_USER)
    {
        /* Old-style 'user' controls */
        struct v4l2_control setting;
        memset(&setting, 0, sizeof(setting));
        setting.id = id;
        setting.value = value;
        retVal = ioctl(m_fd, VIDIOC_S_CTRL, &setting);
        if(retVal != 0)
        {
            LOG_ERRNO_AS_ERROR("VIDIOC_S_CTRL");
        }
    }
    else
    {
        /* New-style controls */
        struct v4l2_ext_control setting;
        struct v4l2_ext_controls container;
        memset(&setting, 0, sizeof(setting));
        memset(&container, 0, sizeof(container));
        container.ctrl_class = V4L2_CTRL_ID2CLASS(id);
	container.count = 1;
        container.controls = &setting;
        setting.id = id;
	setting.size = 0;
        setting.value = value;
        retVal = ioctl(m_fd, VIDIOC_TRY_EXT_CTRLS, &setting);
        if(retVal != 0)
        {
            LOG_ERRNO_AS_ERROR("VIDIOC_TRY_EXT_CTRLS");
        }
        retVal = ioctl(m_fd, VIDIOC_S_EXT_CTRLS, &setting);
        if(retVal != 0)
        {
            LOG_ERRNO_AS_ERROR("VIDIOC_S_EXT_CTRLS");
        }
    }
    LOG_INFO("Control 0x%X set to %i", id, value);
}

/**
 * Get the Control value
 */
__s32 Camera::get_control_value(int id)
{
    int retVal;
    if(V4L2_CTRL_ID2CLASS(id) == V4L2_CTRL_CLASS_USER)
    {
        /* Old-style 'user' controls */
        struct v4l2_control setting;
        memset(&setting, 0, sizeof(setting));
        setting.id = id;
        retVal = ioctl(m_fd, VIDIOC_G_CTRL, &setting);
        if(retVal == -1)
        {
            LOG_ERRNO_AS_ERROR("VIDIOC_G_CTRL");
            return -1;
        }
        return setting.value;
    }
    else
    {
        /* New-style controls */
        struct v4l2_ext_control setting;
        struct v4l2_ext_controls container;
        memset(&setting, 0, sizeof(setting));
        memset(&container, 0, sizeof(container));
        container.ctrl_class = V4L2_CTRL_ID2CLASS(id);
	container.count = 1;
        container.controls = &setting;
        setting.id = id;
	setting.size = 0;
        retVal = ioctl(m_fd, VIDIOC_G_EXT_CTRLS, &setting);
        if(retVal == -1)
        {
            LOG_ERRNO_AS_ERROR("VIDIOC_G_EXT_CTRLS");
            return -1;
        }
        return setting.value;
    }
}


void Camera::set_control(int id, float percent)
{
    struct v4l2_queryctrl control;
    int retVal;
    uint32_t range;
    __s32 new_value;

    memset(&control, 0, sizeof(control));
    control.id = id;
    retVal = ioctl(m_fd, VIDIOC_QUERYCTRL, &control);
    if(retVal == -1)
    {
        LOG_ERRNO_AS_ERROR("VIDIOC_QUERYCTRL");
        return;
    }
    range = control.maximum - control.minimum;
    new_value = control.minimum
            + control.step * (__s32)(0.5 + (range * percent)/control.step);
    if(new_value < control.minimum)
        new_value = control.minimum;
    else if(new_value > control.maximum)
        new_value = control.maximum;

    set_control_value(id, new_value);
}

void Camera::check_controls()
{
    struct v4l2_queryctrl control;
    int i;
    uint32_t id = 0;
    for(i = 0; i < 100; i++)
    {
        int retVal;
        __s32 value;

        memset(&control, 0, sizeof(control));
        control.id = id | V4L2_CTRL_FLAG_NEXT_CTRL;
        retVal = ioctl(m_fd, VIDIOC_QUERYCTRL, &control);
        if(retVal == -1)
        {
            LOG_ERRNO_AS_ERROR("VIDIOC_QUERYCTRL");
            break;
        }
        id = control.id;
        LOG_INFO("Control 0x%X (%.24s)", id, control.name);
        LOG_INFO("Control type: %s", ctrlType2str(control.type));

        value = get_control_value(id);
        LOG_INFO("Control min=%i, max=%i, step=%i, default=%i, actual=%i",
                control.minimum, control.maximum, control.step, control.default_value,
                value);

        if(control.type == V4L2_CTRL_TYPE_MENU)
        {
            int j;
            for(j = control.minimum; j <= control.maximum; j+= control.step)
            {
                struct v4l2_querymenu menu;
                memset(&menu, 0, sizeof(menu));
                menu.id = id;
                menu.index = j;
                retVal = ioctl(m_fd, VIDIOC_QUERYMENU, &menu);
                if(retVal == -1)
                {
                    LOG_ERRNO_AS_ERROR("VIDIOC_QUERYMENU");
                    break;
                }
                LOG_INFO("%s %.32s", j == value ? "*" : " ", menu.name);
            }
        }
        LOG_INFO("Control flags=0x%X, %s", control.flags, ctrlFlag2str(control.flags));

        if(id == V4L2_CID_BRIGHTNESS)
        {
            m_brightness = id;
        }

//        switch(id)
//        {
//            case V4L2_CID_BRIGHTNESS:
//            case V4L2_CID_PAN_ABSOLUTE:
//            case V4L2_CID_TILT_ABSOLUTE:
//            case V4L2_CID_ZOOM_ABSOLUTE:
//            case V4L2_CID_PAN_RELATIVE:
//                set_control_value(info, id, control.maximum);
//                break;
//        }

//#define V4L2_CID_PAN_RELATIVE:
//#define V4L2_CID_TILT_RELATIVE			(V4L2_CID_CAMERA_CLASS_BASE+5)
//#define V4L2_CID_PAN_RESET			(V4L2_CID_CAMERA_CLASS_BASE+6)
//#define V4L2_CID_TILT_RESET			(V4L2_CID_CAMERA_CLASS_BASE+7)

//#define V4L2_CID_PAN_ABSOLUTE			(V4L2_CID_CAMERA_CLASS_BASE+8)
//#define V4L2_CID_TILT_ABSOLUTE			(V4L2_CID_CAMERA_CLASS_BASE+9)

//#define V4L2_CID_ZOOM_ABSOLUTE			(V4L2_CID_CAMERA_CLASS_BASE+13)
//#define V4L2_CID_ZOOM_RELATIVE			(V4L2_CID_CAMERA_CLASS_BASE+14)
//#define V4L2_CID_ZOOM_CONTINUOUS		(V4L2_CID_CAMERA_CLASS_BASE+15)


//    set_control_value(info, id, new_value);
//#define V4L2_CID_CONTRAST		(V4L2_CID_BASE+1)
//#define V4L2_CID_SATURATION		(V4L2_CID_BASE+2)
//#define V4L2_CID_HUE			(V4L2_CID_BASE+3)
//#define V4L2_CID_AUTO_WHITE_BALANCE	(V4L2_CID_BASE+12)
//#define V4L2_CID_DO_WHITE_BALANCE	(V4L2_CID_BASE+13)
//#define V4L2_CID_RED_BALANCE		(V4L2_CID_BASE+14)
//#define V4L2_CID_BLUE_BALANCE		(V4L2_CID_BASE+15)
//#define V4L2_CID_GAMMA			(V4L2_CID_BASE+16)
//#define V4L2_CID_WHITENESS		(V4L2_CID_GAMMA) /* Deprecated */
//#define V4L2_CID_EXPOSURE		(V4L2_CID_BASE+17)
//#define V4L2_CID_AUTOGAIN		(V4L2_CID_BASE+18)
//#define V4L2_CID_GAIN			(V4L2_CID_BASE+19)
    }
}

/**
 * Check and select camera input
 */
void Camera::check_input()
{
    struct v4l2_input input;
    int i;
    for(i = 0; i < MAX_INPUTS; i++)
    {
        int retVal;

        memset(&input, 0, sizeof(input));
        input.index = i;
        retVal = ioctl(m_fd, VIDIOC_ENUMINPUT, &input);
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
        int retVal = ioctl(m_fd, VIDIOC_G_INPUT, &port);
        if(retVal == -1)
        {
            return;
        }
        LOG_INFO("Input selected is: %i", port);
    }
}

/**
 * Check formats supported and go for the best, for now it picks the
 * first :)
 */
void Camera::check_format()
{
    /* There are 12 buffer types, but we are only interested in capture */
    static const int buf_types[] = {
        V4L2_BUF_TYPE_VIDEO_CAPTURE,
	V4L2_BUF_TYPE_VBI_CAPTURE,
	V4L2_BUF_TYPE_SLICED_VBI_CAPTURE,
	V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE,
    };

    struct v4l2_fmtdesc desc;
    unsigned i;
    unsigned j;
    LOG_INFO("Check Formats");
    for(j = 0; j < sizeof(buf_types)/sizeof(int); j++)
    {
        /* Lets stop after a large number of formats (v unlikely) */
        for(i = 0; i < MAX_FORMATS; i++)
        {
            int retVal;

            memset(&desc, 0, sizeof(desc));
            desc.index = i;
            desc.type = buf_types[j];

            retVal = ioctl(m_fd, VIDIOC_ENUM_FMT, &desc);
            if(retVal == -1)
            {
                break;
            }
            LOG_INFO("Fmt %u (%.32s)", desc.index, desc.description);
            LOG_INFO("Buffer type: %s", bufType2str(desc.type));
            LOG_INFO("Format %s", pixelfmt2str(desc.pixelformat));
            LOG_INFO("Fmt flags: 0x%x (%s)", desc.flags, fmtdescflag2str(desc.flags));
            m_buf_type = desc.type;
            m_pix_fmt = desc.pixelformat;
        }
    }
}

/**
 * Set Capture parameters
 */
void Camera::set_capture_params() const
{
    struct v4l2_streamparm params;
    struct v4l2_captureparm * capture = &params.parm.capture;
    int retVal;

    memset(&params, 0, sizeof(params));
    params.type = m_buf_type;
    retVal = ioctl(m_fd, VIDIOC_G_PARM, &params);
    if(retVal == -1)
    {
        LOG_ERRNO_AS_ERROR("VIDIOC_G_PARM");
        return;
    }

    LOG_INFO("Capture capability=0x%X (%s)",
            capture->capability, capcap2str(capture->capability));
    LOG_INFO("Capture mode=0x%X (%s)",
            capture->capturemode, capcap2str(capture->capturemode));
    LOG_INFO("Read buffers=%i", capture->readbuffers);
// struct v4l2_fract  timeperframe;  /*  Time per frame in seconds */
}

static void print_capture_format(struct v4l2_pix_format * pix)
{
    LOG_INFO("Res = %u x %u", pix->width, pix->height);
    LOG_INFO("Colorspace = %u (%s)", pix->colorspace, colorspace2str(pix->colorspace));
    LOG_INFO("Format %s", pixelfmt2str(pix->pixelformat));
    LOG_INFO("Field %u", pix->field);
    LOG_INFO("Bytes per line %u", pix->bytesperline);
    LOG_INFO("Image size %u", pix->sizeimage);
}

void Camera::set_format()
{
    struct v4l2_format fmt;
    struct v4l2_pix_format * pix;
    int retVal;

    memset(&fmt, 0, sizeof(fmt));
    fmt.type = m_buf_type;

    retVal = ioctl(m_fd, VIDIOC_G_FMT, &fmt);
    if(retVal == -1)
    {
        return;
    }
    pix = &fmt.fmt.pix;
    print_capture_format(pix);

    pix->sizeimage = pix->height * pix->bytesperline;

    retVal = ioctl(m_fd, VIDIOC_S_FMT, &fmt);
    if(retVal == -1)
    {
        LOG_ERRNO_AS_ERROR("VIDIOC_S_FMT");
        return;
    }

    retVal = ioctl(m_fd, VIDIOC_G_FMT, &fmt);
    if(retVal == -1)
    {
        return;
    }
    print_capture_format(pix);

    m_height = pix->height;
    m_width = pix->width;

//    memset(&fmt, 0, sizeof(fmt));
//    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
//    retVal = ioctl(fd, VIDIOC_G_FMT, &fmt);
//    if(retVal == -1)
//    {
//        return;
//    }
//    LOG_INFO("Res = %u x %u", fmt.fmt.pix_mp.width, fmt.fmt.pix_mp.height);
//    uint32_t px = fmt.fmt.pix_mp.pixelformat;
//    LOG_INFO("Format %s", pixelfmt2str(px));
//    LOG_INFO("Field %u", fmt.fmt.pix_mp.field);
}

int Camera::request_buffers(int max_num)
{
    unsigned i;
    int status;
    struct v4l2_requestbuffers reqbuf;
    uint8_t ** starts = 0;
    size_t * lengths = 0;

    memset(&reqbuf, 0, sizeof(reqbuf));
    reqbuf.type = m_buf_type;
    reqbuf.memory = V4L2_MEMORY_MMAP;
    reqbuf.count = max_num;

    status = ioctl(m_fd, VIDIOC_REQBUFS, &reqbuf);
    if(status == -1)
    {
        LOG_ERRNO_AS_ERROR("VIDIOC_REQBUFS");
        exit(EXIT_FAILURE);
    }
    starts = new uint8_t*[reqbuf.count * sizeof(void *)];
    lengths = new size_t[reqbuf.count * sizeof(size_t)];
    for(i = 0; i < reqbuf.count; i++)
    {
        struct v4l2_buffer buffer;
        uint8_t * start;

        memset(&buffer, 0, sizeof(buffer));
        buffer.type = reqbuf.type;
        buffer.memory = reqbuf.memory;
        buffer.index = i;

        status = ioctl(m_fd, VIDIOC_QUERYBUF, &buffer);
        if(status == -1)
        {
            LOG_ERROR("Failed to get buffer details");
            exit(EXIT_FAILURE);
        }
        start = reinterpret_cast<uint8_t *>(mmap(NULL, buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED,
                     m_fd, buffer.m.offset));
        if(start == MAP_FAILED)
        {
            LOG_ERROR("Failed to map buffer");
            exit(EXIT_FAILURE);
        }
        LOG_INFO("MMAP, %p, len=%u", start, buffer.length);
        starts[i] = start;
        lengths[i] = buffer.length;
    }
    m_buf_starts = starts;
    m_buf_lengths = lengths;
    m_num_bufs = reqbuf.count;
    return reqbuf.count;
}

/**
 * Enable the capture process
 */
void Camera::enable_capture()
{
    const int arg = m_buf_type;
    const int status = ioctl(m_fd, VIDIOC_STREAMON, &arg);
    if(status == -1)
    {
        LOG_ERROR("Failed to get buffer details");
        exit(EXIT_FAILURE);
    }
}

/**
 * Disable the capture process
 */
void Camera::disable_capture()
{
    const int arg = m_buf_type;
    const int status = ioctl(m_fd, VIDIOC_STREAMOFF, &arg);
    if(status == -1)
    {
        LOG_ERROR("Failed to get buffer details");
        exit(EXIT_FAILURE);
    }
}


void Camera::queue_buffer(int i)
{
    struct v4l2_buffer buffer;
    int status;

    memset(&buffer, 0, sizeof(buffer));
    buffer.type = m_buf_type;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.index = i;

    status = ioctl(m_fd, VIDIOC_QBUF, &buffer);
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
uint32_t Camera::query_buffer(int i)
{
    struct v4l2_buffer buffer;
    int status;

    memset(&buffer, 0, sizeof(buffer));
    buffer.type = m_buf_type;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.index = i;

    status = ioctl(m_fd, VIDIOC_QBUF, &buffer);
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
int Camera::wait_buffer_ready(uint32_t * bytes_avail)
{
    struct v4l2_buffer buffer;
    int status;

    memset(&buffer, 0, sizeof(buffer));
    buffer.type = m_buf_type;
    buffer.memory = V4L2_MEMORY_MMAP;

    status = ioctl(m_fd, VIDIOC_DQBUF, &buffer);
    if(status == -1)
    {
        LOG_ERROR("Failed to get buffer details");
        exit(EXIT_FAILURE);
    }
    *bytes_avail = buffer.bytesused;
    return buffer.index;

}

int Camera::check_quality(int n, int left, uint32_t bytes_avail)
{
    unsigned int min_luma = 255;
    unsigned int max_luma = 0;
    __u64 luma_sum = 0;
    int luma_mean;
    uint8_t * src = m_buf_starts[n];
    unsigned j;

    for(j = 0; j < bytes_avail/2; j++)
    {
        uint8_t val = *src++;
        src++;
        if(val > max_luma)
        {
            max_luma = val;
        }
        else if(val < min_luma)
        {
            min_luma = val;
        }
        luma_sum += val;
    }
    luma_mean = (int)(2*(luma_sum/bytes_avail));
    LOG_INFO("Luma, min=%i, max=%i, mean=%i", min_luma, max_luma,
            luma_mean);
    if(luma_mean > 128)
    {
        set_control_value(m_brightness, get_control_value(m_brightness)-1);
    }
    else
    {
        set_control_value(m_brightness, get_control_value(m_brightness)+1);
    }
    return left == 0;
}

int main(int argc, char * argv[])
{
    Camera info;
    int i;
    int n;
    uint32_t capability;

    (void) argc;
    (void) argv;

    set_logging_level(5);

    capability = info.check_capabilities();
    if(capability == 0) {
        return 1;
    }

    info.check_standards();

    info.check_controls();

    info.check_input();

    info.check_format();

    info.set_format();


    n = info.request_buffers(10);
    for(i = 0; i < n; i++)
    {
        info.queue_buffer(i);
    }

    info.set_capture_params();
    info.enable_capture();
    for(i = 0; i < 100; i++)
    {
        uint32_t bytes_avail;
        int n = info.wait_buffer_ready(&bytes_avail);
        int max_val = 0;
        char fname[30];
        uint8_t frame[640*480];
        FILE * f;

        if(!info.check_quality(n, 99-i, bytes_avail))
        {
            info.queue_buffer(n);
            continue;
        }
        snprintf(fname, sizeof(fname), "image.pgm");
        LOG_INFO("%i %u", n, bytes_avail);
        f = fopen(fname, "wb");
        if(f)
        {
            uint8_t * src = info.buf_start(n);
            uint8_t * dst = frame;
            uint32_t px;
            unsigned j;
            for(j = 0; j < bytes_avail/2; j++)
            {
                uint8_t val = *src++;
                *dst++ = val;
                src++;
                if(val > max_val)
                    max_val = val;
            }
            fprintf(f, "P5\n%i %i\n%i\n", info.width(), info.height(), max_val);
            px = info.pix_fmt();
            fprintf(f, "#FOURCC %c%c%c%c\n",
                px & 0xff, (px >> 8) & 0xff, (px >> 16) & 0xff, (px >> 24) & 0xff);
            fwrite(frame, bytes_avail/2, 1, f);
            fclose(f);
        }
        break;
    }
    info.check_controls();
    info.disable_capture();

    info.close();
    return 0;
}
