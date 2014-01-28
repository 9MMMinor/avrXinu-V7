/*
 * Programming interface for oWL Pico API, wlp_api.
 * Copyright (C) 2011 HD Wireless AB
 *
 */

/*! \file wlp_api.h
 *  
 * \author H&D Wireless AB
 * \brief oWL Pico API
 * 
 * \section intro Introduction
 * This is the documentation for the oWL Pico API.
 *
 * \mainpage oWL Pico API Reference Manual
 * See pico.hd-wireless.se for the lastest documentation, software and
 * tutorials.
 * 
 * oWL Pico API is a programming interface for Wi-Fi (802.11) and network
 * applications. It aims to be a complete interface for embedded
 * applications wanting to use wireless as a communications interface.
 * 
 * The oWL Pico software API has a footprint of a few kilobytes and
 * provides functions to support full Wi-Fi link management, network
 * address management and TCP, UDP and RAW sockets - resembling a
 * standard BSD socket interface.
 *
 * The oWL Pico API has been designed to meet the following goals :
 * \li Simple : The API is as simple as is practicable
 * to make it easy to use. 
 * \li Minimal size : The API is usable on very resource constrained
 * platforms. 
 * \li Portable : The API is deployable on any platform with a standards
 * compliant C compiler and a UART port.
 * \li OS independent : The API is deployable on systems using a real time
 * operating system as well as with applications running on the
 * "bare metal" of a hardware platform (that is without an operating system).
 *
 * 
 * The API is structured into these functional groups:
 *
 * \li \ref wlp_api
 * \li \ref wlp_link
 * \li \ref wlp_network
 * \li \ref wlp_socket
 * \li \ref wlp_ap
 *
 * \section wlp_api_princ Operation Principles
 * 
 * There are three important properties of the oWL Pico API to keep in
 * mind when programming with it.
 * 
 * The first is that oWL Pico API is \b asynchronous. For instance, when the
 * \a wlp_linkup() function is called to attempt connection with an access
 * point it will trigger a sequence of packets being exchanged with the
 * access point after which, if everything is okay, a connection has been
 * established. The \a wlp_linkup() call is asynchronous (or non-blocking)
 * which means that you don't know if the connection attempt has
 * succeeded after the call returns. You only know if the sequence was
 * successfully started or not. To find out if, and when, the connection
 * attempt was successful you must register an event handler using the
 * function \a wlp_set_link_cb(). This is true of a number of API calls
 * (which is indicated in their documentation).
 *
 * The second important property is that the oWL Pico API is \b polled. oWL
 * Pico API never executes "by itself", since it would then have to
 * support interrupts, timers, locks and other operating system dependent
 * features. Instead all asynchronous processes proceed when oWL Pico API
 * is polled by calling the \a wlp_poll() function. When \a wlp_poll() is
 * called the oWL Pico API reacts to any received packets, expires any
 * internal timers and performs any other tasks necessary for forward
 * progress. After \a wlp_poll() returns nothing will happen unless it or
 * some other oWL Pico API function is called again.
 *
 * The third important property is that oWL Pico API is \b not \b thread
 * \b safe. All oWL Pico API calls must execute in the same context since
 * the library has no knowledge of the locking mechanisms available (if
 * any).
 * 
 * \section contactinfo Contact Information
 * For further information, visit
 * <a href="http://www.hd-wireless.se/">H&D Wireless</A>.\n
 * Support and documentation: http://pico.hd-wireless.se/
 * 
 */

#ifndef WLP_API_H
#define WLP_API_H

#include <stdint.h>

#include "wlp_proto.h"


#define WLP_FAILURE      -1  /*!< General failure */
#define WLP_ERR_PARAM    -2  /*!< Parameter error */
#define WLP_ERR_MEM      -3  /*!< Out of memory */
#define WLP_ERR_CONN     -4  /*!< No connection */
#define WLP_ERR_STATE    -5  /*!< Invalid state */
#define WLP_ERR_PARSE    -6  /*!< Input parsing error */
#define WLP_ERR_PATH     -7  /*!< Path not found */
#define WLP_ERR_TYPE     -8  /*!< Incompatible type */
#define WLP_ERR_PERM     -9  /*!< No permission */
#define WLP_ERR_NOTSUPP  -10 /*!< Operation not supported */
#define WLP_ERR_RANGE    -11 /*!< Out of range */
#define WLP_ERR_SIZE     -12 /*!< Bad size */
#define WLP_ERR_AGAIN    -13 /*!< Retry later */
#define WLP_ERR_BUSY     -14 /*!< Busy */
#define WLP_ERR_DISABLED -15 /*!< Function is disabled */
#define WLP_ERR_PROTO    -16 /*!< Protocol error */
#define WLP_ERR_TIMEOUT  -17 /*!< Timeout */
#define WLP_ERR_IGNORED  -18 /*!< Ignored, no effect */
#define WLP_ERR_CANCEL   -19 /*!< Canceled by remote */
#define WLP_ERR_SYNC     -20 /*!< Sync error */
#define WLP_ERR_ABORT    -21 /*!< Aborted */
#define WLP_ERR_INUSE    -22 /*!< Already in use */

/*! \ingroup wlp_api */
#define WLP_INVALID_SOCKET_ID 0


/*! \ingroup wlp_api */
#define WLP_VERSION_CODE WLP_VERSION(1,2,2)



/*! \ingroup wlp_api
 * Wireless network information structure.
 *
 *
 */
struct wlp_network_t {
        struct wl_ssid_t ssid;          /*!< The SSID of the network */
        struct wl_mac_addr_t bssid;     /*!< The BSSID of the network */
};



/*! \defgroup wlp_api Library support functions
 *
 * These functions manage the library in general.
 *
 * @{
 *
 */ 

 
