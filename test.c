#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#define NO_POINT 100
#define AMP 1
#define MEAN 1
#define FREQ 1

typedef struct {
  float amp,
        mean,
        freq;
} chan_para;

typedef int wave_pt[NO_POINT];

wave_pt *wave_type;
wave_pt *test;
chan_para *ch;

void f_malloc(void){
	int i;
  if((ch = (chan_para*)malloc(2 * sizeof(chan_para))) == NULL) {
    printf("Not enough memory.\n");
    exit(1);
  }
  for(i=0; i<2; i++){ //initialize default parameter
    ch[i].amp  = AMP;
    ch[i].mean = MEAN;
    ch[i].freq = FREQ;
  }

  if((wave_type = (wave_pt*)malloc(4 * sizeof(wave_pt))) == NULL) {
    printf("Not enough memory.\n");
    exit(1);
  }

}


void f_WaveGen(){
  int i;
  float dummy;

  //Sine wave array
  float delta = (2.0*3.142)/NO_POINT;
  for(i=0; i<NO_POINT; i++){
    dummy = ((i*delta))*0x7fff/5;
    wave_type[0][i] = dummy;
   //printf("%d\n", wave_type[0][i]);
  }
    //Square wave array
  //the value of 2 in dummy when i<NO_POINT/2 indicates the value of 'ON'
  //the value of 0 in dummy when i<NO_POINT indicates the value of 'OFF'
  for(i=0;i<NO_POINT;i++){
    if(i<=NO_POINT/2) dummy = 1*0x7fff;
    if(i>NO_POINT/2) dummy = -1*0x7fff;
    wave_type[1][i] =  dummy /5;
  }
  //Saw-tooth wave array
  //the delta used is similar to the one used for sine wave
  //the dummy is increased by multiple of delta when i <NO_POINT/2
  //then the dummy is decrease by multiple of delta when i<NO_POINT
  //the value of '1' in dummy when i<NO_POINT indicates the max value of wave form when i=NO_POINT/2
  delta = 2.0/NO_POINT;
  for(i=0;i<NO_POINT;i++){
    if(i<=NO_POINT/2) dummy = i*delta*0x7fff;
    if(i>NO_POINT/2 && i<NO_POINT) dummy = (-2+i*delta)*0x7fff;
    wave_type[3][i] =  dummy /5;
  }
  //Triangular wave array
  //the value of 2 in delta indicates the max vertical value of the wave form when i = NO_POINT-1, the value is closed to 2
  //the min vertical value of wave form is 0 when i = 0
  delta = 4.0/NO_POINT;
  for(i=0;i<NO_POINT;i++){
    if(i<=NO_POINT/4) dummy = i*delta*0x7fff;
    if(i>NO_POINT/4 && i<=NO_POINT*3/4) dummy = (2-i*delta)*0x7fff;
    if(i>NO_POINT*3/4 && i<NO_POINT) dummy = (-4+i*delta)*0x7fff;
    wave_type[2][i] =  dummy /5;
  }
}

int main(int argc, char const *argv[]) {
  f_malloc();
  f_WaveGen();
  test = &wave_type[0];
  for(int i = 0;i<NO_POINT;i++) {
    printf("%d = %d\n",wave_type[0][i], (*(test))[i]);

  }
  return 0;
}
