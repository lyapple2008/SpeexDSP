#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "speex/speex_preprocess.h"
#include "../win32/VS2008/wav_io.h"
#include <stdio.h>

#define NN 160

void save_one_frame(int frm_cur, int frm_dst, short *in, int frm_size)
{
    if (frm_cur != frm_dst) {
        return;
    }

    FILE *pOneFrame = fopen("one_frame.raw", "wb");
    fwrite(in, sizeof(short), frm_size, pOneFrame);
}

int main()
{
   short in[NN];
   int i;
   SpeexPreprocessState *st;
   int count=0;
   float f;
   FILE *in_file, *out_file;
   char *filename = "C:\\MarshallPolyvWorkspace\\NoiseReduction\\test_corpus\\5dB\\sp01_airport_sn5.wav";
   WAV_HEADER header;

   in_file = fopen(filename, "rb");
   if (in_file == NULL){
	   printf("Fail to open file: %s\n", filename);
	   return -1;
   }
   out_file = fopen("audio_test_denoise.wav", "wb");
   if (out_file == NULL){
	   printf("Fail to create result file!\n");
	   return -1;
   }

   if (read_header(&header, in_file) != 0){
	   printf("Fail to parse wav file!\n");
	   return -1;
   }else{
	   print_header(&header);
   }

   if (header.format.bits_per_sample != 16){
	   printf("Just support 16 bits per sample1\n");
	   return -1;
   }
   write_header(&header, out_file);

   st = speex_preprocess_state_init(NN, 8000);
   i=1;
   speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_DENOISE, &i);
   i = -30;
   speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &i);
   //i=1;
   //speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_AGC, &i);
   //i=8000;
   //speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_AGC_LEVEL, &i);
   //i=0;
   //speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_DEREVERB, &i);
   //f=.0;
   //speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_DEREVERB_DECAY, &f);
   //f=.0;
   //speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_DEREVERB_LEVEL, &f);
   while (1)
   {
      int vad;
      if (count == 48) {
          while (0);
      }
      //fread(in, sizeof(short), NN, stdin);
	  read_samples(in, NN, &header, in_file);
      if (feof(in_file))
         break;
      vad = speex_preprocess_run(st, in);
      /*fprintf (stderr, "%d\n", vad);*/
      //fwrite(in, sizeof(short), NN, stdout);
	  write_samples(in, NN, &header, out_file);
      count++;
	  printf("Count = %d\n", count);
      
      //save_one_frame(count, 49, in, NN);
   }
   speex_preprocess_state_destroy(st);
   return 0;
}
