#pragma once
#include <WiFi.h>
#include <WiFiClient.h>

class ServerClient {
public:
    enum Command {
        CMD_POST = 0x01,
        CMD_GET = 0x02,
        CMD_UPDATE = 0x03,
        CMD_DELETE = 0x04
    };

    enum Resource {
        RES_SENSOR = 0x10,
        RES_LED = 0x11
    };

    enum Status {
        STATUS_OK = 0x20,
        STATUS_ERROR = 0x21,
        STATUS_NOT_FOUND = 0x22
    };

    ServerClient(const char* host, const uint16_t port) 
        : m_host(host), m_port(port), m_isConnected(false) {}

    bool wifiConnect(const char* ssid, const char* password) {
        Serial.print("Conectando a WiFi: ");
        Serial.println(ssid);
        WiFi.begin(ssid, password);
        
        for(int i = 0; i < 20; ++i) {
            if(WiFi.status() == WL_CONNECTED) {
                Serial.println("\nConexión WiFi exitosa");
                return true;
            }
            delay(500);
            Serial.print(".");
        }
        Serial.println("\nError conectando a WiFi");
        return false;
    }

    bool connectToServer() {
        Serial.print("Conectando al servidor...");
        m_isConnected = m_client.connect(m_host, m_port);
        
        if(m_isConnected) {
            Serial.println("OK");
            return true;
        }
        Serial.println("FALLÓ");
        return false;
    }

    bool post(Resource res, uint16_t dev_id, const uint8_t* data, uint32_t data_len) {
        return sendPacket(CMD_POST, res, dev_id, data, data_len);
    }

    bool get(Resource res, uint16_t dev_id) {
        return sendPacket(CMD_GET, res, dev_id, nullptr, 0);
    }

    bool update(Resource res, uint16_t dev_id, const uint8_t* data, uint32_t data_len) {
        return sendPacket(CMD_UPDATE, res, dev_id, data, data_len);
    }

    bool del(Resource res, uint16_t dev_id) {
        return sendPacket(CMD_DELETE, res, dev_id, nullptr, 0);
    }

    byte receiveStatus() {
        if(!m_client.available()) return 0xFF;
        return m_client.read();
    }

    bool receiveData(Resource expectedRes, uint16_t expectedDevId, char* buffer, size_t bufferSize) {
        if(!m_client.connected()) return false;

        byte status = receiveStatus();
        if(status != STATUS_OK) return false;

        byte res = m_client.read();
        uint16_t dev_id = (m_client.read() << 8) | m_client.read();
        uint32_t data_len = (uint32_t)m_client.read() << 24 |
                            (uint32_t)m_client.read() << 16 |
                            (uint32_t)m_client.read() << 8 |
                            m_client.read();

        if(res != expectedRes || dev_id != expectedDevId) {
            return false;
        }

        size_t bytesToRead = data_len < bufferSize ? data_len : bufferSize - 1;
        if(bytesToRead > 0) {
            m_client.readBytes(buffer, bytesToRead);
        }
        buffer[bytesToRead] = '\0';
        return true;
    }

    void disconnectFromServer() {
        m_client.stop();
        m_isConnected = false;
        Serial.println("Desconectado del servidor");
    }

    bool isConnected() { 
        return m_isConnected && m_client.connected(); 
    }

private:
    WiFiClient m_client;
    const char* m_host;
    const uint16_t m_port;
    bool m_isConnected;

    bool sendPacket(byte cmd, byte res, uint16_t dev_id, const uint8_t* data, uint32_t data_len) {
        if(!isConnected()) return false;

        uint8_t header[8];
        header[0] = cmd;
        header[1] = res;
        header[2] = (dev_id >> 8) & 0xFF;
        header[3] = dev_id & 0xFF;
        header[4] = (data_len >> 24) & 0xFF;
        header[5] = (data_len >> 16) & 0xFF;
        header[6] = (data_len >> 8) & 0xFF;
        header[7] = data_len & 0xFF;

        // Enviar header
        if(m_client.write(header, sizeof(header)) != sizeof(header)) {
            disconnectFromServer();
            return false;
        }

        // Enviar datos si existen
        if(data_len > 0 && m_client.write(data, data_len) != data_len) {
            disconnectFromServer();
            return false;
        }

        return true;
    }
};