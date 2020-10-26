mutalk
======

![mutalk](https://user-images.githubusercontent.com/6597086/97156868-47bb3200-17b2-11eb-8b4e-137f658459da.png)


UDP Multicast messaging library

API
======

See full example in `example.c`

# Includes

* `#include <stdbool.h>`
* `#include <stdint.h>`

# Constants

* `MUTALK_MAX_NAME_SIZE 1024`

# Structures

### Group definition

```c
typedef struct mutalk_group_t {
    struct mutalk_group_t *next, *prev; // References to nearby groups
    char *name;                         // Group name.
    size_t name_size;                   // Group name size in bytes
    int fd;                             // Socket descriptor
} mutalk_group_t;

```

### Message definition

```c
typedef struct mutalk_msg_t {
    bool error;               // Error(also timeout) flag
    ssize_t count;            // Bytes written to buffer
    mutalk_group_t *stream;   // Reference to group definition
    struct in_addr source_ip; // Sender IP
    uint16_t source_port;     // Sender port
} mutalk_msg_t;
```

### MUTalk definition

```c
typedef struct mutalk_t {
    size_t cache_size;                     // Size of allocated buffer for incoming messages
    int epoll_fd;                          // EPOLL descriptor
    int sender_fd;                         // Socket descriptor, used for outgoing messages
    struct mutalk_group_t *groups;         // Reference to first group definition. May be NULL
    char *cache_data;                      // Buffer for incoming messages
    char cache_name[MUTALK_MAX_NAME_SIZE]; // Buffer for name in incoming messages
} mutalk_t;
```

# Types

* **`mutref`** - `struct mutalk_t*`

# Functions

## mutalk_create

```c
mutref mutalk_create(size_t buffer_size)
```
Create new instance of MUTalk engine.
Returns NULL if something wrong (see errno message) or instance of MUTalk.
Use `mutalk_destory` for cleaning memory.

* `size_t buffer_size` Maximum size of incoming messages

**This function is thread-safe**.

Example:

```c
#include "mutalk.h"

int main() {
  mutref talk = mutalk_create(65535);
  //... Do something
  mutalk_destroy(talk);
  return 0;
}

```

## mutalk_destroy

```c
void mutalk_destroy(mutref talk)
```
Close all sockets and EPoll. Free all allocated memory

* `mutref talk` MUTalk instance ref

**This function is NOT thread-safe for one MUTalk instance**.

Example: see [mutalk_create](#mutalk_create)


## mutalk_group_add

```c
bool mutalk_group_add(mutref talk, const char* name)
```
Join to MUTalk group. Do nothing if group already joined.
Returns `true` if joined to group, otherwise false (also if already joined to group)

* `mutref talk` MUTalk instance ref
*  `const char* name` Group name. Maximum lenght defined in MUTALK_MAX_NAME_SIZE. Name can't has spaces

**This function is NOT thread-safe for one MUTalk instance**

Example:

```c
// See initialization in mutalk_create example
const char *groupName = "some.test.group\0";
if(mutalk_group_add(talk, groupName))
  printf("Listening group %s\n", groupName);
else
  fprintf(stderr, "Failed listen group %s\n", groupName);
```

## mutalk_group_remove

```c
void mutalk_group_remove(mutref talk, const char* name)
```
Leave MUTalk group. Do nothing if group not joined

* `mutref talk` MUTalk instance ref
*  `const char* name` Group name. Maximum lenght defined in MUTALK_MAX_NAME_SIZE. Name can't has spaces

**This function is NOT thread-safe for one MUTalk instance**

## mutalk_send

```c
void mutalk_send(mutref talk, const char *subject,const char * data, size_t size)
```
Send message to group.

**Important!** Big messages (more then MTU size) may be corrupted
(it depends of your network configuration).

* `mutref talk` MUTalk instance ref
* `const char* name` Group name. Maximum lenght defined in MUTALK_MAX_NAME_SIZE. Name can't has spaces
* `const char* data` Any data
* `size_t size` Size of data.

**This function is thread-safe**

Example:

```c
// See initialization in mutalk_create example
const char *message = "Hello World!\0";
const char *groupName = "some.test.group\0";
mutalk_send(talk, groupName, message, strlen(message));
```

## mutalk_wait

```c
mutalk_msg_t mutalk_wait(
  mutref talk,
  void *buffer,
  size_t buf_size,
  int32_t timeout_ms)
```
Wait message from any joined groups. Returns mutalk_msg_t instance.

* `mutref talk` MUTalk instance ref
* `void* buffer` Destination buffer.
* `size_t buf_size` Destination buffer size
* `int32_t timeout_ms` Timeout in milliseconds. Set -1 for infinity.
If no messages received till timeout raised, `msg.error=true` will be set

**This function is NOT thread-safe for one MUTalk instance**

Example:

```c
// See initialization in mutalk_create example
const size_t contentSize = 65535;
mutalk_msg_t msg;
char content[contentSize];

while (!(msg = mutalk_wait(talk, content, contentSize, -1)).error) {
    content[msg.count] = '\0';
    printf("[%s:%u](%s)<%li bytes> %s\n",
            inet_ntoa(msg.source_ip),
            msg.source_port,
            msg.stream->name,
            msg.count,
            content);
}
```
