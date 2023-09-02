#ifndef SOUND_H
#define SOUND_H

#include <mba/cfg.h>
#include <SDL/SDL.h>
#include <SDL/SDL_audio.h>

/* At most CHAM_MAX sounds can be played simultaniously. Additional
 * sound_play calls will simply be ignored.
 *
 * There can be at most NMAT_MAX matching rules in the configuration file.
 */

#define CHAN_MAX 4
#define NMAT_MAX 64

/* The CHAN_MAX sample array is used to mix sounds
 */
struct sample {
	unsigned char *data;
	int len;
	int pos;
};

/* Matchs are created directly from configuration file match rules.
 * Each WAV is preconverted to a sample suitable for use with the
 * SDL library
 */
struct match {
	int proto;
	int srcport;
	int dstport;
	unsigned char *data;
	int len;
};

/* The sound context is used to hold all state associated with playing
 * sounds.
 */
struct sound {
	SDL_AudioSpec fmt;
	struct sample chan[CHAN_MAX];                /* array to mix sounds */
	struct match mat[NMAT_MAX];  /* match rules and associated WAV data */
	int nmat;
};

/* open/close the sound context
 */
int sound_open(struct sound *snd, struct cfg *cfg);
int sound_close(struct sound *snd);

/* play the sound (rule) indexed at snd->mat[num]
 */
int sound_play(struct sound *snd, int num);

#endif /* SOUND_H */
