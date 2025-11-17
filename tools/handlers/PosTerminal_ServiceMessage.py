from typing import Dict
from config import ServerConfig

SUPPORTED_METHODS = ["ServiceMessage"]
PRIORITY = 10

def handle(data: Dict, cfg: ServerConfig) -> Dict:
    params = data.get("params", {}) or {}
    if params.get("msgType") == "identify":
        return {
            "method": "ServiceMessage",
            "step": data.get("step", 0),
            "params": {"msgType": "identify", "result": "true", "vendor": "PAX", "model": "s800"},
            "error": False,
            "errorDescription": ""
        }
    return {
        "method": "ServiceMessage",
        "step": data.get("step", 0),
        "params": {},
        "error": True,
        "errorDescription": "Unsupported ServiceMessage type"
    }