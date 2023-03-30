// Variável responsavel por contar as interrupções períodicas geradas pelo Timer.
volatile unsigned int cont = 0;

// Variável responsavel por contar quantas interrupções períodicas geradas pelo timer são necessárias para se verificar se houve uma alteração no valor lido pelo LDR para evitar transições espurias (farois, passaros).
volatile unsigned int cont_time_para_checar_valor_LDR = 0;

// 
volatile unsigned int cont_time_display = 0;

// botao_pedestre : Variável responsável por armazenar o pressionamento do botão pelo pedestre solicitando a parada dos carros. 
// Valores possíveis: "0" - Parada não foi solicitada | "1"- Parada solicitada.
volatile unsigned int botao_pedestre = 0;

// periodo_do_dia: Variável responsável por armazenar o valor regrado pelo LDR indicando o período do dia. 
// Valores possíveis: "0" - Dia | "1"- Noite.
volatile unsigned int periodo_do_dia  = 0;

// Define o estado da maquina de estados noturna.
// Valores possíveis: "0" - LEDs ligados | "1" - LEDs desligados.
volatile unsigned int estado_noturno = 1;

volatile unsigned int display_selecionado = 0;

// Define o estado da maquina de estados diurna.
// Valores possíveis: "0" - Semaforo aberto para os carros, semaforo fechado para pedestres e displays desligados.
//                    "1" - Semaforo fechando para os carros, semaforo fechado para os pedestres e displays desligados.
//                    "2" - Semaforo fechado para os carros, semaforo aberto para os pedestres, displays ligados e contando o tempo restante para travessia dos pedestres.
//                    "3" - Semaforo fechado para os carros, semaforo piscando vermelho para pedestres, displays ligados,de forma que o display dos carros está contando e do pedestre piscando com o numero "0". 

volatile unsigned int estado_diurno = 1;


// Define o sub estado 2 da maquina de estados do estado 2 da maquina de estados diurna.
// Valores possíveis: "0"
//                    "1"
//                    "2"
//                    "3" 

volatile unsigned int sub_estado_2 = 0;

// Define o sub estado 3 da maquina de estados do estado 3 da maquina de estados diurna.
// Valores possíveis: "0"
//                    "1"
//                    "2"
//                    "3" 

volatile unsigned int sub_estado_3 = 0; 

// Pinos chamados no código várias vezes e que tem um sentido semantico importante em suas chamadas.
const int pino_LDR = 14;
const int pino_botao_pedestre = 6;
const int pino_seletor_display_veiculos = 5;
const int pino_seletor_display_pedestres = 4;


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
  
  pinMode(pino_LDR, INPUT); // LDR.
  pinMode(pino_botao_pedestre, INPUT); // Botão pedestre.
  
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
  
  pinMode(pino_seletor_display_veiculos, OUTPUT); // Display dos veiculos.
  pinMode(pino_seletor_display_pedestres, OUTPUT); // Display dos pedestres. 
}

// ROTINAS DE SERVIÇO DE INTERRUPÇÃO;

ISR(TIMER0_COMPA_vect){
  cont++;
  cont_time_para_checar_valor_LDR++;
  cont_time_display++;
  
  if (display_selecionado == 0 && cont_time_display >= 4){
    display_selecionado = 1;
    cont_time_display = 0;
  }

  if(display_selecionado == 1 && cont_time_display >= 4){
    display_selecionado = 0;
    cont_time_display = 0;
    }
}

ISR(PCINT2_vect){
  //  Interrupção executada quando o pedestre pressiona o botão solicitando a parada dos carros.
  
  if(digitalRead(pino_botao_pedestre) == 1 && botao_pedestre == 0){ // Verifica-se nivel alto no pino da interrupção para indicar que o botão do pedestre foi acionado.
    botao_pedestre = 1; // Indica-se que o botão do pedestre foi selecionado.
    cont = 0; // Inicia-se a contagem do tempo para quando o pedestre solicitar a parada. 
  }


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

    case 10: 
      digitalWrite(11, HIGH);
      digitalWrite(10, LOW);
      digitalWrite(9, HIGH);
      digitalWrite(8, LOW);
      break;    
  }
}

// FUNÇÃO QUE VERIFICA INSTABILIDADE NA LEITURA DO LDR

