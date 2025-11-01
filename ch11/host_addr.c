#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <arpa/inet.h>

int main(int argc, char **argv) {
    if(argc == 1) {
        printf("usage: %s <host-name>*\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct hostent *result;
    char *ptr, **pptr;
    char buf4[INET_ADDRSTRLEN], buf6[INET6_ADDRSTRLEN];

    for(int i=1; i<argc; i++) {
        result = gethostbyname(argv[i]);
        if(result == NULL) continue;

        printf("----- %s result -----\n", argv[i]);
        printf("official hostname: %s\n", result->h_name);

        for(pptr = result->h_aliases; *pptr != NULL; pptr++) {
            printf("alias : %s\n", *pptr);
        }

        switch(result->h_addrtype) {
            case AF_INET:
                for(pptr = result->h_addr_list; *pptr != NULL; pptr++) {
                    printf("address: %s\n", inet_ntop(AF_INET, *pptr, buf4, sizeof(buf4)));
                }
                break;

            case AF_INET6:
                for(pptr = result->h_addr_list; *pptr != NULL; pptr++) {
                    printf("address: %s\n", inet_ntop(AF_INET6, *pptr, buf6, sizeof(buf6)));
                }
                break;
            
            default:
                printf("unknown address type\n"); 
        }
    }
}