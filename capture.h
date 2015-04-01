#ifndef _CAPTURE_H_
#define _CAPTURE_H_

#include <stdint.h>

struct Control_s
{
    uint32_t id;
    int32_t minimum;	/* Note signedness */
    int32_t maximum;
    int32_t step;
    int32_t value;
};

typedef struct Control_s Control_t;

class Camera
{
private:
    int m_fd;
    int m_buf_type; /* see enum v4l2_buf_type */
    
    unsigned m_width;
    unsigned m_height;
    uint32_t m_pix_fmt; /* The pixel format */

    uint8_t ** m_buf_starts;
    size_t * m_buf_lengths;
    int m_num_bufs;
    
    uint32_t m_brightness;
    uint32_t m_contrast;

private:
    void set_control_value(int id, int32_t value);
    int32_t get_control_value(int id);
    void set_control(int id, float percent);
    uint32_t query_buffer(int i);

public:
    Camera();
    int check_quality(int n, int left, uint32_t bytes_avail);
    int request_buffers(int max_num);
    int wait_buffer_ready(uint32_t * bytes_avail);
    void set_format();
    uint32_t check_capabilities();
    void check_standards();
    void close() {::close(m_fd); m_fd=0;};
    void check_controls();
    void disable_capture();
    unsigned height() const {return m_height;};
    unsigned width() const {return m_width;};
    uint8_t * buf_start(int n) const {return m_buf_starts[n];};
    void set_capture_params() const;
    void queue_buffer(int i);
    void enable_capture();
    void check_format();
    void check_input();
    uint32_t pix_fmt() const { return m_pix_fmt;};
};

#endif
