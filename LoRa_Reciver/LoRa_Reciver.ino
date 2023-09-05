#include <SPI.h>  //Resposável pela comunicação Serial do LoRa
#include <LoRa.h>  //Resposável pela comunicação com o WiFi LoRa
#include <Wire.h>  //Resposável pela comunicação i2c
#include "SSD1306.h"  //Resposável pela comunicação com o display
#include "images.h"  //Arquivo com a logo da HelTec

// Define os pinos do WiFi LoRa
#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define DI00    26   // GPIO26 -- SX127x's IRQ(Interrupt Request)

#define BAND    915E6  //Define a frequência do LoRa. 433E6 é a mesma coisa de 433000000MHz. Você também pode usar 868E6 e 915E6.
#define PABOOST true  //Sem conhecimento dessa variavel mas ela deve aparecer para funcionar

#define FREQUENCY 915E6 // 868E6 e 915E6
#define SF 7 // 6 7 8 9 11 12
#define BANDWIDTH 125 //125 250 500
#define CODINGRATE 1 //1 2 3 4
#define SYNCKWORD 0X5E //0X00 ATE 0XFF

SSD1306 display(0x3c, 4, 15);  //Define o endereço do i2c do Oled(0x3c) e os pinos SDA(4) e SCL(15) do ESP32

String rssi = "RSSI --";  //Declara a variável que receberá a potência do sinal
String packSize = "--";  //Declara a variável que receberá o tamanho dos pacotes enviados
String packet;  //Declara a variável do pacote

void setup() {
  pinMode(16, OUTPUT); //Define o pino Reset do Oled
  digitalWrite(16, LOW);  //Coloca o pino em Low para dar um Reset no Oled
  delay(50);
  digitalWrite(16, HIGH); //Coloca em High para o Oled voltar a funcionar

  display.init();  //Inicializa o Oled
  display.flipScreenVertically();  //Vira o Display para a vertical
  display.setFont(ArialMT_Plain_10);  //Define o tipo e tamanho da fonte
  logo();  //Chama a função de mostrar a Logo
  delay(1500);
  display.clear();  //Limpa a tela

  SPI.begin(SCK, MISO, MOSI, SS); //Inicializa a comunicação Serial com o LoRa
  LoRa.setPins(SS, RST, DI00); //Define os pinos que serão utilizados pelo LoRa

  // CONFIGURAÇÃO ARTIGO
  LoRa.setFrequency(FREQUENCY);
  LoRa.setSpreadingFactor(SF);
  LoRa.setSignalBandwidth(BANDWIDTH);
  LoRa.setCodingRate4(CODINGRATE);
  LoRa.setSyncWord(SYNCKWORD);

  if (!LoRa.begin(BAND)) { //Verifica se o LoRa foi iniciado com sussesso
    display.drawString(0, 0, "Falha ao iniciar o LoRa!");  //Seta o X e Y de onde irá imprimir o texto a seguir
    display.display();  //Imprime o texto
    while (1);  //Entra em um While e a execução do programa morre aqui
  }
  display.drawString(0, 0, "Iniciado com sussesso!");
  display.drawString(0, 10, "Esperando por dados...");
  display.display();
  delay(1000);
  //LoRa.onReceive(cbk);
  LoRa.receive();  //Habilita o LoRa para receber dados
}

void loop() {
  int packetSize = LoRa.parsePacket();  //Declara uma variável que recebe o comando LoRa.parsePacket que verifica se recebeu algum dado
  if (packetSize) {  //Verifica se a variável recebeu ou não dados, se recebeu faz o comando a seguir
    cbk(packetSize);  //Chama a função para tratar os dados recebidos
  }
  delay(10);
}

void logo() {
  display.clear();
  //display.drawXbm(0, 5, logo_width, logo_height, logo_bits); //Seta o X e Y de onde irá imprimir. Seta a largura e altura que será utilizado. Imprime uma imagem em hexa.
  display.display();
}

void cbk(int packetSize) {
  packet = "";  //
  packSize = String(packetSize, DEC);  //Converte o valor da variável em quantidade de bytes recebidos
  for (int i = 0; i < packetSize; i++) {  //Executa o código abaixo até que todos os bytes recebidos foram tratados
    packet += (char) LoRa.read();  //Trata byte por byte ate formar o dado enviado
  }
  rssi = "RSSI " + String(LoRa.packetRssi(), DEC) ;  //Converte o valor da potência do sinal e junta a palavra "RSSI"
  //loraData(packetSize);  //Chama a função que imprime os dados no Oled

  LoRa.beginPacket();
  LoRa.print(rssi);
  LoRa.endPacket();

  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);  //Alinha o texto a Esquerda
  display.setFont(ArialMT_Plain_10);  //Define o tipo e tamanho da fonte
  display.drawString(0, 0, rssi);  //Imprime a variável com a potência do sinal
  display.drawString(0 , 15 , "Received " + packSize + " bytes");  //Imprime o tamanho do pacote enviado
  display.drawStringMaxWidth(0 , 26 , 128, packet);  //Imprime os dados enviado do outro LoRa
  display.display();
}

void loraData(int packetSize) {
  String pacote = "";

  if (packet == "hello") {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);  //Alinha o texto a Esquerda
    display.setFont(ArialMT_Plain_10);  //Define o tipo e tamanho da fonte
    display.drawString(0, 0, rssi);  //Imprime a variável com a potência do sinal
    display.drawString(0 , 15 , "Received " + packSize + " bytes");  //Imprime o tamanho do pacote enviado
    display.drawStringMaxWidth(0 , 26 , 128, "Bomba desligada");  //Imprime os dados enviado do outro LoRa
    display.display();
    digitalWrite(25, LOW);

    LoRa.beginPacket();  //Inicia um pacote
    pacote = "RSSI: " + rssi;
    LoRa.print(pacote);  //Envia a seguinte palavra
    LoRa.endPacket();  //Fecha o pacote e envia
  }

}
