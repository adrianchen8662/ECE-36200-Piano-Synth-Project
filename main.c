#include "stm32f0xx.h"
#include <stddef.h>
#include "math.h"

#define N_metronome 1000
#define RATE_metronome 30000

#define N_note 1000
#define RATE_note 100000

#define N 1000
#define RATE 20000

#define Max_Track_Size 196

int pressed[24] = {0};
// TODO: calculate a time that a note is played from offset

// TODO: figure out how to get volume to ADSR

void init_tim1(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    GPIOA->MODER |= 0xAA0000;

    GPIOA->AFR[1] &= ~(0xf << (4*(11-8)));
    GPIOA->AFR[1] |= 0x2 << (4*(11-8));
    GPIOA->AFR[1] &= ~(0xf << (4*(10-8)));
    GPIOA->AFR[1] |= 0x2 << (4*(10-8));
    GPIOA->AFR[1] &= ~(0xf << (4*(9-8)));
    GPIOA->AFR[1] |= 0x2 << (4*(9-8));
    GPIOA->AFR[1] &= ~(0xf << (4*(8-8)));
    GPIOA->AFR[1] |= 0x2 << (4*(8-8));

    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

    TIM1->BDTR |= TIM_BDTR_MOE;

    TIM1->PSC = 0;
    TIM1->ARR = 2399;

    TIM1->CCMR1 |= 0x6060;
    TIM1->CCMR2 |= 0x6860;
    TIM1->CCER |= TIM_CCER_CC1E;
    TIM1->CCER |= TIM_CCER_CC2E;
    TIM1->CCER |= TIM_CCER_CC3E;
    TIM1->CCER |= TIM_CCER_CC4E;
    TIM1->CR1 |= TIM_CR1_CEN;
}

//===========================================================================
// 2.3 PWM Sine Wave Synthesis
//===========================================================================
short int wavetable[N];
#define stepN 24
int step[stepN] = {440};
int volume = 2048;
float notes[24] = {
        420.00, // C 3 - tuned2
        420.18, // C#/Db - tuned
        470.88, // D - tuned2
        499.13, // D#/Eb - tuned
        520.25, // E - tuned2
        557.25, // F - tuned2
        559.99, // F#/Gb - tuned
        642.25, // G - tuned2
        624.3,  // G#/Ab - tuned
        719.99,  // A - tuned2
        730.16, // A#/Bb - tuned
        810.33, // B - tuned2
        850.0, // C 4 - tuned2
        870, // C# - tuned
        1002.33, // D - tuned2
        1004, // D# - tuned
        1080, // E - tuned2
        1150, // F - tuned2
        1155, // F# - tuned
        1280, // G - tuned2
        1300, // G# - tuned
        1470,  // A  - tuned2
        1490, //A# - tuned
        1600  // B - tuned2
};
int offset[stepN] = {0};
void init_wavetable(void)
{
    for(int i = 0; i < N; i++)
    {
        wavetable[i] = 32767 * sin(2 * M_PI * i / N);
    }
}

void set_freqNote(int i, int j)
{
    step[i] = notes[j] * N / RATE * (1 << 16);
}


void init_tim7(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
    int rates = 48000000 / RATE / 48;
    TIM7->PSC = 48 - 1;
    TIM7->ARR = rates - 1;
    TIM7->CR1 |= TIM_CR1_CEN;
    NVIC->ISER[0] |= 1 << TIM7_IRQn;
    TIM7->DIER |= TIM_DIER_UIE;
}

void TIM7_IRQHandler(void)
{
    TIM7->SR &= ~(TIM_SR_UIF);

    for(int i = 0; i < stepN; i++){
        offset[i] += step[i];
        if((offset[i]>>16) >= N){
            offset[i] -= N<<16;
        }
    }
    int sample = 0;
    for(int i = 0; i < stepN; i++){
        sample += wavetable[offset[i]>>16];
    }
    sample = ((sample * volume) >> 17) + 1200;
    if (sample > 4095)
        sample = 4095;
    else if (sample < 0)
        sample = 0;
    TIM1->CCR4 = sample;
}

