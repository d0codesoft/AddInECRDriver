import socket
import json
import random
import datetime
import threading
import time
from dispatcher import load_handlers, dispatch
from config import ServerConfig

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

def start_server(host: str, port: int, cfg: ServerConfig, stop_event: threading.Event):
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

# ========================== КОНСОЛЬНОЕ МЕНЮ (ГЛАВНЫЙ ПОТОК) ==========================

def print_menu(cfg: ServerConfig):
    snap = cfg.snapshot()
    print("\n=== Меню управления POS-эмулятором ===")
    print(f"1) Ошибка Purchase: {'ВКЛ' if snap['force_error'] else 'ВЫКЛ'}")
    print(f"2) Задержка ответа (мс): {snap['delay_ms']}")
    print(f"3) Терминал занят: {'ВКЛ' if snap['busy'] else 'ВЫКЛ'}")
    print("4) Показать текущие параметры")
    print("q) Выход (остановить сервер)")
    print("Выберите пункт: ", end="", flush=True)

def main():
    host = '127.0.0.1'
    port = 2000

    cfg = ServerConfig()
    stop_event = threading.Event()

    load_handlers(reload=False)

    server_thread = threading.Thread(
        target=start_server,
        args=(host, port, cfg, stop_event),
        daemon=True
    )
    server_thread.start()

    try:
        while True:
            print_menu(cfg)
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

            elif choice == 'q':
                print("Остановка сервера...")
                stop_event.set()
                break

            else:
                print("Неизвестный пункт меню. Повторите ввод.")

    except KeyboardInterrupt:
        print("\n[INTERRUPT] Завершение работы...")
        stop_event.set()
    finally:
        # Дожидаемся завершения сервера
        server_thread.join(timeout=3.0)
        print("Готово. Выход.")

if __name__ == "__main__":
    main()
