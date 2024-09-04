/****************************** Module Header ******************************\
* Module Name:  WinReg.h
* Project:      S000
*
*
* Management values of register
*
\***************************************************************************/
#pragma once

#include "framework.h"
#include <string>
#include <vector>
#include <map>

#define	SUCCESS	 0
#define FAILURE	-1

namespace cfm::application {

	class CWindowsRegister {
	private:

		~CWindowsRegister() {};
		HKEY		HKEY_ROOT;	/**< registry root, e.g.HKEY_LOCAL_MACHINE					*/
		std::string	sParentKey;  /**< registry subfolder path, e.g. "/SOFTWARE/CFM/CFM/"	*/

		int SetInternalMap();

	public:
		CWindowsRegister(HKEY	HK_ROOT, std::string sWorkingKey);

		int ExistKey(const std::string& keyName);

		int CreateKey(const std::string& keyName,
			bool persistent = true);

		int DeleteKey(const std::string& parentKeyName,
			const std::string& keyName,
			bool recursive = true);

		int SetKeyParamValue(const std::string& keyName,
			const std::string& ParamName,
			const std::string& ParamValue);

		std::string GetKeyParamValue(const std::string& keyName,
			const std::string& ParamName);

		std::vector<std::string> GetKeyParamsName(const std::string& keyName);

		std::map <std::string, std::string> mapParameters;  /**< hash table containing registry readings */
		std::string sErrMsg;
	};

}