void playAudio(int *pressed,int current)
{
    if ((GPIOB->IDR & (1 << 8)) | (current & (1<<0)))
   {
       for(int i = 0; i < stepN; i++){
           if(step[i] == 0){
               set_freqNote(i, 0);
               pressed[0] = i;
               spi_display2("       ");
               spi_display2("C4");
               break;
           }
       }
       init_tim7();
   }else{
       if(pressed[0] != -1){
           step[pressed[0]] = 0;
           pressed[0] = -1;
       }
   }
    if ((GPIOB->IDR & (1 << 9))  | (current & (1<<1)))
   {
       for(int i = 0; i < stepN; i++){
           if(step[i] == 0){
               set_freqNote(i, 1);
               pressed[1] = i;
               spi_display2("       ");
               spi_display2("C4#/D4b");
               break;
           }
       }
       init_tim7();
   }else{
       if(pressed[1] != -1){
           step[pressed[1]] = 0;
           pressed[1] = -1;
       }
   }
    if ((GPIOB->IDR & (1 << 10))  | (current & (1<<2)))
   {
        for(int i = 0; i < stepN; i++){
            if(step[i] == 0){
                set_freqNote(i, 2);
                pressed[2] = i;
                spi_display2("       ");
                spi_display2("D4");
                break;
            }
        }
        init_tim7();
   }else{
       if(pressed[2] != -1){
           step[pressed[2]] = 0;
           pressed[2] = -1;
       }
   }
   if ((GPIOB->IDR & (1 << 11))  | (current & (1<<4)))
   {
       for(int i = 0; i < stepN; i++){
           if(step[i] == 0){
               set_freqNote(i, 4);
               pressed[4] = i;
               spi_display2("       ");
               spi_display2("D4#/E4b" );
               break;
           }
       }
       init_tim7();
   }else{
       if(pressed[3] != -1){
           step[pressed[3]] = 0;
           pressed[3] = -1;
       }
   }
   if ((GPIOB->IDR & (1 << 12))  | (current & (1<<4)))
   {
       for(int i = 0; i < stepN; i++){
           if(step[i] == 0){
               set_freqNote(i, 4);
               pressed[4] = i;
               spi_display2("       ");
               spi_display2("E4");
               break;
           }
       }
       init_tim7();
   }else{
       if(pressed[4] != -1){
           step[pressed[4]] = 0;
           pressed[4] = -1;
       }
   }
   if (GPIOB->IDR & (1 << 13)   | (current & (1<<5)))
   {
       for(int i = 0; i < stepN; i++){
           if(step[i] == 0){
               set_freqNote(i, 5);
               pressed[5] = i;
                spi_display2("       ");
                       spi_display2("F4");
               break;
           }
       }
       init_tim7();
   }else{
       if(pressed[5] != -1){
           step[pressed[5]] = 0;
           pressed[5] = -1;
       }
   }
   if (GPIOB->IDR & (1 << 14)    | (current & (1<<6)))
   {
       for(int i = 0; i < stepN; i++){
           if(step[i] == 0){
               set_freqNote(i, 6);
               pressed[6] = i;
               spi_display2("       ");
               spi_display2("F4#/G4b");
               break;
           }
       }
       init_tim7();
   }else{
       if(pressed[6] != -1){
           step[pressed[6]] = 0;
           pressed[6] = -1;
       }
   }
   if (GPIOB->IDR & (1 << 15)    | (current & (1<<7)))
   {
       for(int i = 0; i < stepN; i++){
           if(step[i] == 0){
               set_freqNote(i, 7);
               pressed[7] = i;
               spi_display2("       ");
               spi_display2("G4");
               break;
           }
       }
       init_tim7();
   }else{
       if(pressed[7] != -1){
           step[pressed[7]] = 0;
           pressed[7] = -1;
       }
   }
   if (GPIOB->IDR & (1 << 7)    | (current & (1<<8)))
   {
       for(int i = 0; i < stepN; i++){
           if(step[i] == 0){
               set_freqNote(i, 8);
               pressed[8] = i;
               spi_display2("       ");
               spi_display2("G4#/A4b");
               break;
           }
       }
       init_tim7();
   }else{
       if(pressed[8] != -1){
           step[pressed[8]] = 0;
           pressed[8] = -1;
       }
   }
   if (GPIOA->IDR & (1 << 0)    | (current & (1<<9)))
   {
       for(int i = 0; i < stepN; i++){
           if(step[i] == 0){
               set_freqNote(i, 9);
               pressed[9] = i;
               spi_display2("       ");
               spi_display2("A4");
               break;
           }
       }
       init_tim7();
   }else{
       if(pressed[9] != -1){
           step[pressed[9]] = 0;
           pressed[9] = -1;
       }
   }
   if (GPIOA->IDR & (1 << 1)  | (current & (1<<10)))
   {
       for(int i = 0; i < stepN; i++){
           if(step[i] == 0){
               set_freqNote(i, 10);
               pressed[10] = i;
               spi_display2("       ");
               spi_display2("A#/Bb4");
               break;
           }
       }
       init_tim7();
   }else{
       if(pressed[10] != -1){
           step[pressed[10]] = 0;
           pressed[10] = -1;
       }
   }
   if (GPIOA->IDR & (1 << 2)  | (current & (1<<11)))
   {
       for(int i = 0; i < stepN; i++){
           if(step[i] == 0){
               set_freqNote(i, 11);
               pressed[11] = i;
               spi_display2("       ");
               spi_display2("B4");
               break;
           }
       }
       init_tim7();
   }else{
       if(pressed[11] != -1){
           step[pressed[11]] = 0;
           pressed[11] = -1;
       }
   }
   if (GPIOB->IDR & (1 << 6) | (current & (1<<12)))
   {
       for(int i = 0; i < stepN; i++){
           if(step[i] == 0){
               set_freqNote(i, 12);
               pressed[12] = i;
               spi_display2("       ");
               spi_display2("C5");
               break;
           }
       }
       init_tim7();
   }else{
       if(pressed[12] != -1){
           step[pressed[12]] = 0;
           pressed[12] = -1;
       }
   }
   if (GPIOC->IDR & (1 << 8) | (current & (1<<13)))
   {
       for(int i = 0; i < stepN; i++){
           if(step[i] == 0){
               set_freqNote(i, 13);
               pressed[13] = i;
               spi_display2("       ");
               spi_display2("C5#/D5b");
               break;
           }
       }
       init_tim7();
   }else{
       if(pressed[13] != -1){
           step[pressed[13]] = 0;
           pressed[13] = -1;
       }
   }
   if (GPIOC->IDR & (1 << 9) | (current & (1<<14)))
   {
       for(int i = 0; i < stepN; i++){
           if(step[i] == 0){
               set_freqNote(i, 14);
               pressed[14] = i;
               spi_display2("       ");
               spi_display2("D5");
               break;
           }
       }
       init_tim7();
   }else{
       if(pressed[14] != -1){
           step[pressed[14]] = 0;
           pressed[14] = -1;
       }
   }
   if (GPIOC->IDR & (1 << 10) | (current & (1<<15)))
   {
       for(int i = 0; i < stepN; i++){
           if(step[i] == 0){
               set_freqNote(i, 15);
               pressed[15] = i;
               spi_display2("       ");
               spi_display2("D5#/E5b");
               break;
           }
       }
       init_tim7();
   }else{
       if(pressed[15] != -1){
           step[pressed[15]] = 0;
           pressed[15] = -1;
       }
   }
   if (GPIOC->IDR & (1 << 11) | (current & (1<<16)))
   {
       for(int i = 0; i < stepN; i++){
           if(step[i] == 0){
               set_freqNote(i, 16);
               pressed[16] = i;
               spi_display2("       ");
               spi_display2("E5");
               break;
           }
       }
       init_tim7();
   }else{
       if(pressed[16] != -1){
           step[pressed[16]] = 0;
           pressed[16] = -1;
       }
   }
   if (GPIOC->IDR & (1 << 12) | (current & (1<<17)))
   {
       for(int i = 0; i < stepN; i++){
           if(step[i] == 0){
               set_freqNote(i, 17);
               pressed[17] = i;
               spi_display2("       ");
               spi_display2("F5");
               break;
           }
       }
       init_tim7();
   }else{
       if(pressed[17] != -1){
           step[pressed[17]] = 0;
           pressed[17] = -1;
       }
   }
   if (GPIOB->IDR & (1 << 0) | (current & (1<<518)))
   {
       for(int i = 0; i < stepN; i++){
           if(step[i] == 0){
               set_freqNote(i, 18);
               pressed[18] = i;
               spi_display2("       ");
               spi_display2("F5#/G5b");
               break;
           }
       }
       init_tim7();
   }else{
       if(pressed[18] != -1){
           step[pressed[18]] = 0;
           pressed[18] = -1;
       }
   }
   if (GPIOB->IDR & (1 << 1) | (current & (1<<19)))
   {
       for(int i = 0; i < stepN; i++){
           if(step[i] == 0){
               set_freqNote(i, 19);
               pressed[19] = i;
               spi_display2("       ");
               spi_display2("G5");
               break;
           }
       }
       init_tim7();
   }else{
       if(pressed[19] != -1){
           step[pressed[19]] = 0;
           pressed[19] = -1;
       }
   }
   if (GPIOB->IDR & (1 << 2)  | (current & (1<<20)))
   {
       for(int i = 0; i < stepN; i++){
           if(step[i] == 0){
               set_freqNote(i, 20);
               pressed[20] = i;
               spi_display2("       ");
               spi_display2("G5#/A5b");
               break;
           }
       }
       init_tim7();
   }else{
       if(pressed[20] != -1){
           step[pressed[20]] = 0;
           pressed[20] = -1;
       }
   }
   if (GPIOB->IDR & (1 << 3) | (current & (1<<21)))
   {
       for(int i = 0; i < stepN; i++){
           if(step[i] == 0){
               set_freqNote(i, 21);
               pressed[21] = i;
               spi_display2("       ");
               spi_display2("A5");
               break;
           }
       }
       init_tim7();
   }else{
       if(pressed[21] != -1){
           step[pressed[21]] = 0;
           pressed[21] = -1;
       }
   }
   if (GPIOB->IDR & (1 << 4)  | (current & (1<<22)))
   {
       for(int i = 0; i < stepN; i++){
           if(step[i] == 0){
               set_freqNote(i, 22);
               pressed[22] = i;
               spi_display2("       ");
               spi_display2("A5#/B5b");
               break;
           }
       }
       init_tim7();
   }else{
       if(pressed[22] != -1){
           step[pressed[22]] = 0;
           pressed[22] = -1;
       }
   }
   if (GPIOB->IDR & (1 << 5)  | (current & (1<<23)))
   {
       for(int i = 0; i < stepN; i++){
           if(step[i] == 0){
               set_freqNote(i, 23);
               pressed[23] = i;
               spi_display2("       ");
               spi_display2("B5");
               break;
           }
       }
       init_tim7();
   }
    else{
       if(pressed[23] != -1){
           step[pressed[23]] = 0;
           pressed[23] = -1;
       }
   }
   for(int i = 0; i < 24; i++){
       if(pressed[i] != -1){
           step[pressed[i]] = 0;
           pressed[i] = -1;
       }
   }
}

