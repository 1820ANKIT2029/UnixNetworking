#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <arpa/inet.h>

int main(int argc, char** argv) {
    if(argc != 3) {
        printf("usage: %s <serve-name> <protocol-name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct servent *result = getservbyname(argv[1], argv[2]);
    if(result == NULL) return 0;

    printf("official name: %s\n", result->s_name);
    for(char **pptr = result->s_aliases; *pptr != NULL; pptr++) {
        printf("alias: %s\n", *pptr);
    }
    printf("port number: %d\n", ntohs(result->s_port));
    printf("protocol: %s\n", result->s_proto);
}