#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <mba/shellout.h>
#include <mba/text.h>
#include <mba/time.h>
#include <mba/cfg.h>
#include <mba/misc.h>
#include <mba/msgno.h>

#include "parse.h"
#include "sound.h"

static const char *DEFAULT_CONF = "searchpath = /usr/share/sounds:/usr/local/share/sounds\n\nsnd.default   = 1,0,0,generic.wav\nsnd.icmp      = 3,0,0,sonar.wav\nsnd.dns       = 2,53,53,warning.wav\nsnd.http.in   = 2,0,80,pop.wav\nsnd.http.out  = 2,80,0,info.wav\nsnd.win.nam   = 2,137,137,cuckoo.wav\n";

/* struct packet {
 *     unsigned long time;
 *     int proto;
 *     int srcport;
 *     int dstport;
 * };
 */

int
handle_packet(struct sound *snd, struct packet *pkt)
{
	int m;

	for (m = 0; m < NMAT_MAX; m++) {
		struct match *mat = snd->mat + m;

		if (pkt->proto == mat->proto &&
				(mat->srcport == 0 || pkt->srcport == mat->srcport) &&
				(mat->dstport == 0 || pkt->dstport == mat->dstport)) {
							/* packet matches exactly */
			sound_play(snd, m);
			return 0;
		}
	}
	for (m = 0; m < NMAT_MAX; m++) {
		struct match *mat = snd->mat + m;

		if (pkt->proto == mat->proto &&
				(pkt->srcport == mat->srcport ||
				pkt->dstport == mat->dstport)) {
					/* packet matches one of two ports */
			sound_play(snd, m);
			return 0;
		}
	}

	sound_play(snd, 0);

	return 0;
}
int
is_redundant(struct packet *pkt)
{
	static struct packet last = { 0, 0, 0, 0 };
	int red;

	red = (pkt->time - last.time) <= DTIME_MIN &&
			pkt->proto == last.proto &&
			pkt->srcport == last.srcport &&
			pkt->dstport == last.dstport &&
			pkt->srcport != 0;
	last = *pkt;

	return red;
}
int
run(const char *cfgfile, const char *sshhost, const char *tcpdumpcmd)
{
	struct cfg cfg;
	struct sound snd;
	struct sho *sh;
	char buf[1024], remoteprompt[255];
	const unsigned char *pv[] = { remoteprompt, "\n" };
	int i, n;
	long tr, dp, tpl, trl;
	struct packet pkt;

	if (cfgfile == NULL) { /* no config specified, use ~/.tcpsound */
		const char *s;
		char *bp = buf, *blim = buf + 1024;
		struct stat st;

		if ((s = getenv("HOME")) == NULL) {
			PMNO(errno);
			return -1;
		}

		bp += str_copy(s, s + 512, bp, blim - 100, 1024);
		s = "/.tcpsound";
		bp += str_copy(s, s + 11, bp, blim, -1);

		if (stat(buf, &st) == -1) {
			FILE *out;
						/* default config doesn't exit; create it */
			if ((out = fopen(buf, "w")) == NULL) {
				PMNF(errno, ": %s", buf);
				return -1;
			}
			if (fwrite(DEFAULT_CONF, 1, strlen(DEFAULT_CONF), out) < 1) {
				PMNF(EIO, ": %s", buf);
				return -1;
			}
			fclose(out);
		}

		cfgfile = buf;
	}

	cfg_init(&cfg, NULL);
	if (cfg_load(&cfg, cfgfile) == -1) {
		AMSG("");
		return -1;
	}

	if (cfg_get_str(&cfg, remoteprompt, 255, "# ", "remoteprompt") == -1) {
		AMSG("");
		return -1;
	}

	if (sound_open(&snd, &cfg) == -1) {
		AMSG("");
		return -1;
	}
	if ((sh = sho_open("sh", pv[0], 0)) == NULL) {
		AMSG("");
		return -1;
	}

	if (sshhost) {
		char sbuf[512];

		n = snprintf(sbuf, 512, "ssh %s\n", sshhost);
		if (writen(sh->ptym, sbuf, n) == -1) {
			PMNO(errno);
			return -1;
		}

		if ((i = sho_loop(sh, pv, 1, 30)) != 1) {
			AMSG("");
			return -1;
		}
	}

	fflush(stdout);
	if (writen(sh->ptym, tcpdumpcmd, strlen(tcpdumpcmd)) == -1) {
		PMNO(errno);
		return -1;
	}

	trl = tpl = 0;

	for ( ;; ) {
		if ((i = sho_expect(sh, pv, 2, buf, 1024, 0)) == -1) {
			AMSG("");
			return -1;
		}

		/* print tcpdump output unchanged
		 */

		fputs(buf, stdout);

		if (i == 2) {
			tr = time_current_millis() & 0xFFFFFFF;

			if ((n = parse_packet(buf, buf + 1024, &pkt)) == -1) {
				AMSG("");
				return -1;
			} else if (n == 0) {
				continue;
			}

			dp = pkt.time - tpl;
			tpl = pkt.time;

			if (dp > DTIME_MIN || is_redundant(&pkt) == 0) {
				handle_packet(&snd, &pkt);
			}
		} else if (i == 1) { 
			break;
		} else if (i == 0) { 
			fputs("EOF\n", stderr);
			break;
		}
	}

	writen(sh->ptym, "exit $?\n", 8);
	sho_close(sh);

	return 0;
}

int
main(int argc, char *argv[])
{
	char buf[1024], *bp = buf, *blim = buf + 1024;
	const char *cmd = "tcpdump -nn";
	const char *cfgfile = NULL;
	const char *sshhost = NULL;
	const char *tcpdumpoptions;
	int n, i, argi;

	if (argc < 1) {
usage:
		fprintf(stderr, "usage: %s [-c <cfgfile>] [-r <sshhost>] [<tcpdumpoptions>]\n", argv[0]);
		return EXIT_FAILURE;
	}
	for (argi = 1; argi < argc; argi++) {
		if (strcmp(argv[argi], "-h") == 0) {
			goto usage;
		} else if (strcmp(argv[argi], "-c") == 0) {
			argi++;
			if (argi == argc) {
				fprintf(stderr, "-s requires a config file\n");
				goto usage;
			}
			cfgfile = argv[argi];
		} else if (strcmp(argv[argi], "-r") == 0) {
			argi++;
			if (argi == argc) {
				fprintf(stderr, "-r requires user@host\n");
				goto usage;
			}
			sshhost = argv[argi];
		} else {
			break;
		}
	}

		/* collect all remaining argv arguments into one string
		 */
	tcpdumpoptions = bp;

	n = strlen(cmd) + 1;
	bp += str_copy(cmd, cmd + n, bp, blim, -1);
	for (i = argi; i < argc && bp < (blim - 2); i++) {
		*bp++ = ' ';
		n = strlen(argv[i]) + 1;
		bp += str_copy(argv[i], argv[i] + n, bp, blim, -1);
	}
	*bp++ = '\n';
	*bp = '\0';

	if (run(cfgfile, sshhost, tcpdumpoptions) == -1) {
		MMSG("");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

