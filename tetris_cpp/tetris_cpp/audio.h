#pragma once

#include <SDL3/SDL.h>
#include <string>

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    bool init();   // 오디오 초기화
    void play();   // 오디오 재생
    void stop();   // 오디오 정리

private:
    SDL_AudioStream* stream;  // 오디오 스트림
    Uint8* wav_data;          // .wav 데이터
    Uint32 wav_data_len;      // .wav 데이터 길이

    std::string getExecutablePath(const char* filename);  // 실행 파일 경로 + 파일 이름 합치기
};