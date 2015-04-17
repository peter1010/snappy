#include <string>

#include "format.h"

const std::string BaseFormat::pix_fmt_str() const
{
    std::string buf;
    int i;
    const uint32_t px = pix_fmt();
    for(i = 0; i < 4; i++) {
        buf.append(1, static_cast<char>((px >> 8*i) & 0xff));
    }
    return buf;
}

void BaseFormat::init(unsigned width, unsigned height, unsigned bytesperline)
{
    m_width = width;
    m_height = height;
    m_bytesperline = bytesperline;
}

BaseFormat * create_format_obj(uint32_t pixelformat)
{
    switch(pixelformat)
    {
    case YUYV::PIX_FMT:
        return new YUYV();
    }
    return NULL;
}


uint32_t YUYV::pix_fmt() const {return PIX_FMT;};

void YUYV::check_quality(uint8_t * data, unsigned bytes, ImageQuality & qual) const
{
    unsigned int min_luma = 255;
    unsigned int max_luma = 0;
    __u64 luma_sum = 0;
//    int luma_mean;
    unsigned x, y;

    const unsigned extra = m_bytesperline - m_width*2;
    for(y = 0; y < m_height; y++) {
        for(x = 0; x < m_width; x++) {
            const uint8_t val = *data++;
            data++;
            if(val > max_luma) {
                max_luma = val;
            }
            else if(val < min_luma) {
                min_luma = val;
            }
            luma_sum += val;
        }
        data += extra;
    }
    qual.luma_mean = static_cast<int>(luma_sum/(m_width*m_height));
    qual.luma_max = max_luma;
    qual.luma_min = min_luma;
}
