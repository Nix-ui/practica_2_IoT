import socket
import threading
import struct

class ESPCRUDProtocol:
    # Códigos de operación (1 byte)
    CMD_POST = 0x01
    CMD_GET = 0x02
    CMD_UPDATE = 0x03
    CMD_DELETE = 0x04
    
    # Tipos de recurso (1 byte)
    RES_SENSOR = 0x10
    RES_LED = 0x11
    
    # Estados de respuesta (1 byte)
    STATUS_OK = 0x20
    STATUS_ERROR = 0x21
    STATUS_NOT_FOUND = 0x22

    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.data_store = {
            'sensors': {},
            'leds': {}
        }
        self.lock = threading.Lock()
        self.running = False

    def _handle_packet(self, conn):
        """Procesa paquetes binarios con estructura:
        [1:CMD][1:RES][2:id][4:data_len][data]"""
        try:
            header = conn.recv(8)
            if len(header) != 8:
                return None
            
            cmd, res, dev_id, data_len = struct.unpack('!BBHL', header)
            data = conn.recv(data_len) if data_len > 0 else b''
            
            return self._process_command(cmd, res, dev_id, data)
            
        except Exception as e:
            print(f"Error procesando paquete: {e}")
            return struct.pack('!B', self.STATUS_ERROR)

    def _process_command(self, cmd, res, dev_id, data):
        """Ejecuta la operación CRUD correspondiente"""
        resource = 'sensors' if res == self.RES_SENSOR else 'leds'
        
        with self.lock:
            try:
                if cmd == self.CMD_POST:
                    self.data_store[resource][dev_id] = data
                    return struct.pack('!B', self.STATUS_OK)
                    
                elif cmd == self.CMD_GET:
                    value = self.data_store[resource].get(dev_id, b'')
                    return struct.pack(f'!BB{len(value)}s', 
                                     self.STATUS_OK, len(value), value)
                    
                elif cmd == self.CMD_UPDATE:
                    if dev_id in self.data_store[resource]:
                        self.data_store[resource][dev_id] = data
                        return struct.pack('!B', self.STATUS_OK)
                    return struct.pack('!B', self.STATUS_NOT_FOUND)
                    
                elif cmd == self.CMD_DELETE:
                    if dev_id in self.data_store[resource]:
                        del self.data_store[resource][dev_id]
                        return struct.pack('!B', self.STATUS_OK)
                    return struct.pack('!B', self.STATUS_NOT_FOUND)
                    
                else:
                    return struct.pack('!B', self.STATUS_ERROR)
                    
            except Exception as e:
                print(f"Error en operación: {e}")
                return struct.pack('!B', self.STATUS_ERROR)

    def _client_handler(self, conn, addr):
        """Maneja la conexión con un ESP32"""
        print(f"[+] Conexión desde {addr}")
        try:
            while self.running:
                response = self._handle_packet(conn)
                if not response:
                    break
                conn.sendall(response)
        finally:
            conn.close()
            print(f"[-] {addr} desconectado")

    def start(self):
        """Inicia el servidor"""
        self.running = True
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.bind((self.host, self.port))
            s.listen()
            print(f"Servidor CRUD para ESP32 en {self.host}:{self.port}")
            
            try:
                while self.running:
                    conn, addr = s.accept()
                    thread = threading.Thread(
                        target=self._client_handler,
                        args=(conn, addr),
                        daemon=True
                    )
                    thread.start()
            except KeyboardInterrupt:
                print("\nDeteniendo servidor...")
            finally:
                self.running = False
                s.close()

if __name__ == "__main__":
    server = ESPCRUDProtocol('0.0.0.0', 12345)
    server.start()