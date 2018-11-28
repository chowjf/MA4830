#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <hw/pci.h>
#include <hw/inout.h>
#include <sys/neutrino.h>
#include <sys/mman.h>
#include <math.h>
#include <time.h>
#include <process.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>

#define INTERRUPT       iobase[1] + 0               // Badr1 + 0 : also ADC register
#define MUXCHAN         iobase[1] + 2               // Badr1 + 2
#define TRIGGER         iobase[1] + 4               // Badr1 + 4
#define AUTOCAL         iobase[1] + 6               // Badr1 + 6
#define DA_CTLREG       iobase[1] + 8               // Badr1 + 8

#define AD_DATA         iobase[2] + 0               // Badr2 + 0
#define AD_FIFOCLR      iobase[2] + 2               // Badr2 + 2

#define TIMER0          iobase[3] + 0               // Badr3 + 0
#define TIMER1          iobase[3] + 1               // Badr3 + 1
#define TIMER2          iobase[3] + 2               // Badr3 + 2
#define COUNTCTL        iobase[3] + 3               // Badr3 + 3
#define DIO_PORTA       iobase[3] + 4               // Badr3 + 4
#define DIO_PORTB       iobase[3] + 5               // Badr3 + 5
#define DIO_PORTC       iobase[3] + 6               // Badr3 + 6
#define DIO_CTLREG      iobase[3] + 7               // Badr3 + 7
#define PACER1          iobase[3] + 8               // Badr3 + 8
#define PACER2          iobase[3] + 9               // Badr3 + 9
#define PACER3          iobase[3] + a               // Badr3 + a
#define PACERCTL        iobase[3] + b               // Badr3 + b

#define DA_Data         iobase[4] + 0               // Badr4 + 0
#define DA_FIFOCLR      iobase[4] + 2               // Badr4 + 2

int badr[5];                                        // PCI 2.2 assigns 6 IO base addresses

struct pci_dev_info info;
void *hdl;

uintptr_t iobase[6];

#define BILLION 1000000000L
#define NUM_THREADS 4

pthread_t thread[NUM_THREADS];
int t = 0;

int dac0, dac1;     // dac for each dac output
int dac0_wave, dac1_wave;     // wave type identifier for each dac output
int resolution;
float delta;
int errorGlobal;    //tells output_config thread what error message to print

typedef struct {
    float amplitude;
    float frequency;
    float mean;
} wave;

typedef struct {
    short int point_value[200];
} dac_data;

wave *wave_parameter;        // array of wave struct
dac_data *database; // array of dac_data struct
char s[60];         // scanf buffer
char result[30];    // config command extract
char *name_type[] = {"SINE WAVE", "TRIANGULAR WAVE", "SQUARE WAVE"};

// set up condition variable
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int condition = 1;


float toFloat(char input[]) {
    // convert to float
    int i;

    if (strlen(input) == 1) return (-9999.0);

    if (input[(strlen(input)-1)] == '\n') input[(strlen(input)-1)] = NULL;

    for (i=0; i < strlen(input); i++) {
        if (input[i] >= '0' && input[i] <= '9') {
            // check if it can be converted to a digit
            continue;
        }
        else if (input[i] == '.' || input[i] == 'e' || input[i] == 'E') {
                // if they are one of the forms of floating point expressions
                return atof(input);
            }
            else {
                printf("Invalid input! It contains non-digit character(s). Please input again.\n\n");
                return (-9999.0);
            }

    }
    // it is likely input as integer, but we cast it to float
    return atof(input);
}

