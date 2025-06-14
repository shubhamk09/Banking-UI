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
│    ├── Common/
│    │   ├── Components/             # Shared QML components
│    │   │   ├── SplashScreen.qml
│    │   │   ├── Header.qml
│    │   │   ├── Sidebar.qml
│    │   │   ├── ContentArea.qml
│    │   │   ├── Footer.qml
│    │   │   └── Breadcrumb.qml
│    │   │    
│    │   ├── Utils/                  # Shared JS or helpers
│    │   │   ├── Theme.qml
│    │   │   └── Validator.js
│    │   │
│    │   └── Models/                 # Shared models
│    │       └── AccountListModel.cpp/h
│    │
│    └── Resources/
│        ├── Images/
│        └── Themes/
````