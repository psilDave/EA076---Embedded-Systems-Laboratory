// PROJETO 2 - ACIONAMENTO E CONTROLE DE VENTILADOR POR MEIO DE UM APLICATIVO DE CELULAR E DE UMA INTERFACE BLUETOOTH
// DAVI PEREIRA DA SILVA - 233429
// LEONARDO GALLISSIO - 
// TURMA W

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// VARIÁVEIS:

// Variável responsavel por armazenar o comando valido recebido no sistema.
String comando;

// Variável responsável por verificar se ocorreu um erro de comando.

volatile bool erro = false;

// Pinos chamados no código várias vezes e que tem sentido semantico importante em suas chamadas.


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



// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// ROTINAS DE SERVIÇO DE INTERRUPÇÃO

// ROTINA DE INTERRUPÇÕES PERIÓDICAS DE 2ms. 

ISR(TIMER0_COMPA_vect){

  // Rotina de interrução chamada períodicamente a cada 2ms.

}



// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// FUNÇÕES UTEIS: 

// FUNÇÃO QUE CONCATENA O COMANDO RECEBIDO.

void recebe_comando(){

  if (Serial.available() != 0){ // Vefirica se o buffer da entrada serial não está vazio;
    String cmd;
    cmd = Serial.readStringUntil('*'); // lê o comando enviado até o caracter especial que indica final do comando;
    verifica_comando_valido(cmd); // verifica-se o comando é valido, e
    limpar_buffer(); // limpa-se o buffer após receber o comando.
  }
}

// FUNÇÃO LIMPA O BUFFER SERIAL.

void limpar_buffer(){

  while(Serial.available() > 0){ // Verifica-se se existem caracters para serem recebidos, e 
    Serial.read(); // lê esses caracteres sem armazenar o valor recebido, limpando o buffer serial.
  }
}

// FUNÇÃO QUE VERIFICA SE O COMANDO ENVIADO É VALIDO O COMANDO RECEBIDO.

void verifica_comando_valido(String cmd){
  
  if (cmd.substring(0,3) == "VEL"){ // Se o comando VEL for recebido, verifica se:
    verifica_se_existe_parametro_ausente(cmd); // existe parâmetro ausente
    verifica_se_existe_parametro_incorreto(cmd); // existe parâmetro incorreto
    if (erro == false){ // caso não exista algum dos erros mencionados acima,
      define_comando_valido(cmd); // define-se esse comando como valido, 
      
      // EXECUTA O COMANDO;
    }
  }
  else if (cmd.substring(0,4) == "VENT"){ // Se o comando VEL for recebido, 
    Serial.println("OK VENT"); // envia-se que o comando foi recebido corretamente, 
    define_comando_valido(cmd);// define-se esse comando como valido,
    
    // EXECUTA O COMANDO;
    }
  else if (cmd.substring(0,6) == "EXAUST"){ // Se o comando EXAUST for recebido,
    Serial.println("OK EXAUST"); // envia-se que o comando foi recebido corretamente,
    define_comando_valido(cmd); // define-se esse comando como valido,

    // EXECUTA O COMANDO;
    }
  else if (cmd.substring(0,4) == "PARA"){ // Se o comando PARA for recebido,
    Serial.println("OK PARA"); // envia-se que o comando foi recebido corretamente,
    define_comando_valido(cmd); // define-se esse comando como valido,

    // EXECUTA O COMANDO;
    }
  else if (cmd.substring(0,6) == "RETVEL"){ // Se o comando RETVEL for recebido,
    define_comando_valido(cmd); // define-se esse comando como valido,
    
    // EXECUTA O COMANDO;
    Serial.println("VEL: X RPM"); // retorna-se a velocidade estimada atual.
    }
  else { // caso nenhum comando valido tenha sido recebido, 
    Serial.println("ERRO: COMANDO INEXISTENTE"); // envia-se que o comando não é valido e
    erro = true; // define-se que existe um erro no comando recebido.
  }
}

// FUNÇÃO QUE VERIFCA SE EXISTE ALGUM PARAMETRO AUSENTE NO COMANDO RECEBIDO

void verifica_se_existe_parametro_ausente(String cmd){ 
  if((cmd.substring(4)).length() == 7){ // Se a string criada após a posição 4 (pois o unico comando com parametro é o VEL) não existir (tiver tamanho "0"), 
    Serial.println("ERRO: PARÂMETRO AUSENTE"); // envia-se que o comando foi recebido sem parametro e
    erro = true; // define-se que existe um erro no comando recebido.
  }
}

// FUNÇÃO QUE VERIFCA SE EXISTE ALGUM PARAMETRO INCORRETO NO COMANDO RECEBIDO

void verifica_se_existe_parametro_incorreto(String cmd){
  
  String param = (cmd.substring(4,7)); // Define-se a sub string que contem os parametros (pois o único comando com parametro é o VEL),
  if((param.toInt() < 0) || (param.toInt() > 100) || verifica_se_os_parametros_nao_sao_numeros(param)){ // se o range do parametro excede os limites ou se os parametros não são numeros,
    Serial.println("ERRO: PARÂMETRO INCORRETO");  // envia-se que o comando foi recebido com parametro incorreto.
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



// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// MÁQUINAS DE ESTADO


// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// CONFIGURAÇÃO INICIAL DO SISTEMA

void setup() {

  cli();
  configuracao_Timer0();
  Serial.begin(9600);
  sei();
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// REALIZA-SE O LOOP CHAMANDO A MÁQUINA DE ESTADOS DO SISTEMA.

void loop() {
  _delay_ms(1);
  recebe_comando();
  Serial.println(comando);
  Serial.println(erro);
}