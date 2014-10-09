/* 
 * File:   main.c
 * Author: RedDec
 *
 * Created on 6 October 2014, 18:05 (UTC+4)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <getopt.h>
#include "mutalk.h"

int main(int argc, char** argv) {
    const int contentSize = 65535;
    size_t cnt;
    char c;
    int loop = 1;
    char content[contentSize];
    int verbose = 1;

    mutalk_msg_t msg;
    // Create new instance of MUTalk with buffer size = contentSize 
    mutref talk = mutalk_create(contentSize);

    while ((c = getopt(argc, argv, "qs:m:ro")) != -1)
        switch (c) {
            case 's':
                // Join to group
                if (mutalk_group_add(talk, optarg) && verbose) printf("Listening group %s\n", optarg);
                break;
            case 'm':
                cnt = fread(content, 1, contentSize, stdin);
                while (cnt > 0) {
                    // Send message to group
                    mutalk_send(talk, optarg, content, cnt);
                    if (feof(stdin))break;
                }
                break;
            case 'o':
                loop = 0;
            case 'r':
                // Wait for message from groups without timeout
                while (!(msg = mutalk_wait(talk, content, contentSize, -1)).error) {
                    if (verbose) {
                        content[msg.count] = '\0';
                        printf("[%s:%u](%s)<%li bytes> %s\n",
                                inet_ntoa(msg.source_ip),
                                msg.source_port,
                                msg.stream->name,
                                msg.count,
                                content);
                    } else {
                        fwrite(content, msg.count, 1, stdout);
                        fflush(stdout);
                    }
                    if (!loop)break;
                }
                break;
            case 'q':
                verbose = 0;
                break;
            case '?':
                printf("-q quite (do not print message info)\n");
                printf("-s <subject name> subject to listen\n");
                printf("-m <subject name> send message to subject\n");
                printf("-r rcv message\n");
                printf("-o rcv one message\n");
                return 1;
            default:
                abort();
        }
    // Clean memory
    mutalk_destroy(talk);
    return (EXIT_SUCCESS);
}