/**
 * \brief Initialize the wifi device.
 *
 * Initialize the wlp api and configure the uart settings on the wifi device.
 * Must be called before any other function in the wlp api.
 *
 * wlp_init() will send a 'w' character, a 'w' will be received once the wifi
 * device is ready. The 'w' chars must be transmitted and received at 57600 8N1.
 *
 * Note again that the uart which the callback parameters readf and writef
 * uses must be configured for 57600 8N1 before calling wlp_init().
 *
 * Allow the wifi device up to 200 ms to boot before calling wlp_init() after
 * power up.
 *
 * The sequence on the host becomes
 *  1) Power on host and wifi device
 *  2) Wait 200 ms
 *  3) Call wlp_init() with host uart configured as 57600 8N1
 *  4) Done, now the wifi device is ready to use.
 *
 * @param readf is a pointer to a function which reads chars from the bus which
 *              the wifi device is connected to and stores them in the location
 *              pointed to by the data paramter. Note that readf should block
 *              until the number of bytes specified in the len parameter are
 *              read.
 * @param writef should read chars from the location pointed to by the data
 *              paramter send them to the bus which the wifi device is connected
 *              to. Note that writef should block until the number of bytes
 *              specified in the len parameter are written.
 * @param ctx is an opaque context pointer that will be passed back to readf
 *              and writef.
 * 
 * @return
 *   - 0 on success
 *
 */
int wlp_init(int (*readf)(void *ctx, void *data, int len),
			 int (*writef)(void *ctx, const void *data, int len),
             void *ctx);


/**
 * \brief Change the baudrate of the device.
 *
 * Send a uart configuration message which will configure the wifi device
 * according to the provided baudrate and rtscts parameters.
 *
 * Upon successful return, the wifi device will use the new uart configuration.
 * The host must change its uart settings accordingly before any other function
 * in wlp_api is called.
 *
 * The sequence on the host becomes
 *  1) Call wlp_set_baudrate() to reconfigure the device baudrate.
 *  2) Wait 100 ms afer wlp_set_baudrate() has returned to allow the device to
 *     reconfigure its uart.
 *  3) Reconfigure host uart baudrate.
 *  4) Done, now the wifi device is ready to use with the new baudrate setting.
 *
 * The device supports the following baudrates:
 *
 * \verbatim
 *
 *    Baudrate | Error (%)
 *    --------------------
 *       9600  | 0
 *      19200  | 0
 *      38400  | 0
 *      57600  | 0.08
 *     115200  | 0.16
 *     230400  | 0.16
 *     460800  | 0.16
 *     921600  | 0.16
 *    1000000  | 0
 *    1500000  | 0
 *    2000000  | 0
 *    3000000  | 0
 *    4000000  | 0
 *
 * \endverbatim
 *
 * Note that the actual baudrate used might differ from the desired baudrate.
 * This is due to the fractional baud rate generation. It is recommended to
 * keep the sum of the transmitter and receiver error less than 2.00 %. As shown
 * by the table above, the SPB800 has a very small fractional error on the
 * baudrates.
 * 
 * @param baudrate is the baudrate which the wifi device should switch to.
 * @param rtcts is the hardware handshake configuration which the wifi device
 *              should switch to. Set to 1 to enabled hardware handshaking, and
 *              0 to disable hardware handshaking. Any hardware handshake will
 *              use the RTS and CTS pins.
 * @param readf is a pointer to a function which reads chars from the bus which
 *              the wifi device is connected to and stores them in the location
 *              pointed to by the data paramter. Note that readf should block
 *              until the number of bytes specified in the len parameter are
 *              read.
 * @param writef should read chars from the location pointed to by the data
 *              paramter send them to the bus which the wifi device is connected
 *              to. Note that writef should block until the number of bytes
 *              specified in the len parameter are written.
 * @param ctx is an opaque context pointer that will be passed back to readf
 *              and writef.
 * 
 * @return
 *   - 0 on success
 *   - WLP_ERR_PARAM if invalid baudrate or rtscts parameters.
 *
 */
int wlp_set_baudrate(uint32_t baudrate, uint8_t rtscts);
    

/**
 * Reset the device. wlp_init() must be called after wlp_reset() to complete
 * the reset. This function is typically used to switch betweeen AP and STA
 * modes after wlp_set_mode() has been used.
 *
 * NOTE: In this release the baudrate must be 57600 before this function is
 * called. See wlp_set_baudrate() for details on how to change the baudrate.
 *
 */
int wlp_reset(void);
    

/**
 * \brief Get the firmware version.
 *
 * @param code will contain the fw version code upon successful return. The
 *              version can be compared to the driver version (WLP_VERSION_CODE)
 *              using the WLP_VERSION(), WLP_MAJOR_VERSION(),
 *              WLP_MINOR_VERSION() and the WLP_PATCH_VERSION() macros.
 *
 * @param build will hold the build information upon successful return. The
 *              build information is a string of information for the particular
 *              fw build used on the device. No more then \param len characters,
 *              including null termination, will be copied to the string pointed
 *              to by build.
 *
 * @param len   is the maximum amount of chars, including null termination,
 *              that will be copied to \param build.
 *
 * @return
 *   - 0 on success
 *   - WLP_FAILURE if any failure occurs
 *
 *
 */
int wlp_get_fw_version(uint32_t *code, char *build, int len);


/*! @} */


/*! \defgroup wlp_link Wi-Fi management functions
 *
 * These functions manage the Wi-Fi connection such as encryption
 * keys, MAC addresses, and connecting to access points.
 *
 * @{
 *
 */ 

/**
 * \brief Set a wifi link status callback function.
 *
 * Register a callback that will be invoked when the wifi connection status
 * changes.
 *
 * Every time wlp_poll() is invoked, the callback condition will be evaluated
 * and any registered callback will be invoked from same context as the call to
 * wlp_poll().
 *
 * Note that the link callback will be invoked when the wifi device is
 * connected to the access point, an IP address does not have to be
 * configured. See wlp_set_ipaddr_cb() for information on how to get notified 
 * when an IP address is configured.
 *
 * Note that the connection status will also be indicated by any wlp_api that
 * performs socket communication (e.g. wlp_send() and wlp_recv()) through
 * their return values, event though wlp_set_link_cb() is not called.
 *
 * In access point mode, the link callback will be invoked when the first
 * client is connected to the access point. See wlp_set_mode() for more
 * information on how to set access point mode.
 *
 * @param link_cb will be invoked upon a link status change. The callback
 *              parameter \a link will be set to 1 to indicate that the link
 *              has been established and 0 to indicate that the link has been
 *              shut down.
 * @param ctx is an opaque context pointer that will be passed back to link_cb.
 *
 */
