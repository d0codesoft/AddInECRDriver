from typing import Dict
from config import ServerConfig

SUPPORTED_METHODS = ["PingDevice"]
PRIORITY = 10

def handle(data: Dict, cfg: ServerConfig) -> Dict:
    return {
        "method": "PingDevice",
        "step": data.get("step", 0),
        "params": {"code": "00", "responseCode": "0000"},
        "error": False,
        "errorDescription": ""
    }