void verifica_periodo_do_dia_pelo_LDR(){
  // Função de varredura para verificar se houve alteração no período do dia e se essa alteração não foi espúria (farol de carro, passaros).
  
  if(cont_time_para_checar_valor_LDR >2000){ // Espera-se 4 segundos,
    
    int valor_LDR = map(analogRead(pino_LDR), 0, 1023, 0, 100); // Le-se o valor do  LDR para verificar se não houve uma alteração espúria.
    
    if (valor_LDR > 10){ // Se o valor medido for maior que 10 em uma escala de 0 a 100,
      periodo_do_dia = 1;   // atualiza-se o periodo_do_dia para noite.
    }
    else{ // Caso o valor seja menor que 10,
      periodo_do_dia = 0; // atualiza-se o periodo_do_dia para manhã.
    }
    cont_time_para_checar_valor_LDR = 0;
  }  
}

// FUNÇÃO QUE SELECIONA O DISPLAY QUE DEVE MOSTRAR O DIGITO SOLICITADO.
// (ARRUMAR)

void mostra_digito_no_display_selecionado(int display, int digito_d1, int digito_d2){ 
  // Define qual display deve mostrar o digito requerido.

  if (display == 1){
    digitalWrite(pino_seletor_display_veiculos, HIGH); // deseleciona-se o display dos veiculos e
    digitalWrite(pino_seletor_display_pedestres, LOW); // seleciona-se o display de pedestres, 
    conversor_decimal_binario_para_display_7_segmentos(digito_d1);
  }
  else{
    digitalWrite(pino_seletor_display_pedestres, HIGH); // deseleciona-se o display dos pedestres,
    digitalWrite(pino_seletor_display_veiculos, LOW); // seleciona-se o display de veiculos, e
    conversor_decimal_binario_para_display_7_segmentos(digito_d2);
  }
}

// FUNÇÃO
// MAQUINA DE ESTADOS: DIA E NOITE

void maq_estados_dia_e_noite(){
  
  // Define-se a maquina de estados do sistema alternando entre os estados "dia" e "noite" de acordo com o valor lido pelo LDR.
  
  verifica_periodo_do_dia_pelo_LDR(); // Verifica-se o período do dia e se não ouve nenhuma alteração espuría nos valores lidos pelo LDR
  if (periodo_do_dia == 1){ // Se o período for noturno, 
    PCMSK2 &= 0b00000000; // Desabilita PCINT22 como entrada para receber a interrupção quando nivel ALTO (HIGH) for identificado.
    maq_estados_noite(); // chama-se a maquina de estados do período noturno.
  }else{ // Caso contrário,
    PCMSK2 |= 0b01000000; // Habilita PCINT22 como entrada para receber a interrupção quando nivel ALTO (HIGH) for identificado.
    maq_estados_dia(); //  chama-se a maquina de estados do período diurno.
  }
}

// MAQUINA DE ESTADOS: NOITE

void maq_estados_noite(){
  // Define-se a máquina de estados para o sistema no período noturno, no qual os LED amarelo do farol de veiculos,
  // e o LED vermelho devem estar piscando. 
  
  if (estado_noturno == 0 && cont < 250){
    digitalWrite(15, LOW); // LED vermelho para o farol de veiculos apagado.
    digitalWrite(16, HIGH); // LED amarelo para o farol de veiculos aceso.
    digitalWrite(17, LOW); // LED verde para o farol de veiculos apagado.
    digitalWrite(18, HIGH); // LED vermelho para o farol de pedestres aceso.
    digitalWrite(19, LOW); // LED verde para o farol de pedestres apagado.
    estado_noturno = 1;}
  
  if (estado_noturno == 1 && cont >= 250){
    digitalWrite(15, LOW); // LED vermelho para o farol de veiculos apagado.
    digitalWrite(16, LOW); // LED amarelo para o farol de veiculos apagado.
    digitalWrite(17, LOW); // LED verde para o farol de veiculos apagado.
    digitalWrite(18, LOW); // LED vermelho para o farol de pedestres apagado.
    digitalWrite(19, LOW); // LED verde para o farol de pedestres apagado.
    estado_noturno = 0;
  }

  if (cont > 500){
    cont = 0;
  }

  }

// MAQUINA DE ESTADOS: DIA

