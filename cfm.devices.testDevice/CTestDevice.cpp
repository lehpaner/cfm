/****************************** Module Header ******************************\
* Module Name:  CTestDevice
*
* Extends base CFM Device and implements basic finctionalities for the Test Module
*
\***************************************************************************/
#include"CTestDevice.h"
#include <iostream>

namespace cfm::devices {
	CTestDevice::CTestDevice() {

		std::cout << "CTEST DEVICE CONSTRUCTED" << std::endl;

	}

	CTestDevice::~CTestDevice() {

		std::cout << "CTEST DEVICE DESTRUCTED" << std::endl;

	}

	int CTestDevice::InitSID() {

		std::cout << "CTEST DEVICE INITED" << std::endl;
		return 0;
	}

	int CTestDevice::CloseSID() {

		std::cout << "CTEST DEVICE CLOSED" << std::endl;
		return 0;
	}

	void CTestDevice::BeforeRun() {

		std::cout << "CTEST DEVICE BEFORE RUN" << std::endl;
		return;
	}

	void CTestDevice::AfterRun() {

		std::cout << "CTEST DEVICE AFTER RUN" << std::endl;
		return;
	}

	void CTestDevice::ProcessCustomMessages(MSG& m) {

		std::cout << "CTEST DEVICE IS PROCESSING MESSAGE ["<< m.message <<"]" << std::endl;
		return;
	}
}
