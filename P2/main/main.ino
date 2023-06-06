// PROJETO 2 - ACIONAMENTO E CONTROLE DE VENTILADOR POR MEIO DE UM APLICATIVO DE CELULAR E DE UMA INTERFACE BLUETOOTH
// DAVI PEREIRA DA SILVA - 233429
// LEONARDO GALLISSIO - 
// TURMA W

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Bibliotecas para usar o LCD, I2C e bluetooth
#include <LiquidCrystal.h> 
#include <Wire.h>
#include <SoftwareSerial.h>



// VARIÁVEIS:

// Variável responsavel por armazenar o comando valido recebido no sistema.
String comando;

// Variável responsável por verificar se ocorreu um erro de comando.

volatile bool erro = false;

// 

volatile unsigned int velocidade_do_motor = 0;

// 

volatile unsigned int cont = 0;
volatile unsigned int cont_time_display = 0;
volatile unsigned int cont_time_lcd = 0;

volatile unsigned int qtd_pulsos = 0;

volatile unsigned int unidade = 0;
volatile unsigned int dezena = 0;
volatile unsigned int centena = 0;
volatile unsigned int milhar = 0;

SoftwareSerial bluetooth(12,13); 

LiquidCrystal lcd(6, 7, 8, 9, 10, 11);

int escravo = 0b00100000;
// Pinos chamados no código várias vezes e que tem sentido semantico importante em suas chamadas.


int habilita_drivers_do_motor = 3;
int entrada_1_driver_do_motor = 5;
int entrada_2_driver_do_motor = 4;
int pino_do_encoder = 2;

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// CONFIGURAÇÕES


void configura_saidas(){

  pinMode(habilita_drivers_do_motor, OUTPUT);
  pinMode(entrada_1_driver_do_motor, OUTPUT);
  pinMode(entrada_2_driver_do_motor, OUTPUT);
}

void configura_entradas(){
  pinMode(pino_do_encoder, INPUT);
}

void configura_interrupcao_encoder(){
  // Configura a ISR para o codificador.
  
  EIMSK = 0b00000001; // Habilita as insterrupções serem recebidas através do pino PCINT18 (INT0).
  EICRA = 0b00000011; // Habilita PCINT18 como uma entrada que recebe uma interrupção com borda de subida.
}


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



// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// ROTINAS DE SERVIÇO DE INTERRUPÇÃO

// ROTINA DE INTERRUPÇÕES PERIÓDICAS DE 2ms. 

ISR(TIMER0_COMPA_vect){

  // Rotina de interrução chamada períodicamente a cada 2ms.

  cont ++;
  cont_time_display++;
  cont_time_lcd++;

  if (cont_time_display >4){
    cont_time_display = 0;
  }
  if (cont == 250){ // A cada 500 milisegundos, calcula-se a velocidade do motor
    velocidade_do_motor = qtd_pulsos*60; // atualiza a velocidade do motor; velocidade = quantidade_de_pulsos/ quantidade_de_pulsos_por_volta * intervalo_de_medida = qtd_pulsos/(2*500ms) *60s.
    cont = 0; // reseta-se o tempo, e
    qtd_pulsos = 0; // também a quantiadade de pulsos identificados pelo codificador
  }

  if (cont_time_lcd >= 1000){

    String velocidade_do_motor_str = String(velocidade_do_motor);
    lcd.setCursor(0,0);
    lcd.print("ROTACAO:");
    lcd.setCursor(8,0);
    lcd.print(velocidade_do_motor_str + " RPM");
    lcd.setCursor(2,1);
    lcd.print("(ESTIMATIVA)");
    cont_time_lcd = 0;

  }
}

ISR(INT0_vect){

  //  Interrupção acionada pela borda de subida gerada pelo codificador do motor DC.
  qtd_pulsos++; // Sempre que houve uma borda de saída, o contador de pulsos deve ser incrementado.
}



// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// FUNÇÕES UTEIS: 

// FUNÇÃO QUE CONCATENA O COMANDO RECEBIDO.

