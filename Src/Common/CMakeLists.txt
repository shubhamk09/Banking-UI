# Core infrastructure library (StartupManager, ModuleFactory)
add_library(bankingcore_lib STATIC
    inc/StartupManager.hpp
    inc/ModuleFactory.hpp
    inc/ModuleNames.hpp
    Interfaces/IModule.hpp

    StartupManager.cpp
    ModuleFactory.cpp
)

target_include_directories(bankingcore_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Interfaces
)

target_link_libraries(bankingcore_lib PUBLIC Qt6::Core Qt6::Network)

# messagequeue_lib library
add_library(messagequeue_lib STATIC
    # Header files (MUST be uncommented for MOC to process Q_OBJECT)
    inc/MessageQueue.hpp
    Interfaces/IMessage.hpp
    Messages/inc/AuthMessage.hpp
    Messages/inc/ErrorMessage.hpp
    Messages/inc/StatusMessage.hpp
    Messages/inc/TransactionMessage.hpp
    # inc/MessageDispatcher.hpp

    # Implementation files - Message classes
    Messages/Src/AuthMessage.cpp
    Messages/Src/ErrorMessage.cpp
    Messages/Src/StatusMessage.cpp
    Messages/Src/TransactionMessage.cpp

    # MessageQueue implementation (will be created next)
    # src/MessageQueue.cpp
    # src/MessageDispatcher.cpp
)

target_include_directories(messagequeue_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/Messages/inc
)

target_link_libraries(messagequeue_lib PUBLIC Qt6::Core Qt6::Network)