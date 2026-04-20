#define CLOCK 10 // Azul
#define RTS 7    // Vermelho
#define CTS 9    // Amarelo
#define DADOS 13 // Preto
#define BUFFER_SIZE 64
#define BAUD_RATE 32
#include "Temporizador.h"

volatile bool clockState = false;
volatile bool transmitting = false;
volatile bool ready_to_send = false;
// volatile byte current_byte = 0;
// volatile int bit_index = 7;

char buffer[BUFFER_SIZE];
int buffer_begin = 0;
int buffer_end = 0;
char c;
int n = 7; // n para ajudar na transmissão de bits do char
int paridade = 0;
volatile int p = 0;

enum tx_state{
  WAIT_SERIAL,
  RISING_RTS,
  TRANSMITTING
};

enum tx_state estadoAtual = WAIT_SERIAL;

// Rotina de interrupcao do timer1
// O que fazer toda vez que 1s passou?
ISR(TIMER1_COMPA_vect) {

  // Verifica se está em wait serial
  if (estadoAtual == WAIT_SERIAL) {
    // caso ele saia do TRANSMITTING precisa setar clock HIGH
    if (clockState != true) {
      clockState = !clockState;
      digitalWrite(CLOCK, clockState);
    }
    // Verifica se há algo a ser transmitido
    if (buffer_begin != buffer_end) {
      estadoAtual = RISING_RTS;
      return;
    }
  }

  switch (estadoAtual) {
  case WAIT_SERIAL:
    break;
  case RISING_RTS: {
    // Caso ele saia do TRANSMITTING e vai apra RISING_RTS
    // Precisa setar o clock como HIGH
    if (clockState != true) {
      clockState = !clockState;
      digitalWrite(CLOCK, clockState);
    }
    digitalWrite(RTS, HIGH);

    // lopping para esperar o CTS
    while (digitalRead(CTS) == LOW);

    estadoAtual = TRANSMITTING;
    break;
  }
  case TRANSMITTING: {
    // Caso n seja 7 precisa ler o novo char a ser enviado
    if (n == 7) {
      c = buffer[buffer_begin];
      buffer_begin = (buffer_begin+1)%BUFFER_SIZE;
    }

    // caso n<0 transmitimos os 8 bits e falta o de paridade
    if (n < 0) {
      if (clockState == true) {
        digitalWrite(DADOS, paridade % 2);
        paridade = 0;
        n = 7;
        digitalWrite(RTS, LOW);

        if (buffer_begin !=buffer_end) {
          estadoAtual = RISING_RTS;
        }
        else {
          estadoAtual = WAIT_SERIAL;
        }
      }
      clockState = !clockState;
      digitalWrite(CLOCK, clockState);

      return;
    }

    // Caso o clockState seja true estamos indo para uma 
    // borda de descida
    if (clockState == true) {
      int dado = bitRead(c, n);
      n--;
      paridade += dado;
      digitalWrite(DADOS, dado);
    }
    clockState = !clockState;
    digitalWrite(CLOCK, clockState);
    break;
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

void loop() {

  // colocar a frase inteira no meu buffer
  if (Serial.available() > 0) {
    if (buffer_end < BUFFER_SIZE) {
      buffer[buffer_end] = Serial.read();
      Serial.print(buffer[buffer_end]);
      buffer_end = (buffer_end + 1) % BUFFER_SIZE;
    }
  }
}
