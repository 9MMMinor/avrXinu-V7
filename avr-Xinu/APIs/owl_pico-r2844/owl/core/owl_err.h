#ifndef OWL_ERR_H
#define OWL_ERR_H

#define OWL_FAILURE   -1  /* general failure */
#define OWL_ERR_PARAM -2  /* parameter error */
#define OWL_ERR_MEM   -3  /* out of memory */
#define OWL_ERR_CONN  -4  /* no connection */
#define OWL_ERR_STATE -5  /* invalid state */
#define OWL_ERR_PARSE -6  /* input parsing error */
#define OWL_ERR_PATH  -7  /* path not found */
#define OWL_ERR_TYPE  -8  /* incopatible type */
#define OWL_ERR_PERM  -9  /* no permission */
#define OWL_ERR_NOTSUPP -10 /* operation not supported */
#define OWL_ERR_RANGE -11 /* out of range */
#define OWL_ERR_SIZE  -12 /* bad size */
#define OWL_ERR_AGAIN -13 /* retry later */
#define OWL_ERR_BUSY  -14 /* busy */
#define OWL_ERR_DISABLED -15 /* function is disabled */
#define OWL_ERR_PROTO -16 /* protocol error */
#define OWL_ERR_TIMEOUT -17 /* timeout */
#define OWL_ERR_IGNORED -18 /* ignored, no effect */
#define OWL_ERR_CANCEL -19 /* canceled by remote */
#define OWL_ERR_SYNC   -20 /* sync error */
#define OWL_ERR_ABORT  -21  /* aborted */
#define OWL_ERR_INUSE -22 /* already in use */

const char *owl_error(int err);

#endif
