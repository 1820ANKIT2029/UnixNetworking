#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <netinet/sctp.h>

int sctp_get_no_strms(int sock_fd, struct sockaddr *addr, socklen_t len);

int SERV_PORT = 8002;
char read_buf[BUFSIZ];

int main(int argc, char **argv) {
    int serv_fd;
    struct sockaddr_in serv_addr, client_addr;
    int client_addr_len, recv_len, stream_increment = 1;
    struct sctp_event_subscribe evnts;
    struct sctp_sndrcvinfo sri;
    int msg_flags;

    if(argc == 2) {
        stream_increment = atoi(argv[1]);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    serv_fd = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
    if(serv_fd == -1) {
        printf("error in socket init\n");
        exit(1);
    }

    int b_code = bind(serv_fd, (struct sockadrr *) &serv_addr, sizeof(serv_addr));
    if(b_code == -1) {
        printf("error in bind process\n");
        exit(1);
    }

    memset(&evnts, 0, sizeof(evnts));
    evnts.sctp_data_io_event = 1;
    setsockopt(serv_fd, IPPROTO_SCTP, SCTP_EVENTS, &evnts, sizeof(evnts));

    if(listen(serv_fd, 100)) {
        printf("error in listen process\n");
        exit(1);
    }

    while(1) {
        client_addr_len = sizeof(struct sockaddr_in);
        recv_len = sctp_recvmsg(
            serv_fd, read_buf, BUFSIZ, 
            (struct sockaddr *) &client_addr, 
            &client_addr_len,
            &sri, &msg_flags
        );

        if(stream_increment) {
            sri.sinfo_stream++;
            if(sri.sinfo_stream >= 
                sctp_get_no_strms(serv_fd, (struct sockaddr *) &client_addr, client_addr_len)
            ) {
                sri.sinfo_stream = 0;
            }
        }

        sctp_sendmsg(
            serv_fd, 
            read_buf, recv_len,
            (struct sockaddr *) &client_addr, client_addr_len,
            sri.sinfo_ppid, 
            sri.sinfo_flags,
            sri.sinfo_stream,
            0, 0
        );
    }
}

sctp_assoc_t sctp_address_to_associd(int sock_fd, struct sockaddr *sa, socklen_t salen){
	struct sctp_paddrparams sp;
	int siz;

	siz = sizeof(struct sctp_paddrparams);
	memset(&sp, 0, siz);
	memcpy(&sp.spp_address, sa, salen);
	sctp_opt_info(sock_fd, 0, SCTP_PEER_ADDR_PARAMS, &sp, &siz);
	return(sp.spp_assoc_id);
} 

int sctp_get_no_strms(int sock_fd, struct sockaddr *to, socklen_t tolen) {
    int retsz;
	struct sctp_status status;
	retsz = sizeof(status);	
	bzero(&status,sizeof(status));

	status.sstat_assoc_id = sctp_address_to_associd(sock_fd,to,tolen);
	getsockopt(sock_fd,IPPROTO_SCTP, SCTP_STATUS, &status, &retsz);
	return(status.sstat_outstrms);
}