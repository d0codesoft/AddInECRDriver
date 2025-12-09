import threading
import enum
from dataclasses import dataclass, field

class ServerType(enum.IntEnum):
    tcp = 1
    webSocket = 2
    com = 3

    def __str__(self):
        return self.name

@dataclass
class ServerConfig:
    typeServer: ServerType = ServerType.tcp
    host: str = ""
    port: int = 0
    """Живая конфигурация сервера (меняется из главного потока)."""
    force_error: bool = False               # 1: включить/выключить общую ошибку Purchase
    delay_ms: int = 0                       # 2: задержка ответа в миллисекундах (для всех методов)
    busy: bool = False                      # 3: терминал занят (ошибка на все методы)
    lock: threading.Lock = field(default_factory=threading.Lock, repr=False)
    stop_event : threading.Event = field(default=None, repr=False)

    # Геттеры под замком — чтобы избежать torn reads
    def snapshot(self):
        with self.lock:
            return {
                "force_error": self.force_error,
                "delay_ms": self.delay_ms,
                "busy": self.busy,
            }

    # Установщики
    def set_force_error(self, val: bool):
        with self.lock:
            self.force_error = val

    def set_delay_ms(self, ms: int):
        with self.lock:
            self.delay_ms = max(0, int(ms))

    def set_busy(self, val: bool):
        with self.lock:
            self.busy = val
