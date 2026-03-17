# Banking-UI

A modular Qt-based Banking User Interface application with a flexible module management system.

## Project Status

**Phase**: Early Development (Initial Implementation)

### Currently Implemented
- ✅ **Module Management System** - Factory pattern for dynamic module creation
- ✅ **StartupManager** - Orchestrates application startup and module lifecycle
- ✅ **Communications Module** - TCP socket-based banking communication layer
- ✅ **JSON Configuration** - Config-driven module initialization

### Planned Features
- 🔜 **Login Module** - User authentication and login interface
- 🔜 **Dashboard Module** - Main application dashboard
- 🔜 **Transactions Module** - Transaction history and management
- 🔜 **UI Components** - Shared QML components (SplashScreen, Header, Sidebar, etc.)
- 🔜 **Theme System** - Dark/Light theme support

---

## Architecture Overview

The Banking-UI uses a **modular architecture** for scalability and maintainability:

### Core Components

#### 1. **Module System**
- **IModule Interface** (`Src/Common/Interfaces/IModule.hpp`): Base interface for all modules
  - Methods: `init()`, `start()`, `stop()`, `getModuleName()`

- **ModuleFactory** (`Src/Common/inc/ModuleFactory.hpp`): Factory pattern implementation
  - Registers module creators
  - Creates module instances dynamically
  - Singleton pattern ensures single instance

- **StartupManager** (`Src/Common/inc/StartupManager.hpp`): Application orchestrator
  - Loads module configuration from JSON
  - Initializes core systems
  - Manages module lifecycle
  - Handles startup/shutdown sequences

#### 2. **Communications Module**
Located in `Src/Modules/Communications/`

**Components:**
- **BankingSocket** (`inc/BankingSocket.hpp`): TCP socket wrapper
  - Manages socket connections to banking server
  - Handles data transmission
  - Emits signals: `dataReceived()`, `connectionStatusChanged()`, `errorOccurred()`

- **CommunicationInitializer** (`inc/CommunicationInitializer.hpp`): Module initializer
  - Implements IModule interface
  - Manages BankingSocket lifecycle
  - Default server: `127.0.0.1:5020`

---

## Directory Structure

```
Banking-UI/
│
├── main.cpp                         # Application entry point
├── Main.qml                         # Main QML file
├── App.qml                          # QML entry point
├── README.md                        # This file
├── CMakeLists.txt
│
├── Src/
│   ├── Common/
│   │   ├── Interfaces/
│   │   │   └── IModule.hpp          # Module interface
│   │   ├── inc/
│   │   │   ├── ModuleFactory.hpp    # Factory pattern
│   │   │   ├── ModuleNames.hpp      # Module name constants
│   │   │   └── StartupManager.hpp   # Application orchestrator
│   │   ├── ModuleFactory.cpp
│   │   └── StartupManager.cpp
│   │
│   ├── Modules/
│   │   ├── Communications/
│   │   │   ├── inc/
│   │   │   │   ├── BankingSocket.hpp
│   │   │   │   └── CommunicationInitializer.hpp
│   │   │   ├── BankingSocket.cpp
│   │   │   └── CommunicationInitializer.cpp
│   │   │
│   │   ├── LoginModule/              # [Planned]
│   │   ├── DashboardModule/          # [Planned]
│   │   └── TransactionsModule/       # [Planned]
│   │
│   ├── Common/
│   │   ├── Components/              # Shared QML components [Planned]
│   │   ├── Controls/                # Custom controls [Planned]
│   │   ├── Utils/                   # Utilities and helpers [Planned]
│   │   └── Models/                  # Shared data models [Planned]
│   │
│   └── Resources/
│       ├── Images/
│       └── Themes/
│
└── build/                           # Build directory (generated)
```

---

## Module System

### How Modules Work

1. **Registration**: Each module is registered with `ModuleFactory` via a creator function
2. **Configuration**: Modules to load are specified in JSON resource file (`modules_list`)
3. **Initialization**: `StartupManager` loads config and creates module instances
4. **Lifecycle**: Each module follows: `init()` → `start()` → `stop()`

### Module Configuration (JSON)

Example configuration structure:
```json
{
  "modules": [
    {
      "Communications": 1
    },
    {
      "Login": 1
    },
    {
      "Dashboard": 0
    }
  ]
}
```

- **1**: Module is enabled and will be initialized
- **0**: Module is disabled and will be skipped

