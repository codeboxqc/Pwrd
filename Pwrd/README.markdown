# Pwrd

Pwrd is a secure password management application designed to store and manage sensitive credentials with robust encryption. Built in C++ using the Windows API, it provides a user-friendly interface for creating, verifying, and accessing password-protected data stored in XML format. This guide details how to compile, install, and run the Pwrd application, including all dependencies and external packages required.

## Table of Contents
- [Prerequisites](#prerequisites)
- [Dependencies](#dependencies)
- [External Packages](#external-packages)
- [Installation](#installation)
- [Compilation](#compilation)
- [Usage](#usage)
- [Contributing](#contributing)
- [License](#license)

## Prerequisites
Before compiling and installing Pwrd, ensure your system meets the following requirements:
- **Operating System**: Windows 10 or later (64-bit)
- **Development Environment**:
  - Microsoft Visual Studio 2022 (Community, Professional, or Enterprise) with C++ Desktop Development workload
  - Windows SDK (version 10.0.22621.0 or later)
- **Disk Space**: Approximately 500 MB for dependencies and build artifacts
- **Internet Connection**: Required to download external packages and dependencies

## Dependencies
Pwrd relies on the following libraries and tools, which are either included in the repository or need to be installed:

- **Windows API**: Used for GUI components and file operations (included with Windows SDK)
- **MSXML6**: Microsoft's XML parser for handling XML data files (included with Windows)
- **Standard C++ Library**: For core functionality (included with Visual Studio)
- **Crypto++**: A C++ library for cryptographic operations (included as a submodule)

### Crypto++ Submodule
The Crypto++ library is included as a Git submodule in the repository. To initialize and update it:
1. Clone the Pwrd repository with submodules:
   ```bash
   git clone --recursive https://github.com/codeboxqc/Pwrd.git
   ```
2. If you already cloned the repository without submodules, run:
   ```bash
   git submodule init
   git submodule update
   ```

## External Packages
No additional external packages beyond those listed above are required. All necessary libraries (e.g., Crypto++) are bundled with the repository as submodules or provided by the Windows SDK and Visual Studio.

## Installation
Follow these steps to set up the development environment and install dependencies:

1. **Install Visual Studio 2022**:
   - Download from [Visual Studio Downloads](https://visualstudio.microsoft.com/downloads/).
   - During installation, select the **Desktop development with C++** workload.
   - Ensure the following components are included:
     - Windows 10 SDK (10.0.22621.0 or later)
     - C++ CMake tools for Windows
     - English language pack

2. **Verify Windows SDK**:
   - Open Visual Studio Installer, go to **Modify**, and confirm the Windows SDK is installed.
   - Alternatively, check for `C:\Program Files (x86)\Windows Kits\10` on your system.

3. **Clone the Repository**:
   ```bash
   git clone --recursive https://github.com/codeboxqc/Pwrd.git
   cd Pwrd
   ```

4. **Initialize Crypto++ Submodule** (if not already done):
   ```bash
   git submodule init
   git submodule update
   ```

## Compilation
To compile the Pwrd application, follow these steps:

1. **Open the Project**:
   - Launch Visual Studio 2022.
   - Open the `Pwrd.sln` solution file located in the root of the repository.

2. **Configure Build Settings**:
   - Set the **Solution Configuration** to `Release` or `Debug` (recommended for development).
   - Set the **Solution Platform** to `x64` (Pwrd is designed for 64-bit systems).
   - Ensure the C++ Language Standard is set to **C++17** or later (Project Properties > C/C++ > Language).

3. **Build Crypto++ Library**:
   - Navigate to the Crypto++ submodule directory (e.g., `extern/cryptopp`).
   - Open `cryptopp.sln` in Visual Studio.
   - Build the `cryptlib` project for `x64` in