void maq_estados_dia(){
  // Define-se a maquina de estados para o sistema no período diurno.

  if (botao_pedestre == 0 && estado_diurno == 1){ // Estado 0 

    digitalWrite(15, LOW); // LED vermelho para o farol de veiculos apagado.
    digitalWrite(16, LOW); // LED amarelo para o farol de veiculos apagado.
    digitalWrite(17, HIGH); // LED verde para o farol de veiculos aceso.
    digitalWrite(18, HIGH); // LED vermelho para o farol de pedestres aceso.
    digitalWrite(19, LOW); // LED verde para o farol de pedestres apagado.
  }

  if (botao_pedestre == 1 && estado_diurno == 1 && cont > 50) { // Estado 1
    digitalWrite(15, LOW); // LED vermelho para o farol de veiculos apagado.
    digitalWrite(16, HIGH); // LED amarelo para o farol de veiculos aceso.
    digitalWrite(17, LOW); // LED verde para o farol de veiculos apagado.
    digitalWrite(18, HIGH); // LED vermelho para o farol de pedestres aceso.
    digitalWrite(19, LOW); // LED verde para o farol de pedestres apagado.
    
    if (cont == 1550){
      estado_diurno = 2;
    }
  }

  if (estado_diurno == 2 && botao_pedestre == 1 && cont > 1550){ // Estado 2
    maq_estados_dia_estado_2(); // Chama-se a maquina de estados para o estado 2.
    
  }

  if(estado_diurno == 3 && botao_pedestre == 1 && cont > 3800){ //Estado 3
    maq_estados_dia_estado_3(); // Chama-se a maquina de estados para o estado 3.
    if (cont == 6300){
      sub_estado_2 = 0;
      sub_estado_3 = 0;
      botao_pedestre = 0;
      estado_diurno = 1;
      digitalWrite(pino_seletor_display_pedestres, HIGH); // Desativa o display de pedestres.
      digitalWrite(pino_seletor_display_veiculos, HIGH); // Desativa o display de veiculos.
    }
  }
}

// MAQUINA DE ESTADOS: ESTADO 2 DO DIA
// (ARRUMAR)
void maq_estados_dia_estado_2(){
  // Define-se a maquina de estados para o estado 2 da maquina de estados do sistma diurno.

  digitalWrite(15, HIGH); // LED vermelho para o farol de veiculos aceso.
  digitalWrite(16, LOW); // LED amarelo para o farol de veiculos apagado.
  digitalWrite(17, LOW); // LED verde para o farol de veiculos apagado.
  digitalWrite(18, LOW); // LED vermelho para o farol de pedestres apagado.
  digitalWrite(19, HIGH); // LED verde para o farol de pedestres aceso.

  if (sub_estado_2 == 0 && cont > 1550){ // Sub estado 0: Mostrar a contagem "9" para display dos carros e "5" para o display de pedestres.
    
    mostra_digito_no_display_selecionado(display_selecionado, 5, 9);
    
  if(cont == 2050){
    sub_estado_2 = 1;
  }

  }
  if (sub_estado_2 == 1 && cont > 2050){ // Sub estado 1: Mostrar a contagem "8" para display dos carros e "4" para o display de pedestres.
    mostra_digito_no_display_selecionado(display_selecionado, 4, 8);
    
  if(cont == 2550){
    sub_estado_2 = 2;
  }

  }
  if (sub_estado_2 == 2 && cont > 2550) { // Sub estado 2: Mostrar a contagem "7" para display dos carros e "3" para o display de pedestres.
    
    mostra_digito_no_display_selecionado(display_selecionado, 3, 7);
    
  if(cont == 3050){
      sub_estado_2 = 3;
    }
  }
  if (sub_estado_2 == 3 && cont > 3050) { // Sub estado 3: Mostrar a contagem "6" para display dos carros e "2" para o display de pedestres.
    
    mostra_digito_no_display_selecionado(display_selecionado, 2, 6);

  if(cont == 3550){
    sub_estado_2 = 4;
  }
  }
  if (sub_estado_2 == 4 && cont > 3550) { // Sub estado 4: Mostrar a contagem "5" para display dos carros e "1" para o display de pedestres.
    mostra_digito_no_display_selecionado(display_selecionado, 1, 5);


    if (cont == 4050){
          estado_diurno = 3;
    }
  }
}