void wlp_set_link_cb(void (*link_cb)(void *ctx, int link), void *ctx);


/**
 * \brief Connect the wifi link.
 *
 * Enable the wifi link. Upon successful return, the wifi device will try
 * to establish and maintain a connection to the network specified by the
 * parameters. When the link status (connected, disconnected) changes, the
 * caller will be notified of the new status through the callback registered
 * in wlp_set_link_cb(). Information about the current network can be obtained
 * through wlp_get_network().
 *
 * Note again that successful return means that the wifi device will now
 * _initate_ a link using the desired parameters, not that the link
 * is already established.
 *
 * If wlp_linkup() is called when already connected to an access point, the
 * wifi device will try to switch to the network configuration according to
 * the provided parameters.
 *
 * wlp_linkup() will only manage the link layer, not the network or socket
 * layer. See wlp_set_dhcp() and wlp_set_ipaddr() for information about
 * configuring the network layer and wlp_socket() for information about the
 * socket layer.
 *
 * In access point mode, this function will start an access point with the
 * given ssid and key parameters. See wlp_set_mode() for more information on
 * how to set access point mode.
 *
 * NOTE: encryption in access point mode is not supported in this release.
 *
 * @param ssid is the name of the access point to connect to or the name
 *              of the access point to start (in access point mode).
 * @param key is the encryption key to use. If this parameter is NULL,
 *              no enryption will be used. Should be provided as an ascii
 *              string. In case of a WPA key (parameter wep set to 0), the WPA
 *              passphrase key should be provided, e.g. "mysecretkey". In case
 *              of a WEP key (parameter wep set to 1), the key should be
 *              provided as a string containing the hexadecimal chars
 *              (10 or 26 chars, depending on whether a 40-bit or 104-bit key
 *              is used), e.g. "a1b2c3d4e5".
 * @param wep should be set to 1 if WEP encryption should be used (default is
 *              wpa).
 *
 * @return
 *   - 0 on success
 *   - WLP_ERR_PARAM if invalid parameters, e.g. invalid key length.
 *   - WLP_ERR_FAILURE on any other error.
 *
 */
int wlp_linkup(struct wl_ssid_t *ssid, const char *key, int wep);


/**
 * \brief Disconnect the wifi link.
 * 
 * Disable the wifi link. The wifi device will no longer try to maintain the
 * network connection. Any link callback that was registered through
 * wlp_set_link_cb() will be invoked when disconnection is completed,
 * if currently connected to an access point.
 *
 * Note again that successful return means that the wifi device will now
 * _initate_ a link disconnection, not that the link is already disconnected.
 *
 * If wlp_linkup() is called when already disconnected, any ongoing connection
 * attempts will be stopped.
 *
 * wlp_linkdown() will only manage the link layer, not the network or socket
 * layer. See wlp_set_dhcp() and wlp_set_ipaddr() for information about
 * configuring the network layer and wlp_socket() for information about the
 * socket layer.
 *
 * In access point mode, this function will stop the access point and it will
 * no longer transmit beacons. All clients will be disconnected. See
 * wlp_set_mode() for more information on how to set access point mode.
 *
 * NOTE: wlp_linkdown() is not supported in this release. To stop the access
 *       point, wlp_reset() can be used as a workaround.
 *
 * @return
 *   - 0 on success
 *
 */
int wlp_linkdown(void);


/**
 * \brief Get the interface MAC address.
 *
 * Return the 802.3 MAC address of the network interface.
 * 
 * @param hwaddr will hold the wifi device MAC address upon successful return.
 * @return
 *   - 0 on succes
 *
 */
int wlp_get_hwaddr(struct wl_mac_addr_t *hwaddr);


/**
 * \brief Get the 802.11 network.
 * 
 * Get the description of the currently associated network, if any.
 *
 * In access point mode, this function will return the description of the
 * device when the link is up, e.g. first client connected. See wlp_set_mode()
 * for more information on how to set access point mode.
 *
 * @param net will hold the network description upon successful return.
 * @return
 *   - 0 on success
 *   - WLP_FAILURE on failure.
 *
 */
int wlp_get_network(struct wlp_network_t *net);

/*! @} */



/*! \defgroup wlp_network Network address functions
 *
 * These functions manage the network address configuration.
 *
 * @{
 *
 */ 

/**
 * \brief Register a callback that will be invoked when an IP address is set.
 *
 * The provided callback will be invoked when an IP address is set or removed.
 * Every time wlp_poll() is invoked, the callback condition will be evaluated
 * and any registered callback will be invoked from same context as the call to
 * wlp_poll().
 *
 * If a static IP address is configured (through wlp_set_ipaddr()), this
 * function will be called at the same time as the link callback registered in
 * wlp_set_link_cb().
 *
 * If a dynamic IP address is configured (through wlp_set_dhcp()) this function
 * will be called when the link is up and an address has been successfully
 * obatined through DHCP. It will also be called when the wifi link goes down
 * (since the IP address will be removed at that time) or if the IP
 * address is relesed through a call to set_dhcp() with argument 0.
 *
 * Note that the connection status will also be indicated by any wlp_api that
 * performs socket communication (e.g. wlp_send() and wlp_recv()) through
 * their return values, even though wlp_set_ipaddr_cb() has not been called.
 *
 * @param addr_cb is called when the IP address is set or removed. If the
 *              address is set, the current address will be provided in the addr
 *              parameter. If the address is removed, the addr parameter will
 *              be NULL.
 * @param ctx is an opaque context parameter that will be passed back to the
 *              addr_cb.
 *
 */
void wlp_set_ipaddr_cb(void (*addr_cb)(void *ctx, const struct ip_addr *addr),
                       void *ctx);


