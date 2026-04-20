#define CLOCK 10 // Azul
#define RTS 7 // Vermelho
#define CTS 9 // Amarelo
#define DADOS 13 // Preto
#define BUFFER_SIZE 64
#define BAUD_RATE 32
#include "Temporizador.h"

volatile bool clockState = false;
volatile bool transmitting = false;
volatile bool ready_to_send = false;
volatile byte current_byte = 0;
volatile int bit_index = 7;

char buffer[BUFFER_SIZE];
int buffer_begin = 0;
int buffer_end = 0;
volatile int p = 0;

// Calcula bit de paridade - Par ou impar
bool bitParidade(char dado){
 // < ainda fazer o codigo >
}

// Rotina de interrupcao do timer1
// O que fazer toda vez que 1s passou?
ISR(TIMER1_COMPA_vect){

  // alterna o meu clock
  

  if(transmitting){
    clockState = !clockState;
  }
  // na borda de descida
  if(clockState == LOW && transmitting){

    int bit = (current_byte >> bit_index) & 1;

    if(bit_index < 0) {
      digitalWrite(DADOS, p);
      transmitting = false;
      //digitalWrite(CLOCK, HIGH);
      clockState = HIGH;
    }else{
      digitalWrite(DADOS, bit);
      Serial.print(" ");
      Serial.print(bit);
      Serial.print(" ");
      p ^= bit;
      bit_index--;
    }
  }

  digitalWrite(CLOCK, clockState);
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
  timer_setup(2*BAUD_RATE);
  timer_init();
  reset();
}

void loop() {

  // colocar a frase inteira no meu buffer
  if(Serial.available() > 0) {
    if(buffer_end < BUFFER_SIZE){
      buffer[buffer_end] = Serial.read();
      Serial.print(buffer[buffer_end]);
      buffer_end++;
    }
  }
  
  // ver se tenho algo para enviar
  if(buffer_end != 0){

    // settar que estou pronto para enviar
    digitalWrite(RTS, HIGH);

    if(digitalRead(CTS) == HIGH){
      ready_to_send = true;
      Serial.print(" CTS_HIGH ");
      digitalWrite(RTS, LOW);
    }else{
      ready_to_send = false;
    }

    if(ready_to_send && !transmitting){
      current_byte = buffer[buffer_begin++];
      bit_index = 7;
      p = 0;
      transmitting = true;
      Serial.print(" send ");
      digitalWrite(RTS, LOW);
    }
  }

  // reseto minhas variaveis  
  if(buffer_begin == buffer_end){
    buffer_begin = 0;
    buffer_end = 0;
    ready_to_send = false;
    Serial.print(" last ");
  }
}
