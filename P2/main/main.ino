String comando = "";
bool comandoCompleto = false;

void serialEvent(){
  while(Serial.available()){
    char inChar = (char)Serial.read();
    comando += inChar;

    if inChar
  }
}


void setup() {
  Serial.begin(9600);

}

void loop() {
  recebeComando();
  Serial.println(comando);

}
