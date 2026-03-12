## Plan: Detailed Login Flow with Service Locator and Message Queue

**TL;DR:**  
The application will use a Service Locator to provide global access to a shared MessageQueue. All modules (e.g., LoginModule, BankingSocket) will communicate exclusively via this queue, ensuring decoupling and testability. The Service Locator will be initialized in the StartupManager, and modules will retrieve the queue as needed.

**Steps**
1. **Define Message Types**
   - Create base `IMessage` interface and concrete messages: `LoginRequest`, `LoginResponse`, `ConnectionStatus`, etc.

2. **Implement MessageQueue**
   - Thread-safe queue class with subscribe/enqueue/dequeue methods.
   - Supports callback registration for message types.

3. **Implement ServiceLocator**
   - Static class with register/get methods for MessageQueue (and other services if needed).
   - Example: `ServiceLocator::registerMessageQueue(std::shared_ptr<MessageQueue>)` and `ServiceLocator::getMessageQueue()`.

4. **StartupManager Initialization**
   - Create MessageQueue instance.
   - Register it with ServiceLocator before initializing modules.
   - Initialize all modules (LoginModule, BankingSocket, etc.).

5. **Module Communication**
   - Modules retrieve the MessageQueue from ServiceLocator when needed.
   - Modules subscribe to relevant message types during their initialization.
   - All inter-module communication (e.g., login requests, responses) happens via the queue.

6. **Login Flow**
   - User triggers login in LoginPage (QML).
   - LoginPage enqueues `LoginRequest` via a C++ bridge or direct QML/C++ integration.
   - LoginModule subscribes to `LoginRequest`, processes it, and enqueues a network request.
   - BankingSocket subscribes to network requests, communicates with the server, and enqueues `LoginResponse`.
   - LoginModule subscribes to `LoginResponse`, processes it, and enqueues a UI update.
   - LoginPage subscribes to UI updates and displays the result.

7. **Error Handling**
   - All errors are communicated as messages in the queue (e.g., `ErrorMessage`).

8. **Testing**
   - Register a mock MessageQueue in ServiceLocator for unit/integration tests.

**Verification**
- Build and run the application; verify all modules communicate via the queue.
- Test login flow end-to-end.
- Swap MessageQueue implementation in tests to verify decoupling.

**Decisions**
- Service Locator pattern chosen for global, testable access to MessageQueue.
- All module communication is message-based for maximum decoupling and extensibility.