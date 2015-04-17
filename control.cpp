#include "control.h"

BaseControl * create_control(uint32_t id, unsigned ctrlType)
{
    switch(ctrlType)
    {
        case IntControl::CTRL_TYPE:
            return new IntControl(id);
    }
    return NULL;
}


