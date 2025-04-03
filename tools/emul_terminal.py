import socket
import json

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


def handle_request(data: dict) -> dict:
    """Обрабатывает запрос и возвращает ответ."""
    method = data.get("method")
    if method == "PingDevice":
        return {
            "method": "PingDevice",
            "step": data.get("step", 0),
            "params": {"code": "00", "responseCode": "0000"},
            "error": False,
            "errorDescription": ""
        }
    elif method == "ServiceMessage" and data.get("params", {}).get("msgType") == "identify":
        return {
            "method": "ServiceMessage",
            "step": data.get("step", 0),
            "params": {"msgType": "identify", "result": "true", "vendor": "PAX", "model": "s800"},
            "error": False,
            "errorDescription": ""
        }
    return {"error": True, "errorDescription": "Unknown method"}


def start_server(host='127.0.0.1', port=2000):
    """Запускает TCP-сервер на указанном порту."""
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
        server.bind((host, port))
        server.listen(1)
        print(f"Сервер запущен на {host}:{port}")

        while True:
            try:
                conn, addr = server.accept()
                print(f"Подключение от {addr}")

                with conn:
                    buffer = b''

                    while True:
                        try:
                            chunk = conn.recv(1024)
                            if not chunk:
                                print(f"Клиент {addr} отключился.")
                                break  # Выход из внутреннего цикла, но сервер продолжит работу

                            buffer += chunk
                            if buffer.endswith(b'\x00'):
                                request = bytes_to_json(buffer)
                                print(f"Получен запрос: {request}")

                                response = handle_request(request)
                                response_bytes = json_to_bytes(response)
                                print(f"Отправка ответа: {response}")

                                conn.sendall(response_bytes)
                                buffer = b''
                        except ConnectionResetError:
                            print(f"Клиент {addr} разорвал соединение.")
                            break  # Выход из внутреннего цикла, ожидание нового клиента
            except Exception as e:
                print(f"Ошибка: {e}")

if __name__ == "__main__":
    start_server()