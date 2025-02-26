# Внешняя компонента 1С AddIn для POS Терминала (Эквайрингового терминала)

## 📄 Описание

Данная внешняя компонента предназначена для интеграции POS-терминалов (эквайринговых терминалов) с платформой **1С:Предприятие 8.3**. Компонента реализует взаимодействие с терминалами по протоколу ECR (электронный кассовый регистратор) от **PrivatBank**, основанному на формате JSON.  

Подключаемая компонента соответствует стандарту **"Требования к разработке драйверов подключаемого оборудования"**, версия **4.4**, и обеспечивает полный набор методов для интеграции с решениями 1С.

---

## ⚙️ Основные возможности

- ✅ Поддержка протокола ECR от **PrivatBank**  
- 📝 Обмен данными в формате JSON  
- 🔗 Совместимость с платформой **1С:Предприятие 8.3**  
- 💡 Поддержка ключевых методов для интеграции с POS-терминалами  
- 🛡 Подробная обработка ошибок и логирование событий  
- 🌐 Поддержка локализации интерфейса

---

## 📜 Таблица версий и требований к конфигурациям

| Конфигурация                              | Версия конфигурации  | Версия БПО     | Требуемая версия стандарта |
|--------------------------------------------|----------------------:|---------------:|---------------------------:|
| **BAS SmallBusiness**                      | 1.6.21.2             | 2.1.6.16       | 3.4                        |
| **BAS Управление торговлей**               | 3.5.14.2             | 2.1.3.9        | 3.2                        |
| **BAS ERP**                                | 2.5.14.2             | 2.1.3.9        | 3.2                        |

---

## 📚 Используемые внешние библиотеки

- **ICU Package**: `74.2 static`  
- **Boost**:  `1.87`
- **pugixml**: `1.15`

---

## 🖥 Поддерживаемые операционные системы

- **Windows (Server) x64**: версии 7–11, 2019–2022
- **Ubuntu x64**: версия 22.04
- **Debian x64**: последние стабильные релизы
- **macOS**: последние стабильные релизы

---

## 💡 Использование

### 🔄 Инициализация

Для инициализации драйвера необходимо создать экземпляр класса:
```
ПодключитьВнешнююКомпоненту({полный путь}+"\ECR_Driver_PT_win64.dll", "Driver", ТипВнешнейКомпоненты.Native);
ОбъектДрайвера = Новый("AddIn.Driver.ECRDriverPOS");
```

---

### 🔧 Доступные методы

- `GetInterfaceRevision`: Возвращает поддерживаемую версию интерфейса.  
- `GetDescription`: Предоставляет описание драйвера в формате JSON.  
- `GetLastError`: Предоставляет код и описание последней ошибки.  
- `EquipmentParameters`: Возвращает список параметров оборудования.  
- `ConnectEquipment`: Подключает оборудование с заданными параметрами.  
- `DisconnectEquipment`: Отключает оборудование.  
- `EquipmentTest`: Выполняет тестовое подключение к оборудованию.  
- `EquipmentAutoSetup`: Автоматическая настройка оборудования.  
- `SetApplicationInformation`: Устанавливает информацию о приложении.  
- `GetAdditionalActions`: Возвращает дополнительные действия.  
- `DoAdditionalAction`: Выполняет дополнительное действие.  
- `GetLocalizationPattern`: Возвращает шаблон локализации.  
- `SetLocalization`: Устанавливает локализацию для драйвера.

---

### 🏷 Доступные свойства

- `IsEnabled`: Состояние активности драйвера.  
- `IsTimerPresent`: Наличие таймера.  
- `Locale`: Получение и установка локали драйвера.

---

## 🛡 Обработка ошибок

Для регистрации ошибок используется метод `addError`, позволяющий фиксировать коды ошибок и их описания в системном журнале.

---

## 📝 Логирование

Логирование ключевых событий и ошибок производится в указанный каталог логов, который задается при инициализации драйвера.

---

## 📜 Лицензия

Проект лицензирован под лицензией **MIT**. Подробности смотрите в файле [LICENSE](LICENSE).

---

## 👨‍💻 Автор

Разработка: **SCODE**  