/**
 * \brief Set a static IP address
 *
 * Disable DHCP and configure the device static IP address settings.
 * wlp_set_ipaddr() can be invoked either before or after the wifi link is up.
 *
 * If a static IP address is configured using this function, any callback
 * registered with wlp_set_ipaddr_cb() will be invoked as soon as the
 * wifi link status changes (see wlp_linkup()).
 *
 * @param ip is the IP address to set, e.g. 192.168.1.100. Not used if NULL.
 * @param mask is the network mask, e.g. 255.255.255.0. Not used if NULL.
 * @param gw is the gateway to use, e.g. 192.168.1.1. Not used if NULL.
 * @param dns is the dns server to use, e.g. 192.168.1.1. Not used if NULL.
 *
 * @return
 *   - 0 on success.
 *
 */
int wlp_set_ipaddr(const struct ip_addr *ip, const struct ip_addr *netmask,
                   const struct ip_addr *gw, const struct ip_addr *dns);


/**
 * \brief Get the current IP address parameters
 *
 * The currently assigned IP address setttings for the wifi device will be
 * fetched. If DHCP is enabled (through wlp_set_dhcp()) and the IP configuration
 * has not yet been assigned, all IP address values will be IP_ADDR_ANY.
 * If DHCP is disabled the statically assigned IP addresses will be fetched.
 *
 * @param ip will hold the current IP address upon successful return. Not used
 *              if NULL.
 * @param netmask will hold the current netmask upon successful return. Not used
 *              if NULL.
 * @param gw will hold the current default gateway address upon successful
 *              return. Not used if NULL.
 * @param dns will hold the current dns server address upon successful return.
 *              return. Not used if NULL.
 *
 * @return
 *   - 0 on success
 *   - WLP_ERR_CONN if not connected (IP address configuration will still
 *              be filled in into the provided parameters).
 *
 */
int wlp_get_ipaddr(struct ip_addr *ip, struct ip_addr *netmask,
                   struct ip_addr *gw, struct ip_addr *dns);


/**
 * \brief Request or release IP address through DHCP.
 *
 * Enable or disable DHCP for IP address configuration. If this function
 * is called when the wifi link is up, an IP address will be requested or
 * released upon successful return, depending on the value of the enable
 * parameter. See wlp_set_ipaddr_cb() on how to get notified when the IP address
 * configuration is set or removed.
 *
 * wlp_set_dhcp() can be invoked either before or after the wifi link is up.
 * If this function is called when the wifi link is down, any DHCP requests
 * will be performed when the wifi link comes up.
 *
 * If DHCP is enabled through this function, any configuration set through
 * wl_set_ipaddr() will not be used.
 * 
 * Note that the current IP address settings can be obtained through
 * wl_get_ipaddr() at any time.
 *
 * @param enable determines if DHCP should be enabled or disabled.
 *
 * @return 0 on success.
 *
 */
int wlp_set_dhcp(int enable);


/**
 * \brief Get the current DHCP configuration
 *
 * Get information on whether DHCP is enabled or not. Note that wlp_get_dhcp()
 * will not indicate whether an actual IP address has been obtained through DHCP
 * or not. See wlp_set_ipaddr_cb() for information about IP address
 * notifications.
 *
 * @param enabled will hold the DHCP configuration upon successful return. Not
 *              used if NULL.
 * @return
 *   - 0 on success
 *   - WLP_ERR_CONN if not connected (DHCP configuration will still be
 *              filled in into the provided parameter).
 *
 */
int wlp_get_dhcp(int *enabled);


/**
 * \brief Start or stop the device DHCP server.
 *
 * Enable or disable DHCP server for IP address configuration.
 *
 * wlp_set_dhcpd() can be invoked either before or after the wifi link is up.
 * If this function is called when the wifi link is down, any DHCP services
 * will be performed when the wifi link comes up.
 *
 * @param enable determines if the DHCP server should be enabled or disabled.
 *
 * @return
 *   - 0 on success.
 *
 */
int wlp_set_dhcpd(int enable);

    
/*! @} */



/*! \defgroup wlp_socket Network socket functions
 *
 * These functions manage the network sockets.
 *
 * @{
 *
 */ 

/**
 * \brief Creates an endpoint for communication and returns a socket id.
 *
 * To create a TCP socket, set the type to WLP_SOCK_STREAM.
 * To create a UDP socket, set the type to WLP_SOCK_DGRAM.
 * To create a RAW socket, set the type to WLP_SOCK_RAW.
 *
 * Sockets of type WLP_SOCK_STREAM must be in a connected state
 * before any data may be sent or received on it. A connection to
 * another socket is created with wlp_connect(). Once connected, data
 * may be transferred using wlp_recv() and wlp_send() calls. When a
 * session has been completed, wlp_close() can be called. Sockets of
 * type WLP_SOCK_STREAM always uses the TCP protocol. WLP_SOCK_STREAM type
 * sockets does not preserve message boundaries, i.e sequential calls to
 * wlp_send() might result in a single, aggregated, IP packet being transferred
 * on the wireless interface. It is also possible that a single call to
 * wlp_send() results in multiple IP packets being transferred on the wireless
 * interface.
 *
 * WLP_SOCK_DGRAM and WLP_SOCK_RAW sockets are connectionless and
 * allows the sending of datagrams to correspondents named in
 * wlp_sendto(). Further, these sockets does preserve message
 * boundaries, i.e sequential calls to wlp_send() or wlp_sendto()
 * might will result in multiple IP packets being transferred on the
 * wireless interface. See wlp_send() and wlp_sendto() for more
 * information.
 *
 * The protocol parameter specifies a particular protocol to be used
 * with the socket. For the WLP_SOCK_STREAM and WLP_SOCK_DGRAM types,
 * only a single protocol exists to support a particular socket type,
 * in which case the protocol parameter is ignored. However, for
 * WLP_SOCK_RAW many protocols exists, in which case a particular protocol
 * must be specified in this manner.
 *
 * The default Wi-Fi device configuration allows for up to eight WLP_SOCK_STREAM
 * sockets (four in listening mode and four in connection mode), four
 * WLP_SOCK_DGRAM sockets, and four WLP_SOCK_RAW sockets.
 *
 * Note that is is only possible to create a single WLP_SOCK_RAW socket for
 * each protocol type.
 *
 * @param type is the socket type; WLP_SOCK_STREAM, WLP_SOCK_DGRAM or
 *              WLP_SOCK_RAW.
 * @param proto specifies a particular protocol to be used with the
 *              socket, ignored if type is WLP_SOCK_STREAM or WLP_SOCK_DGRAM.
 *
 * @return
 *   - The id of the created socket on success (> 0)
 *   - WLP_ERR_MEM if out of memory or resources (on host or wifi device)
 *   - WLP_ERR_PARAM if invalid parameters
 *   - WLP_ERR_INUSE if trying to multiple sockets with the same protocol,
 *                only for WLP_SOCK_RAW.
 *
 */
