import socket
import threading
import time

class EnhancedESP32Server:
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.clients = {'sensor': None, 'led': None}
        self.lock = threading.Lock()
        self.running = False
        self.connection_timeout = 15  # segundos
        self.heartbeat_interval = 10  # segundos

    def _set_socket_timeout(self, conn):
        """Configura timeout para operaciones de socket"""
        conn.settimeout(self.connection_timeout)

    def _safe_send(self, conn, data):
        """Envía datos con manejo de errores"""
        try:
            conn.sendall(data)
            return True
        except (ConnectionResetError, BrokenPipeError, socket.timeout) as e:
            print(f"Error enviando datos: {type(e).__name__}")
            return False
        except Exception as e:
            print(f"Error inesperado al enviar: {type(e).__name__}")
            return False

    def _safe_recv(self, conn, buffer_size):
        """Recibe datos con manejo de errores"""
        try:
            data = conn.recv(buffer_size)
            return data if data else None
        except socket.timeout:
            #print("Timeout de recepción, verificando conexión...")
            return None
        except (ConnectionResetError, BrokenPipeError) as e:
            print(f"Error en recepción: {type(e).__name__}")
            return None
        except Exception as e:
            print(f"Error inesperado al recibir: {type(e).__name__}")
            return None

    def _client_identification(self, conn):
        """Identificación con timeout y reintentos"""
        try:
            conn.settimeout(5)  # Timeout para identificación
            data = self._safe_recv(conn, 16)
            if data:
                return data.decode('utf-8').strip().lower()
            return None
        except socket.timeout:
            print("Timeout en identificación de cliente")
            return None
        finally:
            self._set_socket_timeout(conn)

    def _connection_heartbeat(self, conn):
        """Verifica conexión con heartbeat"""
        try:
            # Envía ping de verificación
            return self._safe_send(conn, b'PING')
        except:
            return False

    def _handle_sensor(self, conn, addr):
        """Manejo mejorado para sensores"""
        last_activity = time.time()
        
        while self.running:
            data = self._safe_recv(conn, 64)
            
            if data is None:
                print(f"[SENSOR] {addr} - Conexión perdida")
                break
                
            if data == b'':
                print(f"[SENSOR] {addr} - Desconexión limpia")
                break
                
            if time.time() - last_activity > self.heartbeat_interval:
                if not self._connection_heartbeat(conn):
                    print(f"[SENSOR] {addr} - Heartbeat fallido")
                    break
            
            print(f"[Datos sensor] {data.decode('utf-8')}")
            self._relay_to_led(data)
            last_activity = time.time()

    def _handle_led(self, conn, addr):
        """Manejo mejorado para LEDs"""
        last_activity = time.time()
        
        while self.running:
            # Verificar conexión periódicamente
            if time.time() - last_activity > self.heartbeat_interval:
                if not self._connection_heartbeat(conn):
                    print(f"[LED] {addr} - Heartbeat fallido")
                    break
                last_activity = time.time()
            
            # Limpiar buffer de recepción
            data = self._safe_recv(conn, 1)
            if data is None:
                break

    def _cleanup_connection(self, client_type, conn):
        """Limpieza segura de conexión"""
        with self.lock:
            if self.clients[client_type] == conn:
                self.clients[client_type] = None
        try:
            if conn:
                conn.shutdown(socket.SHUT_RDWR)
                conn.close()
        except:
            pass
        print(f"[{client_type.upper()}] Conexión limpiada")

    def _client_handler(self, conn, addr):
        """Manejador principal mejorado"""
        client_type = self._client_identification(conn)
        
        if not client_type or client_type not in ['sensor', 'led']:
            self._cleanup_connection('unknown', conn)
            return

        # Registro seguro
        with self.lock:
            if self.clients[client_type]:
                self._safe_send(conn, b'ERROR:DUPLICATE')
                self._cleanup_connection(client_type, conn)
                return
            self.clients[client_type] = conn

        self._set_socket_timeout(conn)
        print(f"[{client_type.upper()}] {addr} conectado")

        try:
            if client_type == 'sensor':
                self._handle_sensor(conn, addr)
            else:
                self._handle_led(conn, addr)
        except Exception as e:
            print(f"[{client_type.upper()}] Error: {type(e).__name__}")
        finally:
            self._cleanup_connection(client_type, conn)

    def start(self):
        """Inicio seguro del servidor"""
        self.running = True
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            s.bind((self.host, self.port))
            s.listen(5)
            print(f"Servidor mejorado iniciado en {self.host}:{self.port}")

            try:
                while self.running:
                    try:
                        conn, addr = s.accept()
                        print(f"Conexión entrante de {addr}")
                        thread = threading.Thread(
                            target=self._client_handler,
                            args=(conn, addr),
                            daemon=True
                        )
                        thread.start()
                    except socket.timeout:
                        continue
            except KeyboardInterrupt:
                print("\nDeteniendo servidor...")
            finally:
                self.running = False
                s.close()
                print("Socket principal cerrado")

if __name__ == "__main__":
    server = EnhancedESP32Server(HOST='0.0.0.0', PORT=12345)
    server.start()