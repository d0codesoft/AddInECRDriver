from typing import Dict
import random
import datetime
from config import ServerConfig

SUPPORTED_METHODS = ["Purchase"]
PRIORITY = 10

def handle(data: Dict, cfg: ServerConfig) -> Dict:
    # Снимок настроек
    snap = cfg.snapshot()

    # Задержка (глобальная)
    # (Можно убрать отсюда и делать в сервере до вызова handle — как вам удобнее)
    if snap["delay_ms"] > 0:
        import time
        time.sleep(snap["delay_ms"] / 1000.0)

    # Если терминал занят — глобальная ошибка
    if snap["busy"]:
        return {
            "method": "Purchase",
            "step": data.get("step", 0),
            "params": {},
            "error": True,
            "errorDescription": "POS terminal is busy"
        }

    current_year = datetime.datetime.now().year
    current_date = datetime.datetime.now().strftime("%d.%m.%Y")

    # Логика успех/ошибка
    trn_status = 2 if snap["force_error"] else 1
    error = trn_status != 1
    error_description = ""
    response_code = None
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
    if error:
        response_code = f"{random.randint(1, 1008):04}"
        error_description = random.choice(error_mapping[trn_status])

    param = data.get("params", {}) or {}

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