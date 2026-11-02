#ifndef AUDIO_H
#define AUDIO_H

#include "config.h"

typedef struct {
    Sound sndFootstep, sndPickupKey, sndPickupPowerup, sndEnemyDetect;
    Sound sndDamage, sndExitUnlock, sndDecoy;
    Sound sndLevelComplete, sndGameOver;
    Sound sndMenuSelect, sndMenuMove;
    Music bgMusic;
    bool  musicLoaded, soundsLoaded;
    float soundVolume, musicVolume;
} GameAudio;

void initAudio(GameAudio *a);
void unloadAudio(GameAudio *a);
void playGameSound(GameAudio *a, Sound snd);
void updateMusic(GameAudio *a);
void setMusicVolume(GameAudio *a, float v);
void setSoundVolume(GameAudio *a, float v);

#endif /* AUDIO_H */
