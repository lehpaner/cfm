/****************************** Module Header ******************************\
* Module Name:  WinReg.cpp
* Project:      S000
*
*
* Management values of register
*
\***************************************************************************/
#include "WinReg.h"
#include <sstream>
#include <Shlwapi.h>
#pragma comment(lib,"shlwapi.lib")

const std::string& long2string(long lNumber) {
	std::stringstream ret;
	ret << lNumber;
	return ret.str();
}

namespace cfm::application {
	CWindowsRegister::CWindowsRegister(HKEY HK_ROOT, std::string sWorkingKey) {
		try {
			sParentKey = sWorkingKey;
			HKEY_ROOT = HK_ROOT;

			/**
			 * fills the structure in memory with the parameters
			 */

			std::vector<std::string> vKeys = GetKeyParamsName(sWorkingKey);

			std::vector<std::string>::iterator iValues;
			for (iValues = vKeys.begin(); iValues != vKeys.end(); iValues++) {
				std::string strVal = GetKeyParamValue(sParentKey, (*iValues));
				mapParameters.insert(std::pair<std::string, std::string>((*iValues), strVal));
			}
		} catch (std::exception& E) {
			sErrMsg = "Fatal error while reading configuration [";
			sErrMsg += std::string(E.what()) + "].";
			throw sErrMsg;
		}
	}
	//----------------------------------------------------

	/**
	  * @fn		 bool ExistKey(const string& keyName)
	  * @author  Guglielmo Abbruzzese
	  * @version 1.0
	  * @date	 22/06/2007 Creation
	  * @param	 const string& keyName	-	Key name
	  *
	  * @return  SUCCESS	if the key exists
	  *			 FAILURE	in case of troubles
	  *
	  * @brief   Checks the existence of a certain registry key
	  *
	  */
	int CWindowsRegister::ExistKey(const std::string& keyName) {
		HKEY hkey;
		if (RegOpenKeyA(HKEY_LOCAL_MACHINE, keyName.c_str(), &hkey) == ERROR_SUCCESS) {
			sErrMsg = "Key " + keyName + sErrMsg += "' doesn't exist";
			RegCloseKey(hkey);
			return FAILURE;
		}
		return SUCCESS;
	};
	//----------------------------------------------------------------------------

	/**
	  * @fn		 bool CreateKey(const string& keyName, bool persistent)
	  * @author  Guglielmo Abbruzzese
	  * @version 1.0
	  * @date	 22/06/2007 Creation
	  * @param	 const string& keyName	-	Key name
	  * @param	 bool persistent		-	If the key is to be created in the regs or
	  *										just in memory (default: true)
	  *
	  * @return  SUCCESS	if the key is correctly created
	  *			 FAILURE	in case of troubles
	  *
	  * @brief   If a registry key doesn't exist is created
	  *
	  */
	int CWindowsRegister::CreateKey(const std::string& keyName, bool persistent) {
		int iRetVal = SUCCESS;
		HKEY hkey;

		if (RegCreateKeyA(HKEY_LOCAL_MACHINE, keyName.c_str(), &hkey) != ERROR_SUCCESS) {
			sErrMsg = "Error creating the key \"" + keyName + "\".";
			iRetVal = FAILURE;
			throw sErrMsg;
		};
		RegCloseKey(hkey);
		return iRetVal;
	}
	//----------------------------------------------------------------------------

