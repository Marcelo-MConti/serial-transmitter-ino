#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#include <avr/interrupt.h>
#include <avr/io.h>

// Prescaler set to 1:
// #define PRESCALER_LOG2 0
// #define PRESCALER 1
// #define PRESCALER_BITS (1 << CS10)

// Prescaler set to 256:
#define PRESCALER_LOG2 8
#define PRESCALER (1 << PRESCALER_LOG2)
#define PRESCALER_BITS (1 << CS12)

// Prescaler set to 1024:
// #define PRESCALER_LOG2 10
// #define PRESCALER 1024
// #define PRESCALER_BITS ((1 << CS12) | (1 << CS10))

static void timer_setup(uint16_t freq) {
    cli();

    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | PRESCALER_BITS;

    // f_wav (waveform freq) = F_CPU / (2 * N_prescaler * OCR1A) =>
    // f_int (interrupt freq) = F_CPU / (N_prescaler * OCR1A) =>
    // OCR1A = F_CPU / (N_prescaler * freq)
    OCR1A = (F_CPU >> PRESCALER_LOG2) / freq - 1;

    sei();
}

static void timer_init(void) {
    TIMSK1 = (1 << OCIE1A);
}

static void timer_stop(void) {
    TIMSK1 = 0;
}

#endif