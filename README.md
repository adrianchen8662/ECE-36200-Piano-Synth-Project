# ECE-36200-Piano-Synth-Project

We built a synthesizer and mixer. There were 24 buttons that played notes from C4 to B5, and could play multiple notes at once. There was also a recording functionality that allowed you to input information on an ADSR envelope, and replay at will. <br>

The components we used:
 - 1 STM32F091RCT6 microcontroller
 - 24 tactile switches
 - 1 SOC1602A OLED LCD Display 
 - 1 LM324 Quad Op-Amp
 - 1 10k Ohm resistor
 - 1 TRRS adapter
 - 1 10uF capacitor
 - 1 4x4 keypad matrix

We faced a lot of challenges, especially in audio playback and hardware 
limitations. The button couldn't be multiplexed for detecting multiple button 
presses at the same time, so 24 pins had to be dedicated to just the keyboard. 
The DAC output wouldn't work for polyphonic sound generation, so we used the 
PWM instead. 
We also ran into limitations in memory. 32KB was barely enough to hold a simple 
ADSR amplitude wavetable and the recorded tracks. We worked around this by 
finding the lowest and highest value each variable needed and limited the 
memory assigned using uint8_t and uint16_t, instead of the full 4 bytes in int. 
 
For future teams, We would recommend to use the SD card to store information or 
to find a microcontroller with more memory. A faster microcontroller would also 
make multithreading an option to run the various features faster and in 
parallel instead of relying on interrupts. Instead of using tactile buttons, a 
separate PCB with actual switches would be better, along with using wire 
headers to keep things organized. 
