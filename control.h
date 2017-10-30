#ifndef _CONTROL_H_
#define _CONTROL_H_

#include <string>

#include <stdint.h>
#include <stdbool.h>

#include <linux/videodev2.h>

class CtrlCallback
{
public:
    virtual bool set_control_value(int, int32_t) = 0;
    virtual int32_t get_control_value(int) = 0;
};

class BaseControl
{
protected:
    int m_id;
    int32_t m_minimum;	/* Note signedness */
    int32_t m_maximum;
    int32_t m_step;
    int32_t m_value;
    CtrlCallback * m_callbackObj;

public:
    BaseControl(int id) : m_id(id) {};
    int get_id() const { return m_id; };
    void store(int32_t value) { m_value = value; };
};

BaseControl * create_control(uint32_t id, unsigned ctrlType);


class IntControl : public BaseControl
{
public:
    static const unsigned CTRL_TYPE = V4L2_CTRL_TYPE_INTEGER;
    IntControl(uint32_t id) : BaseControl(id) {};
};


#if 0
	V4L2_CTRL_TYPE_BOOLEAN	     = 2,
	V4L2_CTRL_TYPE_MENU	     = 3,
	V4L2_CTRL_TYPE_BUTTON	     = 4,
	V4L2_CTRL_TYPE_INTEGER64     = 5,
	V4L2_CTRL_TYPE_CTRL_CLASS    = 6,
	V4L2_CTRL_TYPE_STRING        = 7,
	V4L2_CTRL_TYPE_BITMASK       = 8,
	V4L2_CTRL_TYPE_INTEGER_MENU  = 9,
#endif

	

#endif