void recebe_comando(){

  if (bluetooth.available() != 0){ // Vefirica se o buffer da entrada serial não está vazio;
    String cmd;
    cmd = bluetooth.readStringUntil('*'); // lê o comando enviado até o caracter especial que indica final do comando;
    verifica_comando_valido(cmd); // verifica-se o comando é valido, e
    limpar_buffer(); // limpa-se o buffer após receber o comando.
  }
}

// FUNÇÃO LIMPA O BUFFER SERIAL.

void limpar_buffer(){

  while(bluetooth.available() > 0){ // Verifica-se se existem caracters para serem recebidos, e 
    bluetooth.read(); // lê esses caracteres sem armazenar o valor recebido, limpando o buffer serial.
  }
}

// FUNÇÃO QUE VERIFICA SE O COMANDO ENVIADO É VALIDO O COMANDO RECEBIDO.

void verifica_comando_valido(String cmd){
  
  if (cmd.substring(0,3) == "VEL"){ // Se o comando VEL for recebido, verifica se:
    verifica_se_existe_parametro_ausente(cmd); // existe parâmetro ausente
    verifica_se_existe_parametro_incorreto(cmd); // existe parâmetro incorreto
    if (erro == false){ // caso não exista algum dos erros mencionados acima,
      define_comando_valido(cmd); // define-se esse comando como valido, 
      define_velocidade_do_motor(comando); //  define-se a velocidade do motor.
      bluetooth.println("OK VEL "+ comando.substring(4,7));
    }

  }
  else if (cmd.substring(0,4) == "VENT"){ // Se o comando VEL for recebido, 
    bluetooth.println("OK VENT"); // envia-se que o comando foi recebido corretamente, 
    define_comando_valido(cmd);// define-se esse comando como valido,
    define_o_giro_do_motor("PARA");
    define_o_giro_do_motor(comando);
    }
  else if (cmd.substring(0,6) == "EXAUST"){ // Se o comando EXAUST for recebido,
    bluetooth.println("OK EXAUST"); // envia-se que o comando foi recebido corretamente,
    define_comando_valido(cmd); // define-se esse comando como valido,
    define_o_giro_do_motor("PARA");
    define_o_giro_do_motor(comando);
    }
  else if (cmd.substring(0,4) == "PARA"){ // Se o comando PARA for recebido,
    bluetooth.println("OK PARA"); // envia-se que o comando foi recebido corretamente,
    define_comando_valido(cmd); // define-se esse comando como valido,
    define_o_giro_do_motor(comando);
    }
  else if (cmd.substring(0,6) == "RETVEL"){ // Se o comando RETVEL for recebido,
    define_comando_valido(cmd); // define-se esse comando como valido,
    bluetooth.print("VEL "); // retorna-se a velocidade estimada atual.
    bluetooth.print(velocidade_do_motor);
    bluetooth.println(" RPM");
    }
  else { // caso nenhum comando valido tenha sido recebido, 
    bluetooth.println("ERRO: COMANDO INEXISTENTE"); // envia-se que o comando não é valido e
    erro = true; // define-se que existe um erro no comando recebido.
  }
}

// FUNÇÃO QUE VERIFCA SE EXISTE ALGUM PARAMETRO AUSENTE NO COMANDO RECEBIDO

void verifica_se_existe_parametro_ausente(String cmd){ 
  if((cmd.substring(4)).length() != 3){ // Se a string criada após a posição 4 (pois o unico comando com parametro é o VEL) não existir (tiver tamanho "0"), 
    bluetooth.println("ERRO: PARÂMETRO AUSENTE"); // envia-se que o comando foi recebido sem parametro e
    erro = true; // define-se que existe um erro no comando recebido.
  }
}

// FUNÇÃO QUE VERIFCA SE EXISTE ALGUM PARAMETRO INCORRETO NO COMANDO RECEBIDO

