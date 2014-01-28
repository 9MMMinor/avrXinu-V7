#ifdef __avrXinu__
#include <avr-Xinu.h>
#else
#include <stdio.h>
#endif

#define OWL_TRACE OWL_TRACE_ERR

#include <string.h>
#include <stdlib.h>

#include <wlp_api.h>
#include <wlp_inet.h>

#include <owl/core/owl_debug.h>
#include <owl/drivers/owl_led.h>
#include <owl/drivers/owl_temp.h>
#include "web_content.h"
#include "web_logo.h"


/*!
 * \file web_content.c
 * \brief wlp_httpd web server content
 *
 * The following steps are needed to add pages to the web server
 * 
 * 1. Add the name of the new page to \a http_get()
 * 2. Implement a file access function in web_content.c. The function
 *    should be called from \a http_get() and should use the local
 *    reply_str() or reply_buf() functions to write the contents of the file
 *    (HTML page) to the web browser at the other end of the connection.
 * 3. If the new page contains HTTP form elements, add a function
 *    to web_contents.c to handle the form fields and call it from
 *    \a http_get() if the request path matches the file name
 *    with an appended '?' (see the handling of "index.html?" as
 *    an example).
 *
 */

static int reply_str(int sock, const char *buf);
static int reply_buf(int sock, const char *buf, int len);

/* Use progmem on AVR8 to save ram memory since we can't keep const strings
 * in flash without using the PROGMEM or PSTR attribute
 */
#if defined __avr8__ || defined __avrXinu__
# include <avr/pgmspace.h>
static int reply_str_P(int sock, const char *buf);
static int reply_buf_P(int sock, const char *buf, int len);
#define HTTP_BUFSIZE 64

/* On linux, we dont use any PROGMEM stuff */
#else
# define PSTR(str) str
# define reply_str_P reply_str
# define reply_buf_P reply_buf
#endif /* __avr8__ */



static int file_index(int sock);
static int file_logo(int sock);
static int file_404(int sock);
static int file_index_form_field(int sock, char *fields);


/* Lookup the file specified by path and send it as a
 * reply on the connected socket. */
int http_get(char *path, int sock)
{

        owl_dbg("path:%s", path);
        
        /* The path "/" is and alias for "index.html" */
        if ( strcmp(path, "/") == 0 || strcmp(path, "/?") == 0) {
                return file_index(sock);
        } 
        /* If we have a trailing ? character we expect
         * the rest of the path to be a list of form fields
         * on the form "<name>=<value>?<name>=<value>..."
         * These fields are handled by a special function.
         */
        else if ( strncmp(path, 
                          "/index.html?", 
                          12) == 0 ) {
                return file_index_form_field(sock, 
                                             path + 12);
        }
        else if ( strcmp(path, "/index.html") == 0 ) {
                return file_index(sock);
        }
        else if ( strcmp(path, "/logo.gif") == 0 ) {
                return file_logo(sock);
        }
        /* If the path didn't match any supported file then
         * we return a HTTP 404 error page. */
        else {
                return file_404(sock);
        }
        return -1;
}


/* Handle form fields on the index page. */
static int file_index_form_field(int sock, char *fields) {
        int led_index;
        owl_info("Field \"%s\"", fields);
        if ( 0 == siscanf(fields, "toggle=LED%d", &led_index) ) {
                return file_index(sock);
        }
        owlled_toggle(led_index);
        
        return file_index(sock);
}


static int file_logo(int sock)
{
        int err;
        
        /* http header */
        if ((err = reply_str_P(
                     sock,
                     PSTR("HTTP/1.0 200 OK\n"
                          "Server: wlp_httpd\n"
                          "Content-type: image/gif\n\n"))) < 0)
                return err;

        /* image data */
        if ((err = reply_buf_P(sock, web_logo, sizeof(web_logo))) < 0)
                return err;

        return 0;
}

