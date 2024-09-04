// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "CTestAdapter.h"
#include "BaseComm.h"


__declspec(dllexport) cfm::adapters::CTestAdapter* CreateObject() {
    cfm::adapters::CTestAdapter* pp = new cfm::adapters::CTestAdapter();
    cfm::application::CComm* r = dynamic_cast<cfm::application::CComm*> (pp);
    return pp;
}
__declspec(dllexport) void DestroyObject(void* p)
{
    cfm::adapters::CTestAdapter* pp = (cfm::adapters::CTestAdapter*)p;
    pp->Stop();
    cfm::application::CComm* r = dynamic_cast<cfm::application::CComm*> (pp);
    delete pp;
}

//Get interface for SID control and monitoring
__declspec(dllexport) cfm::application::CComm* GetCCommInterface(void* pObj)
{
    cfm::adapters::CTestAdapter* pp = (cfm::adapters::CTestAdapter*)pObj;
    cfm::application::CComm* r = dynamic_cast<cfm::application::CComm*> (pp);
    return r;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

