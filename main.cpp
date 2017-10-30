#include <dirent.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>

#include <string>
#include <vector>

#include "capture.h"
#include "format.h"
#include "logging.h"

#define V4L2_MAJOR  (81)

/**
 * Give a devpath, check that it is a devnode for a v4l2 device
 * (should do this before attempting to open a device)
 *
 * @param[in] devpath The devpath
 *
 * @return true if so
 */
static bool check_is_camera_dev(std::string & devpath)
{
    struct stat buf;
    const int status = ::stat(devpath.c_str(), &buf);
    if(status == 0) {
        if((buf.st_mode & (S_IFBLK | S_IFCHR)) != 0) {
            /* A dev node */
            if(major(buf.st_rdev) == V4L2_MAJOR) {
                return true;
            }
        }
    }
    return false;
}

void check_dir_for_camera_dev(std::vector<std::string> & poss,
        const char * path, const char * prefix)
{
    const size_t prefix_len = prefix ? strlen(prefix) : 0;
    DIR * dir = opendir(path);
    if(dir) {
        struct dirent * entry;
        while(NULL != (entry = readdir(dir))) {
            if((entry->d_name[0] == '.') 
                && (((entry->d_name[1] == '.') 
                  && (entry->d_name[2] == '\0'))
                  || (entry->d_name[1] == '\0'))) {
                continue;
            }
            if(prefix_len > 0) {
                if(strncmp(entry->d_name, prefix, prefix_len) != 0) {
                    continue;
                }
            }
            std::string devpath(path);
            devpath += "/";
            devpath += entry->d_name;
            if(check_is_camera_dev(devpath)) {
                LOG_DEBUG("Found file %s", devpath.c_str());
                poss.push_back(devpath);
            }
        }
        closedir(dir);
    }
}

#if 0
void parse_for_dev_details(string & filepath)
{
    filepath += "/dev";
    FILE * in_fp = fopen(filepa
}
#endif

/**
 * Find the a list of devices that are v4l devices
 */
Camera * find_camera_dev()
{
    std::vector<std::string> poss;

    check_dir_for_camera_dev(poss, "/dev/v4l/by-path", NULL);
    if(poss.empty()) {
        check_dir_for_camera_dev(poss, "/dev", "video");
    }
    std::vector<std::string>::iterator p = poss.begin();
    while(p != poss.end()) {
        LOG_INFO("Possible camera %s", p->c_str());
        Camera * info = new Camera(*p);
        if(info->init()) {
            return info;
        }
        delete info;
        p++;
    }
    return NULL;
}


int main()
{
    set_logging_level(10);
    LOG_INFO("Starting");
    Camera * cam = find_camera_dev();
    if(!cam) {
        LOG_ERROR("No camera found");
        return EXIT_FAILURE;
    }
    cam->select_format();

//  cam->check_standards();
    cam->check_controls();

    int i;
    int n = cam->request_buffers(10);
    for(i = 0; i < n; i++) {
        cam->queue_buffer(i);
    }

    cam->set_capture_params();
    cam->enable_capture();

    for(i = 0; i < 100; i++) {
        uint32_t bytes_avail;
        int n = cam->wait_buffer_ready(&bytes_avail);
        int max_val = 0;
        char fname[30];
        uint8_t frame[640*480];
        FILE * f;

        if(!cam->check_quality(n, 99-i, bytes_avail)) {
            cam->queue_buffer(n);
            continue;
        }
        snprintf(fname, sizeof(fname), "image.pgm");
        LOG_INFO("%i %u", n, bytes_avail);
        f = fopen(fname, "wb");
        if(f) {
            uint8_t * src = cam->buf_start(n);
            uint8_t * dst = frame;
            unsigned j;
            for(j = 0; j < bytes_avail/2; j++) {
                uint8_t val = *src++;
                *dst++ = val;
                src++;
                if(val > max_val)
                    max_val = val;
            }
            fprintf(f, "P5\n%i %i\n%i\n", cam->width(), cam->height(), max_val);
            fprintf(f, "#FOURCC %s\n", cam->fmt()->pix_fmt_str().c_str());
            fwrite(frame, bytes_avail/2, 1, f);
            fclose(f);
        }
        break;
    }
    cam->check_controls();
    cam->disable_capture();

    cam->close();
    return 0;
}