---

## Communications Module Details

### Purpose
Handles TCP socket-based communication with the banking server.

### Configuration
- **Server Address**: `127.0.0.1` (hardcoded)
- **Server Port**: `5020` (hardcoded)
- **Connection Timeout**: 5000ms (5 seconds)

### Key Features
- Automatic connection management
- Graceful disconnect handling
- Signal-based error reporting
- Data buffering for incoming data

### API

**BankingSocket Methods:**
```cpp
bool connectToServer(const std::string &host, quint16 port);
void disconnectFromServer();
bool sendData(const QByteArray &data);
QByteArray receiveData();
```

**Signals:**
```cpp
void dataReceived(const QByteArray &data);
void connectionStatusChanged(bool connected);
void errorOccurred(const QString &error);
```

---

## Code Documentation

### Doxygen Documentation
All C++ header files (.hpp) include comprehensive Doxygen documentation.

**Documentation Includes:**
- File-level descriptions
- Class documentation
- Method descriptions with parameters and return values
- Member variable descriptions

**Header Files Documented:**
- ✅ `Src/Common/Interfaces/IModule.hpp`
- ✅ `Src/Common/inc/ModuleFactory.hpp`
- ✅ `Src/Common/inc/ModuleNames.hpp`
- ✅ `Src/Common/inc/StartupManager.hpp`
- ✅ `Src/Modules/Communications/inc/BankingSocket.hpp`
- ✅ `Src/Modules/Communications/inc/CommunicationInitializer.hpp`

**To Generate HTML Documentation:**
```bash
doxygen Doxyfile
```

### Documentation Convention
- **Header files (.hpp)**: Contain full Doxygen documentation
- **Implementation (.cpp)**: Implementation details only, no Doxygen comments

---

## Development Guidelines

### Creating a New Module

1. **Create Interface Implementation**
   ```cpp
   class MyModule : public IModule {
   public:
       const char* getModuleName() const override;
       bool init() override;
       bool start() override;
       void stop() override;
   };
   ```

2. **Register with Factory** (in `StartupManager::registerModule()`)
   ```cpp
   m_moduleFactory.registerModule("MyModule",
       []() { return std::unique_ptr<IModule>(new MyModule()); });
   ```

3. **Add to Module Configuration** (JSON file)
   ```json
   { "MyModule": 1 }
   ```

4. **Document** (Add Doxygen comments to .hpp files)

### Code Standards
- Follow Qt conventions and idioms
- Use `std::unique_ptr` for dynamic memory management
- Implement comprehensive error handling
- Add Doxygen documentation to all header files
- Keep implementation details out of headers

---

## Build & Setup

### Requirements
- Qt 6.9 (or compatible version)
- CMake 3.27+
- C++17 or later
- macOS (current configuration)

### Build Instructions
```bash
cd Banking-UI
mkdir build
cd build
cmake ..
make
```

### Run Application
```bash
./Banking-UI
```

---

## Project Roadmap

### Phase 1: Core Infrastructure (Current)
- [x] Module management system
- [x] Startup manager
- [x] Communications module
- [ ] Configuration system refinement

### Phase 2: Authentication
- [ ] Login module implementation
- [ ] User authentication system
- [ ] Session management

### Phase 3: UI Implementation
- [ ] Dashboard module
- [ ] Transaction management module
- [ ] Shared UI components
- [ ] Theme system

### Phase 4: Advanced Features
- [ ] Data persistence
- [ ] Offline support
- [ ] Advanced caching
- [ ] Performance optimization

---

## Troubleshooting

### Connection Issues
- Verify banking server is running on `127.0.0.1:5020`
- Check firewall settings
- Review error signals emitted by `BankingSocket`

### Module Initialization Failures
- Check JSON configuration format
- Verify module is registered in `StartupManager::registerModule()`
- Review console output for initialization messages

---

## Contributing

When adding new features:
1. Follow the modular architecture
2. Document all public APIs with Doxygen comments
3. Implement error handling
4. Update this README with changes
5. Test module initialization/cleanup

---

## Future Enhancements

- [ ] Thread pool for concurrent module operations
- [ ] Logging system
- [ ] Message queue for inter-module communication
- [ ] Database abstraction layer
- [ ] REST API layer
- [ ] WebSocket support

---

## License

[Add your license information here]

---

## Contact

For questions or issues, please contact the development team.
