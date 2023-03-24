// Variável responsavel por contar as interrupções períodicas geradas pelo Timer.
volatile unsigned int cont = 0;

// botao_pedestre : Variável responsável por armazenar o pressionamento do botão pelo pedestre solicitando a parada dos carros. 
// Valores possíveis:
// "0" - Parada não foi solicitada | "1"- Parada solicitada.
volatile unsigned int botao_pedestre = 0;

// periodo_do_dia: Variável responsável por armazenar o valor regrado pelo LDR indicando o período do dia. 
// Valores possíveis:
// "0" - Dia | "1"- Noite.
volatile unsigned int periodo_do_dia  = 0; 

// CONFIGURAÇÃO DAS INTERRUPÇÕES PERIÓDICAS

void configuracao_Timer0(){
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Configuracao Temporizador 0 (8 bits) para gerar interrupcoes periodicas a cada 2ms no modo Clear Timer on Compare Match (CTC)
  // Relogio = 16e6 Hz
  // Prescaler = 256
  // Faixa = 125 (contagem de 0 a OCR0A = 124)
  // Intervalo entre interrupcoes: (Prescaler/Relogio)*Faixa = (256/16e6)*(124+1) = 0.002s
  
  // TCCR0A – Timer/Counter Control Register A
  // COM0A1 COM0A0 COM0B1 COM0B0 – – WGM01 WGM00
  // 0      0      0      0          1     0
  TCCR0A = 0x02;

  // OCR0A – Output Compare Register A
  OCR0A = 124;

  // TIMSK0 – Timer/Counter Interrupt Mask Register
  // – – – – – OCIE0B OCIE0A TOIE0
  // – – – – – 0      1      0
  TIMSK0 = 0x02;
  
  // TCCR0B – Timer/Counter Control Register B
  // FOC0A FOC0B – – WGM02 CS02 CS01 CS0
  // 0     0         0     1    0    0
  TCCR0B = 0x04;
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

// CONFIGURAÇÃO DAS ROTINAS DE INTERRUPÇÃO

void configura_interrupcao_LDR(){
  // Configura a ISR para o LDR
  

}
void configura_interrupcao_botao_pedestre(){
  // Configura a ISR para o botao_pedestre
   
}

// CONFIGURAÇÃO DAS ENTRADAS

void configura_pinos_entrada(){
  // Configura os pinos utilizados como entrada para o LDR e o botão de pedestre.
  
  pinMode(14, INPUT); // LDR
  pinMode(6, INPUT); // Botão pedestre
  
}

// CONFIGURAÇÃO DAS SAÍDAS

void configura_pinos_saida(){
  // Configura os pinos utilizados como saida para os LEDs dos semaforos (pedestre e veiculos).
  
  pinMode(15, OUTPUT); // LED vermelho para o farol de veiculos
  pinMode(16, OUTPUT); // LED amarelo para o farol de veiculos
  pinMode(17, OUTPUT); // LED verde para o farol de veiculos
  pinMode(18, OUTPUT); // LED vermelho para o farol de pedestres
  pinMode(19, OUTPUT); // LED verde para o farol de pedestres
}

// CONFIGURAÇÃO DO DECODIFICADOR DO DISPLAY DE 7 SEGMENTOS

void configura_decodificador(){
  // Configura os pinos utilizados como saída para decodificador do display de 7 segmentos.
  
  pinMode(11, OUTPUT); // Entrada 4
  pinMode(10, OUTPUT); // Entrada 3
  pinMode(9, OUTPUT); // Entrada 2
  pinMode(8, OUTPUT); // Entrada 1
  
} 

// CONFIGURAÇÃO DO SELETOR DE DISPLAY DE 7 SEGMENTOS

void configura_seletor_de_display_de_sete_segmentos(){
  // Configura os pinos utilizados como saída para selecionar o display que deve mostrar a contagem.
  
  pinMode(5,OUTPUT); // DISPLAY DO VEICULO
  pinMode(4, OUTPUT); // DISPLAY DO PEDESTRE 
}

// ROTINAS DE SERVIÇO DE INTERRUPÇÃO;

ISR(TIMER0_COMPA_vect){
  cont++;  
}

ISR_botao_pedestre(){
  //  Interrupção executada quando o pedestre pressiona o botão solicitando a parada dos carros.
  botao_pedestre = 1; 
}
ISR(){
  //  Interrupção executada quando houver alteração do periodo do dia.
  switch (periodo_do_dia){
    case 0:
      periodo_do_dia = 1;
    case 1:
      periodo_do_dia = 0;
  }
}

void setup() {

  cli();
  configuracao_Timer0();
  configura_pinos_entrada(); // CONFIGURA ENTRADAS
  configura_pinos_saida(); // CONFIGURA SAIDAS
  configura_decodificador(); // CONFIGURA DECODIFICADOR
  configura_seletor_de_display_de_sete_segmentos(); // CONFIGURA SELETOR DE DISPLAY 7 SEGMENTOS
  sei();

}

void loop() {
  
}
