#include <SDL3/SDL.h>
#include "audio.h"
#include <stdio.h>
#include <stdlib.h>

/* We will use this renderer to draw into this window every frame. */
static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_AudioStream* stream = NULL;
static Uint8* wav_data = NULL;
static Uint32 wav_data_len = 0;

char* get_executable_path(const char* filename) {
    char full_path[1024];
    char base_path[1024];
    if (SDL_GetBasePath() != NULL) {
        strcpy_s(base_path, SDL_GetBasePath());  // 실행파일 경로 얻기
    }
    else {
        strcpy_s(base_path, "./");  // 실패 시 현재 디렉토리
    }

    snprintf(full_path, sizeof(full_path), "%s%s", base_path, filename);

    return full_path;
}

int audio_init()
{
    SDL_AudioSpec spec;
    char* wav_path = NULL;

    //SDL_SetAppMetadata("Example Audio Load Wave", "1.0", "com.example.audio-load-wav");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return -1;
    }

    /* Load the .wav file from wherever the app is being run from. */
    SDL_asprintf(&wav_path, get_executable_path("TetrisBg.wav"), SDL_GetBasePath());  /* allocate a string of the full file path */
    if (!SDL_LoadWAV(wav_path, &spec, &wav_data, &wav_data_len)) {
        SDL_Log("Couldn't load .wav file: %s", SDL_GetError());
        return -1;
    }

    SDL_free(wav_path);  /* done with this string. */

    /* Create our audio stream in the same format as the .wav file. It'll convert to what the audio hardware wants. */
    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
    if (!stream) {
        SDL_Log("Couldn't create audio stream: %s", SDL_GetError());
        return -1;
    }

    /* SDL_OpenAudioDeviceStream starts the device paused. You have to tell it to start! */
    SDL_ResumeAudioStreamDevice(stream);

    return 1;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
//SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
//{
//    if (event->type == SDL_EVENT_QUIT) {
//        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
//    }
//    return SDL_APP_CONTINUE;  /* carry on with the program! */
//}

/* This function runs once per frame, and is the heart of the program. */
int audio_play()
{
    /* see if we need to feed the audio stream more data yet.
       We're being lazy here, but if there's less than the entire wav file left to play,
       just shove a whole copy of it into the queue, so we always have _tons_ of
       data queued for playback. */
    if (SDL_GetAudioStreamQueued(stream) < (int)wav_data_len) {
        /* feed more data to the stream. It will queue at the end, and trickle out as the hardware needs more data. */
        SDL_PutAudioStreamData(stream, wav_data, wav_data_len);
    }

    /* we're not doing anything with the renderer, so just blank it out. */
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    return 1;
}

/* This function runs once at shutdown. */
void audio_stop()
{
    SDL_free(wav_data);  /* strictly speaking, this isn't necessary because the process is ending, but it's good policy. */
    /* SDL will clean up the window/renderer for us. */
}
