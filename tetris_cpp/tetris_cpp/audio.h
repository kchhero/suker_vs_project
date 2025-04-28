#pragma once

#include <SDL3/SDL.h>
#include <string>

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    bool init();   // ����� �ʱ�ȭ
    void play();   // ����� ���
    void stop();   // ����� ����

private:
    SDL_AudioStream* stream;  // ����� ��Ʈ��
    Uint8* wav_data;          // .wav ������
    Uint32 wav_data_len;      // .wav ������ ����

    std::string getExecutablePath(const char* filename);  // ���� ���� ��� + ���� �̸� ��ġ��
};