#include <owl/core/owl_debug.h>
#include <owl/core/owl_err.h>

const char *
owl_error(int err)
{
        if (err >= 0)
                return "success";
        
        switch (err) {
        case OWL_FAILURE:
                return "unknown failure";
        case OWL_ERR_PARAM:
                return "invalid parameters";
        case OWL_ERR_MEM:
                return "out of memory/buffers";
        case OWL_ERR_CONN:
                return "not connected";
        case OWL_ERR_STATE:
                return "invalid state";
        case OWL_ERR_PARSE:
                return "parsing error";
        case OWL_ERR_PATH:
                return "path not found";
        case OWL_ERR_TYPE:
                return "incompatible type";
        case OWL_ERR_PERM:
                return "no permission";
        case OWL_ERR_NOTSUPP:
                return "operation not supported";
        case OWL_ERR_RANGE:
                return "out of range"; 
        case OWL_ERR_SIZE:
                return "bad size";
        case OWL_ERR_AGAIN:
                return "retry later";
        case OWL_ERR_BUSY:
                return "busy";
        case OWL_ERR_DISABLED:
                return "disabled";
        case OWL_ERR_PROTO:
                return "protocol error";
        case OWL_ERR_TIMEOUT:
                return "timeout";
        case OWL_ERR_IGNORED:
                return "ignored";
        case OWL_ERR_CANCEL:
                return "canceled by remote";
        case OWL_ERR_SYNC:
                return "sync error";
        case OWL_ERR_ABORT:
                return "aborted";
        case OWL_ERR_INUSE:
                return "already in use";
        default:
                owl_assert(0);
                owl_err("unknown error code:%d\n", err);
                return "unknown";
        };
}
