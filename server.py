import socket
import threading
import struct
from enum import IntEnum

class ESPCRUDProtocol:
    """
    Implementa el protocolo CRUD para dispositivos ESP32
    Estructura del paquete: [CMD(1B)|RES(1B)|DEV_ID(2B)|DATA_LEN(4B)|DATA(nB)]
    """
    
    class Command(IntEnum):
        POST = 0x01
        GET = 0x02
        UPDATE = 0x03
        DELETE = 0x04

    class Resource(IntEnum):
        SENSOR = 0x10
        LED = 0x11

    class Status(IntEnum):
        OK = 0x20
        ERROR = 0x21
        NOT_FOUND = 0x22

    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.data_store = {
            'sensors': {},
            'leds': {}
        }
        self.lock = threading.Lock()
        self.running = False
        self.server_socket = None

    def _handle_packet(self, conn):
        """
        Procesa un paquete completo recibido del cliente
        Retorna: bytes con la respuesta
        """
        try:
            # Leer header (8 bytes)
            header = conn.recv(8)
            if len(header) < 8:
                return None

            # Desempaquetar header
            cmd, res, dev_id, data_len = struct.unpack('!BBHL', header)
            
            # Leer datos si existen
            data = b''
            if data_len > 0:
                data = conn.recv(data_len)
                while len(data) < data_len:
                    packet = conn.recv(data_len - len(data))
                    if not packet:
                        break
                    data += packet

            return self._process_command(cmd, res, dev_id, data)

        except (struct.error, ConnectionResetError) as e:
            print(f"Error procesando paquete: {e}")
            return struct.pack('!B', self.Status.ERROR)

    def _process_command(self, cmd, res, dev_id, data):
        """
        Ejecuta la operación CRUD correspondiente
        Retorna: bytes con la respuesta estructurada
        """
        resource_map = {
            self.Resource.SENSOR: 'sensors',
            self.Resource.LED: 'leds'
        }
        
        resource_type = resource_map.get(res)
        if not resource_type:
            return struct.pack('!B', self.Status.ERROR)

        with self.lock:
            try:
                store = self.data_store[resource_type]
                
                if cmd == self.Command.POST:
                    store[dev_id] = data
                    print(f"[POST] {resource_type} ID {dev_id}: {data.hex()}")
                    return struct.pack('!B', self.Status.OK)

                elif cmd == self.Command.GET:
                    value = store.get(dev_id, b'')
                    header = struct.pack(
                        '!BBHL', 
                        self.Status.OK,
                        res,
                        dev_id,
                        len(value)
                    )
                    return header + value

                elif cmd == self.Command.UPDATE:
                    if dev_id in store:
                        store[dev_id] = data
                        print(f"[UPDATE] {resource_type} ID {dev_id}: {data.hex()}")
                        return struct.pack('!B', self.Status.OK)
                    return struct.pack('!B', self.Status.NOT_FOUND)

                elif cmd == self.Command.DELETE:
                    if dev_id in store:
                        del store[dev_id]
                        print(f"[DELETE] {resource_type} ID {dev_id}")
                        return struct.pack('!B', self.Status.OK)
                    return struct.pack('!B', self.Status.NOT_FOUND)

                else:
                    return struct.pack('!B', self.Status.ERROR)

            except Exception as e:
                print(f"Error en operación CRUD: {e}")
                return struct.pack('!B', self.Status.ERROR)

    def _client_handler(self, conn, addr):
        """
        Maneja la conexión con un cliente ESP32
        """
        print(f"[+] Conexión establecida: {addr}")
        try:
            while self.running:
                response = self._handle_packet(conn)
                if not response:
                    break
                conn.sendall(response)
        finally:
            conn.close()
            print(f"[-] Conexión cerrada: {addr}")

    def start(self):
        """
        Inicia el servidor y maneja conexiones entrantes
        """
        self.running = True
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        
        try:
            self.server_socket.bind((self.host, self.port))
            self.server_socket.listen(5)
            print(f"Servidor iniciado en {self.host}:{self.port}")
            
            while self.running:
                conn, addr = self.server_socket.accept()
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
            if self.server_socket:
                self.server_socket.close()
            print("Servidor detenido")

if __name__ == "__main__":
    server = ESPCRUDProtocol('0.0.0.0', 12345)
    server.start()