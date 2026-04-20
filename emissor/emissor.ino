#define CLOCK 10 // Azul
#define RTS 7    // Vermelho
#define CTS 9    // Amarelo
#define DADOS 13 // Preto
#define BUFFER_SIZE 64
#define BAUD_RATE 32
#include "Temporizador.h"

char buffer[BUFFER_SIZE];
volatile int buffer_begin = 0;
volatile int buffer_end = 0;

// maquina de estados finita
enum tx_state {
    WAIT_CTS,
    DATA_TX
};

// Rotina de interrupcao do timer1
// O que fazer toda vez que t seg passou?
ISR(TIMER1_COMPA_vect){
  static enum tx_state state = WAIT_CTS;
  static uint8_t bit_pos = 0;
  static uint8_t current_char = 0;
  static uint8_t parity = 0;
  static bool clk_state = HIGH;

  switch(state){
    case WAIT_CTS:
      if(buffer_begin != buffer_end){ // tenho algo para enviar
        digitalWrite(RTS, HIGH); 
        if(digitalRead(CTS)){ // ele quer receber?
          current_char = buffer[buffer_begin % BUFFER_SIZE];
          parity = 0;
          bit_pos = 0;
          state = DATA_TX;
        }
      }
      break;
    
    case DATA_TX:
      clk_state = !clk_state; // alterno meu clock quando estou enviando
      digitalWrite(CLOCK, clk_state);

      // estou na parte LOW da onda, preparo o dado para enviar
      if(clk_state == LOW){
        if(bit_pos < 8){
          uint8_t bit = (current_char >> (7 - bit_pos)) & 1; 
          digitalWrite(DADOS, bit);
          parity ^= bit;
          bit_pos++;

        }else if(bit_pos == 8){
          // envia o bit de paridade
          digitalWrite(DADOS, parity);
          bit_pos++;

        }else{
          // finalizo o bit
          digitalWrite(RTS, LOW);
          buffer_begin++; // avançando o buffer
          state = WAIT_CTS;
          clk_state = HIGH;
        }
      }
  }
}

void reset() {
  noInterrupts();
  digitalWrite(DADOS, LOW);
  digitalWrite(RTS, LOW);
  digitalWrite(CLOCK, HIGH);
  interrupts();
}

void setup() {
  Serial.begin(9600);
  pinMode(CLOCK, OUTPUT);
  pinMode(RTS, OUTPUT);
  pinMode(CTS, INPUT);
  pinMode(DADOS, OUTPUT);
  timer_setup(2 * BAUD_RATE);
  timer_init();
  reset();
}

void loop(void) {
    if(Serial.available() > 0){
        buffer[buffer_end % BUFFER_SIZE] = Serial.read();
        Serial.print(buffer[buffer_end]);
        buffer_end++;
    }
}
