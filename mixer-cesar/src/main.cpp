#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "uaifai-apolo"
#define WIFI_PASSWORD "bemvindoaocesar"

/* 2. If work with RTDB, define the RTDB URL and database secret */
#define DATABASE_URL "URL" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
#define DATABASE_SECRET "DATABASE_SECRET"

/* 3. Define the Firebase Data object */
FirebaseData fbdo;

/* 4, Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;

/* Define the FirebaseConfig data for config data */
FirebaseConfig config;

unsigned long dataMillis = 0;
int count = Firebase.getInt(fbdo, F("/test/quantidade/int"));
#define  D0  // Escolha o pino GPIO que deseja usar para o LED, por exemplo, GPIO2

// Defina os pinos para os LEDs de cada bebida
#define WHISKEY_LED_PIN 12
#define ENERGY_LED_PIN 14
#define GIN_LED_PIN 27
#define FINISH_BUTTON 32


void setup()
{

    Serial.println("Hello world!");

    Serial.begin(9600);

    // Configura os pinos GPIO como saída para LEDs de bebida
    pinMode(WHISKEY_LED_PIN, OUTPUT);
    pinMode(ENERGY_LED_PIN, OUTPUT);
    pinMode(GIN_LED_PIN, OUTPUT);
    pinMode(FINISH_BUTTON, INPUT_PULLUP);


    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(WiFi.status());
        Serial.print("\n");
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        delay(3000);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the certificate file (optional) */
    config.cert.file = "/cert.cer";
    config.cert.file_storage = StorageType::FLASH;

    /* Assign the database URL and database secret(required) */
    config.database_url = "https://mixer-sd-cesar-default-rtdb.firebaseio.com/";
    config.signer.tokens.legacy_token = "WWBrMqQ4xIYr9tQQ4WAjhsr0RNrocMBcOLTKoblr";

    // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
    Firebase.reconnectNetwork(true);

    // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
    // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
    fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

    /* Initialize the library with the Firebase authen and config */
    Firebase.begin(&config, &auth);
}

// void addToQueue(const String &data)
// {
//     FirebaseJson json;
//     json.set("valor", data);

//     Firebase.pushJSON(FirebaseData(), DATABASE_URL, QUEUE_PATH, json);
// }


void serveDrink(int drinkType) {
  // Desliga todos os LEDs
  digitalWrite(WHISKEY_LED_PIN, LOW);
  digitalWrite(ENERGY_LED_PIN, LOW);
  digitalWrite(GIN_LED_PIN, LOW);
//   digitalWrite(TONIC_LED_PIN, LOW);
//   digitalWrite(WATER_LED_PIN, LOW);

  // Acende o LED correspondente ao tipo de bebida escolhido
  switch (drinkType) {
    case 1:
      digitalWrite(WHISKEY_LED_PIN, HIGH);
      Serial.println("Servindo Whiskey");
      break;
    case 2:
      digitalWrite(ENERGY_LED_PIN, HIGH);
      Serial.println("Servindo Energético");
      break;
    case 3:
      digitalWrite(GIN_LED_PIN, HIGH);
      Serial.println("Servindo Gin");
      break;
    default:
      Serial.println("Escolha inválida");
  }
}

void loop()
{
    Serial.println("Escolha a bebida (1-Whiskey, 2-Energético, 3-Gin, 4-Visualizar quantidade na fila):");
    while (Serial.available() == 0) {
    delay(100);
    }

    int drinkType = Serial.parseInt();

    if (drinkType >= 1 && drinkType <= 3) {
      serveDrink(drinkType);
      count = count + 1;
      Serial.printf("Inserindo +1 uma pedido da fila %s\n", Firebase.pushInt(fbdo, F("/test/pedido"), drinkType)? "ok" : fbdo.errorReason().c_str());
      Serial.printf("Tamanho da fila aumentou... %s\n", Firebase.setInt(fbdo, F("/test/quantidade/int"), count) ? "+1" : fbdo.errorReason().c_str());
    } else if(drinkType == 4){
        Serial.printf("Quantidade na fila... %s\n", Firebase.getInt(fbdo, F("/test/quantidade/int")) ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
    } else {
      Serial.println("Escolha inválida");
    }

    //Serial.println(digitalRead(FINISH_BUTTON));
    while(digitalRead(FINISH_BUTTON) == LOW){
      count = count--;
      Serial.printf("Tamanho da fila diminuiu... %s\n", Firebase.setInt(fbdo, F("/test/quantidade/int"), count) ? "-1" : fbdo.errorReason().c_str());
      Serial.println("Botão pressionado!");
      delay(1000);
      break;
    }

    delay(1000);  // Aguarda um segundo antes de aceitar outra escolha
}


