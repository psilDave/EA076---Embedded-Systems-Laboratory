// PROJETO 1 - SEMÁFORO COM TEMPORIZAÇÃO CONTROLADA POR INTERRUPÇÕES
// DAVI PEREIRA DA SILVA - 233429
// GABRIEL DANTEE ROQUE - 1972442
// TURMA W

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// VARIÁVEIS:

// Variável responsavel por contar as interrupções períodicas geradas pelo Timer.
volatile unsigned int cont = 0;

// Variável responsavel por contar quantas interrupções períodicas geradas pelo timer são necessárias 
// para se verificar se houve uma alteração no valor lido pelo LDR para evitar transições espurias (farois, passaros).
volatile unsigned int cont_time_para_checar_valor_LDR = 0;

// Variável utilizada para contar quantas interrupções períodiicas geradas pelo timer são necessárias
// para realizar a transição do display que deve mostrar a contagem de tempo. Esse tempo define o refrash rate dos displays para que
// a transição não seja perceptivel aos olhos.
volatile unsigned int cont_time_display = 0;

// Variável responsável por armazenar o pressionamento do botão pelo pedestre solicitando a parada dos carros. 
// Valores possíveis: "0" - Parada não foi solicitada | "1"- Parada solicitada.
volatile unsigned int botao_pedestre = 0;

// Variável responsável por armazenar o valor regrado pelo LDR indicando o período do dia. 
// Valores possíveis: "0" - Dia | "1"- Noite.
volatile unsigned int periodo_do_dia  = 0;

// Variável responsavel por armazenar o estado da Máquina de Estados 2: Sistema Noturno.
// Valores possíveis: "0" - LEDs ligados | "1" - LEDs desligados.
volatile unsigned int estado_noturno = 1;

// Variável responsavel por armazenar qual display deve ser selecionado para mostrar a contagem do semaforo.
// Valores possíveis: "0" - Display dos carros | "1" - Display dos pedestres.
volatile unsigned int display_selecionado = 0;

// Variável responsavel por armazenar o estado da Máquina de Estados 3: Sistema Diurno.
// Valores possíveis: "1" - Semaforo aberto para os carros, semaforo fechado para pedestres e displays desligados.
//                    "2" - Semaforo fechando para os carros, semaforo fechado para os pedestres e displays desligados.
//                    "3" - Semaforo fechado para os carros, semaforo aberto para os pedestres, 
//                          displays ligados e contando o tempo restante para travessia dos pedestres.
//                    "4" - Semaforo fechado para os carros, semaforo piscando vermelho para pedestres, displays ligados,
//                          de forma que o display dos carros está contando e do pedestre piscando com o numero "0". 
volatile unsigned int estado_diurno = 1;


// Variável responsavel por armazenar o estado da Máquina de Estados 4: Estado 3 do Sistema Diurno
volatile unsigned int sub_estado_2 = 0;

// Variável responsavel por armazenar o estado da Máquina de Estados 5: Estado 4 do Sistema Diurno.
volatile unsigned int sub_estado_3 = 0; 

// Pinos chamados no código várias vezes e que tem sentido semantico importante em suas chamadas.
const int pino_LDR = 14;
const int pino_botao_pedestre = 6;
const int pino_seletor_display_veiculos = 5;
const int pino_seletor_display_pedestres = 4;

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// CONFIGURAÇÕES

// CONFIGURAÇÃO DAS INTERRUPÇÕES PERIÓDICAS:

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

// CONFIGURAÇÃO DAS ROTINAS DE INTERRUPÇÃO :

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

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// ROTINAS DE SERVIÇO DE INTERRUPÇÃO

//ROTINA DE INTERRUPÇÕES PERIÓDICAS DE 2ms. 