void verifica_se_existe_parametro_incorreto(String cmd){
  
  String param = (cmd.substring(4,7)); // Define-se a sub string que contem os parametros (pois o único comando com parametro é o VEL),
  if((param.toInt() < 0) || (param.toInt() > 100) || verifica_se_os_parametros_nao_sao_numeros(param)){ // se o range do parametro excede os limites ou se os parametros não são numeros,
    bluetooth.println("ERRO: PARÂMETRO INCORRETO");  // envia-se que o comando foi recebido com parametro incorreto.
    erro = true; // define-se que existe um erro no comando recebido.
  }
}

// FUNÇÃO QUE ATRIBUI UM COMANDO;

void define_comando_valido(String cmd){
  comando = cmd; // Caso o comando recebido não tenha nenhum erro, define-se o mesmo como um comando valido,
  erro = false; // e também que não existe erro no comando recebido.
}

// FUNÇÃO VERIFICA SE O PARAMETRO RECEBIDO É UM NUMERO PELA TABELA ASCI;

bool verifica_se_os_parametros_nao_sao_numeros(String param){
  for (int n =0; n <=2; n++){ // Verifica cada posição da string de parametro,
    char caracter_velocidade = param.charAt(n); // seleciona o caracter naquela posição, 
    if(!(caracter_velocidade >= 48 && caracter_velocidade <= 57)){ // e se esse caracter não estiver no range de simbolos definidos como "numeros" na tabela ASCI
      return true; // retorna-se que pelo menos um desses caracters não é um numero;
    }
  }return false; // caso todos os caracteres estejam no range de simbolos definidos como "numeros" na tabela ASCI, retorna-se false, pois todos são numeros. 
}

void define_o_giro_do_motor(String parametro){

  if (parametro == "VENT"){
    digitalWrite(entrada_1_driver_do_motor, HIGH);
    digitalWrite(entrada_2_driver_do_motor, LOW);
  }
  else if (parametro == "EXAUST"){
    digitalWrite(entrada_1_driver_do_motor, LOW);
    digitalWrite(entrada_2_driver_do_motor, HIGH);
  }
  else{
    digitalWrite(entrada_1_driver_do_motor, LOW);
    digitalWrite(entrada_2_driver_do_motor, LOW);
  }
}

void define_velocidade_do_motor(String str_velocidade){
  int velocidade = (str_velocidade.substring(4,7)).toInt(); // Lê a velocidade definida pelo comando.
  analogWrite(habilita_drivers_do_motor, 2.55*velocidade); // Ajusta-se a velocidade pela formula do pulso de pwm.
}

void mostra_velocidade_nos_displays(){
  // Rotina para enviar dados aos displays de 7 segmentos
  unidade = 0b11100000 + velocidade_do_motor%10; // captura a unidade da rotação do motor para mostrar no display
  dezena = 0b11010000 + (velocidade_do_motor%100)/10; // captura a dezena da rotação do motor para mostrar no display
  centena = 0b10110000 + (velocidade_do_motor%1000)/100; // captura a centena da rotação do motor para mostrar no display
  milhar = 0b01110000 + velocidade_do_motor/1000; // captura o milhar da rotação do motor para mostrar no display
  
  Wire.beginTransmission(escravo); // inicia a tranmissão i2c para o 8cf8574.
    
  switch (cont_time_display){
    case 0:
      Wire.write(unidade);
    break;
    case 1:
      Wire.write(dezena);
    break;
    case 2:
      Wire.write(centena);
    break;
    case 3:
      Wire.write(milhar);
    break;
  }  
  Wire.endTransmission();
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// MÁQUINAS DE ESTADO


// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// CONFIGURAÇÃO INICIAL DO SISTEMA

void setup() {

  cli();
  configuracao_Timer0();
  Serial.begin(9600);
  bluetooth.begin(9600);
  lcd.begin(16,2);
  Wire.begin();
  configura_interrupcao_encoder();
  configura_entradas();
  analogWrite(habilita_drivers_do_motor, 0);
  sei();
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// REALIZA-SE O LOOP CHAMANDO A MÁQUINA DE ESTADOS DO SISTEMA.

void loop() {
  _delay_ms(1);
  recebe_comando();
  mostra_velocidade_nos_displays();
}