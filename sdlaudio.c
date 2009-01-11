#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

#include "sdlaudio.h"

struct OpaqueSound {
    int channel;
    Mix_Chunk *chunk;
};


void sdlaudio_close_audio()
{
    Mix_CloseAudio();
    SDL_Quit();
}


void sdlaudio_play_sound(struct OpaqueSound *sound)
{
    Mix_PlayChannel(sound->channel, sound->chunk, 0);
}


struct OpaqueSound* sdlaudio_load_sound(const char *filename, int channel)
{
    Sound sound = NULL;
    
    sound = (Sound)malloc(sizeof(struct OpaqueSound));

    if (sound == NULL) {
        return NULL;
    }

    sound->channel = channel;
    sound->chunk = Mix_LoadWAV(filename);

    return sound;
}


void sdlaudio_free_sound(struct OpaqueSound* sound)
{
    if (sound != NULL) {
        free(sound);
    }
}


int sdlaudio_init_audio()
{
    /* Init SDL with Audio Support */
    if (SDL_Init(SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "WARNING: unable to initialize audio. Reason: %s\n", SDL_GetError());
        return 1;
    }

    /* TODO: Put 22050, AUDIO_S16, 2, 128 in config */
    if (Mix_OpenAudio(22050, AUDIO_S16, 2, 128) < 0) {
        fprintf(stderr, "WARNING: audio could not be setup for 11025 Hz 16-bit stereo.\nReason: %s\n", SDL_GetError());
        return 2;
    }

    return 0;
}

