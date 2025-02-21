### **🚀 Description**
This project is a **dynamic .NET injector DLL** that can be injected into a process and execute **.NET code dynamically** inside its memory. It supports both **.NET Framework (v2.0 - v4.8)** and **.NET Core / .NET 5 / .NET 6+** applications.

---

## **🎯 Features**
✔ **Supports Injection into Any Process** – Execute .NET assemblies dynamically from a DLL.  
✔ **Works with .NET Framework & .NET Core** – Detects and loads the correct runtime dynamically.  
✔ **Supports x86 and x64 Processes** – Ensures compatibility with both architectures.  
✔ **Executes Methods in Managed Assemblies** – Calls `.NET` methods inside an injected process.  
✔ **Full Debug Logging** – Uses a console for error reporting and debugging.  
✔ **Standalone DLL** – No dependencies other than Windows API and .NET runtime.  

---

## **💻 Supported Platforms**
This project supports the following **Windows versions**:
- **Windows 10 / 11**
- **Windows Server 2016 / 2019 / 2022**
- **Windows 7 / 8.1** *(with .NET installed)*

**Architectures:**
- ✅ **x86 (32-bit)**
- ✅ **x64 (64-bit)**

**Supported .NET Runtimes:**
- ✅ **.NET Framework** (2.0, 3.5, 4.0, 4.8)
- ✅ **.NET Core** (2.1, 3.1)
- ✅ **.NET 5, .NET 6, .NET 7+**

---

## **📜 Requirements**
Before using this project, ensure you have the following installed:

### **🔹 For .NET Framework Injection**
- .NET Framework **(v2.0 - v4.8)** installed
- Target process must be **.NET-based** (if injecting into a .NET app)

### **🔹 For .NET Core/.NET 6+ Injection**
- .NET Core Runtime **or** .NET 6+ installed
- If injecting into a **.NET Core/.NET 6+ application**, make sure `coreclr.dll` is available.

### **🔹 Compilation Requirements**
- **Windows SDK**
- **Visual Studio 2019/2022**
- **C++ Compiler**
- **CMake (Optional for cross-platform builds)**

---

## **🛠️ How It Works**
### **1️⃣ Inject the DLL into a Target Process**
The DLL is injected into a process using standard Windows injection methods **(e.g., CreateRemoteThread, Manual Mapping, etc.)**.

### **2️⃣ Locate the .NET Runtime**
The injected DLL checks:
- If the target **uses .NET Framework**, it loads `clr.dll`.
- If the target **uses .NET Core**, it loads `coreclr.dll`.

### **3️⃣ Execute a .NET Method**
- The DLL **dynamically loads** an **EXE or DLL assembly**.
- It **calls a specified method** inside the assembly.
- Passes a **string parameter** if needed.

### **4️⃣ Return Execution to the Host**
- The injected process continues running normally.

---

## **📌 Usage**
1️⃣ **Compile the DLL**  
```bash
git clone https://github.com/yourrepo/dotnet-injector.git
cd dotnet-injector
cmake .
make
```

2️⃣ **Inject the DLL into a target process**  
Use any DLL injection method:
```cpp
HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, targetProcessID);
LPVOID pRemoteMemory = VirtualAllocEx(hProcess, NULL, dllSize, MEM_COMMIT, PAGE_READWRITE);
WriteProcessMemory(hProcess, pRemoteMemory, dllData, dllSize, NULL);
CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pRemoteMemory, NULL, 0, NULL);
```

3️⃣ **DLL Executes the .NET Assembly**
```cpp
std::wstring assemblyName = L"MyApp.exe";
std::wstring className = L"MyNamespace.MyClass";
std::wstring methodName = L"Start";
std::wstring paramValue = L"Injected!";
```
- The DLL dynamically **loads MyApp.exe** inside the target.
- Calls **MyNamespace.MyClass.Start("Injected!")**.

---

## **⚡ Example: Calling a .NET Method**
To test the DLL, create a **simple .NET Console App**:

```csharp
using System;

namespace MyNamespace
{
    class MyClass
    {
        static int Start(string message)
        {
            Console.WriteLine($"Injected: {message}");
            return 0;
        }
    }
}
```

Compile the DLL and set:
```cpp
std::wstring assemblyName = L"MyApp.exe";
std::wstring className = L"MyNamespace.MyClass";
std::wstring methodName = L"Start";
std::wstring paramValue = L"Injected!";
```

---


## **⚠️ Legal Disclaimer**
This software is provided **for educational and research purposes only**.  
**Misuse of this software for malicious activities is strictly prohibited.**  
The author is **not responsible** for any damage or misuse of this tool.

---

## **📜 License**
This project is licensed under the **MIT License**.  
Feel free to modify, distribute, and use for legal purposes.  

---

## **📢 Contributing**
Want to contribute? Feel free to submit:
- **Feature requests**
- **Bug reports**
- **Pull requests**

Fork the repo and start hacking! 🚀  

🚀 **Enjoy this project? Give it a star ⭐ and share!**
