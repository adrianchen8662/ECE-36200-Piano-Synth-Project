#include "potentiometer.h"

// Potentiometer 1 - PB0
// Potentiometer 2 - PB1
// Potentiometer 3 - PB6
// Potentiometer 4 - PB7
// Potentiometer 5 - PB8
// Potentiometer 6 - PB9

//configured for pull-down

void init_potentiometer()
{
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    RCC->MODER &= ~(0xFF00F);
    RCC->PUPDR |= 0xAA00A;
    RCC->PUPDR &= ~(0x55005);
}