int record = 0;
struct Track
{
    uint8_t note; // value of note, C3 = 48 -> B4 = 71
    int offset; // offset to next note ,NOT FROM TIME = 0
    int time;
    int start;
};

int trackInd = 0;
int count = 0;


int wavetableMetronome[N_metronome]={0};

// sets all values to -1 to rewrite in and also give an indication of when to stop reading that track
void clearTrack(struct Track track[Max_Track_Size])
{
    int i;
    for (i = 0; i < Max_Track_Size; i++)
    {
        track[i].note = -1;
        track[i].time = -1;
        track[i].offset = -1;
        track[i].start = -1;
    }
}

// adds notes to a track
uint32_t addTrack(struct Track currentTrack[Max_Track_Size], uint32_t curr, uint32_t prev)
{

    //Find Which keys change and if that key turned on or off

    uint32_t change = prev ^ curr;
    int index = 0;
    while(index < 24){
        if(change & (1 << index)){
            //Key turned on
            if(curr & (1 << index)){
                currentTrack[trackInd].note = 48 + index;
                currentTrack[trackInd].start = count;
                trackInd++;
            }
            //Key turned off
            else{
                for(int j = trackInd; j >= 0; j--){
                    if(currentTrack[j].note == 48 + index){
                        currentTrack[j].time = count - currentTrack[j].start;
                        j = -1;
                    }
                }
            }
        }
        index++;
    }

    if(trackInd == 195){

        record = 0;
        setOffsets(currentTrack);
        spi_display1("                  ");
        spi_display1("Recording Complete");
        spi_display2("                  ");
        spi_display2("TODO: Users next steps");
        //TODO: fix spi display
    }
    prev = curr;
    return prev;
}
int idkarray = 0;
void readTrack(struct Track currentTrack[Max_Track_Size])
{
    char buffer[20];
    for(int j = 0; j < trackInd; j++){
        if(currentTrack[j].start != -1)
        {
            clear_displays();
            sprintf(buffer, "%d\n", currentTrack[j].note);
            switch (currentTrack[j].note)
            {
                            case 48:
                                spi_display2("C4");
                                break;
                            case 49:
                                spi_display2("C#/Db4");
                                break;
                            case 50:
                                spi_display2("D4");
                                break;
                            case 51:
                                spi_display2("D#/Eb4");
                                break;
                            case 52:
                                spi_display2("E4");
                                break;
                            case 53:
                                spi_display2("F4");
                                break;
                            case 54:
                                spi_display2("F#/Gb4");
                                break;
                            case 55:
                                spi_display2("G4");
                                break;
                            case 56:
                                spi_display2("G#/Ab4");
                                break;
                            case 57:
                                spi_display2("A4");
                                break;
                            case 58:
                                spi_display2("A#/Bb4");
                                break;
                            case 59:
                                spi_display2("B4");
                                break;
                            case 60:
                                spi_display2("C5");
                                break;
                            case 61:
                                spi_display2("C#/Db5");
                                break;
                            case 62:
                                spi_display2("D5");
                                break;
                            case 63:
                                spi_display2("D#/Eb5");
                                break;
                            case 64:
                                spi_display2("E5");
                                break;
                            case 65:
                                spi_display2("F5");
                                break;
                            case 66:
                                spi_display2("F#/Gb5");
                                break;
                            case 67:
                                spi_display2("G5");
                                break;
                            case 68:
                                spi_display2("A5");
                                break;
                            case 69:
                                spi_display2("A#/Bb5");
                                break;
                            case 70:
                                spi_display2("B5");
                                break;
            }


            nano_wait(2*1000000*1000);

        }
    }
}

void setOffsets(struct Track track[Max_Track_Size])
{
    // searches the track and adds in times for how long each note was pressed
    int i;
    for (i = 1; i < Max_Track_Size; i++)
    {
        track[i].offset = track[i].start - track[i-1].start;
    }
    track[0].offset = track[0].start;
}

void wavegenForMetronome(){
    for(int i = 0; i < N_metronome; i++){
        wavetableMetronome[i] = i * 65535.0 / N_metronome - 32768;
    }
}

void nano_wait(unsigned int n) {
    asm(    "        mov r0,%0\n"
            "repeat: sub r0,#83\n"
            "        bgt repeat\n" : : "r"(n) : "r0", "cc");
}

