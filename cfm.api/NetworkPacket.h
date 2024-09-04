/****************************** Module Header ******************************\
* Module Name:  NetworkPacket
*
* Base system interfaces...
*
\***************************************************************************/
#pragma once
#include "CategoryTypes.h"
#include <string>
namespace cfm::application {
#pragma pack(push,1)
	//FOR EVT_OCR_DATA
	struct NET_OCRData {
		unsigned int SIDUID;
		char		info[255];
		char		details[255];   // eventuali dettagli dell'oggetto/operazione
		char		subId[20];      //Sensore che ha scatenato l'evento
		char        Code_1[50];
		char	    Code_2[50];
		char		Code_3[50];
		char	    Code_4[50];
		char		Code_5[50];
		char	    Code_6[50];
		char	    Path_Img_Code_1[150];
		char	    Path_Img_Code_2[150];
		char	    Path_Img_Code_3[150];
		char	    Path_Img_Code_4[150];
		char	    Path_Img_Code_5[150];
		char	    Path_Img_Code_6[150];
		char	    datetime[25];  // data e ora dell'evento (formato ISO)
	};

	struct NET_ALMMGRData {
		unsigned int     SIDUID;		/**< id del SID che sta gestendo l'informazione */
		char		     info[255];     /**<  descrizione oggetto */
		char		     details[255];  /**<  eventuali dettagli dell'oggetto/operazione */
		char		     id[20];        /**<  id dell'allarme rilevato  */
		long             state;			/**<  0=OFF, 1=ON, 2=SHORT, 3=CUT */
		long             count;			/**<  numero di occorrenza della segnalazione specifica   */
		char		     datetime[25];  /**<  data e ora dell'evento */
		//...................
	};

#pragma pack(pop)

#define PACKET_LEN 4096

	struct AsciiPacket {
		unsigned char  STX[2];
		unsigned char  Length[4];     //MAX LENGTH PACKET 2048 ASCII: 512 BIN
		unsigned char  Type[4];       //Pacchetti definiti in CategoryTypes.h
		//Modifica 30 07 2007
		unsigned char  PacketId[4];
		unsigned char  payload[PACKET_LEN - 10];
	};

	extern "C" {

		_declspec(dllexport) class NetworkPacket {
		public:
			//Costruttore
			_declspec(dllexport) NetworkPacket();
			//Distruttore
			_declspec(dllexport) ~NetworkPacket();

			//Tipologie di pacchetto
			//Le tipologie di pacchetto sono quelle definite in CatTypes.h

			//decodifica un pacchetto (data è la codifica ascii esadecimale del pacchetto)
			_declspec(dllexport) bool DecodeAsciiPacket(char* data);
			//codifica di un pacchetto in formato ascii esadecimale
			_declspec(dllexport) int EncodeAsciiPacket();

			//Tipologie di strutture ritornate dalla decodifica
			//TRKData			trackData;
			//ALMMGRData		alarmData;
			//READERData         badgeData;
			//DIData			digitalInputData;
			NET_OCRData			ocrData;
			NET_ALMMGRData      almmgrData;

			//25 10 2007
			_declspec(dllexport) void SetOCRValues(unsigned int SIDUID, char* info, char* details,
				char* subId, char* Code_1, char* Code_2, char* Code_3,
				char* Code_4, char* Code_5, char* Code_6,
				char* Path_Img_Code_1, char* Path_Img_Code_2, char* Path_Img_Code_3,
				char* Path_Img_Code_4, char* Path_Img_Code_5, char* Path_Img_Code_6,
				char* DateTimeISO //YYYYmmdd hh:mm:ss.mmm
			);

			_declspec(dllexport) void SetALMMGRValues(unsigned int SIDUID, char* info, char* details,
				char* subId, unsigned int state, unsigned int count, char* DateTimeISO);

			AsciiPacket* packet;

			//24 10 2007
			_declspec(dllexport) char* GetPacket();

			_declspec(dllexport) unsigned int GetType();
			_declspec(dllexport) unsigned int GetPacketId();
			_declspec(dllexport) void SetType(unsigned int uType);
			_declspec(dllexport) void SetPacketId(unsigned int uPacketId);

		private:
			unsigned int length;
			unsigned int type;
			unsigned int packetId;

			//Funzione di utilità
			char* encodeHex(char* data, int len);

			//Pacchetto ASCII
			char cpacket[PACKET_LEN]; //24 10 2007


		};

		_declspec(dllexport) NetworkPacket* CreateNetworkPacket();
		_declspec(dllexport) void DestroyNetworkPacket(NetworkPacket* p);

	} // extern "C"

}
