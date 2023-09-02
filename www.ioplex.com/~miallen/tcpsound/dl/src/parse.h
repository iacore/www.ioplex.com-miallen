#ifndef PARSE_H
#define PARSE_H

#define DTIME_MIN 50
#define DTIME_MAX 3000

#define PROTO_ARP 1
#define PROTO_IP 2
#define PROTO_ICMP 3

struct packet {
	unsigned long time;
	int proto;
	int srcport;
	int dstport;
};

int parse_time(const char *src, const char *slim, unsigned long *ms);
int parse_address(const char *src, const char *slim, char **addr, int *port, int *numdots);
int parse_packet(const char *src, const char *slim, struct packet *pkt);

#endif /* PARSE_H */