int wlp_socket(int type, int proto);


/**
 * \brief Register a connection status callback
 *
 * wlp_set_conn_cb() will register a function that will be invoked when the
 * socket connection status for the specified socket changes.
 *
 * The connection status will change when wlp_connect() is finished processing
 * or when a the remote end aborts the connection. See wlp_connect() for
 * details.
 *
 * Every time wlp_poll() is invoked, the conn_cb callback conditions
 * will be evaluated and any registered callback will be invoked from same
 * context as the call to wlp_poll().
 *
 * A registered callback can be removed by calling wlp_set_conn_cb() with
 * NULL parameters.
 *
 * @oaram sockid is the socket for which the conn_cb should be bound to.
 * @param conn_cb is a callback function that will be invoked when the
 *              connection is established or lost. The socket id and the
 *              connection state will be passed as arguments to the callback.
 *              Not used if NULL.
 * @param ctx is an opaque context parameter that will be passed back to the
 *              conn_cb.
 *
 * @return
 *   - 0 on success
 *   - WLP_ERR_PARAM if invalid parameters
 *
 */
int wlp_set_conn_cb(int sockid,
                    void (*conn_cb)(void *ctx, int sockid, int connected),
                    void *ctx);

/**
 * \brief Register a data pending callback
 *
 * Any registerd recv_cb() will be invoked when there is pending data to read
 * from the specified socket. See wlp_recv() for more information.
 *
 * Every time wlp_poll() is invoked, the recv_cb callback conditions
 * will be evaluated and any registered callback will be invoked from same
 * context as the call to wlp_poll().
 *
 * @oaram sockid is the socket for which the recv_cb should be bound to.
 * @param recv_cb is a callback function that will be invoked when there is
 *              pending data to read from the socket. The number of bytes
 *              available for reading will be passed in the len parameter.
 *              If 0, then the connection was closed by the remote end.
 *              Not used if NULL.
 * @param ctx is an opaque context parameter that will be passed back to the
 *              recv_cb.
 *
 * @return
 *   - 0 on success
 *   - WLP_ERR_PARAM if invalid parameters
 *
 */
int wlp_set_recv_cb(int sockid,
                    void (*recv_cb)(void *ctx, int sockid, int len),
                    void *ctx);


/**
 * \brief Assign an address for a socket
 *
 * When a socket is created with wlp_socket(), it has no address assigned to it.
 * wlp_bind() assigns the address specified by addr and port to the socket
 * referred to by the sockid parameter.
 *
 * A socket must be disconnected in order to use wlp_bind() on the socket.
 *
 * For sockets of type WLP_SOCK_STREAM, the address and port set through
 * wlp_bind() will be used in wlp_listen(). Also, the source address of any
 * transmitted data will be set to the address configured through wlp_bind().
 * If wlp_bind() is not called, any transmitted data will have a random source
 * port. Incoming connections for the socket will only be allowed on the port
 * which the socket is bound to.
 *
 * For sockets of type WLP_SOCK_DGRAM, the source address of any
 * transmitted data will be set to the address configured through wlp_bind().
 * If wlp_bind() is not called, any transmitted data will have a random source
 * port. Further, incoming data will only be forwarded to this socket if the
 * destination port of the data matches the port the socket is bound to. Note
 * that if the socket was connected through wlp_connect(), incoming data will
 * be forwarded to the socket based on the source address and port of the data.
 * See wlp_connect() for more information.
 *
 * For sockets of type WLP_SOCK_RAW, the the source address of any
 * transmitted data will be set to the address configured through wlp_bind().
 * The bound address will not affect which incoming data that is forwarded to
 * the socket, that is only depending on the protocol used.
 *
 * It is allowed to invoke this function multiple times on the same socket to
 * bind the socket to a new address.
 *
 * @param sockid is the socket to bind
 * @param addr must be IP_ADDR_ANY.
 * @param port is the port to bind to.
 * @return
 *   - 0 on success
 *   - WLP_ERR_PARAM if invalid parameters, e.g sockid not found.
 *   - WLP_ERR_STATE if socket is in invalid state.
 *   - WLP_ERR_NOTSUPP if the operation is not supported on the socket.
 *   - WLP_ERR_INUSE if the port is already bound by another socket.
 */
int wlp_bind(int sockid, const struct ip_addr *addr, uint16_t port);


/**
 * \brief Configure the socket to listen for incoming connections
 *
 * wlp_listen() marks the socket referred to by sockid as a passive socket,
 * that is, as a socket that will be used to accept incoming connection requests
 * wlp_accept(). Only sockets of type WLP_SOCK_STREAM may be put in listening
 * mode.
 *
 * Upon successful return the socket will listen on the port configured in
 * wlp_bind() and the listen_cb will be invoked upon any incoming connection.
 * Every time wlp_poll() is invoked, the callback condition will be evaluated
 * and any registered listen callback will be invoked from same context as the
 * call to wlp_poll().
 *
 * Note that any incoming connections will be silently rejected before the
 * listen_cb is invoked in case the wifi device is low on resources (e.g.
 * the maximum number of connected WLP_SOCK_STREAM sockets are already in use).
 *
 * @param sockid is a socket id that refers to a socket of type WLP_SOCK_STREAM.
 * @param backlog defines the maximum length to which the queue of pending
 *              connections for the socket may grow. If a connection request
 *              arrives when the queue is full, the client may receive an
 *              connection refused indication.
 * @param listen_cb will be invoked when there's a pending connection to
 *              accept. The id of the listening socket will be passed to the
 *              listen_cb. See wlp_accept() for more information.
 * @param ctx is an opaque context pointer that will be passed back to the
 *              listen_cb.
 *
 * @return
 *   - 0 on success
 *   - WLP_ERR_PARAM if invalid parameters, e.g sockid not found.
 *   - WLP_ERR_STATE if the socket is in invalid state, e.g. not bound.
 *   - WLP_ERR_NOTSUPP if the operation is not supported on the socket
 *   - WLP_ERR_MEM if out of memory or resources on the wifi device
 *
 */
