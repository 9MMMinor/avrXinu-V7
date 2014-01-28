#ifndef _HTTPD_H
#define _HTTPD_H

struct httpd;

struct httpd *httpd_create(int port);
void httpd_destroy(struct httpd *priv);

#endif