// Keypad and OLED
void enable_ports(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN; //enable GPIOA
        RCC->AHBENR |= RCC_AHBENR_GPIOBEN; //enable GPIOB
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN; //enable GPIOC

    GPIOA->MODER &= ~(0x3F); //cleared 0,1,2, all set to input
        GPIOA->PUPDR &= ~(0x3F); //0,1,2 resistor states cleared
        GPIOA->PUPDR |= (0x2A); //0,1,2 set to pulldown

    GPIOB->MODER &= ~(0xFFFFFFFF); //cleared 0-15, all set to input
        GPIOB->PUPDR &= ~(0xFFFFFFFF); //0-15 resistor states cleared
    GPIOB->PUPDR |= (0xAAAAAAAA); //0-15 set to pulldown

    GPIOC->MODER &= ~(0x3FFAAFF); //cleared 8-12, all set to to input + adrian pins
        GPIOC->MODER |= 0x5500; //adrian outputs
        GPIOC->PUPDR &= ~(0x3FF0055); //8-12 resistors states cleared + adrian inputs
        GPIOC->PUPDR |= (0x2AA00AA); //8-12 all set to pulldown + adrian inputs
}

int msg_index = 0;
uint16_t msg[8] = { 0x0000,0x0100,0x0200,0x0300,0x0400,0x0500,0x0600,0x0700 };

const char keymap[] = "DCBA#9630852*741";
uint8_t hist[16];
uint8_t col;
char queue[2];
int qin;
int qout;

void drive_column(int c)
{
    GPIOC->BSRR = 0xf00000 | (1 << (c + 4));
}

int read_rows()
{
    return GPIOC->IDR & 0xf;
}

void push_queue(int n)
{
    n = (n & 0xff) | 0x80;
    queue[qin] = n;
    qin ^= 1;
}

uint8_t pop_queue()
{
    uint8_t tmp = queue[qout] & 0x7f;
    queue[qout] = 0;
    qout ^= 1;
    return tmp;
}

void update_history(int c, int rows)
{
    for(int i = 0; i < 4; i++) {
        hist[4*c+i] = (hist[4*c+i]<<1) + ((rows>>i)&1);
        if (hist[4*c+i] == 1)
          push_queue(4*c+i);
    }
}

void init_tim6()
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
    TIM6->PSC = 48 - 1;
    TIM6->ARR = 1000 - 1;
    TIM6->DIER |= TIM_DIER_UIE;
    TIM6->CR1 |= TIM_CR1_CEN;
    NVIC->ISER[0] |= 1 << TIM6_DAC_IRQn;
}

void TIM6_DAC_IRQHandler(void)
{
    TIM6->SR &= ~TIM_SR_UIF;
    int rows = read_rows();
    update_history(col, rows);
    col = (col + 1) & 3;
    drive_column(col);
}

char get_keypress()
{
    for(;;) { // FIXME: For loop over here
        asm volatile ("wfi" : :);   // wait for an interrupt
        if (queue[qout] == 0)
            continue;
        return keymap[pop_queue()];
    }
}

void setup_spi1()
{
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER |= 0x8000a800;
    GPIOA->MODER &= ~0x40005400;
    GPIOA->AFR[1] &= ~0xf0000000;
    GPIOA->AFR[0] &= ~0xfff00000;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    SPI1->CR1 &= ~SPI_CR1_SPE;
    SPI1->CR1 |= SPI_CR1_BR | SPI_CR1_MSTR;
    SPI1->CR2 = SPI_CR2_DS_3 | SPI_CR2_DS_0;
    SPI1->CR2 |= SPI_CR2_SSOE | SPI_CR2_NSSP;
    SPI1->CR1 |= SPI_CR1_SPE;
}

void spi_cmd(unsigned int data)
{
    while((SPI1->SR & SPI_SR_TXE) == 0) ;
    SPI1->DR = data;
}

void spi_data(unsigned int data)
{
    while((SPI1->SR & SPI_SR_TXE) == 0) ;
    SPI1->DR = data | 0x200;
}

void spi_init_oled()
{
    nano_wait(1000000);
    spi_cmd(0x38);
    spi_cmd(0x08);
    spi_cmd(0x01);
    nano_wait(2000000);
    spi_cmd(0x06);
    spi_cmd(0x02);
    spi_cmd(0x0c);
}

//TODO: this isn't efficient, could combine spi_display and clear_display since a new display needs to be cleared in most cases
void spi_display1(const char *string) // display1
{
    spi_cmd(0x02);
    for(int i = 0; string[i] != '\0'; i++)
        spi_data(string[i]);
}

void spi_display2(const char *string) // display2
{
    spi_cmd(0xc0);
    for(int i = 0; string[i] != '\0'; i++)
        spi_data(string[i]);
}

void clear_displays()
{
    spi_display1("                ");
    spi_display2("                ");
}

void clear_display1()
{
    spi_display1("                ");
}

void clear_display2()
{
    spi_display2("                ");
}

void metronome(int tempo, int timeSig){
    wavegenForMetronome();
    int numInMilli = 1000 / (tempo / 60);
    int x = (261.626 * N_metronome / RATE_metronome * (1<<16));
    int stepx = wavetableMetronome[x>>16];
    stepx = stepx / 32 + 2048;

    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER|= 3<<(2*4);

    RCC->APB1ENR |= RCC_APB1ENR_DACEN;
    DAC->CR &= ~DAC_CR_EN1;
    DAC->CR &= ~DAC_CR_BOFF1;
    DAC->CR |= DAC_CR_TEN1;
    DAC->CR |= DAC_CR_TSEL1;
    DAC->CR |= DAC_CR_EN1;

    for(int i = 0; i < timeSig * 2; i++){
        while((DAC->SWTRIGR & DAC_SWTRIGR_SWTRIG1) == DAC_SWTRIGR_SWTRIG1);
        DAC->DHR12R1 = 0;
        DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1;
        nano_wait((numInMilli - 10)*1000000);
        DAC->DHR12R1 = stepx;
        if(i%timeSig){
            DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1;
            nano_wait(10000000);
        }else{
            DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1;
            nano_wait(30000000);
        }


    }
    DAC->DHR12R1 = 0;
    DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1;
}