static int file_index(int sock)
{
        int err;
        struct ip_addr ip;
        struct wl_mac_addr_t mac;
        char macstr[18] = "N/A";
        char *ipstr = "N/A";
        
        if ( WLP_INVALID_SOCKET_ID == sock ) {
                return -1;
        }

        if ((err = reply_str_P(
                     sock,
                     PSTR("HTTP/1.0 200 OK\n"
                          "Server: wlp_httpd\n"
                          "Content-type: text/html\n\n"
                          "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 "
                          "Frameset//EN\" "
                          "\"http://www.w3.org/TR/html4/frameset.dtd\">"
                          "<html>"
                          "<head><title>oWL pico HTTP Web server</title>"
                          "</head>"
                          "<body style=font-family:arial>"
                          "<table><tr><td width=100></td><td>"
                          "<table><tr valign=\"top\"><td with>"
                          "<img src=\"logo.gif\" border=\"0\" alt=\"logo\""
                          "title=\"logo\">" 
                          "</td></tr>"
                          "<tr><td width=\"500\">"
                          "<h2>Web Server running on oWL pico</h2>"
                          "<p>"
                          "The web page is served by wlp_httpd using "
                          "the SPB800 WLAN module and the "
                          "oWL-pico&copy; Software API; "
                          "a unique Wi-Fi solution from <b>H&D Wireless AB</b>."
#if defined __avrXinu__
						  "  The host is an ATmega1284p microprocessor running "
						  "the <b>Xinu</b> operating system."
#endif
                          "</p><hr><table>"))) < 0)		{
				owl_err("reply_str_P: %d", err);
                return err;
				}

        if (wlp_get_ipaddr(&ip, NULL, NULL, NULL) == 0)
                ipstr = inet_ntoa(&ip);
        
        if ((err = reply_str_P(sock, PSTR("<tr><td>IP Address: "))) < 0)
                return err;
        
        if ((err = reply_str(sock, ipstr)) < 0)
                return err;
        
        if ((err = reply_str_P(sock, PSTR("</td><td><b></b></td></tr>"))) < 0)
                return err;
        
        if (wlp_get_hwaddr(&mac) == 0)
                sniprintf(macstr, sizeof(macstr),
                          "%02x:%02x:%02x:%02x:%02x:%02x",
                          mac.octet[0], mac.octet[1], mac.octet[2],
                          mac.octet[3], mac.octet[4], mac.octet[5]);
        
        if ((err = reply_str_P(sock, PSTR("<tr><td>MAC Address: "))) < 0)
                return err;

        if ((err = reply_str(sock, macstr)) < 0)
                return err;

        if ((err = reply_str_P(sock, PSTR("</td><td><b></b></td></tr>"))) < 0)
                return err;

        if ((err = reply_str_P(sock, PSTR("<tr><td>Temperature: "))) < 0)
                return err;
        
        {
                int temp = owltemp_get_temp();
                char tempstr[16];

                sniprintf(tempstr, sizeof tempstr,
                          "%d C", temp);
                if ((err = reply_str(sock, tempstr)) < 0)
                        return err;
        }
        
        if ((err = reply_str_P(sock, PSTR("</td><td><b></b></td></tr>"))) < 0)
                return err;

        if ((err = reply_str_P(
                     sock,
                     PSTR("<tr><td><form method=\"get\">"
                          "<input type=\"submit\" style=\"height: "
                          "50px\" value=\"Refresh\"/>"
                          "</form></td></tr></table><br>"
                          "<form id=\"form\""
                          "name=\"form\" method=\"get\" "
                          "action=\"index.html\">"
                          "<fieldset style=\"width: 300;\">"
                          "<legend>LED control</legend>"
                          "<input type=\"submit\" style=\"height: "
                          "50px\" name=\"toggle\" "
                          "value=\"LED0\"/>&nbsp&nbsp"
                          "<input type=\"submit\" style=\"height: "
                          "50px\" name=\"toggle\" "
                          "value=\"LED1\"/>&nbsp&nbsp"
                          "<input type=\"submit\" style=\"height: "
                          "50px\" name=\"toggle\" "
                          "value=\"LED2\"/>&nbsp&nbsp"
                          "<input type=\"submit\" style=\"height: "
                          "50px\" name=\"toggle\" "
                          "value=\"LED3\"/>&nbsp&nbsp<p>"))) < 0) {
                return err;
        }

        if ((err = reply_str_P(
                     sock,
                     PSTR("<input type=\"submit\" style=\"height: "
                          "50px\" name=\"toggle\" "
                          "value=\"LED4\"/>&nbsp&nbsp"
                          "<input type=\"submit\" style=\"height: "
                          "50px\" name=\"toggle\" "
                          "value=\"LED5\"/>&nbsp&nbsp"
                          "<input type=\"submit\" style=\"height: "
                          "50px\" name=\"toggle\" "
                          "value=\"LED6\"/>&nbsp&nbsp"
                          "<input type=\"submit\" style=\"height: "
                          "50px\" name=\"toggle\" "
                          "value=\"LED7\"/>&nbsp&nbsp"
                          "</fieldset></form></td>"
                          "<td>&nbsp;</td></tr></table>"
                          "</td><td width=20%%></td></tr></table>"
                          "</body></html>"))) < 0) {
                return err;
        }
		
		if ((err = reply_str_P(
				sock,
				PSTR(	"Comments:<br />"
						"<textarea name=\"comments\" id=\"comments\">"
						"Hey... say something!"
						"</textarea><br />"
						"<input type=\"submit\" value=\"Submit\" />"
						"</form><p style=\"font-size:10px;\"></p>"))) < 0)	{
					return err;
				}


        return 0;
}