// MAQUINA DE ESTADOS: ESTADO 3 DO DIA
// (ARRUMAR)
void maq_estados_dia_estado_3(){
  

  if (sub_estado_3 == 0 && cont > 4050){ // Sub estado 0: Mostrar a contagem "4" para display dos carros e "0" piscando para o display de pedestres.

    digitalWrite(15, HIGH); // LED vermelho para o farol de veiculos aceso.
    digitalWrite(18, HIGH); // LED vermelho para o farol de pedestres aceso.
    mostra_digito_no_display_selecionado(display_selecionado, 0, 4);
    if (cont == 4300){
      sub_estado_3 = 1;
    }
    
  }
  if (sub_estado_3 == 1 && cont > 4300){ // Sub estado 1:  Mostrar a contagem "4" para display dos carros e "0" piscando para o display de pedestres.
    
    digitalWrite(15, HIGH); // LED vermelho para o farol de veiculos aceso.
    digitalWrite(18, LOW); // LED vermelho para o farol de pedestres apagado.
    mostra_digito_no_display_selecionado(display_selecionado, 10, 4);
    if (cont == 4550){
      sub_estado_3 = 2;
    }
    
  }
   if (sub_estado_3 == 2 && cont > 4550){ // Sub estado 2:  Mostrar a contagem "3" para display dos carros e "0" piscando para o display de pedestres.

    digitalWrite(15, HIGH); // LED vermelho para o farol de veiculos aceso.
    digitalWrite(18, HIGH); // LED vermelho para o farol de pedestres aceso.
    mostra_digito_no_display_selecionado(display_selecionado, 0, 3);
    if (cont == 4800){
      sub_estado_3 = 3;
      }
    
  }

  if (sub_estado_3 == 3 && cont > 4800){ // Sub estado 3: Mostrar a contagem "3" para display dos carros e "0" piscando para o display de pedestres.
    
    digitalWrite(15, HIGH); // LED vermelho para o farol de veiculos aceso
    digitalWrite(18, LOW); // LED vermelho para o farol de pedestres apagado.
    mostra_digito_no_display_selecionado(display_selecionado, 10, 3);;
    if (cont == 5050){
      sub_estado_3 = 4;
    } 
    }

  if (sub_estado_3 == 4 && cont > 5050){ // Sub estado 4: Mostrar a contagem "2" para display dos carros e "0" piscando para o display de pedestres.

    digitalWrite(15, HIGH); // LED vermelho para o farol de veiculos aceso.
    digitalWrite(18, HIGH); // LED vermelho para o farol de pedestres aceso.
    mostra_digito_no_display_selecionado(display_selecionado, 0, 2);

    if (cont == 5300){
      sub_estado_3 = 5;
    }
  }

   if (sub_estado_3 == 5 && cont > 5300){ // Sub estado 5: Mostrar a contagem "2" para display dos carros e "0" piscando para o display de pedestres.
    
    digitalWrite(15, HIGH); // LED vermelho para o farol de veiculos aceso.
    digitalWrite(18, LOW); // LED vermelho para o farol de pedestres apagado.
    mostra_digito_no_display_selecionado(display_selecionado, 10, 2);

    if (cont == 5550){
      sub_estado_3 = 6;
    }
    }

  if (sub_estado_3 == 6 && cont > 5550){ // Sub estado 6: Mostrar a contagem "1" para display dos carros e "0" piscando para o display de pedestres.

    digitalWrite(15, HIGH); // LED vermelho para o farol de veiculos aceso.
    digitalWrite(18, HIGH); // LED vermelho para o farol de pedestres aceso.
    mostra_digito_no_display_selecionado(display_selecionado, 0, 1);
    if (cont == 5800){
      sub_estado_3 = 7;
    }
  }

  if (sub_estado_3 == 7 && cont > 5800){ // Sub estado 7: Mostrar a contagem "1" para display dos carros e "0" piscando para o display de pedestres.
    
    digitalWrite(15, HIGH); // LED vermelho para o farol de veiculos aceso.
    digitalWrite(18, LOW); // LED vermelho para o farol de pedestres apagado.
    mostra_digito_no_display_selecionado(display_selecionado, 10, 1);
    if (cont == 6050){
         sub_estado_3 = 8;
    }

    }

  if (sub_estado_3 == 8 && cont > 6050){ // Sub estado 8: Mostrar a contagem "0" para display dos carros e "0" piscando para o display de pedestres.

    digitalWrite(15, HIGH); // LED vermelho para o farol de veiculos aceso.
    digitalWrite(18, HIGH); // LED vermelho para o farol de pedestres aceso.
    mostra_digito_no_display_selecionado(display_selecionado, 0, 0);
    if (cont == 6300)
    sub_estado_3 = 9;
  }

  if (sub_estado_3 == 9 && cont > 6300){ // Sub estado 9: Mostrar a contagem "0" para display dos carros e "0" piscando para o display de pedestres.
    
    digitalWrite(15, HIGH); // LED vermelho para o farol de veiculos aceso.
    digitalWrite(18, LOW); // LED vermelho para o farol de pedestres apagado.
    mostra_digito_no_display_selecionado(display_selecionado, 10, 0);
    }
}


void setup() {

  cli();
  configuracao_Timer0();
  configura_pinos_entrada(); // Configura as entradas.
  configura_pinos_saida(); // Configura as saídas.
  configura_decodificador(); // Configura o decodificador.
  configura_seletor_de_display_de_sete_segmentos(); // Configura o seletor de display de sete segmentos.
  configura_interrupcao_botao_pedestre(); // Configura a interrupção para o botão de pedestres.
  digitalWrite(pino_seletor_display_pedestres, HIGH); // Desativa o display de pedestres.
  digitalWrite(pino_seletor_display_veiculos, HIGH); // Desativa o display de veiculos.
  sei();
}

void loop() {
  _delay_ms(1);
  maq_estados_dia_e_noite();

}
