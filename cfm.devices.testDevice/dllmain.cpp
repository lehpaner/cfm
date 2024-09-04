// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "BaseSID.h"
#include "CTestDevice.h"

/**
* DLL entry point for the device interface with CFM Environment  
**/
//FUNZIONI GLOBALI PER ACCEDERE ALL'OGGETTO DAL KERNEL
CFM_API_DLL cfm::devices::CTestDevice* CreateObject() {
    cfm::devices::CTestDevice* pp = new cfm::devices::CTestDevice();
    cfm::application::CSID* r = dynamic_cast<cfm::application::CSID*> (pp);
    return pp;
}

CFM_API_DLL void DestroyObject(void* p) {
    cfm::devices::CTestDevice* pp = (cfm::devices::CTestDevice*)p;
    pp->Stop();
    cfm::application::CSID* r = dynamic_cast<cfm::application::CSID*> (pp);
    delete pp;
}

//Get interface for SID control and monitoring
CFM_API_DLL cfm::application::CSID* GetSIDInterface(void* pObj)
{
    cfm::devices::CTestDevice* pp = (cfm::devices::CTestDevice*)pObj;
    cfm::application::CSID* r = dynamic_cast<cfm::application::CSID*> (pp);
    return r;
}
//Get interfaces for each category
CFM_API_DLL cfm::application::SIDTrkInterface* GetTRKInterface(void* pObj)
{
    cfm::devices::CTestDevice* pp = (cfm::devices::CTestDevice*)pObj;
    cfm::application::SIDTrkInterface* r = dynamic_cast<cfm::application::SIDTrkInterface*> (pp);
    return r;
}

CFM_API_DLL cfm::application::SIDDIInterface* GetDIInterface(void* pObj)
{
    cfm::devices::CTestDevice* pp = (cfm::devices::CTestDevice*)pObj;
    cfm::application::SIDDIInterface* r = dynamic_cast<cfm::application::SIDDIInterface*> (pp);
    return r;
}

CFM_API_DLL cfm::application::SIDDOInterface* GetDOInterface(void* pObj)
{
    cfm::devices::CTestDevice* pp = (cfm::devices::CTestDevice*)pObj;
    cfm::application::SIDDOInterface* r = dynamic_cast<cfm::application::SIDDOInterface*> (pp);
    return r;
}

CFM_API_DLL cfm::application::SIDPtzInterface* GetPTZInterface(void* pObj)
{
    cfm::devices::CTestDevice* pp = (cfm::devices::CTestDevice*)pObj;
    cfm::application::SIDPtzInterface* r = dynamic_cast<cfm::application::SIDPtzInterface*> (pp);
    return r;
}

CFM_API_DLL cfm::application::SIDAlmMgrInterface* GetALMMGRInterface(void* pObj)
{
    cfm::devices::CTestDevice* pp = (cfm::devices::CTestDevice*)pObj;
    cfm::application::SIDAlmMgrInterface* r = dynamic_cast<cfm::application::SIDAlmMgrInterface*> (pp);
    return r;
}


CFM_API_DLL cfm::application::SIDDvrInterface* GetDVRInterface(void* pObj)
{
    cfm::devices::CTestDevice* pp = (cfm::devices::CTestDevice*)pObj;
    cfm::application::SIDDvrInterface* r = dynamic_cast<cfm::application::SIDDvrInterface*> (pp);
    return r;
}

CFM_API_DLL cfm::application::SIDSerialInterface* GetSERIALInterface(void* pObj)
{
    cfm::devices::CTestDevice* pp = (cfm::devices::CTestDevice*)pObj;
    cfm::application::SIDSerialInterface* r = dynamic_cast<cfm::application::SIDSerialInterface*> (pp);
    return r;
}

CFM_API_DLL cfm::application::SIDBadgeInterface* GetREADERInterface(void* pObj)
{
    cfm::devices::CTestDevice* pp = (cfm::devices::CTestDevice*)pObj;
    cfm::application::SIDBadgeInterface* r = dynamic_cast<cfm::application::SIDBadgeInterface*> (pp);
    return r;
}

CFM_API_DLL cfm::application::SIDZoneInterface* GetZONEInterface(void* pObj)
{
    cfm::devices::CTestDevice* pp = (cfm::devices::CTestDevice*)pObj;
    cfm::application::SIDZoneInterface* r = dynamic_cast<cfm::application::SIDZoneInterface*> (pp);
    return r;
}

CFM_API_DLL cfm::application::SIDCameraInterface* GetCameraInterface(void* pObj)
{
    cfm::devices::CTestDevice* pp = (cfm::devices::CTestDevice*)pObj;
    cfm::application::SIDCameraInterface* r = dynamic_cast<cfm::application::SIDCameraInterface*> (pp);
    return r;
}

CFM_API_DLL cfm::application::SIDCtrlAccessInterface* GetCAInterface(void* pObj)
{
    cfm::devices::CTestDevice* pp = (cfm::devices::CTestDevice*)pObj;
    cfm::application::SIDCtrlAccessInterface* r = dynamic_cast<cfm::application::SIDCtrlAccessInterface*> (pp);
    return r;
}

CFM_API_DLL cfm::application::SIDCDCInterface* GetCDCInterface(void* pObj) {
    cfm::devices::CTestDevice* pp = (cfm::devices::CTestDevice*)pObj;
    cfm::application::SIDCDCInterface* r = dynamic_cast<cfm::application::SIDCDCInterface*> (pp);
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