int wlp_listen(int sockid, int backlog,
               void (*listen_cb)(void *ctx, int sockid),
               void *ctx);

/**
 * \brief Initiate a connection on a socket
 *
 * If the socket is of type WLP_SOCK_DGRAM or WLP_SOCK_RAW then IP and port is
 * the address to which datagrams are sent to by default, and the only address
 * from which datagrams are received.
 *
 * If the socket is of type WLP_SOCK_STREAM, this call attempts
 * to make a connection to the socket with the given address. Any conn_cb
 * function registered through wlp_set_conn_cb() will be invoked when the
 * attempt is complete, indicating wheter the connection is established or not
 * through the connected paramater. If the connected parameter is 0 when the
 * conn_cb callback is invoked, the the wifi device will not retry the
 * connection and the socket is considered disconnected again. If the connected
 * parameter is 1, then the socket is successfully connected and data transfer
 * can start by e.g. using the wlp_send() and wlp_recv() functions.
 *
 * For sockets of type WLP_SOCK_DGRAM and WLP_SOCK_RAW, there is no actual
 * concept of a "connected" and "disconnected" sockets since those sockets are
 * connectionless. However, it is still possible to call wlp_connect() for
 * these sockets, indicating that all data sent through wlp_send() will be
 * sent to the address specified in wlp_connect(). Any conn_cb registered
 * through wlp_set_conn_cb() will be invoked upon a successful return from
 * wlp_connect(). Note that a connected socket will only accept incoming data
 * from which has a source address and port which matches the parameters passed
 * to wlp_connect(). See wlp_bind() for more information regarding which packets
 * that gets formwarded to the socket.
 *
 * For sockets of type WLP_SOCK_RAW, the port parameter will be ignored since
 * it is not included in the IP header.
 *
 * On a connected socket any registerd recv_cb() will be invoked when there is
 * pending data to read from the socket. See wlp_recv() and wlp_set_recv_cb()
 * for more information.
 *
 * Note that the socket connection status will also be indicated by
 * any wlp_api calls that performs socket communication
 * (e.g. wlp_send() and wlp_recv()) through their return values, even
 * though any conn_cb was not registered through wlp_set_conn_cb().
 *
 * wlp_connect() can be called multiple times for a single socket but the
 * socket must be disconnected prior to each call.
 *
 * @param sockid is the socket id to connect
 * @param ip is the IP address of the remote end
 * @param port is the port number of the remote end, ignored for sockets of
 *         type WLP_SOCK_RAW.
 *
 * @return
 *   - 0 on success
 *   - WLP_ERR_PARAM if invalid parameters, e.g sockid not found.
 *   - WLP_ERR_CONN if no network address is set or the wifi link is down.
 *   - WLP_ERR_STATE if the socket is in invalid state, e.g. not
 *              disconnected.
 *   - WLP_ERR_NOTSUPP if the operation is not supported on the socket
 *   - WLP_ERR_MEM if out of memory or resources on the wifi device
 *
 */
int wlp_connect(int sockid, const struct ip_addr *ip, uint16_t port);

/**
 * \brief Accept a connection on a socket.
 *
 * wlp_accept() will accept any pending connection on a listening socket.
 * A new socket will be created and returned as a result of this call. The
 * original listening socket is unaffected by this call.
 *
 * This function should typically be called when the callback registered in
 * wlp_listen() for the specific socket is invoked. The listening socket id
 * (as passed to wlp_listen()) should be passed to wlp_accept().
 *
 * However it is also valid to wlp_accept() even through a listen_cb is not
 * invoked for the socket (or not registerd at all). Therefore, it is also
 * possible to accept a pending connection from a specific listening socket in
 * a polling fashion.
 *
 * Upon returning from wlp_accept() the new socket is connected and ready to
 * send and receive data using e.g wlp_send() and wlp_recv(). If the
 * connection is lost, the any conn_cb registered through wlp_set_conn_cb()
 * will be invoked. Any recv_cb registered through wlp_set_recv_cb() will be
 * invoked when there is pending data to read from the socket. See wlp_recv()
 * for more information.
 *
 * @param sockid is the listening to socket to accept the new connection from.
 * @return
 *   - Returns the new socket for the connection (> 0) on success
 *   - WLP_ERR_PARAM if invalid parameters, e.g sockid not found.
 *   - WLP_ERR_STATE if the socket is in invalid state, e.g. not
 *              accepting.
 *   - WLP_ERR_NOTSUPP if the operation is not supported on the socket
 *
 */
int wlp_accept(int sockid);


/**
 * \brief Close a socket.
 *
 * wlp_close() will close and remove a socket that was created with
 * wlp_socket(). All memory used by the socket will be free'd.
 *
 * A connected socket will be cleanly disconnected from the remote end.
 *
 * A socket can be closed form any state. Any connection callbacks registed for
 * the socket (e.g. from wlp_connect(), wlp_accept() or wlp_listen()) will not
 * be invoked when the socket is closed.
 * 
 * @param sockid of the socket to close.
 * @return
 *   - 0 on success
 *
 */
int wlp_close(int sockid);