ISR(TIMER0_COMPA_vect){

  // Rotina de interrução chamada períodicamente a cada 2ms.

  cont++; // Incrementa-se a quantidade de interrupções a cada 2 ms para realizar a troca de estados das Máquinas de Estado 2 e 3.
  cont_time_para_checar_valor_LDR++; // Incrementa-se a quantidade de interrupções a cada 2 ms para realizar a checagem do valor do LDR.
  cont_time_display++; // Incrementa-se a quantidade de interrupções a cada 2 ms para definir o refrash rate do conjunto de displays.
  atualizacao_dos_displays(); // Atualiza-se o display que deve mostrar a contagem para um refrash rate já definido de 125 Hz.
}

 // ROTINA DE INTERRUPÇÃO PARA BOTÃO DO PEDESTRE

ISR(PCINT2_vect){
  //  Interrupção executada quando o pedestre pressiona o botão solicitando a parada dos carros.
  
  if(digitalRead(pino_botao_pedestre) == 1 && botao_pedestre == 0){ // Verifica-se nivel alto no pino da interrupção para indicar que o botão do pedestre foi acionado.
    botao_pedestre = 1; // Indica-se que o botão do pedestre foi selecionado.
    cont = 0; // Inicia-se a contagem do tempo para o inicio da passagem do pedestre. 
  }
}

// FUNÇÃO UTIL PARA REALIZAR A ATUALIZAÇÃO DOS DISPLAYS 

