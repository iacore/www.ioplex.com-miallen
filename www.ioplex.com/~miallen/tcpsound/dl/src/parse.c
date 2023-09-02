#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <mba/msgno.h>
#include "parse.h"

#define A2I(a) (src[a] - 48)

int
parse_time(const char *src, const char *slim, unsigned long *ms)
{
	unsigned long t = 0;

	if ((slim - src) < 15) {
		PMNO(ERANGE);
		return -1;
	}

	t += A2I(3) * 10 * 60 * 10000;
	t += A2I(4) * 60 * 10000;
	t += A2I(6) * 10000;
	t += A2I(7) * 1000;
	t += A2I(9) * 100;
	t += A2I(10) * 10;
	t += A2I(11);

	*ms = t;

	return 15;
}
int
parse_address(const char *src, const char *slim, char **addr, int *port, int *numdots)
{
	const char *start = src, *dot = NULL;

	while (src < slim && *src == ' ') {
		src++;
	}

	*numdots = 0;
	*addr = (char *)src;

	while (src < slim && *src != ' ' && *src != ':') {
		if (*src == '.') {
			dot = src;
			(*numdots)++;
		}
		src++;
	}

	if (*numdots == 4) {
		*port = atoi(dot + 1);
	}

	return src - start;
}
int
parse_packet(const char *src, const char *slim,
		struct packet *pkt)
{
	const char *start = src;
	int n, numdots;
	char *addr;

	if (isdigit(*src) == 0) {
		return 0;
	}

	if ((n = parse_time(src, slim, &pkt->time)) == -1) {
		return 0;
	}
	src += n;

	while (src < slim && *src == ' ') {
		src++;
	}

	if (strncmp(src, "arp ", 4) == 0) {
		pkt->proto = PROTO_ARP;
		pkt->srcport = pkt->dstport = 0;
	} else {
		pkt->proto = PROTO_IP;
		n = parse_address(src, slim, &addr, &pkt->srcport, &numdots);
		if (numdots == 4) {
			src += n;

			if (strncmp(src, " > ", 3) == 0) {
				src += 3;

				parse_address(src, slim, &addr, &pkt->dstport, &numdots);
				if (numdots == 4) {
					return src - start;
				}
			}
		} else if (numdots == 3) {
			const char *icmp = "icmp";
			int m = 0;
			while (m < 4 && src < slim && *src && *src != '\n') {
				if (*src == icmp[m]) {
					m++;
				} else {
					m = 0;
				}
				src++;
			}
			if (m == 4) {
				pkt->proto = PROTO_ICMP;
			}
		}
		pkt->srcport = pkt->dstport = 0;
	}

	return 1;
}

