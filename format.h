#ifndef _FORMAT_H_
#define _FORMAT_H_

#include <stdint.h>
#include <linux/videodev2.h>

#include <string>

struct ImageQuality
{
    unsigned int luma_mean;
    unsigned int luma_max;
    unsigned int luma_min;
};

class BaseFormat 
{
protected:
    unsigned m_width;
    unsigned m_height;
    unsigned m_bytesperline;

public:
    virtual uint32_t pix_fmt() const = 0;
    virtual void check_quality(uint8_t *, unsigned, ImageQuality &) const = 0;
    const std::string pix_fmt_str() const;
    void init(unsigned width, unsigned height, unsigned bytesperline);
    unsigned height() const {return m_height;};
    unsigned width() const {return m_width;};
};

BaseFormat * create_format_obj(uint32_t pixelformat);

/**
 * In this format each four bytes is two pixels.
 * Each four bytes is two Y's, a Cb and a Cr.
 */
class YUYV : public BaseFormat
{
public:
    static const uint32_t PIX_FMT = V4L2_PIX_FMT_YUYV;
    virtual uint32_t pix_fmt() const;
    virtual void check_quality(uint8_t *, unsigned, ImageQuality &) const;
};

#endif
