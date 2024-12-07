#include <SoftwareSerial.h>

#define TX_PIN 10  // Pin for sending data
#define RX_PIN 11  // Pin for receiving data
#define NODE_ID 3  // ID aktualnego urządzenia

SoftwareSerial nodeSerial(RX_PIN, TX_PIN);

// Funkcja obliczająca CRC-8
uint8_t calculateCRC8(uint8_t *data, int length) {
    uint8_t crc = 0x00;
    for (int i = 0; i < length; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

// Funkcja do odbierania i przetwarzania ramki
void processFrame() {
    if (nodeSerial.available() >= 8) {  // Ramka musi mieć co najmniej 8 bajtów
        uint8_t frame[8];
        for (int i = 0; i < 8; i++) {
            frame[i] = nodeSerial.read();
        }

        // Weryfikacja poprawności checksum
        uint8_t receivedChecksum = frame[7];
        uint8_t calculatedChecksum = calculateCRC8(frame, 7);

        if (receivedChecksum != calculatedChecksum) {
            Serial.println("Błąd: Nieprawidłowa suma kontrolna!");
            return;  // Odrzuć ramkę
        }

        // Sprawdzenie, czy ramka jest skierowana do tego urządzenia
        uint8_t dst = frame[2];  // Pole DST (bajt 2)
        if (dst != NODE_ID) {
            Serial.println("Ramka nie jest skierowana do tego urządzenia.");
            return;  // Ignoruj ramkę
        }

        // Dodanie swojego ID do pola wezłów
        frame[6] |= (1 << (NODE_ID - 1));  // Ustawienie odpowiedniego bitu w polu wezłów

        // Aktualizacja pola SRC na aktualny węzeł
        frame[1] = NODE_ID;

        // Obliczenie nowej checksum i dodanie jej do ramki
        frame[7] = calculateCRC8(frame, 7);

        // Wysłanie zaktualizowanej ramki dalej
        nodeSerial.write(frame, 8);

        // Debug: wyświetlenie przesłanej ramki
        Serial.println("Przesłano zaktualizowaną ramkę:");
        for (int i = 0; i < 8; i++) {
            Serial.print(frame[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    }
}

void setup() {
    Serial.begin(9600);      // Debugging via USB
    nodeSerial.begin(9600);  // Komunikacja z następnym węzłem
    Serial.println("Urządzenie gotowe do pracy.");
}

void loop() {
    processFrame();  // Przetwarzanie przychodzących ramek
}
