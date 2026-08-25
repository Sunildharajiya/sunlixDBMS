# SunlixDBMS

SunlixDBMS is a custom database management system built from scratch in C.

The project is focused on understanding how a DBMS works internally instead of relying on an existing database engine or SQL implementation.

SunlixDBMS currently uses JSON for data storage and provides its own command-line interface for performing database operations.

> **Status:** Active development  
> SunlixDBMS is currently a learning-focused and experimental project. APIs, file formats, and internal architecture may change.

---

## Features

- Custom command-line interface
- JSON-based data storage
- CRUD operations
- Record-based unique keys
- File reading and writing
- JSON parsing with cJSON
- Modular C architecture
- Automatic source-file discovery through Makefile
- GCC and Clang compiler support

---

## Current CRUD Operations

| Operation | Status |
|-----------|--------|
| Create | ✅ Implemented |
| Read | 🚧 In development |
| Update | ✅ Implemented |
| Delete | ✅ Implemented |

More database functionality will be added as development continues.

---

## Project Structure

```text
sunlixDBMS/
├── src/
│   ├── main.c
│   └── functions/
│       ├── cli/
│       ├── crud/
│       └── utility/
│
├── data/
│   └── users.json
│
├── packages/
│   └── cJSON/
│
├── documentation/
│   ├── README.md
│   ├── architecture.md
│   ├── crud.md
│   ├── cli.md
│   └── ...
│
├── Makefile
├── README.md
└── .gitignore
```
___________
# Getting Started

## Requirements
Before building SunlixDBMS, make sure you have:
- A C compiler (GCC,Clang
or another C11-compatible compiler )
- GNU Make
  
SunlixDBMS uses the C11 standard

## Clone the Repository
```bash
git clone git@github.com:Sunildharajiya/sunlixDBMS.git && cd sunlixDBMS
```
## Build
Build SunlixDBMS using:
```bash
:$/make
```
The Makefile automatically:
- Detects the available compiler
- Finds C source files inside src/
- Compiles the source files
- Compiles cJSON
- Links everything into the sunlixDBMS executable

## Run
After building Run:
```bash
./sunlixDBMS
```
You should see:
```bash
Starting the main application...
Welcome to SunlixDBMS CLI!
Type 'help' for available commands.
my_cli>
```

# Documentation

Detailed documentation is maintained separately from the main README.

- [Documentation](./Documentetion)

# Contributing
Contributions, ideas, bug reports, and improvements are welcome.

Before making major changes, please check the existing issues and documentation.

For larger architectural changes, opening an issue first is recommended and than make pull req.

## License
See the repository license file for licensing information.
