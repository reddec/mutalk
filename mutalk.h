/* 
 * File:   mutalk.h
 * Author: RedDec
 * 
 * MUTalk - UDP MUlticast messaging
 * 
 * Created on 8 October 2014, 20:19 (UTC+4)
 */

#ifndef MUTALK_H
#define	MUTALK_H
#define MUTALK_MAX_NAME_SIZE 1024
#include <stdint.h>
#include <stdbool.h>

/**
 * Group definition
 */
typedef struct mutalk_group_t {
    struct mutalk_group_t *next, *prev; // References to nearby groups
    char *name; // Group name.
    size_t name_size; // Group name size in bytes
    int fd; // Socket descriptor
} mutalk_group_t;

/**
 * Message definition
 */
typedef struct mutalk_msg_t {
    bool error; // Error(also timeout) flag
    ssize_t count; // Bytes written to buffer
    mutalk_group_t *stream; // Reference to group definition
    struct in_addr source_ip; // Sender IP
    uint16_t source_port; // Sender port
} mutalk_msg_t;

/**
 * MUTalk definition
 */
typedef struct mutalk_t {
    size_t cache_size; // Size of allocated buffer for incoming messages
    int epoll_fd; // EPOLL descriptor
    int sender_fd; // Socket descriptor, used for outgoing messages
    struct mutalk_group_t *groups; // Reference to first group definition. May be NULL
    char *cache_data; // Buffer for incoming messages
    char cache_name[MUTALK_MAX_NAME_SIZE]; // Buffer for name in incoming messages

} mutalk_t;

typedef struct mutalk_t* mutref;

/**
 * Create new instance of MUTalk engine. 
 * Use mutalk_destory for cleaning memory.
 * [This function is thread-safe]
 * @param buffer_size Maximum size of incoming messages
 * @return NULL if something wrong (see errno message) or instance of MUTalk
 */
mutref mutalk_create(size_t buffer_size);

/**
 * Close all sockets and EPoll. Free all allocated memory
 * [This function is NOT thread-safe for one MUTalk instance]
 * @param talk MUTalk instance ref
 */
void mutalk_destroy(mutref talk);
/**
 * Join to MUTalk group. Do nothing if group already joined
 * [This function is NOT thread-safe for one MUTalk instance]
 * @param talk MUTalk instance ref
 * @param name Group name. Maximum lenght defined in MUTALK_MAX_NAME_SIZE. Name can't has spaces
 * @return std_true if joined to group, otherwise false (also if already joined to group)
 */
bool mutalk_group_add(mutref talk, const char* name);

/**
 * Leave MUTalk group. Do nothing if group not joined
 * [This function is NOT thread-safe for one MUTalk instance] 
 * @param talk MUTalk instance ref
 * @param name Group name.  Maximum lenght defined in MUTALK_MAX_NAME_SIZE. Name can't has spaces
 */
void mutalk_group_remove(mutref talk, const char* name);

/**
 * Send message to group. 
 * Important! Big messages (more then MTU size) may be corrupted
 * [This function is thread-safe]
 * @param talk MUTalk instance ref
 * @param subject  Group name.  Maximum lenght defined in MUTALK_MAX_NAME_SIZE. Name can't has spaces
 * @param data Any data
 * @param size Size of data. 
 */
void mutalk_send(mutref talk, const char *subject,const char * data, size_t size);

/**
 * Wait message from any joined groups.
 * [This function is NOT thread-safe for one MUTalk instance]
 * @param talk MUTalk instance ref
 * @param buffer Destination buffer.
 * @param buf_size Destination buffer size
 * @param timeout_ms Timeout in milliseconds. Set -1 for infinity. 
 * If no messages received till timeout raised, error=true will be returned
 * @return Message instance.
 */
mutalk_msg_t mutalk_wait(mutref talk, void *buffer, size_t buf_size, int32_t timeout_ms);


#endif	/* MUTALK_H */

