// ---------- Importações das bibliotecas
#include <SPI.h>  //Resposável pela comunicação Serial do LoRa
#include <LoRa.h>  //Resposável pela comunicação com o WiFi LoRa
#include <Wire.h>  //Resposável pela comunicação i2c
#include <SSD1306.h>  //Resposável pela comunicação com o display
#include "images.h"  //Arquivo com a logo da HelTec

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

// ---------- Configuração do display
SSD1306 display(0x3c, 4, 15);  //Define o endereço do i2c do Oled(0x3c) e os pinos SDA(4) e SCL(15) do ESP32

// ---------- Declaração das variáveis que iremos utilizar
String rssi = "RSSI --";  //Declara a variável que receberá a potência do sinal
String snr = "SNR --";  //Declara a variável que receberá a potência do sinal
String packSize = "--";  //Declara a variável que receberá o tamanho dos pacotes enviados
String packet;  //Declara a variável do pacote

// ---------- Função de inicialização e configuração do ArduinoIDE
void setup() {
  // ---------- Definição dos pinos
  pinMode(16, OUTPUT); //Define o pino Reset do Oled
  digitalWrite(16, LOW);  //Coloca o pino em Low para dar um Reset no Oled
  delay(50);
  digitalWrite(16, HIGH); //Coloca em High para o Oled voltar a funcionar

  // ---------- Inicializando a OLED (tela)
  display.init();  //Inicializa o Oled
  display.flipScreenVertically();  //Vira o Display para a vertical
  display.setFont(ArialMT_Plain_10);  //Define o tipo e tamanho da fonte
  logo();  //Chama a função de mostrar a Logo
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
  display.drawString(0, 10, "Esperando por dados...");
  display.display();
  delay(1000);

  // ---------- Habilita o LoRa para receber dados
  //LoRa.onReceive(cbk);
  LoRa.receive();  //Habilita o LoRa para receber dados
}

// ---------- Função de Loop Infinito do ArduinoIDE
void loop() {
  // ---------- Criação da variável que vai receber o comando do LoRa para verificar se recebeu algum dado
  int packetSize = LoRa.parsePacket();  //Declara uma variável que recebe o comando LoRa.parsePacket que verifica se recebeu algum dado

  // ---------- Se recebeu mensagem, chama a função CBK
  if (packetSize) {  //Verifica se a variável recebeu ou não dados, se recebeu faz o comando a seguir
    cbk(packetSize);  //Chama a função para tratar os dados recebidos
  }

  // ---------- Da um delay de 10 millesegundos
  delay(10);
}

// ---------- Função para desenhar a logo na OLED
void logo() {
  display.clear();
  //display.drawXbm(0, 5, logo_width, logo_height, logo_bits); //Seta o X e Y de onde irá imprimir. Seta a largura e altura que será utilizado. Imprime uma imagem em hexa.
  display.display();
}

// ---------- Função que recebe o pacote, trata os dados e exibe na tela
void cbk(int packetSize) {

  // ---------- Declara a variável do pacote
  packet = "";  //

  // ---------- Declara a variável para receber o conteudo convertendo de decimal para bytes
  packSize = String(packetSize, DEC);  //Converte o valor da variável em quantidade de bytes recebidos

  // ---------- Faz um for em cada BYTE recebido, converte para CHAR e vai concatenando em PACKET
  for (int i = 0; i < packetSize; i++) {  //Executa o código abaixo até que todos os bytes recebidos foram tratados
    packet += (char) LoRa.read();  //Trata byte por byte ate formar o dado enviado
  }

  // ---------- A variável rssi recebe o valor do RSSI lido nesta comunicação
  rssi = "RSSI " + String(LoRa.packetRssi(), DEC) ;  //Converte o valor da potência do sinal e junta a palavra "RSSI"
  snr = "SNR " + String(LoRa.packetSnr(), DEC) ;
  
  // ---------- Chama a função para ligar/desligar uma bomba utilizando o LoRa
  loraData(packetSize);  //Chama a função que imprime os dados no Oled

  // ---------- Abre a comunicação
  LoRa.beginPacket();

  // ---------- Manda o RSSI de volta para o Node
  LoRa.print(rssi);

  // ---------- Fecha a comunicação
  LoRa.endPacket();
  
  // ---------- No OLED é exibido o RSSI e o tamanho do pacote recebido
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);  //Alinha o texto a Esquerda
  display.setFont(ArialMT_Plain_10);  //Define o tipo e tamanho da fonte
  display.drawString(0, 0, rssi);  //Imprime a variável com a potência do sinal
  display.drawString(0 , 15 , "Received " + packSize + " bytes");  //Imprime o tamanho do pacote enviado
  display.drawStringMaxWidth(0 , 26 , 128, packet);  //Imprime os dados enviado do outro LoRa
  display.drawString(0, 36, snr);  //Imprime a variável com a potência do sinal
  display.display();
}

// ---------- Função para ligar/desligar uma bomba utilizando o LoRa
void loraData(int packetSize) {

  // ---------- Declara a variável de pacote
  String pacote = "";

  // ---------- Se o pacote de dados for "desligar"
  if (packet == "desligar") {

    // ---------- exibe na tela: "Recebido, bomba desligada"
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);  //Alinha o texto a Esquerda
    display.setFont(ArialMT_Plain_10);  //Define o tipo e tamanho da fonte
    display.drawString(0, 0, rssi);  //Imprime a variável com a potência do sinal
    display.drawString(0 , 15 , "Received " + packSize + " bytes");  //Imprime o tamanho do pacote enviado
    display.drawStringMaxWidth(0 , 26 , 128, "Bomba desligada");  //Imprime os dados enviado do outro LoRa
    display.drawString(0, 36, snr);
    display.display();
    digitalWrite(25, LOW);

    // ---------- Inicia um novo pacote
    LoRa.beginPacket();  //Inicia um pacote

    // ---------- Coleta o RSSI
    pacote = "RSSI: " + rssi;

    // ---------- Manda o RSSI de volta para o node
    LoRa.print(pacote);  //Envia a seguinte palavra

    // ---------- Fecha o pacote
    LoRa.endPacket();  //Fecha o pacote e envia
  } else if (packet == "ligar") {

    // ---------- exibe na tela: "Recebido, bomba desligada"
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);  //Alinha o texto a Esquerda
    display.setFont(ArialMT_Plain_10);  //Define o tipo e tamanho da fonte
    display.drawString(0, 0, rssi);  //Imprime a variável com a potência do sinal
    display.drawString(0 , 15 , "Received " + packSize + " bytes");  //Imprime o tamanho do pacote enviado
    display.drawStringMaxWidth(0 , 26 , 128, "Bomba ligada");  //Imprime os dados enviado do outro LoRa
    display.drawString(0, 36, snr);
    display.display();
    digitalWrite(25, HIGH);

    // ---------- Inicia um novo pacote
    LoRa.beginPacket();  //Inicia um pacote

    // ---------- Coleta o RSSI
    pacote = "RSSI: " + rssi;

    // ---------- Manda o RSSI de volta para o node
    LoRa.print(pacote);  //Envia a seguinte palavra

    // ---------- Fecha o pacote
    LoRa.endPacket();  //Fecha o pacote e envia
  }

}
