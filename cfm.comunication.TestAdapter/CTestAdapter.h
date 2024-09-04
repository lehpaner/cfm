/****************************** Module Header ******************************\
* Module Name:  CTestAdapter
*
* Extends base CFM Comunication adapter and implements basic finctionalities for the Test Module
*
\***************************************************************************/
#pragma once
#include "BaseComm.h"
#include <iostream>
#include "CfmDomainObjects.h"
#include "CallbackTypes.h"

namespace cfm::adapters {

	extern "C" {
		class __declspec(dllexport) CTestAdapter : public cfm::application::CComm {
		public:
			//**********************************
			//Constructor and deconstructor
			//**********************************
			CTestAdapter() {

				std::cout << "CTEST ADAPTER CONSTRUCTED" << std::endl;

			}

			~CTestAdapter();

		protected:
			int InitSID();
			int CloseSID();
			void BeforeRun();
			void AfterRun();
			void ProcessCustomMessages(MSG& msg);
			bool OnPushDeviceEvent(long idEvent, cfm::application::domain::CfmDevices_Table deviceData, std::string MethodName);
			bool OnPushSysMessage(cfm::application::SystemMessage sysMessage, std::string MethodName);
		};

	}

}
