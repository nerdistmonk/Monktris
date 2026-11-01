// src/apu.c
#include "apu.h"
#include <SDL3/SDL.h>
#include <vorbis/vorbisfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static SDL_AudioDeviceID audio_device = 0;
static SDL_AudioStream* music_stream = NULL;
static SDL_AudioStream* sfx_stream = NULL;

static int16_t* music_pcm = NULL;
static int music_pcm_len_bytes = 0;
static int music_channels = 2;
static int music_rate = 44100;

static int16_t* load_ogg(const char* path, int* out_len, int* out_channels, int* out_rate) {
    OggVorbis_File vf;
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;

    if (ov_open_callbacks(f, &vf, NULL, 0, OV_CALLBACKS_DEFAULT) < 0) {
        fclose(f);
        return NULL;
    }

    vorbis_info* vi = ov_info(&vf, -1);
    *out_channels = vi->channels;
    *out_rate = vi->rate;

    int capacity = 1024 * 1024;
    int total_bytes = 0;
    char* data = (char*)malloc(capacity);

    int bitstream = 0;
    while (1) {
        char chunk[4096];
        long bytes = ov_read(&vf, chunk, sizeof(chunk), 0, 2, 1, &bitstream);
        if (bytes <= 0) break;

        if (total_bytes + bytes > capacity) {
            capacity *= 2;
            data = (char*)realloc(data, capacity);
        }
        memcpy(data + total_bytes, chunk, bytes);
        total_bytes += bytes;
    }

    ov_clear(&vf);
    *out_len = total_bytes;
    return (int16_t*)data;
}

void apu_init(void) {
    SDL_AudioSpec default_spec;
    default_spec.channels = 2;
    default_spec.format = SDL_AUDIO_S16LE;
    default_spec.freq = 44100;

    audio_device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &default_spec);
    if (!audio_device) {
        fprintf(stderr, "SDL3 Audio Device error: %s\n", SDL_GetError());
        return;
    }

    music_stream = SDL_CreateAudioStream(&default_spec, &default_spec);
    if (music_stream) {
        SDL_BindAudioStream(audio_device, music_stream);
    }

    SDL_AudioSpec sfx_spec;
    sfx_spec.channels = 1;
    sfx_spec.format = SDL_AUDIO_S16LE;
    sfx_spec.freq = 44100;

    sfx_stream = SDL_CreateAudioStream(&sfx_spec, &default_spec);
    if (sfx_stream) {
        SDL_BindAudioStream(audio_device, sfx_stream);
    }

    apu_set_music(1);
}

void apu_shutdown(void) {
    if (music_pcm) {
        free(music_pcm);
        music_pcm = NULL;
    }
    if (music_stream) {
        SDL_DestroyAudioStream(music_stream);
        music_stream = NULL;
    }
    if (sfx_stream) {
        SDL_DestroyAudioStream(sfx_stream);
        sfx_stream = NULL;
    }
    if (audio_device) {
        SDL_CloseAudioDevice(audio_device);
        audio_device = 0;
    }
}

void apu_set_music(uint8_t song_id) {
    if (!music_stream) return;

    SDL_ClearAudioStream(music_stream);

    if (music_pcm) {
        free(music_pcm);
        music_pcm = NULL;
        music_pcm_len_bytes = 0;
    }

    if (song_id == 0) return;

    char path[128];
    snprintf(path, sizeof(path), "snd/%d.ogg", (song_id >= 1 && song_id <= 3) ? song_id : 1);

    int channels = 0, rate = 0, len = 0;
    int16_t* decoded = load_ogg(path, &len, &channels, &rate);

    if (!decoded) {
        snprintf(path, sizeof(path), "../snd/%d.ogg", (song_id >= 1 && song_id <= 3) ? song_id : 1);
        decoded = load_ogg(path, &len, &channels, &rate);
    }

    if (decoded && len > 0) {
        music_channels = channels;
        music_rate = rate;

        SDL_AudioSpec file_spec;
        file_spec.channels = channels;
        file_spec.format = SDL_AUDIO_S16LE;
        file_spec.freq = rate;

        SDL_SetAudioStreamFormat(music_stream, &file_spec, NULL);

        music_pcm = decoded;
        music_pcm_len_bytes = len;
        SDL_PutAudioStreamData(music_stream, music_pcm, music_pcm_len_bytes);
    }
}

void apu_tick_frame(void) {
    if (music_stream && music_pcm && music_pcm_len_bytes > 0) {
        // Threshold: 1 full second of audio buffer remaining
        int threshold = music_rate * music_channels * (int)sizeof(int16_t);
        if (SDL_GetAudioStreamAvailable(music_stream) < threshold) {
            SDL_PutAudioStreamData(music_stream, music_pcm, music_pcm_len_bytes);
        }
    }
}

void apu_play_sfx(uint8_t sfx_id) {
    if (!sfx_stream) return;

    int duration_ms = 40;
    float freq = 440.0f;
    float vol = 0.3f;
    int type = 0;

    switch (sfx_id) {
        case SFX_MOVE:
            freq = 880.0f; duration_ms = 35; vol = 0.25f; type = 0;
            break;
        case SFX_ROTATE:
            freq = 587.3f; duration_ms = 50; vol = 0.30f; type = 0;
            break;
        case SFX_LOCK:
            freq = 140.0f; duration_ms = 70; vol = 0.40f; type = 1;
            break;
        case SFX_LINE_CLEAR:
            freq = 1174.6f; duration_ms = 200; vol = 0.45f; type = 0;
            break;
        default:
            return;
    }

    int sample_rate = 44100;
    int samples = (sample_rate * duration_ms) / 1000;
    int16_t* buffer = (int16_t*)malloc(samples * sizeof(int16_t));
    if (!buffer) return;

    float phase = 0.0f;
    float phase_step = (2.0f * 3.14159265f * freq) / (float)sample_rate;
    uint32_t lfsr = 0x7FFF;

    for (int i = 0; i < samples; i++) {
        float sample = 0.0f;
        if (type == 0) {
            sample = (phase < 3.14159265f) ? 1.0f : -1.0f;
        } else {
            uint32_t fb = (lfsr & 1) ^ ((lfsr >> 1) & 1);
            lfsr = (lfsr >> 1) | (fb << 14);
            sample = (lfsr & 1) ? 1.0f : -1.0f;
        }

        float decay = 1.0f - ((float)i / (float)samples);
        buffer[i] = (int16_t)(sample * decay * vol * 28000.0f);

        phase += phase_step;
        if (phase >= 2.0f * 3.14159265f) phase -= 2.0f * 3.14159265f;
    }

    SDL_PutAudioStreamData(sfx_stream, buffer, samples * sizeof(int16_t));
    free(buffer);
}