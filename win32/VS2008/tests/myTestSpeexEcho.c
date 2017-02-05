#include "speex/speex_echo.h"
#include "speex/speex_preprocess.h"
#include "../win32/VS2008/wav_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NN 480

SpeexEchoState *echo_state = NULL;
SpeexPreprocessState *denoise_state;
char* rec_buffer = NULL;
char* play_buffer = NULL;
int sampleRate = 8000;

void aec_init(int frame_size)
{
	if (echo_state || denoise_state)
		return;
	rec_buffer = (char*)malloc(frame_size * 2);
	play_buffer = (char*)malloc(frame_size * 2);

	echo_state = speex_echo_state_init(frame_size, frame_size * 10);
	denoise_state = speex_preprocess_state_init(frame_size, sampleRate);
	speex_echo_ctl(echo_state, SPEEX_ECHO_SET_SAMPLING_RATE, &sampleRate);
	speex_preprocess_ctl(denoise_state, SPEEX_PREPROCESS_SET_ECHO_STATE, echo_state);
}

void aec_uninit()
{
	if (echo_state)
		speex_echo_state_destroy(echo_state);
	if (denoise_state)
		speex_preprocess_state_destroy(denoise_state);
	echo_state = NULL;
	denoise_state = NULL;
	if (rec_buffer)
		free(rec_buffer);
	if (play_buffer)
		free(play_buffer);
	play_buffer = NULL;
	rec_buffer = NULL;
}

void aec_record_audio(void* audio_rec_buffer, unsigned int audio_data_len)
{
	if (echo_state == NULL || denoise_state == NULL || rec_buffer == NULL)
		return;
	memset(rec_buffer, 0, audio_data_len);
	speex_echo_capture(echo_state, (spx_int16_t*)audio_rec_buffer, (spx_int16_t*)rec_buffer);
	speex_preprocess_run(denoise_state, (spx_int16_t*)rec_buffer);
	memcpy(audio_rec_buffer, rec_buffer, audio_data_len);
}

void aec_play_audio(void* audio_play_buffer, unsigned int audio_data_len)
{
	if (echo_state == NULL || denoise_state == NULL || play_buffer == NULL)
		return;
	speex_echo_playback(echo_state, (spx_int16_t*)audio_play_buffer);
}



int main(int argc, char *argv[])
{
	FILE *mic_file, *speak_file, *result_file;
	WAV_HEADER mic_header, speaker_header;
	int frequency, channels, length;
	int frm_cnt = 0;

	if (argc != 4){
		printf("Usage: pro.exe mic_signal speak_signal result_signal\n");
		return -1;
	}

	if (fopen_s(&mic_file, argv[1], "rb") ||
		fopen_s(&speak_file, argv[2], "rb") ||
		fopen_s(&result_file, argv[3], "wb")){
		printf("Fail to open file !!!\n");
		return -1;
	}
	
	if (read_header(&mic_header, mic_file) != 0){
		printf("Fail to parse wav file: %s\n", argv[1]);
		return -1;
	}
	if (read_header(&speaker_header, speak_file) != 0){
		printf("Fail to parse wav file: %s\n", argv[2]);
		return -1;
	}

	if (mic_header.format.bits_per_sample != 16 ||
		speaker_header.format.bits_per_sample != 16){
		printf("Now only support 16 bits per sample!\n");
		return -1;
	}

	write_header(&mic_header, result_file);

	frequency = mic_header.format.sample_per_sec;//16000;
	length = frequency / 100;
	channels = mic_header.format.channels;

	aec_init(length);

	while (!feof(mic_file) && !feof(speak_file)){
		read_samples(play_buffer, length, &speaker_header, speak_file);
		aec_play_audio(play_buffer, length);

		read_samples(rec_buffer, length, &mic_header, mic_file);
		aec_record_audio(rec_buffer, length);

		write_samples(rec_buffer, length, &mic_header, result_file);
		printf("Frame #%d\n", frm_cnt++);
	}

	aec_uninit();
	fclose(mic_file);
	fclose(speak_file);
	fclose(result_file);

	return 0;
}