import socket
import json
import random
import datetime
import threading
import time
import asyncio
from websockets.asyncio.server import serve
from dispatcher import load_handlers, dispatch
from config import ServerConfig, ServerType

def json_to_bytes(data: dict, null_terminated=True) -> bytes:
    """Преобразует JSON в байты с UTF-8 кодировкой и добавляет NULL-терминатор."""
    json_bytes = json.dumps(data, ensure_ascii=False).encode('utf-8')
    return json_bytes + b'\x00' if null_terminated else json_bytes

def bytes_to_json(data: bytes) -> dict:
    """Декодирует байты в JSON, убирая NULL-терминатор."""
    try:
        json_str = data.rstrip(b'\x00').decode('utf-8')
        return json.loads(json_str)
    except (json.JSONDecodeError, UnicodeDecodeError):
        return {}
# ========================== КОНФИГ СЕРВЕРА (ПОТОКОБЕЗОПАСНО) ==========================

def handle_request_via_plugins(data: dict, cfg: ServerConfig) -> dict:
    snap = cfg.snapshot()

    # Глобальная задержка
    if snap["delay_ms"] > 0:
        time.sleep(snap["delay_ms"] / 1000.0)

    # Глобальная занятость
    if snap["busy"]:
        return {
            "method": "ServiceMessage",
            "step": data.get("step", 0),
            "params": { "msgType": "deviceBusy" },
            "error": False,
            "errorDescription": ""
        }

    # Переход к плагинам
    return dispatch(data, cfg)

# ========================== TCP-СЕРВЕР (В ОТДЕЛЬНОМ ПОТОКЕ) ==========================

def start_server(host: str, port: int, stop_event: threading.Event, cfg: ServerConfig):
    """Запускает TCP-сервер. Работает, пока не установлен stop_event."""
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
        # Позволяет быстро перезапускать после остановки
        server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server.bind((host, port))
        server.listen(5)
        server.settimeout(1.0)  # чтобы периодически проверять stop_event
        print(f"Сервер запущен на {host}:{port}")

        try:
            while not stop_event.is_set():
                try:
                    conn, addr = server.accept()
                except socket.timeout:
                    continue  # Проверяем stop_event заново
                except OSError:
                    break

                print(f"Подключение от {addr}")
                # Обработка в том же потоке (можно вынести в отдельный поток под клиента, если нужно)
                with conn:
                    buffer = b''
                    conn.settimeout(5.0)  # таймаут на чтение от клиента
                    while not stop_event.is_set():
                        try:
                            chunk = conn.recv(1024)
                            if not chunk:
                                print(f"Клиент {addr} отключился.")
                                break

                            buffer += chunk

                            # Ожидаем NULL-терминатор
                            if buffer.endswith(b'\x00'):
                                request = bytes_to_json(buffer)
                                print(f"Получен запрос: {request}")

                                response = handle_request_via_plugins(request, cfg)
                                response_bytes = json_to_bytes(response)
                                print(f"Отправка ответа: {response}")

                                conn.sendall(response_bytes)
                                buffer = b''
                            else:
                                # Можно не спамить лог, пока пакет не завершён
                                pass

                        except socket.timeout:
                            # Ждём следующих данных
                            continue
                        except ConnectionResetError:
                            print(f"Клиент {addr} разорвал соединение.")
                            break
                        except Exception as e:
                            print(f"Ошибка при обработке клиента {addr}: {e}")
                            break
        finally:
            print("Сервер остановлен.")

# ========================== Сервер TCP/IP ==========================
def startServerTCPIP(cfg: ServerConfig):
    print("Start TCP/IP server")
    """Создаёт и запускает поток с реальным TCP сервером."""
    t = threading.Thread(target=start_server, args=(cfg.host, cfg.port, cfg.stop_event, cfg), daemon=True)
    t.start()
    return t

# ========================== Сервер WebSocket ==========================
def startServerWebSocket(cfg: ServerConfig):

    def ws_thread():

        async def handler(websocket):
            print(f"WS client connected: {websocket.id} {websocket.remote_address} {websocket.request.headers}")
            try:
                async for msg in websocket:
                    try:
                        if isinstance(msg, str):
                            raw = msg.rstrip('\x00')
                            request = json.loads(raw) if raw else {}
                        else:
                            raw = msg.rstrip(b'\x00')
                            request = json.loads(raw.decode('utf-8')) if raw else {}
                    except Exception:
                        request = {}

                    print(f"Received WS request: {request}")

                    loop = asyncio.get_running_loop()
                    try:
                        response = await loop.run_in_executor(
                            None,
                            handle_request_via_plugins,
                            request,
                            cfg
                        )
                    except Exception as e:
                        print(f"[ERR] Handler execution failed: {e}")
                        response = {
                            "method": "ServiceMessage",
                            "step": request.get("step", 0),
                            "params": {"msgType": "internalError"},
                            "error": True,
                            "errorDescription": str(e),
                        }

                    resp_text = json.dumps(response, ensure_ascii=False) + '\x00'
                    try:
                        await websocket.send(resp_text)
                    except Exception as e:
                        print(f"[ERR] Failed to send WS response: {e}")
                        break
            except websocket.exceptions.ConnectionClosed:
                pass
            except Exception as e:
                print(f"[ERR] WS handler error: {e}")
            finally:
                print("WS client disconnected")

        async def run_server():
            # Start server and keep it alive until stop_event is set
            async with serve(handler, cfg.host, cfg.port) as server:
                await server.serve_forever()
                while not cfg.stop_event.is_set():
                    await asyncio.sleep(0.1)
                    server.close()
                    await server.wait_closed()

        try:
            asyncio.run(run_server())
        except Exception as e:
            print(f"[ERR] WebSocket server error: {e}")
        finally:
            print("WebSocket server (thread) stopped")

    t = threading.Thread(target=ws_thread, daemon=True)
    t.start()
    return t


