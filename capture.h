#ifndef _CAPTURE_H_
#define _CAPTURE_H_

#include <string>

#include <stdint.h>
#include <stdbool.h>

#include "format.h"

class BaseFormat;
class BaseControl;

class Camera
{
private:
    int m_fd;
    int m_buf_type; /* see enum v4l2_buf_type */
    
    BaseFormat * m_formatObj;

    uint8_t ** m_buf_starts;
    size_t * m_buf_lengths;
    int m_num_bufs;
    
    BaseControl * m_brightness;
    uint32_t m_contrast;
    int m_input;

private:
    void set_control_value(int id, int32_t value);
    int32_t get_control_value(int id);
    void set_control(int id, float percent);
    uint32_t query_buffer(int i);
    bool check_can_do_capture() const;
    bool select_camera_input();
    bool find_suitable_input();
    bool set_input();
    uint32_t find_suitable_format();
    bool set_format(uint32_t pixelformat);

public:
    Camera(std::string &);
    ~Camera();
    bool init();
    bool select_format();
    unsigned height() const {return m_formatObj ? m_formatObj->height() : 0;};
    unsigned width() const {return m_formatObj ? m_formatObj->width() : 0;};

    int check_quality(int n, int left, uint32_t bytes_avail);
    int request_buffers(int max_num);
    int wait_buffer_ready(uint32_t * bytes_avail);
    void check_standards();
    void close();
    void check_controls();
    void disable_capture();
    uint8_t * buf_start(int n) const {return m_buf_starts[n];};
    void set_capture_params() const;
    void queue_buffer(int i);
    void enable_capture();
    void check_format();
    void check_input();
    BaseFormat * fmt() const { return m_formatObj;};
};

#endif