	/**
	  * @fn		 bool DeleteKey(const string& parentKeyName, const string& keyName, bool recursive)
	  * @author  Guglielmo Abbruzzese
	  * @version 1.0
	  * @date	 22/06/2007 Creation
	  * @param	 const string& parentKeyName	- HKEY_LOCAL_MACHINE | HKEY_CLASSES_ROOT | HKEY_CURRENT_CONFIG |
	  *											  HKEY_CURRENT_USER  | HKEY_USERS
	  * @param	 const string& keyName			- Address of a null terminated string specifying the name of the key to delete
	  * @param	 bool recursive					- Cleans every subfolder too
	  *
	  * @return  SUCCESS	if the key is correctly deleted
	  *			 FAILURE	in case of troubles
	  *
	  * @brief   If a registry key doesn't exist it is created
	  *
	  */
	int CWindowsRegister::DeleteKey(const std::string& parentKeyName, const std::string& keyName, bool recursive) {
		HKEY hkey;
		int iRetVal = SUCCESS;
		if (RegOpenKeyA(HKEY_LOCAL_MACHINE, parentKeyName.c_str(), &hkey) != ERROR_SUCCESS)
		{
			sErrMsg = "Error opening the key \"" + parentKeyName + "\".";
			iRetVal = FAILURE;
		}
		if (recursive) {
			if (SHDeleteKeyA(hkey, (LPCSTR)keyName.c_str()) != ERROR_SUCCESS) {
				RegCloseKey(hkey);
				// sErrMsg = "Errore deleting the key \"" + keyName + "\", under \"" + parentKeyName + "\".";
				iRetVal = FAILURE;
			}
		} else {
			if (RegDeleteKeyA(hkey, keyName.c_str()) != ERROR_SUCCESS) {
				RegCloseKey(hkey);
				// sErrMsg = "Error deleting key \"" + keyName + "\", under \"" + parentKeyName + "\".";
				iRetVal = FAILURE;
			}
		}
		RegCloseKey(hkey);
		return iRetVal;
	}
	//----------------------------------------------------------------------------

	/**
	  * @fn		 GetKeyParamValue(const string& keyName, const string& ParamName)
	  * @author  Guglielmo Abbruzzese
	  * @version 1.0
	  * @date	 22/06/2007 Creation
	  * @param	 const string& keyName	-
	  * @param	 const string& ParamName-
	  *
	  * @return
	  *
	  * @brief
	  *
	  */
	std::string CWindowsRegister::GetKeyParamValue(const std::string& keyName, const std::string& ParamName) {
		HKEY hkey;
		std::string paramValue = "";
		sErrMsg = "";
		if (RegOpenKeyA(HKEY_LOCAL_MACHINE, (LPCSTR)keyName.c_str(), &hkey) != ERROR_SUCCESS) {
			sErrMsg = "The key \"" + keyName + "\" doesn't exist.";
		}
		DWORD wNumOfKey, dwMaxNameLen, dwMaxValueLen;
		DWORD dwNameSize, dwValueSize, dwType;
		if (RegQueryInfoKey(hkey, NULL, NULL, NULL, NULL, NULL, NULL, &wNumOfKey, &dwMaxNameLen, &dwMaxValueLen, NULL, NULL) != ERROR_SUCCESS) {
			sErrMsg = "Error retrieving info about the key \"" + keyName + "\"." "";
		}
		BYTE* data = new BYTE[++dwMaxValueLen];
		char* name = new char[++dwMaxNameLen];
		for (DWORD i = 0; i < wNumOfKey; i++) {
			dwNameSize = dwMaxNameLen;
			dwValueSize = dwMaxValueLen;
			if (RegEnumValue(hkey, i, (LPTSTR)name, &dwNameSize, NULL, &dwType, data, &dwValueSize) != ERROR_SUCCESS) {
				sErrMsg = "Error retrieving " + long2string((long)i) +
					"° parameter of " + keyName;
			}
			else
			{
				name[dwNameSize] = '\0';
				std::string tmp = name;
				if (tmp == ParamName)
				{
					data[dwValueSize] = '\0';
					switch (dwType)
					{
					case REG_DWORD:
						paramValue = long2string((long)data[0]);
						break;
					case REG_EXPAND_SZ:
					{
						for (DWORD counter = 0; counter < dwValueSize - 2; counter++)
							paramValue += (unsigned char)data[counter];
					}
					case REG_SZ:
					{
						for (DWORD counter = 0; counter < dwValueSize - 1; counter++)
							paramValue += (unsigned char)data[counter];
					}
					break;
					default:
						paramValue = "NOT_SUPPORTED_TYPE";
					}
					break;
				}
			}
		}
		delete[] name;
		delete[] data;
		RegCloseKey(hkey);
		return paramValue;
	};
	//----------------------------------------------------------------------------