/**
 * \brief Read data from a socket.
 * 
 * Read pending data from a socket. This function will attempt to
 * read up to len bytes from the given socket.
 *
 * If no data is pending on the socket, 0 will be returned.
 *
 * Sockets of type WLP_SOCK_STREAM will not preserve message boundaries in
 * general. If the socket has less than len bytes available for reading, the
 * available bytes will be read and the function will return. The actual number
 * of bytes read will be returned.
 *
 * Sockets of type WLP_SOCK_DGRAM and WLP_SOCK_RAW will preserve
 * message boundaries by requiring that the len parameter is large enough
 * to hold the complete message. Otherwise WLP_ERR_SIZE will be returned and
 * no data will be received. In this case, the caller must invoke wlp_recv()
 * again with a larger buffer that can hold the complete message.
 *
 * For sockets of type WLP_SOCK_RAW, the IP header will be included in the
 * received message.
 *
 * See wlp_connect() for more information about message boundaries for the
 * differenct socket types.
 *
 * This function should typically be called from the recv_cb registered through
 * wlp_set_recv_cb(). However it is also valid to wlp_recv() even through a
 * recv_cb is not invoked for the socket (or not registerd at all). Therefore,
 * it is possible to recieve data from a specific socket in a polling fashion.
 *
 * Note that the error code WLP_ERR_CONN will be returned if the socket was
 * closed from the remote end. Note that any pending, buffered, data will be
 * returned before WLP_ERR_CONN is returned. In case the socket connection was
 * closed by the remote end, any registered recv_cb (see wlp_set_recv_cb())
 * will also be called with len set to 0.
 *
 * The error code WLP_ERR_ABORT will be returned if the socket connection
 * was terminated in any other way, e.g. link goes down. In this case any
 * registered conn_cb (see wlp_connect() or wlp_accept()) will also be called
 * with the connected parameter set to 0. Any non-read data that is buffered
 * by the wifi device will be lost.
 * 
 * @param sockid is the socket to read
 * @param buf should point to a buffer which will hold the read data.
 * @param len is the number of bytes to read.
 * @return
 *   - The number of bytes read on success
 *   - WLP_ERR_PARAM if invalid parameters, e.g sockid not found
 *   - WLP_ERR_MEM if out of memory or resources on the wifi device
 *   - WLP_ERR_CONN if the socket was disconnected by the remote end and all
 *              buffered data has been read or if the socket has not been
 *              connected.
 *   - WLP_ERR_ABORT if the socket connection was lost.
 *   - WLP_ERR_STATE if the socket is in an invalid state (e.g. listening).
 *   - WLP_ERR_SIZE if the complete message does not fit into the user
 *              supplied buffer. Only for sockets of type WLP_SOCK_DGRAM or
 *              WLP_SOCK_RAW.
 *   - WLP_ERR_PROTO if unrecoveraly protocol error. If this happens, there
 *              is probably a problem in the serial commuincation between the
 *              host and the wifi device.
 *
 */
int wlp_recv(int sockid, char *buf, int16_t len);


/**
 * \brief Send data to a socket.
 *
 * Send data to a connected socket.
 *
 * Sockets of type WLP_SOCK_STREAM will not preserve message boundaries in
 * general.  This function will attempt to send up to len bytes of data to the
 * given socket. If the socket has room for less than
 * len bytes in its input buffer, the maximum allowed bytes will be written
 * and then this function will return. The actual number of bytes written will
 * be returned.
 *
 * Sockets of type WLP_SOCK_DGRAM and WLP_SOCK_RAW will preserve
 * message boundaries by ensuring that the requested len is also the length of
 * the single IP packet transmitted over the wireless link (possibly fragmented)
 * If the call to wlp_send() returns WLP_ERR_SIZE, the provide buffer is too
 * large to be transferred as a single message. No data is transferred in this
 * case.
 *
 * Note that for sockets of type WLP_SOCK_RAW, you can not modify the IP
 * headers (this is inconsistent with the data read through wlp_recv() where
 * you actually get the IP headers), you can only specify the IP payload here.
 *
 * See wlp_connect() for more information about message boundaries for the
 * differenct socket types.
 *
 * It is valid to call wlp_send() multple times event though all requested data
 * was not transmitted (according to the return value) to send a large chunk
 * of data. Therefore, it is possible to send data to specific socket in a
 * blocking fashion.
 *
 * Note that the error code WLP_ERR_CONN will be returned if the socket was
 * closed by the remote end. In case the socket connection was closed
 * by the remote end, any registered recv_cb (see wlp_set_recv_cb())
 * will also be called with len set to 0.
 *
 * The error code WLP_ERR_ABORT will be returned if the socket connection
 * was terminated in any other way, e.g. link goes down. In this case the
 * conn_cb will also be called with the connected parameter set to 0.
 *
 * @param sockid is the socket to write
 * @param buf should pointer to a buffer which will hold the data to be written.
 * @param len is the number of bytes to write.
 * @return
 *   - The number of bytes actually written on success
 *   - WLP_ERR_PARAM if invalid parameters, e.g sockid not found
 *   - WLP_ERR_MEM if out of memory or resources on the wifi device
 *   - WLP_ERR_CONN if the socket was disconnected by the remote end (only
 *              for sockets of type WLP_SOCK_STREAM) or if the socket has not
 *              been connected.
 *   - WLP_ERR_ABORT if the socket connection was lost (only for sockets
 *              of type WLP_SOCK_STREAM).
 *   - WLP_ERR_STATE if the socket is in an invalid state (e.g. listening).
 *   - WLP_ERR_SIZE if the message is too long to pass atomically through
 *              the underlying protocol (IP). Only for sockets of type
 *              WLP_SOCK_DGRAM or WLP_SOCK_RAW.
 *   - WLP_ERR_PROTO if unrecoveraly protocol error. If this happens, there
 *              is probably a problem in the serial commuincation between the
 *              host and the wifi device.
 *   - WLP_FAILURE on any other error
 *
 */
int wlp_send(int sockid, const char *buf, int16_t len);


