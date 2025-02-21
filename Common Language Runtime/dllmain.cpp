#define WIN32_LEAN_AND_MEAN // Optimize Windows headers
#include "pch.h"
#include <metahost.h>  // For CLR (Common Language Runtime) hosting
#include <Windows.h>   // Windows API
#include <mscoree.h>   // CLR interfaces (ICLRMetaHost, ICLRRuntimeHost, etc.)
#include <wchar.h>
#include <shlwapi.h>   // Safe file path operations (PathCombineW, PathRemoveFileSpecW)
#include <fstream>
#include <iostream>
#include <process.h>
#include "CorError.h"  // .NET error codes

// Link necessary libraries
#pragma comment(lib, "mscoree.lib") // .NET Core execution engine
#pragma comment(lib, "Shlwapi.lib") // Windows Shell Lightweight Utility API (path handling)

// Global variable to store the DLL handle
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
HMODULE g_hModule = NULL;

// ✅ Dynamic Variables (Configuration)
std::wstring assemblyName = L"Name.Dll or Name.Exe";  // Target .NET assembly
std::wstring className = L"WindowsFormsApp1.Program"; // Target class (namespace.class)
std::wstring methodName = L"";  // Target method inside the class
std::wstring paramValue = L"It works!!"; // Parameter to pass to the managed method

// ✅ Print error messages to the console
void PrintError(const std::wstring& message, HRESULT hr) {
    std::wcerr << L"[ERROR] " << message << L" (HRESULT: 0x" << std::hex << hr << L")" << std::endl;
}

// ✅ Print info/debug messages to the console
void PrintInfo(const std::wstring& message) {
    std::wcout << L"[INFO] " << message << std::endl;
}

// ✅ Get the full path of the module (DLL or EXE)
wchar_t* GetModulePath(HMODULE hModule, const std::wstring& fileName) {
    wchar_t buffer[MAX_PATH];   // Buffer to store the module path
    wchar_t fullPath[MAX_PATH]; // Buffer for the final path

    // Get the file path of the currently executing module
    if (!GetModuleFileNameW(hModule, buffer, MAX_PATH)) {
        PrintError(L"Failed to get module file name.", GetLastError());
        return nullptr;
    }

    // Remove the file name, leaving only the directory
    PathRemoveFileSpecW(buffer);

    // Combine the directory path with the target file name
    if (!PathCombineW(fullPath, buffer, fileName.c_str())) {
        PrintError(L"Failed to combine path.", E_FAIL);
        return nullptr;
    }

    // Allocate memory for the full path and return it
    wchar_t* dllLocation = new wchar_t[wcslen(fullPath) + 1];
    wcscpy_s(dllLocation, wcslen(fullPath) + 1, fullPath);
    return dllLocation;
}

// ✅ Check if a file exists
bool FileExists(const wchar_t* path) {
    DWORD dwAttrib = GetFileAttributesW(path);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

// ✅ Check if the target EXE is using .NET Core / .NET 6+
bool IsNetCoreAssembly(const std::wstring& exePath) {
    std::wstring runtimeConfigPath = exePath + L".runtimeconfig.json"; // Check if a runtime config exists
    return FileExists(runtimeConfigPath.c_str()); // If the config file exists, it's a .NET Core assembly
}

// ✅ Initialize the console for debugging output
void InitializeConsole() {
    AllocConsole();  // Create a new console window
    freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout); // Redirect stdout
    freopen_s(reinterpret_cast<FILE**>(stderr), "CONOUT$", "w", stderr); // Redirect stderr
    PrintInfo(L"Console initialized."); // Debug message
}