	/**
	  * @fn		 void SetKeyParamValue(const string& keyName, const string& ParamName, const string& ParamValue)
	  * @author  Guglielmo Abbruzzese
	  * @version 1.0
	  * @date	 22/06/2007 Creation
	  * @param	 const string& keyName		-
	  * @param   const string& ParamName	-
	  * @param	 const string& ParamValue	-
	  *
	  * @return  void
	  *
	  * @brief   <create> or <set> a configuration key parameter.
	  *
	  * Description: It manages extended strings too.
	  *
	  */
	  ///Crea o setta un parametro di una chiave di configurazione. Gestisce solo stringhe estese
	int CWindowsRegister::SetKeyParamValue(const std::string& keyName, const std::string& ParamName, const std::string& ParamValue) {
		HKEY hkey;
		int iRetValue = SUCCESS;
		if (RegOpenKeyA(HKEY_LOCAL_MACHINE, (LPCSTR)keyName.c_str(), &hkey) != ERROR_SUCCESS) {
			sErrMsg = "The key \"" + keyName + "\" doesn't exist.";
			iRetValue = FAILURE;
		}
		DWORD res = RegSetValueA(hkey, ParamName.c_str(), REG_SZ, ParamValue.c_str(), ParamValue.length() + 1);
		if (res != ERROR_SUCCESS) {
			RegCloseKey(hkey);
			sErrMsg = "Errorsetting the following parameter: \"" + ParamName + "\" , " + ParamValue;
			iRetValue = FAILURE;
		}
		RegCloseKey(hkey);
		return iRetValue;
	}
	//----------------------------------------------------------------------------

	/**
	  * @fn		 std::vector<string> GetKeyParamsName(const string& keyName)
	  * @author  Guglielmo Abbruzzese
	  * @version 1.0
	  * @date	 22/06/2007 Creation
	  * @param	 const string& keyName -
	  *
	  * @return  std::vector<string>
	  *
	  * @brief
	  *
	  * Description: Manages DWORD and string
	  *
	  */

	std::vector<std::string> CWindowsRegister::GetKeyParamsName(const std::string& keyName) {
		HKEY hkey;
		std::vector<std::string> keyList;

		if (RegOpenKeyA(HKEY_LOCAL_MACHINE, (LPCSTR)keyName.c_str(), &hkey) != ERROR_SUCCESS) {
			sErrMsg = "The key \"" + keyName + "\" doesn't exist ";
		}
		DWORD wNumOfKey, dwMaxNameSize;
		if (RegQueryInfoKey(hkey, NULL, NULL, NULL, NULL, NULL, NULL, &wNumOfKey, &dwMaxNameSize, NULL, NULL, NULL) != ERROR_SUCCESS) {
			RegCloseKey(hkey);
			sErrMsg = "Error reading key \"" + keyName + "\".";
		}
		char* name = new char[++dwMaxNameSize];
		DWORD dwCurrentSize;
		for (DWORD i = 0; i < wNumOfKey; i++) {
			dwCurrentSize = dwMaxNameSize;
			if (RegEnumValue(hkey, i, (LPTSTR)name, &dwCurrentSize, NULL, NULL, NULL, NULL) != ERROR_SUCCESS) {
				sErrMsg = "Error reading the" + long2string((long)i) +
					"° key " + keyName + " parameter";
			} else {
				name[dwCurrentSize] = '\0';
				std::string tmp = name;
				keyList.push_back(tmp);
			}
		}
		delete[] name;
		RegCloseKey(hkey);
		return keyList;
	}
	//----------------------------------------------------------------------------

}