# ========================== КОНСОЛЬНОЕ МЕНЮ (ГЛАВНЫЙ ПОТОК) ==========================

def print_menu(cfg: ServerConfig, running: bool):
    snap = cfg.snapshot()
    print("\n=== Меню управления POS-эмулятором ===")
    print(f"Server type: {cfg.typeServer} address: {cfg.host}:{cfg.port} | Running: {'YES' if running else 'NO'}")
    print(f"1) Ошибка Purchase: {'ВКЛ' if snap['force_error'] else 'ВЫКЛ'}")
    print(f"2) Задержка ответа (мс): {snap['delay_ms']}")
    print(f"3) Терминал занят: {'ВКЛ' if snap['busy'] else 'ВЫКЛ'}")
    print("4) Показать текущие параметры")
    print("5) Перечитать обработчики")
    print("6) Переключить тип сервера (TCP/IP <-> WebSocket)")
    print("s) Запустить/Остановить сервер")
    print("q) Выход (остановить сервер)")
    print("Выберите пункт: ", end="", flush=True)

def main():

    cfg = ServerConfig()
    cfg.host = '127.0.0.1'
    cfg.port = 2000

    server_thread = None

    load_handlers(reload=False)

    try:
        while True:
            running = server_thread is not None and server_thread.is_alive()
            print_menu(cfg, running)
            choice = input().strip().lower()

            if choice == '1':
                # Тумблер ошибки Purchase
                snap = cfg.snapshot()
                cfg.set_force_error(not snap["force_error"])
                state = "ВКЛ" if not snap["force_error"] else "ВЫКЛ"
                print(f"[OK] Ошибка Purchase -> {state}")

            elif choice == '2':
                print("Введите задержку в миллисекундах (например, 250): ", end="", flush=True)
                val = input().strip()
                try:
                    ms = int(val)
                    cfg.set_delay_ms(ms)
                    print(f"[OK] Задержка ответа = {ms} мс")
                except ValueError:
                    print("[ERR] Неверное число, попробуйте снова.")

            elif choice == '3':
                snap = cfg.snapshot()
                cfg.set_busy(not snap["busy"])
                state = "ВКЛ" if not snap["busy"] else "ВЫКЛ"
                print(f"[OK] Терминал занят -> {state}")

            elif choice == '4':
                snap = cfg.snapshot()
                print(f"Текущие параметры: {snap}")

            elif choice == '5':
                load_handlers(reload=True)
                print("[OK] Обработчики перечитаны")

            elif choice == '6':
                cfg.typeServer = ServerType.webSocket if cfg.typeServer == ServerType.tcp else ServerType.tcp
                print(f"[OK] Тип сервера -> {cfg.typeServer}")

            elif choice == 's':
                # Start/stop server
                if server_thread is None or not server_thread.is_alive():
                    cfg.stop_event = threading.Event()
                    if cfg.typeServer == ServerType.tcp:
                        server_thread = startServerTCPIP(cfg)
                    elif cfg.typeServer == ServerType.webSocket:
                        server_thread = startServerWebSocket(cfg)
                    else:
                        print(f"[ERR] Неизвестный тип сервера: {cfg.typeServer}")
                        cfg.stop_event = None
                        continue
                    print(f"[OK] Сервер ({cfg.typeServer}) запущен")
                else:
                    print("Остановка сервера...")
                    if cfg.stop_event:
                        cfg.stop_event.set()
                    server_thread.join(timeout=3.0)
                    server_thread = None
                    stop_event = None
                    print("[OK] Сервер остановлен")

            elif choice == 'q':
                print("Остановка сервера...")
                if cfg.stop_event:
                    cfg.stop_event.set()
                break

            else:
                print("Неизвестный пункт меню. Повторите ввод.")

    except KeyboardInterrupt:
        print("\n[INTERRUPT] Завершение работы...")
        if cfg.stop_event:
            cfg.stop_event.set()
    finally:
        if server_thread is not None and server_thread.is_alive():
            cfg.stop_event.set()
            server_thread.join(timeout=3.0)
        print("Готово. Выход.")

if __name__ == "__main__":
    main()
