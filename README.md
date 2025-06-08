Pwrd
Pwrd is a secure password management application designed to store and manage sensitive credentials with robust encryption. Built in C++ using the Windows API, it provides a user-friendly interface for creating, verifying, and accessing password-protected data stored in XML format. This guide details how to compile, install, and run the Pwrd application, including all dependencies and external packages required.
Table of Contents

Prerequisites
Dependencies
External Packages
Installation
Compilation
Usage
Contributing
License

Prerequisites
Before compiling and installing Pwrd, ensure your system meets the following requirements:

Operating System: Windows 10 or later (64-bit)
Development Environment:
Microsoft Visual Studio 2022 (Community, Professional, or Enterprise) with C++ Desktop Development workload
Windows SDK (version 10.0.22621.0 or later)


Disk Space: Approximately 500 MB for dependencies and build artifacts
Internet Connection: Required to download external packages and dependencies

Dependencies
Pwrd relies on the following libraries and tools, which are either included in the repository or need to be installed:

Windows API: Used for GUI components and file operations (included with Windows SDK)
MSXML6: Microsoft's XML parser for handling XML data files (included with Windows)
Standard C++ Library: For core functionality (included with Visual Studio)
Crypto++: A C++ library for cryptographic operations (included as a submodule)

Crypto++ Submodule
The Crypto++ library is included as a Git submodule in the repository. To initialize and update it:

Clone the Pwrd repository with submodules:git clone --recursive https://github.com/codeboxqc/Pwrd.git


If you already cloned the repository without submodules, run:git submodule init
git submodule update



External Packages
No additional external packages beyond those listed above are required. All necessary libraries (e.g., Crypto++) are bundled with the repository as submodules or provided by the Windows SDK and Visual Studio.
Installation
Follow these steps to set up the development environment and install dependencies:

Install Visual Studio 2022:

Download from Visual Studio Downloads.
During installation, select the Desktop development with C++ workload.
Ensure the following components are included:
Windows 10 SDK (10.0.22621.0 or later)
C++ CMake tools for Windows
English language pack




Verify Windows SDK:

Open Visual Studio Installer, go to Modify, and confirm the Windows SDK is installed.
Alternatively, check for C:\Program Files (x86)\Windows Kits\10 on your system.


Clone the Repository:
git clone --recursive https://github.com/codeboxqc/Pwrd.git
cd Pwrd


Initialize Crypto++ Submodule (if not already done):
git submodule init
git submodule update



Compilation
To compile the Pwrd application, follow these steps:

Open the Project:

Launch Visual Studio 2022.
Open the Pwrd.sln solution file located in the root of the repository.


Configure Build Settings:

Set the Solution Configuration to Release or Debug (recommended for development).
Set the Solution Platform to x64 (Pwrd is designed for 64-bit systems).
Ensure the C++ Language Standard is set to C++17 or later (Project Properties > C/C++ > Language).


Build Crypto++ Library:

Navigate to the Crypto++ submodule directory (e.g., extern/cryptopp).
Open cryptopp.sln in Visual Studio.
Build the cryptlib project for x64 in Release or Debug mode to generate cryptlib.lib.
Note the output path (e.g., extern/cryptopp/x64/Output/Release/cryptlib.lib).


Link Crypto++ in Pwrd:

In the Pwrd project properties:
Add extern/cryptopp to Additional Include Directories (C/C++ > General).
Add the Crypto++ library path (e.g., extern/cryptopp/x64/Output/Release) to Additional Library Directories (Linker > General).
Add cryptlib.lib to Additional Dependencies (Linker > Input).




Build the Solution:

In Visual Studio, select Build > Build Solution or press F7.
The executable (Pwrd.exe) will be generated in the output directory (e.g., x64/Release or x64/Debug).


Verify Build:

Check the output directory for Pwrd.exe.
Ensure no unresolved external symbols or missing library errors occurred during the build.



Usage
To run Pwrd after compilation:

Launch the Application:

Double-click Pwrd.exe in the output directory, or run it from Visual Studio (F5 for Debug mode).
On first run, you’ll be prompted to create a password if password.pgp does not exist.


Create or Verify Password:

New Users: Enter a strong password to initialize the password.pgp file, which stores the salt and derived key.
Existing Users: Enter the previously set password to access the encrypted data.xml file.


Manage Passwords:

Use the GUI to add, edit, or delete credentials stored in data.xml.
All data is encrypted using a key derived from your password and a random salt.


File Locations:

password.pgp: Stores the salt and derived key (created in the executable’s directory).
data.xml: Stores encrypted credentials (created in the executable’s directory).



Contributing
Contributions to Pwrd are welcome! To contribute:

Fork the repository.
Create a feature branch (git checkout -b feature/your-feature).
Commit your changes (git commit -m "Add your feature").
Push to the branch (git push origin feature/your-feature).
Open a pull request with a detailed description of your changes.

Please read CONTRIBUTING.md for guidelines and the code of conduct.
License
This project is licensed under the MIT License. See the LICENSE file for details.