void ADSRGenerator(int As, int Aa, int Ds, int Sa, int Ss, int Rs, uint8_t* volume)
{
    int adsrTicks = 1000;
    //Attack
    int aTicks = adsrTicks * As / 100; //Number of Ticks in Attack
    float aInc = (float)Aa / aTicks;            //Amplitude Increment per tick
    //Decay
    int dTicks = adsrTicks * Ds / 100; //Number of Ticks for Delay
    float dDec = (float)(Sa - Aa) / dTicks;    //Amplitude Decrease per tick
    //Sustain
    int sTicks = adsrTicks * Ss / 100; //Number of Ticks for Sustain
    //Release
    int rTicks = adsrTicks * (float)Rs / 100; //Number of Ticks for Release
    float rDec = (float)(0 - Sa) / rTicks;    //Amplitude Decrease per tick

    // printf("aTicks: %d\n",aTicks);
    // printf("aInc: %lf\n",aInc);
    // printf("dTicks: %d\n",dTicks);
    // printf("dDec: %lf\n",dDec);
    // printf("sTicks: %d\n",sTicks);
    // printf("rTicks: %d\n",rTicks);
    // printf("rDec: %lf\n",rDec);

    for(int i = 0; i < aTicks + dTicks + sTicks + rTicks; i++){
        if(i <= aTicks){
            volume[i] = (aInc * i) * 100 / Aa;
            //printf("A volume %d: %d\n",i,volume[i]);
        }
        else if(i > aTicks && i <= aTicks + dTicks){
            volume[i] = (Aa + dDec * (i - aTicks)) * 100 / Aa;
            //printf("D volume %d: %d\n",i,volume[i]);
        }
        else if(i > aTicks + dTicks && i <= aTicks + dTicks + sTicks){
            volume[i] = (Sa) * 100 / Aa;
            //printf("S volume %d: %d\n",i,volume[i]);
        }
        else if(i > aTicks + dTicks + sTicks){
            volume[i] = (Sa + rDec * (i - (sTicks + dTicks + aTicks))) * 100 / Aa;
            //printf("R volume %d: %d\n",i,volume[i]);
        }
    }
}
void init_tim2(int tempo){

    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->CR1 &= ~TIM_CR1_CEN;

    TIM2->PSC = 48-1;
    TIM2->ARR = 1000000/((tempo /60.0)*24) - 1;

    TIM2->DIER |= TIM_DIER_UIE;
    TIM2->CR1 |= TIM_CR1_CEN;

    NVIC->ISER[0] |= 1 << TIM2_IRQn;
}

struct Track* currentTrack = NULL;

uint32_t prev = 0;
uint32_t curr = 0;


void TIM2_IRQHandler(void) {
    TIM2->SR &= ~TIM_SR_UIF;

    curr = 0;

    if (GPIOB->IDR & (1 << 8))
    {
        //spi_display2("       ");
        //spi_display2("C3");
        curr |= 1 << 0;
    }
    if (GPIOB->IDR & (1 << 9))
    {
        //spi_display2("       ");
        //spi_display2("C3#/D3b");
        curr |= 1 << 1;
    }
    if (GPIOB->IDR & (1 << 10))
    {
        //spi_display2("       ");
        //spi_display2("D3");
        curr |= 1 << 2;
    }
    if (GPIOB->IDR & (1 << 11))
    {
        //spi_display2("       ");
        //spi_display2("D3#/E3b" );
        curr |= 1 << 3;
    }
    if (GPIOB->IDR & (1 << 12))
    {
        //spi_display2("       ");
        //spi_display2("E3");
        curr |= 1 << 4;
    }
    if (GPIOB->IDR & (1 << 13))
    {
       // spi_display2("       ");
        //spi_display2("F3");
        curr |= 1 << 5;
    }
    if (GPIOB->IDR & (1 << 14))
    {
       // spi_display2("       ");
        //spi_display2("F3#/G3b");
        curr |= 1 << 6;
    }
    if (GPIOB->IDR & (1 << 15))
    {
        //spi_display2("       ");
        //spi_display2("G3");
        curr |= 1 << 7;
    }
    if (GPIOB->IDR & (1 << 7))
    {
        //spi_display2("       ");
        //spi_display2("G3#/A3b");
        curr |= 1 << 8;
    }
    if (GPIOA->IDR & (1 << 0))
    {
        //spi_display2("       ");
        //spi_display2("A3");
        curr |= 1 << 9;
    }
    if (GPIOA->IDR & (1 << 1))
    {
        //spi_display2("       ");
        //spi_display2("A#/Bb");
        curr |= 1 << 10;
    }
    if (GPIOA->IDR & (1 << 2))
    {
        //spi_display2("       ");
        //spi_display2("B3");
        curr |= 1 << 11;
    }
    if (GPIOB->IDR & (1 << 6))
    {
        //spi_display2("       ");
        //spi_display2("C4");
        curr |= 1 << 12;
    }
    if (GPIOC->IDR & (1 << 8))
    {
        //spi_display2("       ");
        //spi_display2("C4#/D4b");
        curr |= 1 << 13;
    }
    if (GPIOC->IDR & (1 << 9))
    {
        //spi_display2("       ");
        //spi_display2("D4");
        curr |= 1 << 14;
    }
    if (GPIOC->IDR & (1 << 10))
    {
        //spi_display2("       ");
        //spi_display2("D4#/E4b");
        curr |= 1 << 15;
    }
    if (GPIOC->IDR & (1 << 11))
    {
        //spi_display2("       ");
        //spi_display2("E4");
        curr |= 1 << 16;
    }
    if (GPIOC->IDR & (1 << 12))
    {
        //spi_display2("       ");
        //spi_display2("F4");
        curr |= 1 << 17;
    }
    if (GPIOB->IDR & (1 << 0))
    {
        //spi_display2("       ");
        //spi_display2("F4#/G4b");
        curr |= 1 << 18;
    }
    if (GPIOB->IDR & (1 << 1))
    {
        //spi_display2("       ");
        //spi_display2("G4");
        curr |= 1 << 19;
    }
    if (GPIOB->IDR & (1 << 2))
    {
        //spi_display2("       ");
        //spi_display2("G4#/A4b");
        curr |= 1 << 20;
    }
    if (GPIOB->IDR & (1 << 3))
    {
        ///spi_display2("       ");
        //spi_display2("A4");
        curr |= 1 << 21;
    }
    if (GPIOB->IDR & (1 << 4))
    {
        //spi_display2("       ");
        //spi_display2("A4#/B4b");
        curr |= 1 << 22;
    }
    if (GPIOB->IDR & (1 << 5))
    {
        //spi_display2("       ");
        //spi_display2("B4");
        curr |= 1 << 23;
    }
    count++;
    if (currentTrack != NULL && record == 1)
        prev = addTrack(currentTrack,curr,prev);
}

void waitMetronome(int tempo, int timeSig)
{
    int numInMilli = 1000 / (tempo / 60);

    for(int i = 0; i < timeSig * 2; i++){
        nano_wait((numInMilli - 10)*1000000);
    }

}
struct Track track1[Max_Track_Size];

