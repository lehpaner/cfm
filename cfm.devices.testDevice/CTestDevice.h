/****************************** Module Header ******************************\
* Module Name:  CTestDevice
*
* Extends base CFM Device and implements basic finctionalities for the Test Module
*
\***************************************************************************/
#pragma once
#include "Thread.h"
#include "BaseSID.h"

namespace cfm::devices {
	extern "C" {
		class CFM_API_DLL CTestDevice : public cfm::application::CSID {
		public:
			//**********************************
			//Constructor and deconstructor
			//**********************************
			CTestDevice();
			~CTestDevice();

		protected:
			int InitSID();
			int CloseSID();
			void BeforeRun();
			void AfterRun();
			void ProcessCustomMessages(MSG& msg);
		};

	}
}