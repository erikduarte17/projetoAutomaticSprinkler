// ############ BIBLIOTECAS ############

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Stepper.h>
#include <DHT.h>

// ############ DEFINES ############

// PARA OS PINOS DOS BOTOES (4 ENTRADAS DE INPUT-PULLUP)
#define buttonRight 12
#define buttonEnter 11
#define buttonBack 10
#define buttonLeft 9

// PARA OS PINOS DO MOTOR (4 ENTRADAS)
#define IN1 8
#define IN2 7
#define IN3 6
#define IN4 5

// PARA DEFINIR O TAMANHO DOS MENUS (3 MENUS E 2 SUBMENUS)
#define menuMax 3
#define subMenuMax 2

// PARA CONTROLE DOS ESTADOS
#define estado1 1
#define estado2 2
#define estado3 3
#define estado4 4

// PARA OS PINOS DOS SENSORES (2 ENTRADAS ANALÓGICAS)
#define pinoHigrometro A0
#define pinoDHT A1

// PARA O PINO DE CONTROLE DO RELE (1 SAIDA)
#define pinoRele 4

// ############ VARIAVEIS ############

// PARA CONTROLE DOS ESTADOS
int estadoAtual = estado1;
int proximoEstado = estado1;

// PARA CONTROLE DOS MENUS
int menuPrincipal = 1;
int menuControle = 1;
int subMenu = 1;

// PARA CONTROLE DA PLANTA
String plantaNome; // NOME DAQUELA PLANTA
int plantaUmidade; // UMIDADE ADEQUADA PARA AQUELA PLANTA
float temperatura; // TEMPERATURA MEDIDA PELO SENSOR
int percentualUmidade; // UMIDADE MEDIDA PELO SENSOR

// PARA CONTROLE DO MOTOR
const int passosPorRevolucao = 2048; 

// ########### OBJETOS ###########

LiquidCrystal_I2C lcd(0x27, 20, 4); // INSTANCIA UM LCD I2C NO ENDEREÇO 0x27 COM TAMANHO 20x4 
Stepper motor(passosPorRevolucao, IN1, IN3, IN2, IN4); // INSTANCIA UM MOTOR COM PASSOS POR VOLTA E ENTRADAS
DHT dht(pinoDHT, DHT11); // INSTANCIA UM SENSOR DHT COM SEU PINO E SEU TIPO (DHT11)

// ############ FUNCOES ############

// INICIALIZA O DISPLAY
void inicializaDisplay() {
  lcd.init();
  lcd.backlight();
  lcd.clear(); 
  lcd.setCursor(0,0);
  lcd.print("  Starting LCD  "); 
  delay(1000); 
  lcd.clear();
}

// INICIALIZA A COMUNICACAO SERIAL
void inicializaSerial() {
  Serial.println("CONFIGURE SUAS PLANTAS POR AQUI...");
    lcd.setCursor(0,0);
    lcd.print("CONFIGURACAO VIA");
    lcd.setCursor(0,1);
    lcd.print("     SERIAL     "); 
}

// RESPONSAVEL PELA LEITURA DOS BOTOES
void tecladoBotoes(){
 
 if(!digitalRead(buttonRight) && menuControle == 1){
    delay(100);
    if(menuPrincipal < menuMax){
      menuPrincipal++;
    } 
  } 
    
  if(!digitalRead(buttonLeft) && menuControle == 1){
    delay(100);
    if(menuPrincipal > 1){
      menuPrincipal--; 
    } 
  } 
    
  if(!digitalRead(buttonEnter)){
    delay(100);
    if(subMenu < subMenuMax){
      subMenu++; 
    } 
  } 
  
  if(!digitalRead(buttonBack)){
    delay(100);
    if(subMenu > 1) {
      subMenu--; 
    } 
  } 
  
} 


// DESTINADAS A EXECUCAO DOS MENUS (3 MENUS)
void menu1() {
  switch(subMenu){
    case 1:
      lcd.setCursor(0, 0);
      lcd.print("    Planta 1   >");
      lcd.setCursor(0,1);
      lcd.print("                ");
      break;
    case 2:
      inicializaSerial();
      proximoEstado = estado2;
      break;
  } 
  delay(100);
}

void menu2() {
  switch(subMenu){
    case 1:
      lcd.setCursor(0, 0);
      lcd.print("<   Planta 2   >");
      lcd.setCursor(0,1);
      lcd.print("                ");
      break;
    case 2:
      inicializaSerial();
      proximoEstado = estado2;
      break;
  } 
  delay(100);
}

void menu3() {
  switch(subMenu){
    case 1:
      lcd.setCursor(0, 0);
      lcd.print("<   Planta 3    ");
      lcd.setCursor(0,1);
      lcd.print("                ");
      break;
    case 2:
      inicializaSerial();
      proximoEstado = estado2;
      break;
  }
  delay(100);
}

// TRAVA O CODIGO PARA ESPERAR A SERIAL
void esperaSerial (){
  while(Serial.available() == 0) retornaDisplay();
}