void read_data_file(char filename[]) {
    FILE *fp;
    //int i;
    //int length;
    int wave;
    int count = 0;

    char type;
    float value;
    int terminate = 1;
    printf("filename = %s", filename);
    delay(2000);

    // We assume argv[1] is a filename to open
    fp = fopen( filename, "r" );
    /* fopen returns exit(1), the NULL pointer, on failure */
    if ( fp == NULL ) {
        printf( "Could not open file\n" );
        delay(2000);
        exit(1);
    }
    do {
        fscanf(fp, "%f %c", &value, &type);
        switch (type) {
        case 'A': wave_parameter[wave].amplitude = value; printf("A is ok\n"); break;
        case 'M': wave_parameter[wave].mean = value; printf("M is ok\n"); break;
        case 'F': wave_parameter[wave].frequency = value; printf("F is ok\n"); break;
        case 'Z':   if(value == 0.0) {
                        printf("I have done reading\n");
                            terminate=0;
                    }
                    else {
                        printf("Z: Exit with error message\n");
                        exit(1);
                    }
                    break;
        case 'W': printf("I know which wave type\n"); wave = value; break;
        case 'E':   if(value == 0.0) {
                        printf("I have finished reading one wave\n");
                        if(count = 0) dac0_wave = wave;
                        if(count = 1) dac1_wave = wave;
                        count++;
                    }
                    else {
                        printf("E: Exit with error message\n");
                        exit(1);
                    }
                    break;
        default: printf("No matches\n"); exit(1);
        }
    } while (terminate);

    fclose(fp);

}

void write_data_file(char filename[]) {
    FILE *fp;
    char str[200];

    printf("Wave parameters are being saved to file %s\n", filename);

    fp=fopen(filename,"w");
    if (fp==NULL)
    {
        perror("cannot open");
        exit(1);
    }
    printf("I am priting text to file now\n");
    fprintf(fp, "%d W\n%f A\n%f M\n%f F\n0.0 E\n", dac0_wave,wave_parameter[dac0_wave].amplitude,wave_parameter[dac0_wave].mean,wave_parameter[dac0_wave].frequency);

    fprintf(fp, "%d W\n%f A\n%f M\n%f F\n0.0 E\n", dac1_wave,wave_parameter[dac1_wave].amplitude,wave_parameter[dac1_wave].mean,wave_parameter[dac1_wave].frequency);
    fprintf(fp, "%f %c", 0.0, 'Z');
    delay(2000);
    printf("%s is now saved\n", filename);
    fclose(fp);
}

void wave_input_param(int wave_type) {
    // capture selected wave parameters
    do{
    	printf("Enter amplitude: ");
    	fgets(s, 30, stdin);
	} while(toFloat(s) == -9999.0);
	wave_parameter[wave_type].amplitude = toFloat(s) ;
	fflush(stdin);

	do{
    	printf("Enter frequency: ");
    	fgets(s, 30, stdin);
	} while(toFloat(s) == -9999.0);
	wave_parameter[wave_type].frequency = toFloat(s);
	fflush(stdin);

	do{
    	printf("Enter mean value: ");
    	fgets(s, 30, stdin);
	}while(toFloat(s) == -9999.0);
	wave_parameter[wave_type].mean = toFloat(s);
	fflush(stdin);
}

void wave_select(int dac, char wave_type) {
    // wave identifier from command line argument
	int wave;

	switch(wave_type) {
		case 's':  printf("SINE WAVE is selected\n");
				   wave = 0;
				   wave_input_param(0);
				   break;
		case 't':  printf("TRIANGULAR WAVE is selected\n");
				   wave = 1;
				   wave_input_param(1);
				   break;
		case 'q':  printf("SQUARE WAVE is selected\n");
				   wave = 2;
				   wave_input_param(2);
				   break;
		default: printf("[Error!] Invalid Option\n\n"); break;
	}

	if (dac) {
	  dac1_wave = wave;
	}
	else {
	  dac0_wave = wave;
	}
}

void sine_wave(wave value, int dac) {
    float dummysine;
    int i;
    for(i=0; i<resolution; i++) {
        dummysine= ((value.amplitude * sinf((float)(i*delta))) + value.mean); //equation for sine wave
        if(dummysine > 5)        dummysine = 5;
        else if(dummysine < 0)   dummysine = 0;
        database[dac].point_value[i]= (unsigned) (dummysine * 0x3333);   // scale to 5V
    }
}

void square_wave(wave value, int dac) {
    float dummysquare;
    int i;
    //square wave//
    for(i=0; i<resolution; i++) {
        if(i < resolution/2) {
            dummysquare = value.mean - value.amplitude; //equation for lower square wave
            if(dummysquare<0) dummysquare = 0;
            else if(dummysquare>5)dummysquare = 5;
            database[dac].point_value[i] = (unsigned)(dummysquare*0x3333); //scale to 5V
        }
        else {
            dummysquare=value.mean + value.amplitude;  //equation for upper square wave
            if(dummysquare<0) dummysquare = 0;
            else if(dummysquare>5) dummysquare = 5;
            database[dac].point_value[i] = (unsigned)(dummysquare*0x3333); //scale to 5V
        }
    }
}

