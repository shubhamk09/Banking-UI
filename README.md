# Banking-UI
UI For Banking Project

## Prposed Directory structure

````
Banking-UI/
│
├── main.cpp
├── Main.qml
├── qml.qrc                     # We will see if its needed
├── App.qml                     # Entry point
│
├── Src/
│    ├── Modules/
│    │   ├── LoginModule/
│    │   │   ├── LoginPage.qml
│    │   │   ├── LoginController.cpp/h
│    │   │   ├── LoginModule.qrc     # Optional: for resource encapsulation
│    │   │   └── LoginModule.pri     # Include file for building
│    │   │
│    │   ├── DashboardModule/
│    │   │   ├── DashboardPage.qml
│    │   │   ├── DashboardController.cpp/h
│    │   │   └── DashboardModule.pri
│    │   │
│    │   └── TransactionsModule/
│    │       ├── TransactionsPage.qml
│    │       ├── TransactionsController.cpp/h
│    │       └── TransactionsModule.pri
│    │
│    ├── common/
│    │   ├── Components/             # Shared QML components
│    │   │    ├── Header.qml
│    │   │    └── Sidebar.qml
│    │   ├── Utils/                  # Shared JS or helpers
│    │   │     ├── Theme.qml
│    │   │     └── Validator.js
│    │   └── Models/                 # Shared models
│    │         └── AccountListModel.cpp/h
│    │
│    └── Resources/
│        ├── Images/
│        └── Themes/
````