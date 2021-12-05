#include "keyboard.h"

// C - PA0
// C#/Db - PA1
// D - PA2
// D#/Eb - PA6
// E - PA8
// F - PA9
// F#/Gb - PA10 
// G - PA11
// G#/Ab - PA12
// A - PB6
// A#/Bb - PC12 
// B - PC11

//Octave up - PC10
//Octave down - PC9

//configured for pull-down

void init_buttons()
{
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER &= ~(0x3FF303F);
    GPIOA->PUPDR |= 0x2AA202A;
    GPIOA->PUPDR &= ~(0x1551015);
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB->MODER &= ~(0x3000);
    GPIOB->PUPDR |= 0x2000;
    GPIOB->PUPDR &= ~(0x1000);
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC->MODER &= ~(0x3FC0000);
    GPIOC->PUPDR |= 0x2A80000;
    GPIOC->PUPDR &= ~(0x1540000);
}