void tri_wave(wave value, int dac) {
    float dummytri;
    int i;
    for(i=0; i<resolution; i++) {
        dummytri= ((value.amplitude*2*asinf((sinf((float)(i*delta))))/3.1416) + value.mean);  //equation for triangle wave
        if(dummytri > 5)        dummytri = 5;
        else if(dummytri < 0)   dummytri = 0;
        database[dac].point_value[i]= (unsigned) (dummytri * 0x3333); //scale to 5V
    }
}

void data_point_calculation(int wave, int dac) {
    switch (wave) {
        case 0: sine_wave(wave_parameter[wave], dac); break;
        case 1: tri_wave(wave_parameter[wave], dac); break;
        case 2: square_wave(wave_parameter[wave], dac); break;
        default: printf("Something wrong, no data point calculated\n");break;
    }

}
void cmd_line(int count, char *arg[]) {
    int number;
	char **p_to_arg = &arg[1];

	number = 0;

	while(--count && ((*p_to_arg)[0] == '-')) {
		switch((*p_to_arg)[1]) {
			case 'a':   printf("DAC (0) is selected\n");
            			p_to_arg++;
            			wave_select(dac0, *p_to_arg[0]);
                        data_point_calculation(dac0_wave, dac0);
                        number++;
            			--count;
            			p_to_arg++;
            			break;
			case 'b':   printf("DAC (1) is selected\n");
            			p_to_arg++;
            			wave_select(dac1, *p_to_arg[0]);
                        data_point_calculation(dac1_wave, dac1);
                        number++;
            			--count;
            			p_to_arg++;
            			break;
			case 'f':   printf("File Option\n");
            			p_to_arg++;
                        read_data_file((*p_to_arg));
            			--count;
            			p_to_arg++;
            			break;
			default:    printf("[Error!] Invalid Option. Please use argument -a | -b | -f. Exiting program.\n\n");
                        delay(2000);
                        exit(0);
                        break;
		}
	}

	if (number == 1) {
	printf("[Error!] We need to have two waves. Exiting program\n");
	delay(2000);
	exit(1);
	} else if( number > 2) {
	printf("[Error!] Seems like there are too many waves. Exiting program\n");
	delay(2000);
	exit(1);
	}

}

void return_string(char text[], int start, int end) {
    // capture argument in config command
    int length = end - start + 1;
    char  temp[length];
    int i;

    for(i=0; i<length; i++) {
        temp[i] = text[start];
        start++;
    }

    strncpy(result, temp, length);
    result[length] = '\0';      // add NULL as "result" string terminator
}

void dac_capture(int dac, int dac_wave, char command[], int count, int indexing[]) {
    int i;
    for(i=1; i<(count/2); i++) {
        return_string(command, indexing[2*i], indexing[2*i+1]);
        if (result[0] == '-') {
            i++;
            switch(result[1]) {
                case('a'):  return_string(command, indexing[2*i], indexing[2*i+1]);
                            //printf("it is amplitude of %f\n", toFloat(result));
                            if(toFloat(result) == -9999.0){
                                errorGlobal = 2;
                                break;
                            }
                            else
                                wave_parameter[dac_wave].amplitude = toFloat(result);
                            break;
                case('f'):  return_string(command, indexing[2*i], indexing[2*i+1]);
                            //printf("it is frequency of %f\n", toFloat(result));
                            if(toFloat(result) == -9999.0){
                                errorGlobal = 2;
                                break;
                            }
                            else
                                wave_parameter[dac_wave].frequency = toFloat(result);
                            break;
                case('m'):  return_string(command, indexing[2*i], indexing[2*i+1]);
                            //printf("it is mean of %f\n", toFloat(result));
                            if(toFloat(result) == -9999.0){
                                errorGlobal = 2;
                                break;
                            }
                            else
                                wave_parameter[dac_wave].mean = toFloat(result);
                            break;
                case('n'):  return_string(command, indexing[2*i], indexing[2*i+1]);
                            //printf("it is mean of %f\n", toFloat(result));
                            printf("filename is %s\n", result);
                            write_data_file(result);
                            break;
                default:    printf("No matches\n");
                            break;
            }
        }
    }
    data_point_calculation(dac_wave, dac);
}

