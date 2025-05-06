import socket
import json
import random
import datetime

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
    elif method == "Purchase":
        current_year = datetime.datetime.now().year
        current_date = datetime.datetime.now().strftime("%d.%m.%Y")
        trn_status = random.choice([1, 2, 3, 4])  # Random transaction status
        error = False
        response_code = None
        error_description = ""
        param = data.get("params", {})

        error_mapping = {
            2: [
                "1002 – EMV Decline",
                "1003 – Transaction log is full. Need close batch",
                "1004 – No connection with host"
            ],
            3: [
                "1001 – Transaction canceled by user",
                "1007 – Card reader is not connected"
            ],
            4: [
                "1005 – No paper in printer",
                "1006 – Error Crypto keys",
                "1008 – Transaction is already complete"
            ]
        }

        if trn_status in [2, 3, 4]:  # Declined, Reversed, or Canceled
            response_code = f"{random.randint(1, 1008):04}"
            error_description = random.choice(error_mapping[trn_status])
            error = True

        return {
            "method": "Purchase",
            "step": 0,
            "params": {
                "amount": f"{param.get('amount')}",
                "approvalCode": f"{random.randint(100000, 999999)}",
                "captureReference": "",
                "cardExpiryDate": f"{random.randint(current_year, current_year + 10)}",
                "cardHolderName": f"USER{random.randint(1000, 9999)}",
                "date": current_date,
                "discount": "0.00",
                "hstFld63Sf89": "",
                "invoiceNumber": f"{random.randint(100000, 999999)}",
                "issuerName": "VISA ПРИВАТ",
                "merchant": "TSTTTTTT",
                "pan": "4731XXXXXXXX9838",
                "posConditionCode": "00",
                "posEntryMode": "022",
                "processingCode": "000000",
                "receipt": "text-of-receipt",
                "responseCode": response_code if response_code else "0000",
                "rrn": f"{random.randint(1000000000000, 9999999999999)}",
                "rrnExt": f"{random.randint(1000000000000, 9999999999999)}",
                "terminalId": "TSTSALE1",
                "time": datetime.datetime.now().strftime("%H:%M:%S"),
                "track1": "",
                "signVerif": "0",
                "txnType": "1",
                "trnStatus": str(trn_status),
                "adv": "ПриватБанк",
                "adv2p": "Беремо і робимо!",
                "bankAcquirer": "ПриватБанк",
                "paymentSystem": "VISA",
                "subMerchant": ""
            },
            "error": error,
            "errorDescription": error_description
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