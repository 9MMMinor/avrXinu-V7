#ifndef _MATCH_H_
#define _MATCH_H_

/*! A reentry struct. 
 * Necessary to handle input that straddle packets.
 * The reentr is used to store progress in an operation
 * and each function can/should be fed the same reentr
 * repeatedly until the operation is complete.
 * Invariants : 
 *              The reentr must be reset before being used
 *              as a read/write parameter to a new function
 *              (the state is only valid for one consumer).
 */
struct match_reentr {
        char *buf;
        size_t len;
        /* If we're matching something and have a partial match when
         * the source data buffer runs out then match_offset tells us
         * how many bytes we've already matched. The next time we
         * get more data we can then use match_offset to skip
         * the right number of bytes in the match string. */
        size_t match_offset; 
        /* The flags field can be used to keep state during a
         * an parsing operation. */
        uint8_t flags;
};

/*! String search with optional case insensitivity.
 *
 * Finds a regular string in a buffer with arbitrary contents
 * (including NUL-bytes).  The comparison is optionally
 * case-insensitive so that field "Content-Type:" will match
 * "content-type:". 
 *
 * Supports matches straddling input buffers through the
 * match_offset parameter.
 *
 * @param match Contains a NUL-terminated string to find.
 * @param match_offset Can be NULL.
 *                     INPUT : Number of bytes to offset the
 *                     \a match string with initially.
 *                     OUTPUT : Number of bytes of partial match.
 *                     This is only set to non-0 on output if
 *                     the input string had a partial match at the end.
 * @param src Contains the data in which to find the \a match
 *            string. This buffer can contain any data.
 * @param src_len Length of the \a src buffer.
 * @param case_sens Define if the search should be case sensitive
 *                  (1) or not (0).
 * @return 
 *        - Returns a pointer to the first byte in \a src after the
 *          matching substring if the \a match string was found.
 *        - Return NULL if the \a match string was not found.
 */
char *match_string(char *match, 
                   size_t *match_offset,
                   char *src, 
                   size_t src_len,
                   int case_sens);


#endif /* _MATCH_H_ */
