# dispatcher.py
import importlib
import pkgutil
import sys
from types import ModuleType
from typing import Dict, Callable, List, Tuple, Any

# Папка-пакет с обработчиками
HANDLERS_PACKAGE = "handlers"

# method -> список (priority, module)
_registry: Dict[str, List[Tuple[int, ModuleType]]] = {}
_loaded_modules: Dict[str, ModuleType] = {}

def _register_module(mod: ModuleType):
    """Регистрирует модуль-обработчик в реестре по SUPPORTED_METHODS."""
    supported = getattr(mod, "SUPPORTED_METHODS", None)
    if not supported or not isinstance(supported, (list, tuple)):
        return
    priority = getattr(mod, "PRIORITY", 100)

    for method in supported:
        method = str(method)
        _registry.setdefault(method, [])
        _registry[method].append((priority, mod))
        # поддержим сортировку по приоритету (меньше — выше)
        _registry[method].sort(key=lambda x: x[0])

def load_handlers(reload: bool = False) -> None:
    """
    Сканирует пакет handlers и загружает/перезагружает модули PosTerminal_*.
    Если reload=True — делает importlib.reload для уже загруженных.
    """
    package = importlib.import_module(HANDLERS_PACKAGE)
    package_path = package.__path__  # type: ignore[attr-defined]

    # Обнулим реестр
    global _registry
    _registry = {}

    for finder, name, ispkg in pkgutil.iter_modules(package_path):
        if not name.startswith("PosTerminal_"):
            continue
        full_name = f"{HANDLERS_PACKAGE}.{name}"

        if full_name in _loaded_modules and reload:
            mod = importlib.reload(_loaded_modules[full_name])
        else:
            mod = importlib.import_module(full_name)
            _loaded_modules[full_name] = mod

        _register_module(mod)

def dispatch(data: Dict, cfg) -> dict[str, str | int | dict[Any, Any] | bool] | Callable:
    """
    Находит подходящий модуль по data['method'] и вызывает его handle(data, cfg).
    Если ничего не найдено — возвращает стандартную ошибку.
    """
    method = str(data.get("method", ""))
    candidates = _registry.get(method, [])

    if not candidates:
        return {
            "method": method or "Unknown",
            "step": data.get("step", 0),
            "params": {},
            "error": True,
            "errorDescription": f"No handler for method '{method}'"
        }

    # Берём модуль с наивысшим приоритетом
    _, mod = candidates[0]
    handle = getattr(mod, "handle", None)
    if not callable(handle):
        return {
            "method": method or "Unknown",
            "step": data.get("step", 0),
            "params": {},
            "error": True,
            "errorDescription": f"Handler for '{method}' does not expose callable 'handle'"
        }

    try:
        return handle(data, cfg)
    except Exception as e:
        # чтобы не ронять сервер из-за обработчика
        return {
            "method": method or "Unknown",
            "step": data.get("step", 0),
            "params": {},
            "error": True,
            "errorDescription": f"Handler exception: {e}"
        }