/*
 * audio.c — Audio loading/playback with graceful fallback
 */

#include "audio.h"
#include <stdio.h>

void initAudio(GameAudio *a)
{
    a->soundVolume = 0.7f; a->musicVolume = 0.5f;
    a->soundsLoaded = a->musicLoaded = false;

    if (FileExists("assets/sounds/footstep.wav"))
        { a->sndFootstep = LoadSound("assets/sounds/footstep.wav"); a->soundsLoaded = true; }
    if (FileExists("assets/sounds/key.wav"))
        { a->sndPickupKey = LoadSound("assets/sounds/key.wav"); a->soundsLoaded = true; }
    if (FileExists("assets/sounds/powerup.wav"))
        a->sndPickupPowerup = LoadSound("assets/sounds/powerup.wav");
    if (FileExists("assets/sounds/damage.wav"))
        a->sndDamage = LoadSound("assets/sounds/damage.wav");
    if (FileExists("assets/sounds/enemy_detect.wav"))
        a->sndEnemyDetect = LoadSound("assets/sounds/enemy_detect.wav");
    if (FileExists("assets/sounds/exit_unlock.wav"))
        a->sndExitUnlock = LoadSound("assets/sounds/exit_unlock.wav");
    if (FileExists("assets/sounds/decoy.wav"))
        a->sndDecoy = LoadSound("assets/sounds/decoy.wav");
    if (FileExists("assets/sounds/levelcomplete.wav"))
        a->sndLevelComplete = LoadSound("assets/sounds/levelcomplete.wav");
    if (FileExists("assets/sounds/gameover.wav"))
        a->sndGameOver = LoadSound("assets/sounds/gameover.wav");
    if (FileExists("assets/sounds/select.wav"))
        a->sndMenuSelect = LoadSound("assets/sounds/select.wav");
    if (FileExists("assets/sounds/move.wav"))
        a->sndMenuMove = LoadSound("assets/sounds/move.wav");

    if (FileExists("assets/music/background.ogg")) {
        a->bgMusic = LoadMusicStream("assets/music/background.ogg");
        a->musicLoaded = true;
        SetMusicVolume(a->bgMusic, a->musicVolume);
    }
}

void unloadAudio(GameAudio *a)
{
    if (a->sndPickupKey.frameCount)     UnloadSound(a->sndPickupKey);
    if (a->sndFootstep.frameCount)       UnloadSound(a->sndFootstep);
    if (a->sndPickupPowerup.frameCount) UnloadSound(a->sndPickupPowerup);
    if (a->sndEnemyDetect.frameCount)    UnloadSound(a->sndEnemyDetect);
    if (a->sndExitUnlock.frameCount)     UnloadSound(a->sndExitUnlock);
    if (a->sndDecoy.frameCount)          UnloadSound(a->sndDecoy);
    if (a->sndDamage.frameCount)        UnloadSound(a->sndDamage);
    if (a->sndLevelComplete.frameCount) UnloadSound(a->sndLevelComplete);
    if (a->sndGameOver.frameCount)      UnloadSound(a->sndGameOver);
    if (a->sndMenuSelect.frameCount)    UnloadSound(a->sndMenuSelect);
    if (a->sndMenuMove.frameCount)      UnloadSound(a->sndMenuMove);
    if (a->musicLoaded) { StopMusicStream(a->bgMusic); UnloadMusicStream(a->bgMusic); }
}

void playGameSound(GameAudio *a, Sound s)
{
    if (s.frameCount > 0 && a->soundVolume > 0) { SetSoundVolume(s, a->soundVolume); PlaySound(s); }
}

void updateMusic(GameAudio *a) { if (a->musicLoaded) UpdateMusicStream(a->bgMusic); }
void setMusicVolume(GameAudio *a, float v) { a->musicVolume=v; if (a->musicLoaded) SetMusicVolume(a->bgMusic,v); }
void setSoundVolume(GameAudio *a, float v) { a->soundVolume=v; }
