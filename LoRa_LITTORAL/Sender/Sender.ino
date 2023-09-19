// ---------- Importações das bibliotecas
#include <SPI.h>  //Resposável pela comunicação Serial do LoRa
#include <LoRa.h>  //Resposável pela comunicação com o WiFi LoRa
#include <Wire.h>  //Resposável pela comunicação i2c
#include <SSD1306.h>  //Resposável pela comunicação com o display

// ---------- Define os pinos do WiFi LoRa
#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define DI00    26   // GPIO26 -- SX127x's IRQ(Interrupt Request)

// ---------- Define a frequência do LoRa
#define BAND    915E6  //Define a frequência do LoRa. 433E6 é a mesma coisa de 433000000MHz. Você também pode usar 868E6 e 915E6.
#define FREQUENCY 915E6 // 868E6 e 915E6

// ---------- Define as configurações do LoRa
#define SF 12 // 7 8 9 10 11 12
#define BANDWIDTH 125 //125 250 500
#define CODINGRATE 1 //1 2 3 4
#define SYNCKWORD 0X5E //0X00 ATE 0XFF

// ---------- Defini variáveis globais para tempo e contador
unsigned long tempoinicial, tempofinal;
unsigned int counter = 1;  //Declara a variável que irá receber incrementos e enviará os dados para o outro LoRa

bool mensagemRecebida = false;
unsigned long tempoInicioEnvio;

// ---------- Configuração do display
SSD1306 display(0x3c, 4, 15);  //Define o endereço do i2c do Oled(0x3c) e os pinos SDA(4) e SCL(15) do ESP32

// ---------- Declaração das variáveis que iremos utilizar
String rssi = "RSSI --";  //Declara a variável que receberá a potência do sinal
String packSize = "--";  //Declara a variável que receberá o tamanho dos pacotes enviados
String packet;  //Declara a variável do pacote
int contador = 0;

// ---------- Função de inicialização e configuração do ArduinoIDE
void setup() {
  // ---------- Definição dos pinos
  pinMode(16, OUTPUT); //Define o pino Reset do Oled
  pinMode(25, OUTPUT); //Define o pino do led da placa
  digitalWrite(16, LOW);  //Coloca o pino em Low para dar um Reset no Oled
  delay(50);
  digitalWrite(16, HIGH); //Coloca em High para o Oled voltar a funcionar

  // ---------- Inicializando a OLED (tela)
  display.init();  //Inicializa o Oled
  display.flipScreenVertically();  //Vira o Display para a vertical
  display.setFont(ArialMT_Plain_10);  //Define o tipo e tamanho da fonte
  delay(1500);
  display.clear();  //Limpa a tela

  // ---------- Inicializando a comunicação Serial do ESP com o LoRa
  SPI.begin(SCK, MISO, MOSI, SS); //Inicializa a comunicação Serial com o LoRa
  LoRa.setPins(SS, RST, DI00); //Define os pinos que serão utilizados pelo LoRa

  // ---------- Configurações do LoRa
  LoRa.setFrequency(FREQUENCY);
  LoRa.setSpreadingFactor(SF);
  LoRa.setSignalBandwidth(BANDWIDTH);
  LoRa.setCodingRate4(CODINGRATE);
  LoRa.setSyncWord(SYNCKWORD);

  // ---------- Verificando se o LoRa foi inicializado
  if (!LoRa.begin(BAND)) { //Verifica se o LoRa foi iniciado com sussesso
    display.drawString(0, 0, "Falha ao iniciar o LoRa!");  //Seta o X e Y de onde irá imprimir o texto a seguir
    display.display();  //Imprime o texto
    while (1);  //Entra em um While e a execução do programa morre aqui
  }

  // ---------- Exibi na OLED que foi inicializado
  display.drawString(0, 0, "Iniciado com sussesso!");
  display.display();
  delay(1000);

  LoRa.receive();

  // ---------- Habilita a comunicação Serial com o ArduinoIDE (computador)
  Serial.begin(115200);

  contador = 0;
}

void loop() {
  // Define a mensagem a ser enviada
  String message = "pacote: "+String(contador);

  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();

  contador++;

  display.clear();
  display.drawString(0, 0, "Mensagem: " + message);
  display.drawString(0, 16, "Enviei: " + String(contador) + " mensagens");
  display.display();
  delay(500);

}

  
/*
void loop() {
  // Define a mensagem a ser enviada
  String message = "Pacote: "+contador;

  // Inicia o envio da mensagem LoRa
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();

  // Registra o tempo de início do envio
  tempoInicioEnvio = millis();

  // Limpa o display OLED e exibe a mensagem enviada
  display.clear();
  display.drawString(0, 0, "Mensagem: " + message);
  display.display();

  // Aguarda a confirmação do receiver ou a expiração de 10 segundos
  while (!mensagemRecebida && (millis() - tempoInicioEnvio < 1000)) {
    if (LoRa.parsePacket()) {
      String response = "";
      while (LoRa.available()) {
        response += (char)LoRa.read();
      }
      if (response == "Confirmacao") {
        mensagemRecebida = true;
        display.clear();
        display.drawString(0, 0, "Mensagem: " + message);
        display.drawString(0, 16, "Recebido: Sim");
        display.display();
        Serial.println("Mensagem recebida pelo receiver.");
        break;
      }
    }
  }

  // Verifica se a mensagem não chegou em 10 segundos
  if (!mensagemRecebida) {
    display.clear();
    display.drawString(0, 0, "Mensagem: " + message);
    display.drawString(0, 16, "Recebido: Nao");
    display.display();
    Serial.println("A mensagem não chegou.");
  }
  contador++;
  // Aguarda antes de enviar outra mensagem
  delay(1000); // Espere 5 segundos antes de enviar outra mensagem
}
*/