int main(void)
{
    // metronome
    int tempo = 240;
    int temptempo = 0;
    int time = 0;
    char buffer[10] = "          ";

    int track = 0;

    // ADSR curve variables
    int Aa = 0;
    int As = 0;
    int Ds = 0;
    int Sa = 0;
    int Ss = 0;
    int Rs = 0;

    // keypad enable
    enable_ports();

    // setup keypad
    init_tim6();


    // setup note taking
    //init_tim2(240);

    // SPI OLED
    setup_spi1();
    spi_init_oled();
    spi_display1("Loading");

    // reset DAC to zero
    metronome(200,2);

    // load in track structs

    struct Track track2[Max_Track_Size];
    struct Track track3[Max_Track_Size];
    struct Track track4[Max_Track_Size];

    // prepares the track to be cleared
    clearTrack(track1);
    clearTrack(track2);
    clearTrack(track3);
    clearTrack(track4);

    // create ADSR volumes
    /*uint8_t* volume1 = malloc(sizeof(uint8_t) * tempo * 24); // 240 * 24 * 1 = 5760
    uint8_t* volume2 = malloc(sizeof(uint8_t) * tempo * 24);
    uint8_t* volume3 = malloc(sizeof(uint8_t) * tempo * 24);
    uint8_t* volume4 = malloc(sizeof(uint8_t) * tempo * 24);
    */


    spi_display1("Synth Online"); // display1
    spi_display2("Armed and Ready"); // display2


    init_wavetable();
    init_tim1();

/*
   */
    char key = 'N';

    for (;;)
    {
        key = get_keypress();
        if (key == '0') // Freeplay
                       {
                           clear_displays();
                           spi_display1("Free-Playing");
                           metronome(tempo, time); // enable metronome here
                           while(1)
                           {
                               playAudio(pressed,curr);
                           }
                           nano_wait(10000000000);
                           clear_displays();
                           spi_display1("Synth Online");
                           spi_display2("Armed and Ready");
                       }
        if (key == 'A') // Metronome
        {
            key = 'N';

            clear_displays();
            spi_display1("Metronome");
            spi_display2("A to enter");
            temptempo = 0;
            while (key != 'A') // Metronome
            {
                key = get_keypress();
                switch (key)
                {
                    case 'C':
                        clear_display1();
                        spi_display1("Cancelled tempo");
                    case '1':
                        if (temptempo == 0)
                        {
                            temptempo = 1;
                        }
                        else
                        {
                            temptempo *= 10;
                            temptempo += 1;
                        }
                        break;
                    case '2':
                        if (temptempo == 0)
                        {
                            temptempo = 2;
                        }
                        else
                        {
                            temptempo *= 10;
                            temptempo += 2;
                        }
                        break;
                    case '3':
                        if (temptempo == 0)
                        {
                            temptempo = 3;
                        }
                        else
                        {
                            temptempo *= 10;
                            temptempo += 3;
                        }
                        break;
                    case '4':
                        if (temptempo == 0)
                        {
                            temptempo = 4;
                        }
                        else
                        {
                            temptempo *= 10;
                            temptempo += 4;
                        }
                        break;
                    case '5':
                        if (temptempo == 0)
                        {
                            temptempo = 5;
                        }
                        else
                        {
                            temptempo *= 10;
                            temptempo += 5;
                        }
                        break;
                    case '6':
                        if (temptempo == 0)
                        {
                            temptempo = 6;
                        }
                        else
                        {
                            temptempo *= 10;
                            temptempo += 6;
                        }
                        break;
                    case '7':
                        if (temptempo == 0)
                        {
                            temptempo = 7;
                        }
                        else
                        {
                            temptempo *= 10;
                            temptempo += 7;
                        }
                        break;
                    case '8':
                        if (temptempo == 0)
                        {
                            temptempo = 8;
                        }
                        else
                        {
                            temptempo *= 10;
                            temptempo += 8;
                        }
                        break;
                    case '9':
                        if (temptempo == 0)
                        {
                            temptempo = 9;
                        }
                        else
                        {
                            temptempo *= 10;
                            temptempo += 9;
                        }
                        break;
                    case '0':
                        if (temptempo == 0)
                        {
                            temptempo = 0;
                        }
                        else
                        {
                            temptempo *= 10;
                            temptempo += 0;
                        }
                        break;
                }
                if (temptempo != 0)
                {
                    clear_display2();
                    sprintf(buffer,"%d",temptempo);
                    spi_display2(buffer);
                }
            }
            if (temptempo != 0)
            {
                tempo = temptempo;
                init_tim2(tempo);
                clear_displays();
                spi_display1("Tempo set");
                spi_display2(buffer);
                // TODO: reinitalize tim2 with new tempo

                nano_wait(10000000000);
                clear_displays();
                spi_display1("Time Signature");
                spi_display2("_/4");
                key = get_keypress();
                switch (key)
                {
                    case ('2'):
                        time = 2;
                        clear_displays();
                        spi_display1("Time set");
                        sprintf(buffer,"%d/4",time);
                        spi_display2(buffer);
                        break;
                    case ('3'):
                        time = 3;
                        clear_displays();
                        spi_display1("Time set");
                        sprintf(buffer,"%d/4",time);
                        spi_display2(buffer);
                        break;
                    case ('4'):
                        time = 4;
                        clear_displays();
                        spi_display1("Time set");
                        sprintf(buffer,"%d/4",time);
                        spi_display2(buffer);
                        break;
                    default:
                        clear_displays();
                        spi_display1("Invalid time");
                }
            }
            nano_wait(10000000000);
            clear_displays();
            spi_display1("Synth Online");
            spi_display2("Armed and Ready");
        }
        if (key == 'D') // Recording
        {
            clear_displays();
            spi_display1("Recording");
            spi_display2("Select track");
            key = get_keypress();
            switch(key)
            {
                case ('C'):
                    clear_displays();
                    spi_display1("Cancel record");
                    track = 0;
                    break;
                case ('1'):
                    clear_displays();
                    spi_display1("Track 1");
                    currentTrack = track1;
                    track = 1;
                    break;
                case ('2'):
                    clear_display2();
                    spi_display2("Track 2");
                    currentTrack = track2;
                    track = 2;
                    break;
                case ('3'):
                    clear_display2();
                    spi_display2("Track 3");
                    currentTrack = track3;
                    track = 3;
                    break;
                case ('4'):
                    clear_display2();
                    spi_display2("Track 4");
                    currentTrack = track4;
                    track = 4;
                    break;

                default:
                    clear_display2();
                    spi_display2("Invalid Track");
                    track = 0;
                    break;
                }
            /*
            if (track != 0)
            {
                nano_wait(10000000000);
                clear_displays();
                spi_display1("Enter ADSR curve");
                spi_display2("Aa: ");
                key = get_keypress();
                switch (key)
                {
                    case '0':
                        Aa = 0;
                        break;
                    case '1':
                        Aa = 1;
                        break;
                    case '2':
                        Aa = 2;
                        break;
                    case '3':
                        Aa = 3;
                        break;
                    case '4':
                        Aa = 4;
                        break;
                    case '5':
                        Aa = 5;
                        break;
                    case '6':
                        Aa = 6;
                        break;
                    case '7':
                        Aa = 7;
                        break;
                    case '8':
                        Aa = 8;
                        break;
                    case '9':
                        Aa = 9;
                        break;
                }
                clear_display2();
                sprintf(buffer, "Aa: %d",Aa);
                spi_display2(buffer);
                key = get_keypress();
                switch (key)
                {
                    case '0':
                        Aa *= 10;
                        Aa += 0;
                        break;
                    case '1':
                        Aa *= 10;
                        Aa += 1;
                        break;
                    case '2':
                        Aa *= 10;
                        Aa += 2;
                        break;
                    case '3':
                        Aa *= 10;
                        Aa += 3;
                        break;
                    case '4':
                        Aa *= 10;
                        Aa += 4;
                        break;
                    case '5':
                        Aa *= 10;
                        Aa += 5;
                        break;
                    case '6':
                        Aa *= 10;
                        Aa += 6;
                        break;
                    case '7':
                        Aa *= 10;
                        Aa += 7;
                        break;
                    case '8':
                        Aa *= 10;
                        Aa += 8;
                        break;
                    case '9':
                        Aa *= 10;
                        Aa += 9;
                        break;
                }
                clear_display2();
                sprintf(buffer, "Aa: %d",Aa);
                spi_display2(buffer);
                nano_wait(10000000000);
                clear_display2();
                spi_display2("As: ");
                key = get_keypress();
                switch (key)
                {
                    case '0':
                        As = 0;
                        break;
                    case '1':
                        As = 1;
                        break;
                    case '2':
                        As = 2;
                        break;
                    case '3':
                        As = 3;
                        break;
                    case '4':
                        As = 4;
                        break;
                    case '5':
                        As = 5;
                        break;
                    case '6':
                        As = 6;
                        break;
                    case '7':
                        As = 7;
                        break;
                    case '8':
                        As = 8;
                        break;
                    case '9':
                        As = 9;
                        break;
                }
                clear_display2();
                sprintf(buffer, "As: %d",As);
                spi_display2(buffer);
                key = get_keypress();
                switch (key)
                {
                    case '0':
                        As *= 10;
                        As += 0;
                        break;
                    case '1':
                        As *= 10;
                        As += 1;
                        break;
                    case '2':
                        As *= 10;
                        As += 2;
                        break;
                    case '3':
                        As *= 10;
                        As += 3;
                        break;
                    case '4':
                        As *= 10;
                        As += 4;
                        break;
                    case '5':
                        As *= 10;
                        As += 5;
                        break;
                    case '6':
                        As *= 10;
                        As += 6;
                        break;
                    case '7':
                        As *= 10;
                        As += 7;
                        break;
                    case '8':
                        As *= 10;
                        As += 8;
                        break;
                    case '9':
                        As *= 10;
                        As += 9;
                        break;
                }
                clear_display2();
                sprintf(buffer, "As: %d",As);
                spi_display2(buffer);
                nano_wait(10000000000);
                clear_display2();
                spi_display2("Ds: ");
                key = get_keypress();
                switch (key)
                {
                    case '0':
                        Ds = 0;
                        break;
                    case '1':
                        Ds = 1;
                        break;
                    case '2':
                        Ds = 2;
                        break;
                    case '3':
                        Ds = 3;
                        break;
                    case '4':
                        Ds = 4;
                        break;
                    case '5':
                        Ds = 5;
                        break;
                    case '6':
                        Ds = 6;
                        break;
                    case '7':
                        Ds = 7;
                        break;
                    case '8':
                        Ds = 8;
                        break;
                    case '9':
                        Ds = 9;
                        break;
                }
                clear_display2();
                sprintf(buffer, "Ds: %d",Ds);
                spi_display2(buffer);
                key = get_keypress();
                switch (key)
                {
                    case '0':
                        Ds *= 10;
                        Ds += 0;
                        break;
                    case '1':
                        Ds *= 10;
                        Ds += 1;
                        break;
                    case '2':
                        Ds *= 10;
                        Ds += 2;
                        break;
                    case '3':
                        Ds *= 10;
                        Ds += 3;
                        break;
                    case '4':
                        Ds *= 10;
                        Ds += 4;
                        break;
                    case '5':
                        Ds *= 10;
                        Ds += 5;
                        break;
                    case '6':
                        Ds *= 10;
                        Ds += 6;
                        break;
                    case '7':
                        Ds *= 10;
                        Ds += 7;
                        break;
                    case '8':
                        Ds *= 10;
                        Ds += 8;
                        break;
                    case '9':
                        Ds *= 10;
                        Ds += 9;
                        break;
                }
                clear_display2();
                sprintf(buffer, "Ds: %d",Ds);
                spi_display2(buffer);
                nano_wait(10000000000);
                clear_display2();
                spi_display2("Sa: ");
                key = get_keypress();
                switch (key)
                {
                    case '0':
                        Sa = 0;
                        break;
                    case '1':
                        Sa = 1;
                        break;
                    case '2':
                        Sa = 2;
                        break;
                    case '3':
                        Sa = 3;
                        break;
                    case '4':
                        Sa = 4;
                        break;
                    case '5':
                        Sa = 5;
                        break;
                    case '6':
                        Sa = 6;
                        break;
                    case '7':
                        Sa = 7;
                        break;
                    case '8':
                        Sa = 8;
                        break;
                    case '9':
                        Sa = 9;
                        break;
                }
                clear_display2();
                sprintf(buffer, "Sa: %d",Sa);
                spi_display2(buffer);
                key = get_keypress();
                switch (key)
                {
                    case '0':
                        Sa *= 10;
                        Sa += 0;
                        break;
                    case '1':
                        Sa *= 10;
                        Sa += 1;
                        break;
                    case '2':
                        Sa *= 10;
                        Sa += 2;
                        break;
                    case '3':
                        Sa *= 10;
                        Sa += 3;
                        break;
                    case '4':
                        Sa *= 10;
                        Sa += 4;
                        break;
                    case '5':
                        Sa *= 10;
                        Sa += 5;
                        break;
                    case '6':
                        Sa *= 10;
                        Sa += 6;
                        break;
                    case '7':
                        Sa *= 10;
                        Sa += 7;
                        break;
                    case '8':
                        Sa *= 10;
                        Sa += 8;
                        break;
                    case '9':
                        Sa *= 10;
                        Sa += 9;
                        break;
                }
                clear_display2();
                sprintf(buffer, "Sa: %d",Sa);
                spi_display2(buffer);
                nano_wait(10000000000);
                clear_display2();
                spi_display2("Ss: ");
                key = get_keypress();
                switch (key)
                {
                    case '0':
                        Ss = 0;
                        break;
                    case '1':
                        Ss = 1;
                        break;
                    case '2':
                        Ss = 2;
                        break;
                    case '3':
                        Ss = 3;
                        break;
                    case '4':
                        Ss = 4;
                        break;
                    case '5':
                        Ss = 5;
                        break;
                    case '6':
                        Ss = 6;
                        break;
                    case '7':
                        Ss = 7;
                        break;
                    case '8':
                        Ss = 8;
                        break;
                    case '9':
                        Ss = 9;
                        break;
                }
                clear_display2();
                sprintf(buffer, "Ss: %d",Ss);
                spi_display2(buffer);
                key = get_keypress();
                switch (key)
                {
                    case '0':
                        Ss *= 10;
                        Ss += 0;
                        break;
                    case '1':
                        Ss *= 10;
                        Ss += 1;
                        break;
                    case '2':
                        Ss *= 10;
                        Ss += 2;
                        break;
                    case '3':
                        Ss *= 10;
                        Ss += 3;
                        break;
                    case '4':
                        Ss *= 10;
                        Ss += 4;
                        break;
                    case '5':
                        Ss *= 10;
                        Ss += 5;
                        break;
                    case '6':
                        Ss *= 10;
                        Ss += 6;
                        break;
                    case '7':
                        Ss *= 10;
                        Ss += 7;
                        break;
                    case '8':
                        Ss *= 10;
                        Ss += 8;
                        break;
                    case '9':
                        Ss *= 10;
                        Ss += 9;
                        break;
                }
                clear_display2();
                sprintf(buffer, "Ss: %d",Ss);
                spi_display2(buffer);
                nano_wait(10000000000);
                clear_display2();
                spi_display2("Rs: ");
                key = get_keypress();
                switch (key)
                {
                    case '0':
                        Rs = 0;
                        break;
                    case '1':
                        Rs = 1;
                        break;
                    case '2':
                        Rs = 2;
                        break;
                    case '3':
                        Rs = 3;
                        break;
                    case '4':
                        Rs = 4;
                        break;
                    case '5':
                        Rs = 5;
                        break;
                    case '6':
                        Rs = 6;
                        break;
                    case '7':
                        Rs = 7;
                        break;
                    case '8':
                        Rs = 8;
                        break;
                    case '9':
                        Rs = 9;
                        break;
                }
                clear_display2();
                sprintf(buffer, "Rs: %d",Rs);
                spi_display2(buffer);
                key = get_keypress();
                switch (key)
                {
                    case '0':
                        Rs *= 10;
                        Rs += 0;
                        break;
                    case '1':
                        Rs *= 10;
                        Rs += 1;
                        break;
                    case '2':
                        Rs *= 10;
                        Rs += 2;
                        break;
                    case '3':
                        Rs *= 10;
                        Rs += 3;
                        break;
                    case '4':
                        Rs *= 10;
                        Rs += 4;
                        break;
                    case '5':
                        Rs *= 10;
                        Rs += 5;
                        break;
                    case '6':
                        Rs *= 10;
                        Rs += 6;
                        break;
                    case '7':
                        Rs *= 10;
                        Rs += 7;
                        break;
                    case '8':
                        Rs *= 10;
                        Rs += 8;
                        break;
                    case '9':
                        Rs *= 10;
                        Rs += 9;
                        break;
                }
            clear_display2();
            sprintf(buffer, "Rs: %d",Rs);
            spi_display2(buffer);
            }*/

            nano_wait(10000000000);
            clear_displays();
            spi_display1("Ready to record");
            spi_display2("Prepare to play");
            // TODO: generate amptable here
            metronome(tempo, time); // enable metronome here

            waitMetronome(tempo,time);
            record = 1;
            init_tim2(tempo);


            clear_displays();
            spi_display1("Recording");
            spi_display2("Play Notes");

        }
        if (key == '#') // Deleting
        {
            clear_displays();
            spi_display1("Deleting");
            spi_display2("Select Track");
            key = get_keypress();
            switch (key)
            {
                case('C'):
                    clear_displays();
                    spi_display1("Cancelled delete");
                    break;
                case ('1'):
                    clear_display2();
                    spi_display2("Track 1");
                    clearTrack(track1);
                    break;
                case ('2'):
                    clear_display2();
                    spi_display2("Track 2");
                    clearTrack(track2);
                    break;
                case ('3'):
                    clear_display2();
                    spi_display2("Track 3");
                    clearTrack(track3);
                    break;
                case ('4'):
                    clear_display2();
                    spi_display2("Track 4");
                    clearTrack(track4);
                    break;

                default:
                    clear_display2();
                    spi_display2("Invalid track");
                    break;
            }
            nano_wait(10000000000);
            clear_displays();
            spi_display1("Synth Online");
            spi_display2("Armed and Ready");
        }
        if (key == 'B') // Stopping
        {
            setOffsets(currentTrack);
            clear_displays();
            spi_display1("Stopping");
            //spi_display2("Select Track");
            nano_wait(10000000000);
            clear_displays();
            spi_display1("Synth Online");
            spi_display2("Armed and Ready");
        }
        if (key == '*') // Playing
        {
            clear_displays();
            spi_display1("Playing");
            spi_display2("Select Track");
            key = get_keypress();
            switch (key)
            {
                case('C'):
                    clear_displays();
                    spi_display1("Cancelled play");
                    break;
                case ('1'):
                    clear_display2();
                    readTrack(track1);
                    spi_display2("Track 1");
                    break;
                case ('2'):
                    clear_display2();
                    readTrack(track2);
                    spi_display2("Track 2");
                    break;
                case ('3'):
                    clear_display2();
                    readTrack(track3);
                    spi_display2("Track 3");
                    break;
                case ('4'):
                    clear_display2();
                    readTrack(track4);
                    spi_display2("Track 4");
                    break;


                default:
                    clear_display2();
                    spi_display2("Invalid track");
                    break;
            }
            nano_wait(10000000000);
            clear_displays();
            spi_display1("Synth Online");
            spi_display2("Armed and Ready");
        }
    }
        if (key == 'B') // Stopping
        {
            clear_displays();
            spi_display1("Stopping");
            record = 0;
            trackInd = 196;
            key = get_keypress();

            switch (key)
            {
                case('C'):
                    clear_displays();
                    spi_display1("Cancelled stop");
                    break;
                case ('1'):
                    clear_display2();

                    spi_display2("Track 1");
                    break;
                case ('2'):
                    clear_display2();

                    spi_display2("Track 2");
                    break;
                case ('3'):
                    clear_display2();

                    spi_display2("Track 3");
                    break;
                case ('4'):
                    clear_display2();

                    spi_display2("Track 4");
                    break;


                default:
                    clear_display2();
                    spi_display2("Invalid track");

            }
            nano_wait(10000000000);
            clear_displays();
            spi_display1("Synth Online");
            spi_display2("Armed and Ready");
        }

    }
    /*
    free(volume1);
    free(volume2);
    free(volume3);
    free(volume4);
*/

