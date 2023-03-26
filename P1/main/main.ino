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


// PINOS IMPORTANTES
int pino_LDR = 14;
int pino_botao_pedestre = 6;


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

void configura_interrupcao_botao_pedestre(){
  // Configura a ISR para o botao_pedestre
  PCICR |= 0b00000100; // Habilita as insterrupções serem recebidas através dos pinos PCINT16 a PCINT23 (no caso utilizaremos PCINT22).
  PCMSK2 |= 0b01000000; // Habilita PCINT22 como entrada para receber a interrupção quando nivel ALTO (HIGH) for identificado.
   
}

// CONFIGURAÇÃO DAS ENTRADAS

void configura_pinos_entrada(){
  // Configura os pinos utilizados como entrada para o LDR e o botão de pedestre.
  
  pinMode(14, INPUT); // LDR.
  pinMode(6, INPUT); // Botão pedestre.
  
}

// CONFIGURAÇÃO DAS SAÍDAS

void configura_pinos_saida(){
  // Configura os pinos utilizados como saida para os LEDs dos semaforos (pedestre e veiculos).
  
  pinMode(15, OUTPUT); // LED vermelho para o farol de veiculos.
  pinMode(16, OUTPUT); // LED amarelo para o farol de veiculos.
  pinMode(17, OUTPUT); // LED verde para o farol de veiculos.
  pinMode(18, OUTPUT); // LED vermelho para o farol de pedestres.
  pinMode(19, OUTPUT); // LED verde para o farol de pedestres.
}

// CONFIGURAÇÃO DO DECODIFICADOR DO DISPLAY DE 7 SEGMENTOS

void configura_decodificador(){
  // Configura os pinos utilizados como saída para enviar a cadeida binária ao decodificador do display de 7 segmentos.
  
  pinMode(11, OUTPUT); // Bit 4 
  pinMode(10, OUTPUT); // Bit 3
  pinMode(9, OUTPUT); // Bit 2
  pinMode(8, OUTPUT); // Bit 1 
  
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

ISR(PCINT2_vect){
  //  Interrupção executada quando o pedestre pressiona o botão solicitando a parada dos carros.
  
  if(digitalRead(pino_botao_pedestre) == 1){ // Verifica-se nivel alto no pino da interrupção para indicar que o botão do pedestre foi acionado.
    botao_pedestre = 1; // Indica-se que o botão do pedestre foi selecionado.
    count = 0; // Inicia-se a contagem do tempo para  
  }
  
}

// TIMERS 

void conta_1_segundo(){
  // Contador de 1 segundo utilizando as interrupções periodicas geradas pelo timer.
  cont = 0;
  while(cont < 500){}
}
void conta_meio_segundo(){
  // Contador de 0.5 segundos utilizando as interrupções periodicas geradas pelo timer.
  cont = 0;
  while(cont < 250){}

}
void conta_3_segundos(){
  // Contador de 3 segundos utilizando as interrupções periodicas geradas pelo timer.
  cont = 0;
  while(cont < 1500){}
}

// VALORES DO DISPLAY DE 7 SEGMENTOS

void conversor_decimal_binario_para_display_7_segmentos(int valor_decimal){
  // Converte valores decimais para binário para serem utilizados no decodificador do display de 7 segmentos CD4511.

  switch (valor_decimal) {
    case 0:
      digitalWrite(11, LOW);
      digitalWrite(10, LOW);
      digitalWrite(9, LOW);
      digitalWrite(8, LOW);
      break;
    
    case 1:
      digitalWrite(11, LOW);
      digitalWrite(10, LOW);
      digitalWrite(9, LOW);
      digitalWrite(8, HIGH);
      break;

    case 2:
      digitalWrite(11, LOW);
      digitalWrite(10, LOW);
      digitalWrite(9, HIGH);
      digitalWrite(8, LOW);
      break;
    
    case 3:
      digitalWrite(11, LOW);
      digitalWrite(10, LOW);
      digitalWrite(9, HIGH);
      digitalWrite(8, HIGH);
      break;
    
    case 4:
      digitalWrite(11, LOW);
      digitalWrite(10, HIGH);
      digitalWrite(9, LOW);
      digitalWrite(8, LOW);
      break;
    
    case 5:
      digitalWrite(11, LOW);
      digitalWrite(10, HIGH);
      digitalWrite(9, LOW);
      digitalWrite(8, HIGH);
      break;
    
    case 6:
      digitalWrite(11, LOW);
      digitalWrite(10, HIGH);
      digitalWrite(9, HIGH);
      digitalWrite(8, LOW);
      break;
    
    case 7:
      digitalWrite(11, LOW);
      digitalWrite(10, HIGH);
      digitalWrite(9, HIGH);
      digitalWrite(8, HIGH);
      break;
    
    case 8:
      digitalWrite(11, HIGH);
      digitalWrite(10, LOW);
      digitalWrite(9, LOW);
      digitalWrite(8, LOW);
      break;
    
    case 9:
      digitalWrite(11, HIGH);
      digitalWrite(10, LOW);
      digitalWrite(9, LOW);
      digitalWrite(8, HIGH);
      break;    
  }
}

// FUNÇÃO QUE VERIFICA INSTABILIDADE NA LEITURA DO LDR

void verifica_periodo_do_dia_pelo_LDR(){
  // Função de varredura para verificar se houve alteração no período do dia e se essa alteração não foi espúria (farol de carro, passaros).
  
  valor_LDR = map(analogRead(pino_LDR), 0, 1023, 0, 100); // Le-se o valor do  LDR.
  conta_3_segundos(); // Espera-se 3 segundos para verificar se a alteração não foi espúria.
  valor_LDR = map(analogRead(pino_LDR), 0, 1023, 0, 100); // Le-se o valor do  LDR novamente para confirmar o valor medido.
  if (valor_LDR < 10){ // Se o valor medido for menor que 10 em uma escala de 0 a 100,
    periodo_do_dia = 1;   // atualiza-se o periodo_do_dia para noite.
  }
  else{ // Caso o valor seja maior que 10,
    periodo_do_dia = 0; // atualiza-se o periodo_do_dia para manhã.
  }
}

// MAQUINA DE ESTADOS: DIA E NOITE

void maq_estados_dia_e_noite(){
  
  // Define-se a maquina de estados do sistema alternando entre os estados "dia" e "noite" de acordo com o valor lido pelo LDR.

  verifica_periodo_do_dia_pelo_LDR(); // Verifica-se em qual período do dia o sistema está aferindo os dados do LDR. 
  
  if (periodo_do_dia == 1){ // Se o período for noturno, chama-se a maquina 
    maq_estados_noite();
  } else{
    maq_estados_dia();
  }

  
}

void setup() {

  cli();
  configuracao_Timer0();
  configura_pinos_entrada(); // CONFIGURA ENTRADAS
  configura_pinos_saida(); // CONFIGURA SAIDAS
  configura_decodificador(); // CONFIGURA DECODIFICADOR
  configura_seletor_de_display_de_sete_segmentos(); // CONFIGURA SELETOR DE DISPLAY 7 SEGMENTOS
  configura_interrupcao_botao_pedestre();
  digitalWrite(5, HIGH);
  digitalWrite(4, LOW);
  sei();

}

void loop() {

  maq_estados_dia_e_noite();

}
