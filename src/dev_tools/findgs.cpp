#include <iostream>
#include <cstring>
#include <iapi.h>
#include <gserrors.h>
#include <ierrors.h>
#include <gdevdsp.h>
using namespace std;

#ifdef _WIN32
    #include <windows.h>
    const char* libName ="gsdll64.dll";  // or "gsdll32.dll" for 32-bit
    typedef HMODULE LibHandle;
    #define LOAD_LIBRARY(name) LoadLibraryA(name)
    #define GET_SYMBOL(handle, name) GetProcAddress(handle, name)
    #define CLOSE_LIBRARY(handle) FreeLibrary(handle)
    #define LAST_ERROR GetLastError()
#else
    #include <dlfcn.h>
    #ifdef __APPLE__
    const char* libName = "libgs.dylib";
    #else
    const char* libName = "libgs.so";
    #endif
    typedef void* LibHandle;
    #define LOAD_LIBRARY(name) dlopen(name, RTLD_LAZY)
    #define GET_SYMBOL(handle, name) dlsym(handle, name)
    #define CLOSE_LIBRARY(handle) dlclose(handle)
    #define LAST_ERROR dlerror()
#endif

// Define the function pointer type
//typedef int (*gsapi_revision_func)(gsapi_revision_t*, int);
//typedef void* (*gsapi_new_instance_func)(void**, void*);
//typedef void (*gsapi_delete_instance_func)(void*);

int main() {
    cout << "Finding Ghostscript Library"<<endl;
    LibHandle handle = LOAD_LIBRARY(libName);
    if (!handle) {
        cerr << "Failed to load " << libName << ": " << LAST_ERROR << endl;
        return 1;
    }
    // Clear any existing errors
    LAST_ERROR;
    // load without definition
    {
        void* gsapi_revision = (void*)GET_SYMBOL(handle, "gsapi_revision");
        if(!gsapi_revision){
            cerr << "ERROR cannot find gsapi_revision" << endl;
        }
        // get the path to the shared library
        #ifdef _WIN32
            HMODULE hMod = nullptr;
            if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)gsapi_revision, &hMod)) {
                char path[MAX_PATH];
                if (GetModuleFileNameA(hMod, path, MAX_PATH)) {
                    cout << "(void*) Library loaded from: " << path << endl;
                }
            }
        #else
            Dl_info dlinfo;
            if (dladdr((void*)gsapi_revision, &dlinfo) && dlinfo.dli_fname) {
                cout << "(void*) Library loaded from: " << dlinfo.dli_fname << endl;
            } else {
                cerr << "dladdr failed to retrieve library path." << endl;
            }
        #endif
    }

    // Load the gsapi_revision symbol
    PFN_gsapi_revision gsapi_revision = (PFN_gsapi_revision)GET_SYMBOL(handle, "gsapi_revision");
    if(!gsapi_revision){
        cerr << "ERROR cannot find gsapi_revision" << endl;
    }
    // get the path to the shared library
    #ifdef _WIN32
        HMODULE hMod = nullptr;
        if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)gsapi_revision, &hMod)) {
            char path[MAX_PATH];
            if (GetModuleFileNameA(hMod, path, MAX_PATH)) {
                cout << "Library loaded from: " << path << endl;
            }
        }
    #else
        Dl_info dlinfo;
        if (dladdr((void*)gsapi_revision, &dlinfo) && dlinfo.dli_fname) {
            cout << "Library loaded from: " << dlinfo.dli_fname << endl;
        } else {
            cerr << "dladdr failed to retrieve library path." << endl;
        }
    #endif

    // Call the function to gete the revision information
    gsapi_revision_t info;

    int result = gsapi_revision(&info, sizeof(info));

    if (result == 0) {
        cout << "Ghostscript Version Info:" << endl;
        cout << "Product: " << info.product << endl;
        cout << "Copyright: " << info.copyright << endl;
        cout << "Revision: " << info.revision << endl;
        cout << "Revision Date: " << info.revisiondate << endl;
    } else {
        cerr << "gsapi_revision call failed with code: " << result << endl;
    }
    CLOSE_LIBRARY(handle);
    return 0;
}
