# ECR Common Driver for 1C 8.3

## Overview

This project provides a driver for payment terminals using the ECR (electronic cash registrar) protocol from PrivatBank (JSON based). The driver is designed to be used with the 1C 8.3 platform and adheres to the "Требования к разработке драйверов подключаемого оборудования" (version 4.4).

## Features

- Supports ECR protocol from PrivatBank
- JSON-based communication
- Compatible with 1C 8.3 platform
- Implements required methods and properties for 1C integration
- Provides detailed error handling and logging

## Installation

1. Clone the repository:
    
2. Open the project in Visual Studio 2022.

3. Build the project to generate the driver DLL.

4. Register the DLL in 1C 8.3 platform.

## Usage

### Initialization

To initialize the driver, create an instance of `CAddInECRCommon` and call the `Init` method with the connection pointer.

### Methods

The driver provides several methods to interact with the payment terminal:

- `GetInterfaceRevision`: Returns the supported interface revision.
- `GetDescription`: Returns the driver description in JSON format.
- `GetLastError`: Returns the last error code and description.
- `EquipmentParameters`: Returns the list of equipment parameters.
- `ConnectEquipment`: Connects to the equipment with the specified parameters.
- `DisconnectEquipment`: Disconnects the equipment.
- `EquipmentTest`: Performs a test connection to the equipment.
- `EquipmentAutoSetup`: Performs auto-setup of the equipment.
- `SetApplicationInformation`: Sets the application information.
- `GetAdditionalActions`: Returns additional actions for the equipment.
- `DoAdditionalAction`: Executes an additional action.
- `GetLocalizationPattern`: Returns the localization pattern.
- `SetLocalization`: Sets the localization for the driver.

### Properties

The driver exposes several properties:

- `IsEnabled`: Indicates whether the driver is enabled.
- `IsTimerPresent`: Indicates whether a timer is present.
- `Locale`: Gets or sets the locale for the driver.

## Error Handling

The driver provides detailed error handling. Use the `addError` method to log errors with specific codes and descriptions.

## Logging

The driver logs important events and errors to a specified log directory. The log directory can be set during the driver initialization.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Author

Developed by SCODE.