// ATRIBUIR VALORES AS PLANTAS NA SERIAL
void atribuiValoresPlanta(){
    Serial.println("Entre com o nome da planta a ser cultivada");
    esperaSerial();
    plantaNome = Serial.readString();
    Serial.println("Planta selecionada. Agora entre com a umidade do solo adequada");
    esperaSerial();
    plantaUmidade = Serial.parseInt();
    Serial.println("Agora retorne para o display");
}

// FAZER LEITURA E CONVERSAO DOS VALORES NAS PORTAS ANALOGICAS DOS SENSORES
void leituraConversaoSensores() {
  temperatura = dht.readTemperature(); // A BIBLIOTECA DHT CONVERTE OS VALORES INTERNAMENTE
  percentualUmidade = map(analogRead(pinoHigrometro), 270, 1023, 100, 0);
}

// VERIFICAR SE É NECESSÁRIO OU NÃO IRRIGAR O SOLO
int verificaSeIrrigaSolo() {
  if(percentualUmidade > plantaUmidade){
    // SE NAO EXECUTAR, DEVE MOSTRAR NO DISPLAY AS INFORMACOES
      lcd.setCursor(0,0);
      lcd.print(plantaNome);
      lcd.print("          ");
      lcd.setCursor(0,1);
      lcd.print(percentualUmidade);
      lcd.print("%   ");
      lcd.print(temperatura);
      lcd.print("C");
      return 0;
    }
    else {
    // SE EXECUTAR DEVE FUNCIONAR O MOTOR
      lcd.setCursor(0,0);
      lcd.print("MOTOR IRRIGANDO");
      lcd.setCursor(0,1);
      lcd.print("                  ");
      return 1;
    }
}

// RETORNAR AO ESTADO INICIAL CASO APERTE BOTAO BACK
void retornaDisplay(){
  if(!digitalRead(buttonBack)){
      proximoEstado = estado1;
    }
}

// FUNCAO QUE EXECUTA O GIRO 360 GRAUS (STEP MOTOR)
void executaMotor() {
    ligaRele();
    delay(1000);
    motor.step(passosPorRevolucao);
    delay(1000);
    motor.step(-passosPorRevolucao);
    delay(1000);
    desligaRele();
}

// FUNCAO QUE LIGA O RELE PARA ACIONAR A BOMBA D'AGUA
void ligaRele() {
  digitalWrite(pinoRele, HIGH);
  delay(250);
}

// FUNCAO QUE DESLIGA O RELE PARA DESLIGAR A BOMBA D'AGUA
void desligaRele() {
  digitalWrite(pinoRele, LOW);
}

// ############ EXECUCAO #############

void setup() {  
  // DEFININDO OS PINOS QUE SERAO UTILIZADOS
  pinMode(buttonRight, INPUT_PULLUP);
  pinMode(buttonLeft, INPUT_PULLUP);
  pinMode(buttonEnter, INPUT_PULLUP);
  pinMode(buttonBack, INPUT_PULLUP);
  pinMode(pinoHigrometro, INPUT);
  pinMode(pinoDHT, INPUT);
  pinMode(pinoRele, OUTPUT);

  // INICIANDO O DISPLAY E DHT
  inicializaDisplay();
  dht.begin();

  motor.setSpeed(8);

  // SETANDO A LEITURA DA SERIAL PARA 50 PARA LER MAIS RAPIDO AS STRINGS (PARA NAO AFETAR TANTO O MILLIS)
  Serial.setTimeout(50);
  Serial.begin(9600);
}

void loop() {
  
  // ESTADO 1 É RESPONSAVEL POR FAZER AS INTERAÇÕES ENTRE OS MENUS
  if(estadoAtual == estado1) {
    tecladoBotoes();
    
    switch(menuPrincipal){
    case 1:
      menu1();
      break;
    case 2:
      menu2();
      break;
    case 3:
      menu3();
      break;
    }
  }

  // ESTADO 2 É RESPONSAVEL PELA COMUNICACAO SERIAL PARA COLOCAR OS VALORES NA PLANTA SELECIONADA
  if(estadoAtual == estado2) {
    atribuiValoresPlanta();
    proximoEstado = estado3;
  }

  // ESTADO 3 É RESPONSAVEL PELOS SENSORES E PELA VERIFICACAO SE DEVE IRRIGAR A PLANTA OU NAO (PARA NESTE ESTADO ATE QUE APERTE BACK OU LIGUE O MOTOR)
  if(estadoAtual == estado3) {
   leituraConversaoSensores();
   if(verificaSeIrrigaSolo()){
    proximoEstado = estado4;
   } else {
    proximoEstado = estado3;
   }
   retornaDisplay();
  }

  // ESTADO 4 É RESPONSAVEL PELO MOTOR E PELA BOMBA D'AGUA, DEVE RETORNAR AO ESTADO 3 SEMPRE
  if(estadoAtual == estado4){
    executaMotor(); // INTERNAMENTE EXECUTA O RELE
    proximoEstado = estado3;
  }
  
  estadoAtual = proximoEstado;
}
