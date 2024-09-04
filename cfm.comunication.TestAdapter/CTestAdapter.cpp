/****************************** Module Header ******************************
* Module Name:  CTestAdapter
*
* Extends base CFM Comunication adapter implementation
*
***************************************************************************/
#include "CTestAdapter.h"
#include <iostream>

namespace cfm::adapters {


	CTestAdapter::~CTestAdapter() {

		std::cout << "CTEST ADAPTER DESTRUCTED" << std::endl;

	}

	int CTestAdapter::InitSID() {

		std::cout << "CTEST ADAPTER INITED" << std::endl;
		return 0;
	}

	int CTestAdapter::CloseSID() {

		std::cout << "CTEST ADAPTER CLOSED" << std::endl;
		return 0;
	}

	void CTestAdapter::BeforeRun() {

		std::cout << "CTEST ADAPTER BEFORE RUN" << std::endl;
		return;
	}

	void CTestAdapter::AfterRun() {

		std::cout << "CTEST ADAPTER AFTER RUN" << std::endl;
		return;
	}

	void CTestAdapter::ProcessCustomMessages(MSG& m) {

		std::cout << "CTEST ADAPTER IS PROCESSING MESSAGE [" << m.message << "]" << std::endl;
		return;
	}

	bool CTestAdapter::OnPushDeviceEvent(long idEvent, cfm::application::domain::CfmDevices_Table deviceData, std::string MethodName) {

		std::cout << "CTEST ADAPTER IS PUSHING EVENT [" << MethodName << "]" << std::endl;
		return true;
	}
	bool CTestAdapter::OnPushSysMessage(cfm::application::SystemMessage sysMessage, std::string MethodName) {

		std::cout << "CTEST ADAPTER IS PUSHING SYSMESSAGE [" << MethodName << "]" << std::endl;
		return true;
	}
}