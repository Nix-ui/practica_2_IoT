#include <WiFi.h>
#include <WiFiClient.h>

class ServerClient {
private:
    WiFiClient client;
    const char* host;
    const uint16_t port;
    bool isConnectedToServer = false;

    // Envía un paquete binario al servidor
    bool sendPacket(byte cmd, byte res, uint16_t dev_id, const uint8_t* data, uint32_t data_len) { // data_len ahora es uint32_t
        bool  success = false;
        if(isServerConnected()){
            uint8_t header[8]; // Encabezado de 8 bytes
            header[0] = cmd;
            header[1] = res;
            header[2] = (dev_id >> 8) & 0xFF;
            header[3] = dev_id & 0xFF;
            header[4] = (data_len >> 24) & 0xFF; // Byte más significativo de data_len
            header[5] = (data_len >> 16) & 0xFF;
            header[6] = (data_len >> 8) & 0xFF;
            header[7] = data_len & 0xFF;        // Byte menos significativo de data_len
            if(client.write(header,8) == 8){
                if (data_len > 0 && client.write(data, data_len) == data_len) {
                    success = true;
                } else if (data_len > 0) {
                    Serial.println("Error sending data.");
                    disconnectFromServer();
                } else {
                    success = true; // No hay datos para enviar
                }
            } else {
                Serial.println("Error sending header.");
                disconnectFromServer();
            }
        }else{
            Serial.println("Error: Not connected to server, cannot send packet.");
        }
        return success;
    }

    // Recibe un byte del servidor
    byte receiveByte() {
        byte response =  -1;
        if (client.available()) {
            response = client.read();
        }
        return response;
    }

public:
    // Códigos de operación (deben coincidir con el servidor)
    enum Command {
        CMD_POST = 0x01,
        CMD_GET = 0x02,
        CMD_UPDATE = 0x03,
        CMD_DELETE = 0x04
    };

    // Tipos de recurso (deben coincidir con el servidor)
    enum Resource {
        RES_SENSOR = 0x10,
        RES_LED = 0x11
    };

    // Estados de respuesta (deben coincidir con el servidor)
    enum Status {
        STATUS_OK = 0x20,
        STATUS_ERROR = 0x21,
        STATUS_NOT_FOUND = 0x22
    };

    ServerClient(const char* _host, const uint16_t _port) : host(_host), port(_port) {}

    bool wifiConnect(const char* ssid, const char* password) {
        Serial.print("Connecting to WiFi network: ");
        Serial.println(ssid);
        WiFi.begin(ssid, password);
        int attempts = 0;
        bool connected = false;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        Serial.println("");
        if (WiFi.status() == WL_CONNECTED) {
            Serial.print("Connected to WiFi.");
            connected =  true;
        }else {
            Serial.println("Failed to connect to WiFi.");
        }
        return connected;
    };

    bool connectToServer() {
        Serial.print("Connecting to server");
        bool response = false;
        if(client.connect(host,port)){
            Serial.println("Connected to server successfully.");
            isConnectedToServer = true;
            response = true;
        }else{
            Serial.println("Connection to server failed!");
            isConnectedToServer = false;
        }
        return response;
    }

    bool isServerConnected() {
        return client.connected() && isConnectedToServer;
    }

    // Envía una solicitud POST
    bool post(Resource res, uint16_t dev_id, const uint8_t* data, uint16_t data_len) {
        return sendPacket(Command::CMD_POST, res, dev_id, data, static_cast<uint32_t>(data_len)); // Castear a uint32_t
    }

    // Envía una solicitud GET
    bool get(Resource res, uint16_t dev_id) {
        return sendPacket(Command::CMD_GET, res, dev_id, nullptr, 0);
    }

    // Envía una solicitud UPDATE
    bool update(Resource res, uint16_t dev_id, const uint8_t* data, uint16_t data_len) {
        return sendPacket(Command::CMD_UPDATE, res, dev_id, data, static_cast<uint32_t>(data_len)); // Castear a uint32_t
    }

    // Envía una solicitud DELETE
    bool del(Resource res, uint16_t dev_id) {
        return sendPacket(Command::CMD_DELETE, res, dev_id, nullptr, 0);
    }

    // Recibe la respuesta del servidor (solo el byte de estado por ahora)
    byte receiveStatus() {
        byte response =  -1;
        if(isServerConnected()){
            response = receiveByte();
        }else{
            Serial.println("Error: Not connected to server, cannot receive status.");
        }
        return response;
    }

    // Recibe la respuesta GET con datos
    String receiveData(Resource expectedRes, uint16_t expectedDevId) {
        String response ="";
        if(isServerConnected()){
            int statusByte = receiveByte();
            if (statusByte == Status::STATUS_OK) {
                int resByte = receiveByte();
                uint32_t dataLen = 0;
                dataLen |= (static_cast<uint32_t>(receiveByte()) << 24);
                dataLen |= (static_cast<uint32_t>(receiveByte()) << 16);
                dataLen |= (static_cast<uint32_t>(receiveByte()) << 8);
                dataLen |= receiveByte();
                if (resByte == expectedRes) {
                    if (dataLen > 0) {
                        for (int i = 0; i < dataLen; i++) {
                            int byte = receiveByte();
                            if (byte != -1) {
                                response += (char)byte;
                            } else {
                                Serial.println("Error receiving data byte.");
                            }
                        }
                    }
                } else {
                    Serial.println("Error: Received data for unexpected resource.");
                }
            } else if (statusByte == Status::STATUS_NOT_FOUND) {
                Serial.println("Resource not found on server.");
            } else if (statusByte == Status::STATUS_ERROR) {
                Serial.println("Server returned an error.");
            } else if (statusByte != -1) {
                Serial.print("Received unexpected status: 0x");
                Serial.println(statusByte, HEX);
            }
        }
        return response;
    }

    void disconnectFromServer() {
        Serial.println("Disconnecting from server.");
        client.stop();
        isConnectedToServer = false;
    }
};