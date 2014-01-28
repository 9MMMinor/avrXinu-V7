#ifndef WLP_NTP_H
#define WLP_NTP_H

#include <stdint.h>

#include <wlp_api.h>

#include <owl/core/owl_shell.h>

struct ntp_tm {
        uint8_t sec;
        uint8_t min;
        uint8_t hour;
        uint8_t mday;
        uint8_t mon;   /* 0-indexed */
        uint16_t year; /* since 1900 */

        uint16_t yday;
        uint8_t wday;
};


/**
 * Request time from an NTP server. done_cb will be invoked upon completion
 * with the time formatted in a ntp_tm struct. If tm is NULL, the request
 * failed.
 *
 * @param server is the NTP server
 * @param done_cb will be invoked when a reply with the current time has
 *                been received or upon a timeout.
 * @param ctx opaque context pointer that will be passed to done_cb.
 *
 * @return 0 if NTP request could be sent, error code otherwise.
 *  
 *
 */
int ntp_gettime(struct ip_addr *server,
                 void done_cb(void *ctx, struct ntp_tm *tm, int err),
                 void *ctx);


/**
 * Convert a unix time to ntp_tm struct.
 *
 * @param unixtime is the time in UNIX format to convert
 * @param tm is the time struct that will be filled in
 *
 */
int ntp_offtime(const uint32_t *t, struct ntp_tm *tm);

extern struct owlsh_cmd cmd_ntp;


#endif