/* Write a HTTP 404 error page to the socket. */
static int file_404(int sock) {
        int err;
        
        if ( WLP_INVALID_SOCKET_ID == sock ) {
                return -1;
        }

        if ((err = reply_str_P(
                     sock,
                     PSTR("HTTP/1.0 404 File not found\n"
                          "Server: wlp_httpd\n"
                          "Content-type: text/html\n\n"
                          "<html>"
                          "<head><title>oWL pico HTTP Web server</title></head>"
                          "<body>"
                          "<h2>404 - Page not found</h2>"
                          "<p>"
                          "Sorry, the page you are requesting was not found on "
                          "this server."
                          "</p></body></html>"))) < 0)
                return err;

        return 0;
}


/* Write the specified buffer to the socket, looping until
 * the whole buffer has been sent. */
static int reply_buf(int sock, const char *buf, int len)
{
        int err;
        int pos = 0;
        while (pos < len) {
                if ((err = wlp_send(sock, buf + pos, len - pos)) < 0)
                        return err;

                pos += err;
        }
        
        return 0;
}


/* Convenience function for writint a NULL terminated string to the socket */
static int reply_str(int sock, const char *str)
{
        return reply_buf(sock, str, strlen(str));
}



#if defined __avr8__ || defined __avrXinu__

/* Write the specified PROGMEM buffer to the socket using the wlp_send()
 * method in the wlp_api, looping until the whole buffer has been sent.
 *
 * This function is specific for avr8 architectures when sending data that
 * lives in the internal flash memory.
 */
static int reply_buf_P(int sock, const char *buf, int len)
{
        char *rambuf;
        int pos = 0;
		int err;

        if ((rambuf = malloc(HTTP_BUFSIZE)) < 0)
                return -1;
        
        while (pos < len) {
                int chunklen = len - pos;

                /* copy chunk of data from progmem to ram buffer */
                if (chunklen > HTTP_BUFSIZE)
                        chunklen = HTTP_BUFSIZE;
                memcpy_P(rambuf, &buf[pos], chunklen);

                /* send the chunk */
				if ((err = wlp_send(sock, rambuf, chunklen)) < 0)	{
                        free(rambuf);
                        return err;
                }
                /* update input buffer position by the number of bytes
                 * actually sent.
                 */
                pos += err;
        }
        free(rambuf);
        return 0;
}

/* Convenience function for writint a NULL terminated string to the socket */
static int reply_str_P(int sock, const char *str)
{
        return reply_buf_P(sock, str, strlen_P(str));
} 

#endif /* __avr8__ || __avrXinu__ */



