import socket
import threading
import struct

class ESPCRUDProtocol:
    """
    Implementa un protocolo CRUD básico para comunicarse con dispositivos ESP32
    a través de sockets TCP. Los paquetes tienen una estructura binaria definida.
    """
    # Códigos de operación (1 byte): Identifican la acción a realizar.
    CMD_POST = 0x01  # Crear o enviar datos.
    CMD_GET = 0x02   # Solicitar datos.
    CMD_UPDATE = 0x03# Actualizar datos existentes.
    CMD_DELETE = 0x04# Eliminar datos.

    # Tipos de recurso (1 byte): Especifican a qué tipo de dato se refiere la operación.
    RES_SENSOR = 0x10
    RES_LED = 0x11

    # Estados de respuesta (1 byte): Indica el resultado de la operación.
    STATUS_OK = 0x20
    STATUS_ERROR = 0x21
    STATUS_NOT_FOUND = 0x22

    def __init__(self, host, port):
        """
        Inicializa el servidor con la dirección y el puerto especificados.
        También inicializa un diccionario para almacenar los datos (data_store)
        y un candado (lock) para asegurar el acceso seguro a los datos en entornos
        multihilo. Se precargan datos estáticos para pruebas.
        """
        self.host = host
        self.port = port
        self.data_store = {
            'sensors': {
                1: b'\x01',  # Sensor ID 1, valor 1 (en bytes)
                2: b'\x02', # Sensor ID 2, valor 2 y 5 (en bytes)
                3: b'\x03'   # Sensor ID 3, valor 3 (en bytes)
            },
            'leds': {
            }
        }
        self.lock = threading.Lock()
        self.running = False

    def _handle_packet(self, conn):
        """
        Recibe y procesa un paquete binario del cliente.
        Estructura del paquete: [1:CMD][1:RES][2:id][4:data_len][data]
        Desempaqueta el encabezado para determinar la operación, el recurso,
        el ID del dispositivo y la longitud de los datos, luego recibe los datos.
        Finalmente, llama a _process_command para ejecutar la acción.
        """
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
        """
        Ejecuta la operación CRUD correspondiente basada en el comando (cmd),
        el recurso (res) y el ID del dispositivo (dev_id). Utiliza un candado
        para acceder de forma segura al diccionario de datos (self.data_store).
        """
        resource = 'sensors' if res == self.RES_SENSOR else 'leds'

        with self.lock:
            try:
                if cmd == self.CMD_POST:
                    self.data_store[resource][dev_id] = data
                    print(f"[+] Recibido POST para {resource} con ID {dev_id}: {data}")
                    return struct.pack('!B', self.STATUS_OK)

                elif cmd == self.CMD_GET:
                    resource = 'sensors' if res == self.RES_SENSOR else 'leds'
                    print(f"[+] Recibido GET para {resource} con ID {dev_id}")
                    value = self.data_store[resource].get(dev_id, b'')
                    print(f"[+] Valor encontrado para ID {dev_id}: {value}")
                    response = struct.pack(f'!BBHL{len(value)}s',
                                            self.STATUS_OK, res, dev_id, len(value), value)
                    print(f"[+] Enviando respuesta GET: {response}")
                    return response

                elif cmd == self.CMD_UPDATE:
                    response =struct.pack('!B', self.STATUS_NOT_FOUND);
                    if dev_id in self.data_store[resource]:
                        self.data_store[resource][dev_id] = data
                        print(f"[+] Recibido UPDATE para {resource} con ID {dev_id}: {data}")
                        print(f"[+] Valores: {self.data_store}")
                        response =struct.pack('!B', self.STATUS_OK)
                    return response

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
        """
        Maneja la conexión individual con un cliente (ESP32).
        Imprime la dirección del cliente conectado, luego entra en un bucle
        para recibir y procesar paquetes mientras el servidor esté en ejecución.
        Si no se recibe un paquete válido, se rompe el bucle y se cierra la conexión.
        """
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
        """
        Inicia el servidor socket. Se enlaza a la dirección y puerto configurados,
        y comienza a escuchar conexiones entrantes. Por cada conexión aceptada,
        se crea un nuevo hilo para manejar la comunicación con ese cliente,
        permitiendo manejar múltiples clientes simultáneamente. El servidor
        se detiene al recibir una interrupción de teclado (Ctrl+C).
        """
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