void atualizacao_dos_displays(){
  // Função util das rotinas de interrupção para atualizar o display que deve mostrar a contagem para um refrash rate de 125 Hz.

  if (cont_time_display > 4 ){ // Verifica se passaram 8ms para realizar a atualização do display que deve mostrar a contagem,
    if (display_selecionado == 0){ // Verifica se display que está selecionado é o dos carros 
      display_selecionado = 1; // Troca-se o display que deve mostrar a contagem para o display de pedestres.
    }
  else{ // Caso display selecionado seja o de pedestres,
    display_selecionado = 0; // seleciona-se o display de carros.
  }
  cont_time_display = 0; // E por fim, reseta-se o contador para realizar uma nova atualização após 8 ms.
}
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// FUNÇÕES UTEIS: 

// VALORES DO DISPLAY DE 7 SEGMENTOS

void conversor_decimal_binario_para_display_7_segmentos(int valor_decimal){
  // Converte valores decimais para binário para serem utilizados no decodificador do display de 7 segmentos CD4511.

  switch (valor_decimal) {
    case 0: // Número 0 : 
      digitalWrite(11, LOW); // 0
      digitalWrite(10, LOW); // 0
      digitalWrite(9, LOW);  // 0
      digitalWrite(8, LOW);  // 0
      break;
    
    case 1: // Número 1 :
      digitalWrite(11, LOW); // 0
      digitalWrite(10, LOW); // 0
      digitalWrite(9, LOW);  // 0
      digitalWrite(8, HIGH); // 1
      break;

    case 2: // Número 2 :
      digitalWrite(11, LOW); // 0
      digitalWrite(10, LOW); // 0
      digitalWrite(9, HIGH); // 1
      digitalWrite(8, LOW);  // 0
      break;
    
    case 3: // Número 3 :
      digitalWrite(11, LOW); // 0
      digitalWrite(10, LOW); // 0
      digitalWrite(9, HIGH); // 1
      digitalWrite(8, HIGH); // 1
      break;
    
    case 4: // Número 4 :
      digitalWrite(11, LOW); // 0
      digitalWrite(10, HIGH);// 1
      digitalWrite(9, LOW);  // 0
      digitalWrite(8, LOW);  // 0
      break;
    
    case 5: // Número 5 :
      digitalWrite(11, LOW); // 0
      digitalWrite(10, HIGH);// 1
      digitalWrite(9, LOW);  // 0 
      digitalWrite(8, HIGH); // 1
      break;
    
    case 6: // Número 6 :
      digitalWrite(11, LOW); // 0
      digitalWrite(10, HIGH);// 1
      digitalWrite(9, HIGH); // 1
      digitalWrite(8, LOW);  // 0
      break;
    
    case 7: // Número 7 : 
      digitalWrite(11, LOW); // 0
      digitalWrite(10, HIGH);// 1
      digitalWrite(9, HIGH); // 1
      digitalWrite(8, HIGH); // 1
      break;
    
    case 8: // Número 8 :
      digitalWrite(11, HIGH); // 1
      digitalWrite(10, LOW);  // 0
      digitalWrite(9, LOW);   // 0
      digitalWrite(8, LOW);   // 0
      break;
    
    case 9: // Número 9 :
      digitalWrite(11, HIGH); // 1
      digitalWrite(10, LOW);  // 0
      digitalWrite(9, LOW);   // 0
      digitalWrite(8, HIGH);  // 1
      break;

    case 10: // Número 10 :
      digitalWrite(11, HIGH); // 1
      digitalWrite(10, LOW);  // 0
      digitalWrite(9, HIGH);  // 1
      digitalWrite(8, LOW);   // 0
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
    cont_time_para_checar_valor_LDR = 0; // Reseta-se o contador para verificar alterção no LDR após 4 segundos.
  }  
}

// FUNÇÃO QUE SELECIONA O DISPLAY QUE DEVE MOSTRAR O DIGITO SOLICITADO.

void mostra_digito_no_display_selecionado(int display, int digito_d1, int digito_d2){ 
  // Define qual display deve mostrar o digito requerido.

  if (display == 1){
    digitalWrite(pino_seletor_display_veiculos, HIGH); // deseleciona-se o display dos veiculos e
    digitalWrite(pino_seletor_display_pedestres, LOW); // seleciona-se o display de pedestres, 
    conversor_decimal_binario_para_display_7_segmentos(digito_d1); // mostra o digito solicitado no display dos pedestres.
  }
  else{
    digitalWrite(pino_seletor_display_pedestres, HIGH); // deseleciona-se o display dos pedestres,
    digitalWrite(pino_seletor_display_veiculos, LOW); // seleciona-se o display de veiculos, e
    conversor_decimal_binario_para_display_7_segmentos(digito_d2); // mostra o digito solicitado no display dos veículos.
  }
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


// MÁQUINAS DE ESTADO

// MAQUINA DE ESTADOS 1: Sistema Diurno e Noturno

void maq_estados_dia_e_noite(){
  
  // Define-se a Máquina de Estados do sistema alternando entre os estados "dia" e "noite" de acordo com o valor lido pelo LDR.
  
  verifica_periodo_do_dia_pelo_LDR(); // Verifica-se o período do dia e se não ouve nenhuma alteração espuría nos valores lidos pelo LDR
  if (periodo_do_dia == 1){ // Se o período for noturno, 
    PCMSK2 &= 0b00000000; // Desabilita PCINT22 como entrada para receber a interrupção quando nivel ALTO (HIGH) for identificado.
    maq_estados_noite(); // chama-se a maquina de estados do período noturno.
  }else{ // Caso contrário,
    PCMSK2 |= 0b01000000; // Habilita PCINT22 como entrada para receber a interrupção quando nivel ALTO (HIGH) for identificado.
    maq_estados_dia(); //  chama-se a maquina de estados do período diurno.
  }
}

// MAQUINA DE ESTADOS 2: Sistema Noturno

void maq_estados_noite(){
  // Define-se a máquina de estados para o sistema no período noturno, no qual os LED amarelo do farol de veiculos,
  // e o LED vermelho devem estar piscando. 
  
  if (estado_noturno == 0 && cont < 250){ // Se os LEDs estiverem apagados e o tempo for menor que 0.5 s, tem-se: 

    digitalWrite(15, LOW); // LED vermelho para o farol de veiculos apagado.
    digitalWrite(16, HIGH); // LED amarelo para o farol de veiculos aceso.
    digitalWrite(17, LOW); // LED verde para o farol de veiculos apagado.
    digitalWrite(18, HIGH); // LED vermelho para o farol de pedestres aceso.
    digitalWrite(19, LOW); // LED verde para o farol de pedestres apagado.
    estado_noturno = 1;} // atualiza-se o estado noturno para o proximo estado que desligará os leds.
  
  if (estado_noturno == 1 && cont >= 250){ // Se os LEDs estiverem acesos e o tempo for maior ou igual que 0.5 s, tem-se:

    digitalWrite(15, LOW); // LED vermelho para o farol de veiculos apagado.
    digitalWrite(16, LOW); // LED amarelo para o farol de veiculos apagado.
    digitalWrite(17, LOW); // LED verde para o farol de veiculos apagado.
    digitalWrite(18, LOW); // LED vermelho para o farol de pedestres apagado.
    digitalWrite(19, LOW); // LED verde para o farol de pedestres apagado.
    estado_noturno = 0; // atualiza-se o estado noturno para o proximo estado que ligará os leds.
  }

  if (cont > 500){ // Após 1 segundo, 
    cont = 0; // o contador será reiniciado para realizar o piscar dos leds novamente.
  }

}

// MAQUINA DE ESTADOS 3: Sistema Diurno

void maq_estados_dia(){
  // Define-se a maquina de estados para o sistema no período diurno.

  if (botao_pedestre == 0 && estado_diurno == 1){ // Estado 1 

    digitalWrite(15, LOW); // LED vermelho para o farol de veiculos apagado.
    digitalWrite(16, LOW); // LED amarelo para o farol de veiculos apagado.
    digitalWrite(17, HIGH); // LED verde para o farol de veiculos aceso.
    digitalWrite(18, HIGH); // LED vermelho para o farol de pedestres aceso.
    digitalWrite(19, LOW); // LED verde para o farol de pedestres apagado.
  }

  if (botao_pedestre == 1 && estado_diurno == 1 && cont > 50) { // Estado 2
    digitalWrite(15, LOW); // LED vermelho para o farol de veiculos apagado.
    digitalWrite(16, HIGH); // LED amarelo para o farol de veiculos aceso.
    digitalWrite(17, LOW); // LED verde para o farol de veiculos apagado.
    digitalWrite(18, HIGH); // LED vermelho para o farol de pedestres aceso.
    digitalWrite(19, LOW); // LED verde para o farol de pedestres apagado.
    
    if (cont == 1550){ // Após passar 3.1 segs( 3s - tempo de amarelo minimo definido pelo CONTRAN + 100 ms - solicitado no projeto),
      estado_diurno = 2; // atualiza-se o estado atual da máquina para o 2.
    }
  }

  if (estado_diurno == 2 && botao_pedestre == 1 && cont > 1550){ // Estado 3
    maq_estados_dia_estado_3(); // Chama-se a maquina de estados para o estado 3.
    
  }

  if(estado_diurno == 3 && botao_pedestre == 1 && cont > 3800){ // Estado 4
    maq_estados_dia_estado_4(); // Chama-se a maquina de estados para o estado 4.
    if (cont == 6300){ // Após 12.6 segs (tempo para completar o ciclo da máquina de estados 3), 
      sub_estado_2 = 0; // reseta-se o estado atual da máquina de estados 4
      sub_estado_3 = 0; // reseta-se o estado atual da máquina de estados 5
      botao_pedestre = 0; //  reseta-se o estado do botão do pedestre
      estado_diurno = 1; //  atualiza-se o estado atual da maquina de estados 3 para o estado 1.
      digitalWrite(pino_seletor_display_pedestres, HIGH); // Desativa o display de pedestres.
      digitalWrite(pino_seletor_display_veiculos, HIGH); // Desativa o display de veiculos.
    }
  }
}

// MAQUINA DE ESTADOS 4: Estado 3 do Sistema Diurno

void maq_estados_dia_estado_3(){
  // Define-se a maquina de estados para o estado 2 da maquina de estados do sistma diurno.

  digitalWrite(15, HIGH); // LED vermelho para o farol de veiculos aceso.
  digitalWrite(16, LOW); // LED amarelo para o farol de veiculos apagado.
  digitalWrite(17, LOW); // LED verde para o farol de veiculos apagado.
  digitalWrite(18, LOW); // LED vermelho para o farol de pedestres apagado.
  digitalWrite(19, HIGH); // LED verde para o farol de pedestres aceso.

  if (sub_estado_2 == 0 && cont > 1550){ // Sub estado 0: Mostrar a contagem "9" para display dos carros e "5" para o display de pedestres.
    
    mostra_digito_no_display_selecionado(display_selecionado, 5, 9);
    
  if(cont == 2050){ // Após 1 segundo, 
    sub_estado_2 = 1; // atualiza-se o estado atual da maquina.
  }

  }
  if (sub_estado_2 == 1 && cont > 2050){ // Sub estado 1: Mostrar a contagem "8" para display dos carros e "4" para o display de pedestres.
    mostra_digito_no_display_selecionado(display_selecionado, 4, 8);
    
  if(cont == 2550){ // Após 1 segundo,
    sub_estado_2 = 2; // atualiza-se o estado atual da maquina.
  }

  }
  if (sub_estado_2 == 2 && cont > 2550) { // Sub estado 2: Mostrar a contagem "7" para display dos carros e "3" para o display de pedestres.
    
    mostra_digito_no_display_selecionado(display_selecionado, 3, 7);
    
  if(cont == 3050){ // Após 1 segundo,
      sub_estado_2 = 3; // atualiza-se o estado atual da maquina.
    }
  }
  if (sub_estado_2 == 3 && cont > 3050) { // Sub estado 3: Mostrar a contagem "6" para display dos carros e "2" para o display de pedestres.
    
    mostra_digito_no_display_selecionado(display_selecionado, 2, 6);

  if(cont == 3550){ // Após 1 segundo,
    sub_estado_2 = 4; // atualiza-se o estado atual da maquina.
  }
  }
  if (sub_estado_2 == 4 && cont > 3550) { // Sub estado 4: Mostrar a contagem "5" para display dos carros e "1" para o display de pedestres.
    mostra_digito_no_display_selecionado(display_selecionado, 1, 5);


    if (cont == 4050){ // Após 1 segundo,
          estado_diurno = 3; // atualiza-se o estado atual da maquina.
    }
  }
}

// MAQUINA DE ESTADOS 5: Estado 4 do Sistema Diurno

void maq_estados_dia_estado_4(){
  

  if (sub_estado_3 == 0 && cont > 4050){ // Sub estado 1 Mostrar a contagem "4" para display dos carros e "0" piscando para o display de pedestres.

    digitalWrite(15, HIGH); // LED vermelho para o farol de veiculos aceso.
    digitalWrite(18, HIGH); // LED vermelho para o farol de pedestres aceso.
    mostra_digito_no_display_selecionado(display_selecionado, 0, 4);
    if (cont == 4300){ // Após 0.5 segundos,
      sub_estado_3 = 1; // atualiza-se o estado atual da maquina.
    }
    
  }
  if (sub_estado_3 == 1 && cont > 4300){ // Sub estado 2:  Mostrar a contagem "4" para display dos carros e "0" piscando para o display de pedestres.
    
    digitalWrite(15, HIGH); // LED vermelho para o farol de veiculos aceso.
    digitalWrite(18, LOW); // LED vermelho para o farol de pedestres apagado.
    mostra_digito_no_display_selecionado(display_selecionado, 10, 4);
    
    if (cont == 4550){  // Após 0.5 segundos,
      sub_estado_3 = 2; // atualiza-se o estado atual da maquina.
    }
    
  }
   if (sub_estado_3 == 2 && cont > 4550){ // Sub estado 3:  Mostrar a contagem "3" para display dos carros e "0" piscando para o display de pedestres.

    digitalWrite(15, HIGH); // LED vermelho para o farol de veiculos aceso.
    digitalWrite(18, HIGH); // LED vermelho para o farol de pedestres aceso.
    mostra_digito_no_display_selecionado(display_selecionado, 0, 3);
    if (cont == 4800){ // Após 0.5 segundos,
      sub_estado_3 = 3; // atualiza-se o estado atual da maquina.
      }
    
  }

  if (sub_estado_3 == 3 && cont > 4800){ // Sub estado 4: Mostrar a contagem "3" para display dos carros e "0" piscando para o display de pedestres.
    
    digitalWrite(15, HIGH); // LED vermelho para o farol de veiculos aceso
    digitalWrite(18, LOW); // LED vermelho para o farol de pedestres apagado.
    mostra_digito_no_display_selecionado(display_selecionado, 10, 3);;
    if (cont == 5050){ // Após 0.5 segundos,
      sub_estado_3 = 4; // atualiza-se o estado atual da maquina.
    } 
    }

  if (sub_estado_3 == 4 && cont > 5050){ // Sub estado 5: Mostrar a contagem "2" para display dos carros e "0" piscando para o display de pedestres.

    digitalWrite(15, HIGH); // LED vermelho para o farol de veiculos aceso.
    digitalWrite(18, HIGH); // LED vermelho para o farol de pedestres aceso.
    mostra_digito_no_display_selecionado(display_selecionado, 0, 2);

    if (cont == 5300){ // Após 0.5 segundos,
      sub_estado_3 = 5; // atualiza-se o estado atual da maquina.
    }
  }

   if (sub_estado_3 == 5 && cont > 5300){ // Sub estado 6: Mostrar a contagem "2" para display dos carros e "0" piscando para o display de pedestres.
    
    digitalWrite(15, HIGH); // LED vermelho para o farol de veiculos aceso.
    digitalWrite(18, LOW); // LED vermelho para o farol de pedestres apagado.
    mostra_digito_no_display_selecionado(display_selecionado, 10, 2);

    if (cont == 5550){ // Após 0.5 segundos,
      sub_estado_3 = 6; // atualiza-se o estado atual da maquina.
    }
    }

  if (sub_estado_3 == 6 && cont > 5550){ // Sub estado 7: Mostrar a contagem "1" para display dos carros e "0" piscando para o display de pedestres.

    digitalWrite(15, HIGH); // LED vermelho para o farol de veiculos aceso.
    digitalWrite(18, HIGH); // LED vermelho para o farol de pedestres aceso.
    mostra_digito_no_display_selecionado(display_selecionado, 0, 1);
    if (cont == 5800){ // Após 0.5 segundos,
      sub_estado_3 = 7; // atualiza-se o estado atual da maquina.
    }
  }

  if (sub_estado_3 == 7 && cont > 5800){ // Sub estado 8: Mostrar a contagem "1" para display dos carros e "0" piscando para o display de pedestres.
    
    digitalWrite(15, HIGH); // LED vermelho para o farol de veiculos aceso.
    digitalWrite(18, LOW); // LED vermelho para o farol de pedestres apagado.
    mostra_digito_no_display_selecionado(display_selecionado, 10, 1);
    if (cont == 6050){ // Após 0.5 segundos,
         sub_estado_3 = 8; // atualiza-se o estado atual da maquina.
    }

    }

  if (sub_estado_3 == 8 && cont > 6050){ // Sub estado 9: Mostrar a contagem "0" para display dos carros e "0" piscando para o display de pedestres.

    digitalWrite(15, HIGH); // LED vermelho para o farol de veiculos aceso.
    digitalWrite(18, HIGH); // LED vermelho para o farol de pedestres aceso.
    mostra_digito_no_display_selecionado(display_selecionado, 0, 0);
    if (cont == 6300) // Após 0.5 segundos,
    sub_estado_3 = 9; // atualiza-se o estado atual da maquina.
  }

  if (sub_estado_3 == 9 && cont > 6300){ // Sub estado 10: Mostrar a contagem "0" para display dos carros e "0" piscando para o display de pedestres.
    
    digitalWrite(15, HIGH); // LED vermelho para o farol de veiculos aceso.
    digitalWrite(18, LOW); // LED vermelho para o farol de pedestres apagado.
    mostra_digito_no_display_selecionado(display_selecionado, 10, 0);
    }
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// CONFIGURAÇÃO INICIAL DO SISTEMA

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

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// REALIZA-SE O LOOP CHAMANDO A MÁQUINA DE ESTADOS DO SISTEMA.

void loop() {
  _delay_ms(1); // delay adicionado para não travar a simulação.
  maq_estados_dia_e_noite(); // Chama-se a Máquina de Estados do sistema.
}