// ✅ Load .NET Core (coreclr.dll) for .NET 6+ applications
bool LoadNetCore(const std::wstring& exePath) {
    std::wstring coreclrPath = L"C:\\Program Files\\dotnet\\shared\\Microsoft.NETCore.App\\";
    coreclrPath += L"6.0.0\\coreclr.dll"; // Hardcoded .NET 6.0 path (should be dynamic in a real implementation)

    if (!FileExists(coreclrPath.c_str())) {
        PrintError(L"coreclr.dll not found! Ensure .NET Core/.NET 6+ is installed.", E_FAIL);
        return false;
    }

    PrintInfo(L"Loading .NET Core runtime: " + coreclrPath);
    HMODULE hCoreCLR = LoadLibraryW(coreclrPath.c_str()); // Load coreclr.dll

    if (!hCoreCLR) {
        PrintError(L"Failed to load coreclr.dll (NET 6+).", GetLastError());
        return false;
    }

    PrintInfo(L".NET Core successfully loaded!");
    return true;
}

// ✅ Main function that runs inside the injected process
DWORD WINAPI Main(LPVOID lpParam) 
{
#if __DEBUG
    InitializeConsole();
#endif // __DEBUG

    PrintInfo(L"Injected DLL Main function started...");

    // Get the full path to the target .NET assembly
    wchar_t* tempPath = GetModulePath(g_hModule, assemblyName);
    if (!tempPath) {
        PrintError(L"Failed to get module path.", E_FAIL);
        return 1;
    }

    PrintInfo(L"Resolved assembly path: " + std::wstring(tempPath));

    if (!FileExists(tempPath)) {
        PrintError(L"ERROR: Target .DLL or .EXE not found!", 0x80070002);
        delete[] tempPath;
        return 1;
    }

    bool isNetCore = IsNetCoreAssembly(tempPath);
    PrintInfo(isNetCore ? L"Detected .NET Core / .NET 6+" : L"Detected .NET Framework");

    // If the target EXE is a .NET Core app, load the CoreCLR
    if (isNetCore) {
        if (!LoadNetCore(tempPath)) {
            PrintError(L"Failed to load .NET Core runtime!", E_FAIL);
            delete[] tempPath;
            return 1;
        }
        PrintInfo(L"Successfully initialized .NET Core runtime!");
        delete[] tempPath;
        return 0;
    }

    // ✅ Load .NET Framework using CLR
    ICLRMetaHost* metaHost = nullptr;
    ICLRRuntimeInfo* runtimeInfo = nullptr;
    ICLRRuntimeHost* runtimeHost = nullptr;

    HRESULT hr = CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost, (LPVOID*)&metaHost);
    if (FAILED(hr)) {
        PrintError(L"CLRCreateInstance failed.", hr);
        delete[] tempPath;
        return 1;
    }

    // Find the highest available .NET Framework version
    wchar_t bestVersion[50] = L"";
    IEnumUnknown* runtimeEnum = nullptr;
    IUnknown* unknownRuntime = nullptr;

    hr = metaHost->EnumerateInstalledRuntimes(&runtimeEnum);
    while (runtimeEnum->Next(1, &unknownRuntime, NULL) == S_OK) {
        if (unknownRuntime->QueryInterface(IID_ICLRRuntimeInfo, (LPVOID*)&runtimeInfo) == S_OK) {
            wchar_t versionString[50];
            DWORD versionSize = 50;
            runtimeInfo->GetVersionString(versionString, &versionSize);
            PrintInfo(L"Found .NET Version: " + std::wstring(versionString));
            if (wcscmp(versionString, bestVersion) > 0) {
                wcscpy_s(bestVersion, versionString);
            }
            runtimeInfo->Release();
        }
        unknownRuntime->Release();
    }

    PrintInfo(L"Using highest available .NET Framework: " + std::wstring(bestVersion));

    // Start the .NET runtime and execute the method
    runtimeHost->Start();
    DWORD pReturnValue = 0;
    hr = runtimeHost->ExecuteInDefaultAppDomain(tempPath, className.c_str(), methodName.c_str(), paramValue.c_str(), &pReturnValue);

    if (FAILED(hr))
    {
        PrintError(L"Failed to execute managed method!", hr);
    }
    else 
    {
        PrintInfo(L"Managed method executed successfully!");
    }

    // Cleanup
    runtimeHost->Release();
    runtimeInfo->Release();
    metaHost->Release();
    delete[] tempPath;

    return 0;
}



BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        g_hModule = hModule;  // ✅ Store the DLL module handle globally
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, Main, hModule, 0, nullptr);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