void *output_config() {
    // thread to display wave parameters
    while(1) {
        pthread_mutex_lock(&mutex);
        while(condition == 0) pthread_cond_wait(&cond, &mutex);

        printf("\f*************************************************************************************************************\n");
        printf("                                             WAVE FORM GENERATOR\n\n");
        printf("*************************************************************************************************************\n");
        printf("\n\nDAC (0)\n");
        printf("-------------------------------------------------------------------------------------------------------------\n");
        printf("Type (0): %s\n", name_type[dac0_wave]);
        printf("Amplitude (0): %lf\n", wave_parameter[dac0_wave].amplitude);
        printf("Mean Value (0): %lf\n", wave_parameter[dac0_wave].mean);
        printf("Frequency (0): %lf\n", wave_parameter[dac0_wave].frequency);
        printf("\n\nDAC (1)\n");
        printf("-------------------------------------------------------------------------------------------------------------\n");
        printf("Type (1): %s\n",name_type[dac1_wave]);
        printf("Amplitude (1): %lf\n", wave_parameter[dac1_wave].amplitude);
        printf("Mean Value (1): %lf\n", wave_parameter[dac1_wave].mean);
        printf("Frequency (1): %lf\n", wave_parameter[dac1_wave].frequency);
        if(errorGlobal == 1){
        	printf("\n\n*************************************************************************************************************\n[Error!] Too many switches turned on!\n[Error!] Please only turn on 1 of the 3 switches: Frequency, Amplitude, Mean\n\n*************************************************************************************************************\n");
            errorGlobal = 0;
        }
        else if(errorGlobal == 2){
            printf("\n\n*************************************************************************************************************\n[Error!] Input contain non-digit character, please key in again.\n\n*************************************************************************************************************\n");
        	errorGlobal = 0;
        }

        printf("\n\n[*] To adjust the different parameters using keyboard input [*]\nPlease enter through command:\n dac0 for DAC (0) port\n dac1 for DAC (1) port\n -a follow by a space and a value to adjust the Amplitude\n -m follow by a space and a value to adjust the Mean Value\n -f follow by a space and a value to adjust the Frequency\nExample: dac0 -a 1.0 -m 1.0 -f 1.0\n");
 +      printf("\n[*] To save parameters to a file [*]\nPlease enter through command:\n saveparam -n filename.txt\n");
 +      printf("\n[*] To exit the program [*]\nPlease press Crtl + c\n\n");

        printf("Enter command: ");

        condition = 0;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
}

void *wait_server() {
    while(1) {
        pthread_mutex_lock(&mutex);
        while(condition == 0) pthread_cond_wait(&cond, &mutex);

        printf("Enter something to proceed: \n");
        gets(s);
        condition = 0;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
}

void *input_server() {
    int i,j;
    int index[20];
    while(1) {
        // capture config command

        //printf("Enter command: ");
        fflush(stdin);
        fgets(s, 30, stdin);

        j = 0;
        index[j] = 0;
        j++;

        for(i=0; i<strlen(s); i++) {
            if(s[i] == ' ' || s[i] == '\n') {
                index[j] = i-1;
                j++;
                index[j] = i+1;
                j++;
            }
        }

        i = 0;

        pthread_mutex_lock(&mutex);
        while(condition == 1) pthread_cond_wait(&cond, &mutex);

        return_string(s, index[2*i], index[2*i+1]);
        if(strcmp(result, "dac0") == 0) {
            //printf("DAC0 is captured\n");
            dac_capture(dac0, dac0_wave, s, j, index);
        }
        else if (strcmp(result, "dac1") == 0) {
            //printf("DAC1 is captured\n");
            dac_capture(dac1, dac1_wave, s, j, index);
        }
        else if (strcmp(result, "saveparam") == 0) {
            //printf("DAC1 is captured\n");
            dac_capture(dac1, dac1_wave, s, j, index);
        }
        else {
            printf("[Error!]Please input correct command\n");
            printf("[Error!]Please wait while page refreshing\n");
            delay(2000);
        }
        // printf("Enter something to proceed: \n");
        // gets(s);
        condition = 1;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
}

void *board_input(void* arg)
{
    //Raw digital switches reading
    uintptr_t digitalSwitch;
    //Raw potentiometer readings
	uint16_t potentiometerReading1, potentiometerReading2;
    //Previous Readings
    int previousSwitch = -1;
    int previousPot1 = -1;
    int previousPot2 = -1;
    //Limits & Increments
    const float freqMax = 100;
    const float freqMin = 0;
    const float ampMax = 5.0;
    const float ampMin = 0.0;
    const float meanMax = 5.0;
    const float meanMin = 0.0;
    const float freqIncrement = (freqMax - freqMin)/127;
    const float ampIncrement = (ampMax - ampMin)/127;
    const float meanIncrement = (meanMax - meanMin)/127;

    //Local Variables
    float frequency;
    float mean;
    float amplitude;
    int potentiometerSelection;//1- adjust potentiometer 1, 2-adjust potentiometer 2
    int freqSwitch;//switch to turn on/off edit mode for frequency.0 = OFF,1 = ON
    int meanSwitch;//switch to turn on/off edit mode for mean value.0 = OFF,1 = ON
    int ampSwitch;//switch to turn on/off edit mode for amplitude.0 = OFF,1 = ON
    int pot1Scale;//Scale of ADC0 reading
    int pot2Scale;//Scale of ADC1 reading


    unsigned int i, count;
    unsigned short chan;


    while(1)
    {
        int updated = 0;
        //*****************************************************************************
        //Digital Port Functions
        //*****************************************************************************
        out8(DIO_CTLREG,0x90);                  // Port A : Input,  Port B : Output,  Port C (upper | lower) : Output | Output
        digitalSwitch=in8(DIO_PORTA);           // Read Port A

        //******************************************************************************
        // ADC Port Functions
        //******************************************************************************
        // Initialise Board
        out16(INTERRUPT,0x60c0);                // sets interrupts   - Clears
        out16(TRIGGER,0x2081);                  // sets trigger control: 10MHz, clear, Burst off,SW trig. default:20a0
        out16(AUTOCAL,0x007f);                  // sets automatic calibration : default
        out16(AD_FIFOCLR,0);                    // clear ADC buffer
        out16(MUXCHAN,0x0D00);                  // Write to MUX register - SW trigger, UP, SE, 5v, ch 0-0
                                                // x x 0 0 | 1  0  0 1  | 0x 7   0 | Diff - 8 channels
                                                // SW trig |Diff-Uni 5v| scan 0-7| Single - 16 channels
        count=0x00;
        while(count <0x02)
        {
            chan= ((count & 0x0f)<<4) | (0x0f & count);
            out16(MUXCHAN,0x0D00|chan);                  // Set channel   - burst mode off.
            delay(1);                                    // allow mux to settle
            out16(AD_DATA,0);                            // start ADC
            while(!(in16(MUXCHAN) & 0x4000));
            if(count == 0x00)
            {
                potentiometerReading1=in16(AD_DATA);
            }
            else
            {
                potentiometerReading2=in16(AD_DATA);
            }
            fflush( stdout );
            count++;
            delay(5);                               // Write to MUX register - SW trigger, UP, DE, 5v, ch 0-7
        }



        //Process Raw data to filter out noise and unwanted bits
        digitalSwitch= digitalSwitch & 0x0f;
        potentiometerReading1 = potentiometerReading1 & 0xfe00;  //bitwise AND operation to remove noise(LSB 9bits)
        potentiometerReading2 = potentiometerReading2 & 0xfe00;  //bitwise AND operation to remove noise(LSB 9bits)
        potentiometerSelection = (digitalSwitch & 0x08)>>3;      //rightshift 3 bits

        //Check conditions to update global parameters
        if(digitalSwitch != 0x00 && digitalSwitch != 0x08)
        {
            if(digitalSwitch != previousSwitch)
            {
                previousSwitch = digitalSwitch;
                updated = 1;
                if(potentiometerSelection == 0)
                {
                    previousPot1 = potentiometerReading1;
                }
                else if(potentiometerSelection == 1)
                {
                    previousPot2 = potentiometerReading2;
                }
            }
            else
            {
                if(potentiometerSelection == 0)
                {
                    if(potentiometerReading1 != previousPot1)
                    {
                        previousPot1 = potentiometerReading1;
                        updated = 1;
                    }
                }
                else if(potentiometerSelection == 1)
                {
                    if(potentiometerReading2 != previousPot2)
                    {
                        previousPot2 = potentiometerReading2;
                        updated = 1;
                    }
                }
            }
        }



        if(updated)
        {
            //Extract the value for each bit of port A digital i/o switches
            freqSwitch = (digitalSwitch & 0x04)>>2;
            ampSwitch = (digitalSwitch & 0x02)>>1;
            meanSwitch = digitalSwitch & 0x01;


            //Right shift the potentiometer reading
            pot1Scale = potentiometerReading1 >> 9;  //Rightshift 9 bits
            pot2Scale = potentiometerReading2 >> 9;  //Rightshift 9 bits


            //Error Checking for frequency,amplitude and mean switches
            if((freqSwitch && ampSwitch) || (freqSwitch && meanSwitch) || (ampSwitch && meanSwitch) || (freqSwitch && ampSwitch && meanSwitch))
            {
                previousSwitch = digitalSwitch;
                pthread_mutex_lock(&mutex);
                while(condition == 1)
                {
                    pthread_cond_wait(&cond,&mutex);
                }
                condition = 1;
                errorGlobal = 1;
                pthread_cond_broadcast( &cond );
                pthread_mutex_unlock( &mutex );

            }
            else //Calculate the value of the parameter to be changed
            {
                if(potentiometerSelection == 0)
                {
                    if(freqSwitch == 1)
                    {
                        frequency = freqMin + (pot1Scale * freqIncrement);
                        pthread_mutex_lock(&mutex);
                        while(condition == 1)
                        {
                            pthread_cond_wait(&cond, &mutex);
                        }
                        wave_parameter[dac0_wave].frequency = frequency;
                    }
                    else if(ampSwitch == 1)
                    {
                        amplitude = ampMin + (pot1Scale * ampIncrement);
                        pthread_mutex_lock(&mutex);
                        while(condition == 1)
                        {
                            pthread_cond_wait(&cond, &mutex);
                        }
                        wave_parameter[dac0_wave].amplitude = amplitude;
                    }
                    else if(meanSwitch == 1)
                    {
                        mean = meanMin + (pot1Scale * meanIncrement);
                        pthread_mutex_lock(&mutex);
                        while(condition == 1)
                        {
                            pthread_cond_wait(&cond, &mutex);
                        }
                        wave_parameter[dac0_wave].mean = mean;
                    }
                    data_point_calculation(dac0_wave, dac0);
                }
                else if(potentiometerSelection == 1)
                {
                    if(freqSwitch == 1)
                    {
                        frequency = freqMin + (pot2Scale * freqIncrement);
                        pthread_mutex_lock(&mutex);
                        while(condition == 1)
                        {
                            pthread_cond_wait(&cond, &mutex);
                        }
                        wave_parameter[dac1_wave].frequency = frequency;
                    }
                    else if(ampSwitch == 1)
                    {
                        amplitude = ampMin + (pot2Scale * ampIncrement);
                        pthread_mutex_lock(&mutex);
                        while(condition == 1)
                        {
                            pthread_cond_wait(&cond, &mutex);
                        }
                        wave_parameter[dac1_wave].amplitude = amplitude;
                    }
                    else if(meanSwitch == 1)
                    {
                        mean = meanMin + (pot2Scale * meanIncrement);
                        pthread_mutex_lock(&mutex);
                        while(condition == 1)
                        {
                            pthread_cond_wait(&cond, &mutex);
                        }
                        wave_parameter[dac1_wave].mean = mean;
                    }
                    data_point_calculation(dac1_wave, dac1);
                }
                condition = 1;

                pthread_cond_broadcast(&cond);
                pthread_mutex_unlock(&mutex);
            }

        }
        delay(500);
    }
}

void *wave_generator() {
    struct timespec start, stop;

    int tick;
    double i, j;
    double accum;

    i = 0;
    j = 0;
    tick = 0;

    if(clock_gettime(CLOCK_REALTIME,&start) == -1) {
        perror("clock gettime");
        exit(EXIT_FAILURE);
    }

    while(1) {
        if(tick == 10000) {
            if(clock_gettime(CLOCK_REALTIME,&stop)==-1) {
                perror("clock gettime");
                exit(EXIT_FAILURE);
            }
            accum = (double)(stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec) / BILLION; //time interval for each loop
            if(clock_gettime(CLOCK_REALTIME, &start) == -1) {
                perror("clock gettime");
                exit(EXIT_FAILURE);
            }
            tick = 0;
        }

        i += ((wave_parameter[dac0_wave].frequency * resolution) * (accum / 10000));    //frequency offset for DAC0
        j += ((wave_parameter[dac1_wave].frequency * resolution) * (accum / 10000));    //frequency offset for DAC1

        if(i>200) i = 0;
        if(j>200) j = 0;


        out16(DA_CTLREG,0x0a23);            // DA Enable, #0, #1, SW 5V unipolar
        out16(DA_FIFOCLR, 0);                   // Clear DA FIFO  buffer
        out16(DA_Data,(short) database[0].point_value[(int)i]);
        out16(DA_CTLREG,0x0a43);            // DA Enable, #0, #1, SW 5V unipolar
        out16(DA_FIFOCLR, 0);                   // Clear DA FIFO  buffer
        out16(DA_Data,(short) database[1].point_value[(int)j]);

        tick += 1;
    }
}

void initialize() {
    int i;
    // to initialize PCI-DAS1602

    memset(&info,0,sizeof(info));
    if(pci_attach(0)<0) {
        perror("pci_attach");
        exit(EXIT_FAILURE);
    }

    /* Vendor and Device ID */
    info.VendorId=0x1307;
    info.DeviceId=0x01;

    if ((hdl=pci_attach_device(0, PCI_SHARE|PCI_INIT_ALL, 0, &info))==0) {
        perror("pci_attach_device");
        exit(EXIT_FAILURE);
    }

    // Determine assigned BADRn IO addresses for PCI-DAS1602
    for(i=0;i<5;i++) {
        badr[i]=PCI_IO_ADDR(info.CpuBaseAddress[i]);
    }

    for(i=0;i<5;i++) {                                     // expect CpuBaseAddress to be the same as iobase for PC
        iobase[i]=mmap_device_io(0x0f,badr[i]);
    }

    if(ThreadCtl(_NTO_TCTL_IO,0)==-1) {
        perror("Thread Control");
        exit(1);
    }
}

void signal_handler()
{
    int i;

    int rc;

    if (t==0) {
        printf("Program is exiting.\n");
        pthread_exit(NULL);
    }
    else{
        for(i = 5; i>-1; i--)
        {
            printf("Program exiting in: %d\n", i);
            sleep(1);
        }
        while (t > -1) {
            printf("Terminating threads %d\n", t);
            pthread_cancel(thread[t--]);
            if(t==0) printf("\n");
        }
    }
	exit(0);
}

int main(int argc, char *argv[]) {
    int rc;     // configure thread settings

    // initialize global variable
    dac0 = 0;
    dac1 = 1;
    resolution = 200;
    delta = 2.0 * 3.1416 / (float)resolution;

    signal(SIGINT, signal_handler);

    // initialize PCI board
    initialize();
    printf("Initialization complete.\n\n");

    // array of 3 wave struct
	if ((wave_parameter=(wave *)malloc(4 * sizeof(wave))) == NULL) {
	    printf("Not enough memory\n");
	    exit(0);
	}

    wave_parameter[3].amplitude = 0;
    wave_parameter[3].mean = 0;
    wave_parameter[3].frequency = 0;

    // array of 2 dac_data struct
    if ((database=(dac_data *)malloc(2 * sizeof(dac_data))) == NULL) {
        printf("Not enough memory\n");
        exit(0);
    }

    // extract command line argument
    if (argc < 3)  {
        printf("Missing argument. Exiting program\n");
        delay(2000);
        exit(0);
    }

    cmd_line(argc, argv);

    // start threads
    t = 0;
    rc = pthread_create(&thread[t], NULL, &wave_generator, NULL);
    if (rc) {
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }

    t++;
    rc = pthread_create(&thread[t], NULL, &output_config, NULL);
    if (rc) {
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }

    t++;
    rc = pthread_create(&thread[t], NULL, &input_server, NULL);
    if (rc) {
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }

    t++;
    rc = pthread_create(&thread[t], NULL, &board_input, NULL);
    if (rc) {
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }

    pthread_exit(NULL);     // wait all threads complete before terminating main

    return EXIT_SUCCESS;
}
