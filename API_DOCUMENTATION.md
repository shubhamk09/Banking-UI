# Banking System API Documentation

## Table of Contents
1. [Overview](#overview)
2. [Connection Details](#connection-details)
3. [General Format](#general-format)
4. [Account Operations](#account-operations)
5. [Customer Operations](#customer-operations)
6. [Employee Operations](#employee-operations)
7. [Branch Operations](#branch-operations)
8. [Error Handling](#error-handling)
9. [Best Practices](#best-practices)

## Overview

This document provides comprehensive documentation for the Banking System API. All operations are performed through TCP socket connections with JSON-formatted messages.

## Connection Details

- **Protocol**: TCP
- **Default Port**: 8080
- **Message Format**: JSON
- **Encoding**: UTF-8
- **ZMQ Internal Port**: 5501 (for internal services)

## General Format

All requests follow this basic structure:

```json
{
    "Operation": "<operation_type>",
    "OperationType": "<action>",
    "ColumnName": "<field_name>",
    "Data": [<array_of_values>]
}
```


Common Operation Types:
- `get`: Retrieve information
- `set`: Modify existing records
- `add`: Create new records
- `delete`: Remove records

---

## Account Operations

### 1. Get Account Balance

**Format:**
```json
{
    "Operation": "Account",
    "OperationType": "get",
    "ColumnName": "Account_balance",
    "Data": ["<account_id>"]
}
```

**Example:**
```json
{
    "Operation": "Account",
    "OperationType": "get",
    "ColumnName": "Account_balance",
    "Data": ["20230809MYS00101"]
}
```

**Response:**
```json
{
    "status": "success",
    "data": [{
        "Account_balance": "5000.00"
    }]
}
```


### 2. Update Account Balance

**Format:**
```json
{
    "Operation": "Account",
    "OperationType": "set",
    "ColumnName": "Account_balance",
    "Data": ["<account_id>", "<new_balance>"]
}
```

**Example:**
```json
{
    "Operation": "Account",
    "OperationType": "set",
    "ColumnName": "Account_balance",
    "Data": ["20230809MYS00101", "6000.00"]
}
```


### 3. Create New Account

**Format:**
```json
{
    "Operation": "Account",
    "OperationType": "add",
    "Data": [
        "<account_id>",
        "<account_type>",
        "<initial_balance>",
        "<customer_id>",
        "<branch_id>"
    ]
}
```

**Example:**
```json
{
    "Operation": "Account",
    "OperationType": "add",
    "Data": [
        "20230809MYS00102",
        "Savings",
        "1000.00",
        "CUS001",
        "BR001"
    ]
}
```


### 4. Delete Account

**Format:**
```json
{
    "Operation": "Account",
    "OperationType": "delete",
    "Data": ["<account_id>"]
}
```

**Example:**
```json
{
    "Operation": "Account",
    "OperationType": "delete",
    "Data": ["20230809MYS00101"]
}
```

---

## Customer Operations

### 1. Get Customer Details

**Format:**
```json
{
    "Operation": "Customer",
    "OperationType": "get",
    "ColumnName": "<field_name>",
    "Data": ["<customer_id>"]
}
```

**Example:**
```json
{
    "Operation": "Customer",
    "OperationType": "get",
    "ColumnName": "Customer_name",
    "Data": ["CUS001"]
}
```


### 2. Create New Customer

**Format:**
```json
{
    "Operation": "Customer",
    "OperationType": "add",
    "Data": [
        "<customer_id>",
        "<name>",
        "<password>",
        "<account_number>",
        "<address>",
        "<branch_id>"
    ]
}
```

**Example:**
```json
{
    "Operation": "Customer",
    "OperationType": "add",
    "Data": [
        "CUS002",
        "John Doe",
        "pass456",
        "ACC123",
        "123 Main St, City",
        "BR002"
    ]
}
```


### 3. Update Customer Information

**Format:**
```json
{
    "Operation": "Customer",
    "OperationType": "set",
    "ColumnName": "<field_to_update>",
    "Data": ["<customer_id>", "<new_value>"]
}
```

**Example:**
```json
{
    "Operation": "Customer",
    "OperationType": "set",
    "ColumnName": "Customer_address",
    "Data": ["CUS001", "888 New Address"]
}
```

---

## Employee Operations

### 1. Get Employee Details

**Format:**
```json
{
    "Operation": "Employee",
    "OperationType": "get",
    "ColumnName": "<field_name>",
    "Data": ["<employee_id>"]
}
```

**Example:**
```json
{
    "Operation": "Employee",
    "OperationType": "get",
    "ColumnName": "Employee_name",
    "Data": ["EMP001"]
}
```


### 2. Create New Employee

**Format:**
```json
{
    "Operation": "Employee",
    "OperationType": "add",
    "Data": [
        "<employee_id>",
        "<name>",
        "<password>",
        "<designation>",
        "<address>",
        "<branch_id>"
    ]
}
```

**Example:**
```json
{
    "Operation": "Employee",
    "OperationType": "add",
    "Data": [
        "EMP002",
        "Jane Smith",
        "pass789",
        "Teller",
        "456 Main St",
        "BR001"
    ]
}
```

---

## Branch Operations

### 1. Get Branch Details

**Format:**
```json
{
    "Operation": "Branch",
    "OperationType": "get",
    "ColumnName": "<field_name>",
    "Data": ["<branch_id>"]
}
```

**Example:**
```json
{
    "Operation": "Branch",
    "OperationType": "get",
    "ColumnName": "Branch_address",
    "Data": ["BR001"]
}
```


### 2. Create New Branch

**Format:**
```json
{
    "Operation": "Branch",
    "OperationType": "add",
    "Data": [
        "<branch_id>",
        "<name>",
        "<city>",
        "<address>",
        "ACTIVE",
        "<manager_id>"
    ]
}
```

**Example:**
```json
{
    "Operation": "Branch",
    "OperationType": "add",
    "Data": [
        "BR002",
        "Downtown Branch",
        "Metropolis",
        "456 Main St, City",
        "ACTIVE",
        "EMP001"
    ]
}
```

---

## Error Handling

All responses include a status field indicating success or failure:

```json
{
    "status": "<success/error>",
    "message": "<description>",
    "data": [<array_of_results>]  // Only present for GET operations
}
```

Common Error Responses:
```json
{
    "status": "error",
    "message": "Operation not found"
}
```

```json
{
    "status": "error",
    "message": "Invalid data format"
}
```

## Best Practices

1. **Data Validation**
   - Validate all input data before sending
   - Ensure proper data types (strings for IDs, numbers for amounts)
   - Use proper date formats (YYYY-MM-DD)

2. **Error Handling**
   - Implement proper error handling for all requests
   - Handle timeout scenarios
   - Implement retry logic for failed connections

3. **Security**
   - Use secure connections when possible
   - Validate all input data
   - Implement proper authentication (if required)
   - Handle sensitive data appropriately

4. **Performance**
   - Keep connections alive when making multiple requests
   - Batch operations when possible
   - Handle large datasets appropriately

5. **Testing**
   - Test all operations with valid and invalid data
   - Test error scenarios
   - Test with different data types and sizes

## Field Formats

1. **IDs**
   - Account ID: YYYYMMDDLOC#####
   - Customer ID: CUS###
   - Employee ID: EMP###
   - Branch ID: BR###

2. **Monetary Values**
   - Use strings with 2 decimal places
   - Example: "1000.00"

3. **Dates**
   - Format: YYYY-MM-DD
   - Example: "2025-11-02"

4. **Phone Numbers**
   - Include country code
   - Example: "+1234567890"

5. **Email Addresses**
   - Standard email format
   - Example: "user@domain.com"