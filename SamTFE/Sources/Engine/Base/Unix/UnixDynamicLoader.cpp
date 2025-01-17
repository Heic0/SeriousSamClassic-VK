/* Copyright (c) 2002-2012 Croteam Ltd. All rights reserved. */

/* rcg10072001 Implemented. */

#include <dlfcn.h>

#include <Engine/Engine.h>
#include <Engine/Base/DynamicLoader.h>

class CUnixDynamicLoader : public CDynamicLoader
{
public:
    CUnixDynamicLoader(const char *libname);
    virtual ~CUnixDynamicLoader(void);
    virtual void *FindSymbol(const char *sym);
    virtual const char *GetError(void);

protected:
    void DoOpen(const char *lib);
    void SetError(void);
    void *module;
    CTString *err;
};


CDynamicLoader *CDynamicLoader::GetInstance(const char *libname)
{
    return(new CUnixDynamicLoader(libname));
}


void CUnixDynamicLoader::SetError(void)
{
    const char *errmsg = ::dlerror();
    delete err;
    err = NULL;

    if (errmsg != NULL)
    {
        CPrintF("CUnixDynamicLoader error: %s\n", errmsg);
        err = new CTString(errmsg);
    }
}


const char *CUnixDynamicLoader::GetError(void)
{
    return((err) ? (const char *) (*err) : NULL);
}


void *CUnixDynamicLoader::FindSymbol(const char *sym)
{
    //printf("Looking for symbol %s\n", sym);
    void *retval = NULL;
    if (module != NULL) {
        retval = ::dlsym(module, sym);
        SetError();
    }

    return(retval);
}


void CUnixDynamicLoader::DoOpen(const char *lib)
{
    #ifdef PLATFORM_FREEBSD
    dlerror(); // need for clean Undefined symbol "_nss_cache_cycle_prevention_function" message
    #endif

    CTFileName fnmLib = CTString(lib);
    CTFileName fnmLibname = fnmLib.FileName();
    int _libvorbisfile   = strncmp((const char *)fnmLibname, (const char *) "libvorbisfile", (size_t) 13 ); // skip
    if( _pShell->GetINDEX("sys_iSysPath") == 1 && _libvorbisfile !=0 ) {
        fnmLib = _fnmModLibPath + _fnmMod + fnmLib.FileName() + fnmLib.FileExt();
    }
    module = ::dlopen((const char *)fnmLib, RTLD_LAZY | RTLD_GLOBAL);
    #ifndef PLATFORM_MACOSX
    if (_libvorbisfile == 0 && module == NULL) { // if libvorbisfile.so not open trying libvorbisfile.so.3
        CPrintF("Trying load libvorbisfile.so.3 ...\n");
        fnmLib = fnmLibname + CTString(".so.3"); // libvorbisfile.so.3
        module = ::dlopen((const char *)fnmLib, RTLD_LAZY | RTLD_GLOBAL);
    }
    #endif
    SetError();
}


CTFileName CDynamicLoader::ConvertLibNameToPlatform(const char *libname)
{
    #if PLATFORM_MACOSX
    const char *DLLEXTSTR = ".dylib";
    #else
    const char *DLLEXTSTR = ".so";
    #endif
    CTFileName fnm = CTString(libname);
    CTString libstr((strncmp("lib", fnm.FileName(), 3) == 0) ? "" : "lib");
    return(fnm.FileDir() + libstr + fnm.FileName() + DLLEXTSTR);
}


CUnixDynamicLoader::CUnixDynamicLoader(const char *libname)
    : module(NULL),
      err(NULL)
{
    if (libname == NULL) {
        DoOpen(NULL);
    } else {
        CTFileName fnm = ConvertLibNameToPlatform(libname);

        // Always try to dlopen from inside the game dirs before trying
        //  system libraries...
        if (fnm.FileDir() == "") {
            char buf[MAX_PATH];
            _pFileSystem->GetExecutablePath(buf, sizeof (buf));
            CTFileName fnmDir = CTString(buf);
            fnmDir = fnmDir.FileDir() + fnm;
            DoOpen(fnmDir);
            if (module != NULL) {
                return;
            }
        }

        DoOpen(fnm);
    }
}


CUnixDynamicLoader::~CUnixDynamicLoader(void)
{
    delete err;
    if (module != NULL)
        ::dlclose(module);
}


// end of UnixDynamicLoader.cpp ...


