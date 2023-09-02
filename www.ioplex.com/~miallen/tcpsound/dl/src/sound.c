#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <mba/csv.h>
#include <mba/msgno.h>
#include "sound.h"

#define MIN(a,b) ((a) < (b) ? (a) : (b))

static const char *protos[] = { NULL, "ARP", "IP", "ICMP" };

int
sound_play(struct sound *snd, int num)
{
	struct match *mat = &snd->mat[num];
	struct sample *smp;
	int i;

	for (i = 0; i < CHAN_MAX; i++) {
		smp = &snd->chan[i];
		if (smp->pos == smp->len) {
			break;
		}
	}

	if (i == CHAN_MAX) {
		PMNO(ENOBUFS);
		return -1;
	}

	SDL_LockAudio();

	smp->data = mat->data;
	smp->len = mat->len;
	smp->pos = 0;

	SDL_UnlockAudio();

	return 0;
}
static int
path_search(const char *dirs[], int dn, const char *dlim, char *path, char *plim, const char *filename)
{
	int di;
	struct stat st;

	for (di = 0; di < dn; di++) {
		char *p = path;

		p += str_copy(dirs[di], dlim, p, plim, -1);
		*p++ = '/';
		p += str_copy(filename, filename + 255, p, plim, -1);

		if (stat(path, &st) == 0) {
			return 0;
		}
	}

	errno = ENOENT;
	PMNF(errno, ": %s", filename);

	return -1;
}
static int
sound_load_cfg(struct sound *snd, struct cfg *cfg)
{
	char searchpath[PATH_MAX], *sp = searchpath;
	const char *dirs[16];
	const char *key;
	int di;
	iter_t iter;

	if (cfg_get_str(cfg, searchpath, PATH_MAX, NULL, "searchpath") == -1) {
		AMSG("searchpath must be specified");
		return -1;
	}
	if (*sp == '\0') {
		PMSG("searchpath is empty");
		return -1;
	}

printf("searchpath: %s\nproto  src   dst wav path\n", searchpath);

	for (di = 0; *sp; di++) {
		dirs[di] = sp;
		while (*sp) {
			if (*sp == ':') {
				*sp = '\0';
				break;
			}
			sp++;
		}
		sp++;
	}

	snd->nmat = 0;

	cfg_iterate(cfg, &iter);
	while ((key = cfg_next(cfg, &iter)) && snd->nmat < NMAT_MAX) {
		const char *sd = "snd.";
		unsigned char val[1024], buf[1024], path[PATH_MAX];
		unsigned char *row[10];
		int i;

		for (i = 0; i < 4 && key[i] == sd[i]; i++) {
			;
		}
		if (i < 4) {
			continue;
		}

		/* If we make it here, key starts with 'snd.'
		 */

		if (cfg_get_str(cfg, val, 1024, NULL, key) == -1 ||
					csv_row_parse(val, 1024, buf, 1024, row, 10, ',', CSV_TRIM | CSV_QUOTES) == -1) {
			AMSG("");
			return -1;
		}
		if (path_search(dirs, di, searchpath + PATH_MAX, path, path + PATH_MAX, row[3]) == 0) {
			struct match *mat = snd->mat + snd->nmat;
			SDL_AudioSpec wav;
			SDL_AudioCVT cvt;
			unsigned char *data;
			int len;

			mat->proto = atoi(row[0]);
			mat->srcport = atoi(row[1]);
			mat->dstport = atoi(row[2]);

printf("%-4s %5d %5d %s\n", protos[mat->proto], mat->srcport, mat->dstport, path);

			/* Load the sound file and convert it to 16-bit stereo at 22kHz */
			if (SDL_LoadWAV(path, &wav, &data, &len) == NULL) {
				PMSG("Failed to load wav: %s: %s\n", path, SDL_GetError());
				return -1;
			}
			SDL_BuildAudioCVT(&cvt, wav.format, wav.channels, wav.freq, AUDIO_S16, 2, 22050);
			cvt.buf = malloc(len * cvt.len_mult);
			cvt.len = len;
			memcpy(cvt.buf, data, len);
			SDL_ConvertAudio(&cvt);
			SDL_FreeWAV(data);

			mat->data = cvt.buf;
			mat->len = cvt.len_cvt;

			snd->nmat++;
		} else {
			MMSG("Rule will be ignored.");
		}
	}

	if (snd->nmat == NMAT_MAX) {
		fprintf(stderr, "warning: only NMAT_MAX sounds (%d) loaded\n", NMAT_MAX);
	}

	return 0;
}
static void
sound_callback(void *userdata, unsigned char *dst, int dlen)
{
	struct sound *snd = userdata;
	int i;

	/* taken largely from the SDL tutorial
	 */
	for (i = 0; i < CHAN_MAX; i++) {
		struct sample *s = &snd->chan[i];
		int n = MIN(s->len - s->pos, dlen);
		SDL_MixAudio(dst, s->data + s->pos, n, SDL_MIX_MAXVOLUME);
		s->pos += n;
	}
}
int
sound_open(struct sound *snd, struct cfg *cfg)
{
	memset(snd, 0, sizeof *snd);

	if (sound_load_cfg(snd, cfg) == -1) {
		AMSG("");
		return -1;
	}

	/* Set 16-bit stereo audio at 22Khz */
	snd->fmt.freq = 22050;
	snd->fmt.format = AUDIO_S16;
	snd->fmt.channels = 2;
	snd->fmt.samples = 512;        /* A good value for games */
	snd->fmt.callback = sound_callback;
	snd->fmt.userdata = snd;

	/* Open the audio device and start playing sound! */
	if (SDL_OpenAudio(&snd->fmt, NULL) < 0) {
		PMSG("Unable to open audio: %s\n", SDL_GetError());
		return -1;
	}

	SDL_PauseAudio(0);

	return 0;
}
int
sound_close(struct sound *snd)
{
	memset(snd, 0, sizeof *snd);
	SDL_CloseAudio();
	return 0;
}