/**
 * \brief Send data to a specified address from a socket
 *
 * wlp_sendto() will send data to a specified address. This is generally used
 * for connectionless sockets, e.g. of type WLP_SOCK_DGRAM or WLP_SOCK_RAW.
 * For WLP_SOCK_TYPE_STREAM sockets, the IP and port parameters will be ignored
 * and the data will be sent to any remote endpoint established through
 * wlp_connect().
 *
 * For sockets of type WLP_SOCK_RAW, the port parameter will be ignored since
 * it is not included in the IP header.
 *
 * Except for the extra parameters, this function will behave exactly the same
 * as wlp_send(). See wlp_send() for more details.
 *
 * @param sockid is the socket to write
 * @param buf should pointer to a buffer which will hold the data to be written.
 * @param len is the number of bytes to write.
 * @param ip is the IP address to send to
 * @param port is the port to send to, ignored for sockets of type WLP_SOCK_RAW.
 * @return
 *   - The number of bytes actually written on success
 *   - WLP_ERR_PARAM if invalid parameters, e.g sockid not found
 *   - WLP_ERR_MEM if out of memory or resources on the wifi device
 *   - WLP_ERR_CONN if the socket was disconnected by the remote end (only
 *              for sockets of type WLP_SOCK_STREAM) or if the socket has not
 *              been connected.
 *   - WLP_ERR_ABORT if the socket connection was lost (only for sockets
 *              of type WLP_SOCK_STREAM).
 *   - WLP_ERR_STATE if the socket is in an invalid state (e.g. listening).
 *   - WLP_ERR_PROTO if unrecoveraly protocol error. If this happens, there
 *              is probably a problem in the serial commuincation between the
 *              host and the wifi device.
 *   - WLP_ERR_SIZE if the message is too long to pass atomically through
 *              the underlying protocol (IP). Only for sockets of type
 *              WLP_SOCK_DGRAM and WLP_SOCK_RAW.
 *   - WLP_FAILURE on any other error
 *
 */
int wlp_sendto(int sockid, const char *buf, int16_t len,
               const struct ip_addr *ip, uint16_t port);


/**
 * \brief Get name of connected peer socket.
 *
 * wlp_get_peeraddr() returns the address of the peer connected to the socket.
 * Only supported on sockets of type WLP_SOCK_STREAM.
 *
 * @param sockid is the socket for which the peer should be obtained.
 * @param peer will hold the peer IP address upon successful return.
 * @return
 *   - 0 on success
 *   - WLP_ERR_PARAM if invalid parameters, e.g sockid not found
 *   - WLP_ERR_CONN if the socket was disconnected by the remote end or if
 *              the socket has not been connected.
 *   - WLP_ERR_ABORT if the socket connection was lost (only for sockets
 *              of type WLP_SOCK_STREAM).
 *   - WLP_ERR_STATE if the socket is in an invalid state (e.g. listening).
 *   - WLP_ERR_NOTSUPP if the operation is not supported on the socket
 *
 */
int wlp_get_peeraddr(int sockid, struct ip_addr *peer);

/**
 * \brief Get network address given a dns name
 * 
 * If name is an IPv4 address, no lookup is performed and wlp_get_hostbyname()
 * copies host into the parameter provided to the lookup_cb.
 *
 * Every time wlp_poll() is invoked, the callback condition will be evaluated
 * and any registered callback will be invoked from same context as the call to
 * wlp_poll().
 *
 * @param host is the dns name to lookup
 * @param lookup_cb will be invoked when lookup is complete or considered
 *              failed. On success, the IP parameter will be set to the IP
 *              address that was looked up. On failure, IP will be NULL.
 * @param ctx is an opaque context parameter that will be passed back to the
 *        lookup_cb.
 *
 * @return
 *   - 0 on success
 *   - WLP_ERR_PARAM if invalid parameters
 *   - WLP_ERR_MEM if out of resources on device
 *   - WLP_ERR_BUSY if a previous call to wlp_get_hostbyname() is not
 *              yet completed.
 */
int wlp_get_hostbyname(const char *host,
                       void (*lookup_cb)(void *ctx, const struct ip_addr *ip),
                       void *ctx);

/**
 * \brief Event progress function.
 *
 * This function must be called in stand-alone environments to
 * ensure forward progress of events. The call can be made periodically from
 * the main application loop.
 *
 * Every time wlp_poll() is invoked, the callback conditions for all callbacks
 * registered through wlp_api will be evaluated and any registered callback
 * will be invoked from same context as the call to wlp_poll().
 *
 * If no callbacks are registered, wlp_poll() does not have to be invoked.
 *
 * In systems using an OS, a task could be scheduled to run this task
 * periodicallly.
 *
 * Note that this function should not be invoked from interrupt context.
 *
 *
 */
void wlp_poll(void);

/*! @} */



/*! \defgroup wlp_ap Soft access point functions
 *
 * These functions manage the soft access point mode.
 *
 * @{
 *
 */ 


/**
 * \brief Set the Wi-Fi mode to AP or STA.
 *
 * wlp_set_mode() will configure the Wi-Fi device to operate either as a
 * station or as an access point. The current mode can be obtained through
 * wlp_get_mode(). Note that the mode configured through wlp_set_mode() only
 * takes effect after wlp_reset() has been called. The configured mode persists
 * after power-cycling the device.
 *
 * @param mode is the mode to configure. This must be either WL_MODE_STA or
 *              WL_MODE_AP.
 *
 * @return
 *   - 0 on success
 *   
 */
int wlp_set_mode(uint8_t mode);



/**
 * \brief Check if the device is currently in AP or STA mode.
 *
 * wlp_get_mode() will be return the current operation mode of the Wi-Fi device.
 * Since the operation mode is persistent between power-cycles, it is generally
 * a good idea to call this function at startup to ensure the desired operation
 * mode.
 *
 * @param mode will hold the current mode upon successful return. This will be
 *              either WL_MODE_STA or WL_MODE_AP.
 *
 * @return
 *   - 0 on success
 *   
 */
int wlp_get_mode(uint8_t *mode);



/**
 * \brief Set the channel. Only supported when mode is WLP_MODE_AP.
 *
 * This function is only supported when wlp_get_mode() returns WLP_MODE_AP.
 * wlp_set_channel() must be called before wlp_linkup() to have effect.
 *
 * @return
 *   - 0 on success
 *   - WLP_ERR_PARAM if invalid channel
 *   - WLP_ERR_STATE the operation can not be carried out (e.g. ap already
 *              started).
 *   - WLP_ERR_NOTSUPP if operation is not supported
 */
int wlp_set_channel(uint8_t channel);


/*! @} */


#endif /* WLP_API_H */
