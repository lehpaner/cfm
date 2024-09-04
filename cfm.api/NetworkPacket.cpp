/****************************** Module Header ******************************\
* Module Name:  NetworkPacket
*
* Base system interfaces...
*
\***************************************************************************/
#include "NetworkPacket.h"
namespace cfm::application {

	NetworkPacket::NetworkPacket() {
		packet = new AsciiPacket();
		packetId = 0;
		//Metto a 0 tutti i byte del cpacket - 24 10 2007
		memset(cpacket, 0, PACKET_LEN);
	}

	NetworkPacket::~NetworkPacket() {
		//NOP
	}

	bool NetworkPacket::DecodeAsciiPacket(char* data) {
		//Verifica lunghezza pacchetto
		length = strlen(data);

		//Copio il pacchetto da 1Kb sulla struttura base binary packet
		memcpy(packet, data, length);

		//Verifica dell'STX
		if (packet->STX[0] == 'A' && packet->STX[1] == 'B') {
			//Verifica lunghezza
			char lenBuf[5]; memset(lenBuf, 0, 5);
			memcpy(lenBuf, packet->Length, 4);
			long l = strtol(lenBuf, 0, 16);
			this->length = l;

			//Verifica tipo pacchetto in RX
			char typeBuf[5]; memset(typeBuf, 0, 5);
			memcpy(typeBuf, packet->Type, 4);
			long t = strtol(typeBuf, 0, 16);
			this->type = t;

			//Verifica id pacchetto in RX
			char idBuf[5]; memset(idBuf, 0, 5);
			memcpy(idBuf, packet->PacketId, 4);
			long id = strtol(idBuf, 0, 16);
			this->packetId = id;


			//Conversione pacchetto in binario
			unsigned char binaryPacket[(PACKET_LEN - 14)]; memset(binaryPacket, 0, (PACKET_LEN - 14));
			int bpi = 0;
			for (unsigned int i = 14; i < (PACKET_LEN - 14); i = i + 2) {
				char dataBuf[3]; memset(dataBuf, 0, 3);
				memcpy(dataBuf, (void*)&data[i], 2);
				binaryPacket[bpi] = (unsigned char)strtol(dataBuf, 0, 16);
				bpi++;
			}

			//Memcpy sulla struttura di interesse in base al tipo
			if (type == EVT_OCR_DATA) //4025
			{
				memcpy(&ocrData, binaryPacket, sizeof(NET_OCRData));
			}

			if (type == EVT_ALMMGR_ALARM_ON) //WM_USER +6002
			{
				memcpy(&almmgrData, binaryPacket, sizeof(NET_ALMMGRData));
			}

			if (type == EVT_ALMMGR_ALARM_OFF) //WM_USER + 6003
			{
				memcpy(&almmgrData, binaryPacket, sizeof(NET_ALMMGRData));
			}

			return true;
		}
		else
			return false;

	}

	int NetworkPacket::EncodeAsciiPacket() {
		int packetLen = 0;

		//Alloco il pacchetto
		memset(packet, 48, PACKET_LEN); //48 è la codifica decimale ASCCI del carattere '0'

		void* data = NULL;

		switch (type)
		{
		case EVT_OCR_DATA:
		{
			data = (void*)&this->ocrData;

			int len = sizeof(NET_OCRData); //Il pacchetto in arrivo è del tipo TRKData
			int packetLen = len + 10;  //Lunghezza complessiva del pacchetto comprensiva di header
			packet->STX[0] = 'A'; packet->STX[1] = 'B';

			char bLen[5]; memset(bLen, 0, sizeof(bLen));
			sprintf_s(bLen, sizeof(bLen), "%04x", packetLen);
			memcpy(packet->Length, bLen, 4);

			char bType[5]; memset(bType, 0, sizeof(bType));
			sprintf_s(bType, sizeof(bType), "%04x", EVT_OCR_DATA);
			memcpy(packet->Type, bType, 4);

			char bId[5]; memset(bId, 0, sizeof(bId));
			sprintf_s(bId, sizeof(bId), "%04x", this->packetId);
			memcpy(packet->PacketId, bId, 4);

			char dataBuf[PACKET_LEN - 10]; memset(dataBuf, 0, PACKET_LEN - 10);
			memcpy(dataBuf, data, len);
			char* hexData = encodeHex((char*)data, len);
			memcpy(&packet->payload, hexData, PACKET_LEN);
			packetLen = strlen(hexData);
			delete[] hexData;

		}
		break;

		case EVT_ALMMGR_ALARM_ON:
		{
			data = (void*)&this->almmgrData;

			int len = sizeof(NET_ALMMGRData); //Il pacchetto in arrivo è del tipo TRKData
			int packetLen = len + 10;  //Lunghezza complessiva del pacchetto comprensiva di header

			//Header
			packet->STX[0] = 'A'; packet->STX[1] = 'B';

			//Lunghezza pacchetto
			char bLen[5]; memset(bLen, 0, sizeof(bLen));
			sprintf_s(bLen, sizeof(bLen), "%04x", packetLen);
			memcpy(packet->Length, bLen, 4);

			//Tipo dato
			char bType[5]; memset(bType, 0, sizeof(bType));
			sprintf_s(bType, sizeof(bType), "%04x", EVT_ALMMGR_ALARM_ON);
			memcpy(packet->Type, bType, 4);

			//Id pacchetto
			char bId[5]; memset(bId, 0, sizeof(bId));
			sprintf_s(bId, sizeof(bId), "%04x", this->packetId);
			memcpy(packet->PacketId, bId, 4);

			//Payload
			char dataBuf[PACKET_LEN - 10]; memset(dataBuf, 0, PACKET_LEN - 10);
			memcpy(dataBuf, data, len);
			char* hexData = encodeHex((char*)data, len);
			memcpy(&packet->payload, hexData, PACKET_LEN);
			packetLen = strlen(hexData);
			delete[] hexData;

		}
		break;

		case EVT_ALMMGR_ALARM_OFF:
		{
			data = (void*)&this->almmgrData;

			int len = sizeof(NET_ALMMGRData); //Il pacchetto in arrivo è del tipo TRKData
			int packetLen = len + 10;  //Lunghezza complessiva del pacchetto comprensiva di header

			//Header
			packet->STX[0] = 'A'; packet->STX[1] = 'B';

			//Lunghezza pacchetto
			char bLen[5]; memset(bLen, 0, sizeof(bLen));
			sprintf_s(bLen, sizeof(bLen), "%04x", packetLen);
			memcpy(packet->Length, bLen, 4);

			//Tipo dato
			char bType[5]; memset(bType, 0, sizeof(bType));
			sprintf_s(bType, sizeof(bType), "%04x", EVT_ALMMGR_ALARM_OFF);
			memcpy(packet->Type, bType, 4);

			//Id pacchetto
			char bId[5]; memset(bId, 0, sizeof(bId));
			sprintf_s(bId, sizeof(bId), "%04x", this->packetId);
			memcpy(packet->PacketId, bId, 4);

			//Payload
			char dataBuf[PACKET_LEN - 10]; memset(dataBuf, 0, PACKET_LEN - 10);
			memcpy(dataBuf, data, len);
			char* hexData = encodeHex((char*)data, len);
			memcpy(&packet->payload, hexData, PACKET_LEN);
			packetLen = strlen(hexData);
			delete[] hexData;

		}
		break;

		default:
			break;
		}

		return packetLen;

	}

	char* NetworkPacket::encodeHex(char* data, int len)
	{

		unsigned int txtLen = len; //text.length();

		//Alloco e inizializzo il buffer
		char* buffer = new char[(txtLen * 2) + 1]; //Il doppio più il terminatore di stringa
		memset(buffer, 0, (txtLen * 2) + 1);

		for (size_t i = 0; i < txtLen; i++)
		{
			unsigned char a = data[i];
			char hexbuf[3];
			memset(hexbuf, 0, sizeof(hexbuf));
			sprintf_s(hexbuf, sizeof(hexbuf), "%02x", a);
			buffer[i * 2] = hexbuf[0];
			buffer[i * 2 + 1] = hexbuf[1];
		}

		return buffer;

	}


	unsigned int NetworkPacket::GetType() { return type; }

	unsigned int NetworkPacket::GetPacketId() { return packetId; }

	void NetworkPacket::SetType(unsigned int uType) { type = uType; }

	void NetworkPacket::SetPacketId(unsigned int uPacketId) { packetId = uPacketId; }

	//24 10 2007
	char* NetworkPacket::GetPacket()
	{
		memcpy(cpacket, packet, PACKET_LEN);
		return cpacket;
	}

	//25 10 2007
	void NetworkPacket::SetOCRValues(unsigned int SIDUID,
		char* info, char* details, char* subId,
		char* Code_1, char* Code_2, char* Code_3,
		char* Code_4, char* Code_5, char* Code_6,
		char* Path_Img_Code_1, char* Path_Img_Code_2,
		char* Path_Img_Code_3, char* Path_Img_Code_4,
		char* Path_Img_Code_5, char* Path_Img_Code_6,
		char* DateTimeISO //YYYYmmdd hh:mm:ss.mmm
	)
	{
		this->ocrData.SIDUID = SIDUID;
		strcpy_s(this->ocrData.info, 255, info);
		strcpy_s(this->ocrData.details, 255, details);
		strcpy_s(this->ocrData.subId, 20, subId);

		strcpy_s(this->ocrData.Code_1, 50, Code_1);
		strcpy_s(this->ocrData.Code_2, 50, Code_2);
		strcpy_s(this->ocrData.Code_3, 50, Code_3);
		strcpy_s(this->ocrData.Code_4, 50, Code_4);
		strcpy_s(this->ocrData.Code_5, 50, Code_5);
		strcpy_s(this->ocrData.Code_6, 50, Code_6);

		strcpy_s(this->ocrData.Path_Img_Code_1, 150, Path_Img_Code_1);
		strcpy_s(this->ocrData.Path_Img_Code_2, 150, Path_Img_Code_2);
		strcpy_s(this->ocrData.Path_Img_Code_3, 150, Path_Img_Code_3);
		strcpy_s(this->ocrData.Path_Img_Code_4, 150, Path_Img_Code_4);
		strcpy_s(this->ocrData.Path_Img_Code_5, 150, Path_Img_Code_5);
		strcpy_s(this->ocrData.Path_Img_Code_6, 150, Path_Img_Code_6);

		strcpy_s(this->ocrData.datetime, 25, DateTimeISO);
	}


	void NetworkPacket::SetALMMGRValues(unsigned int SIDUID, char* info, char* details,
		char* subId, unsigned int  state, unsigned int  count, char* DateTimeISO)
	{
		this->almmgrData.SIDUID = SIDUID;
		strcpy_s(this->almmgrData.info, 255, info);
		strcpy_s(this->almmgrData.details, 255, details);
		strcpy_s(this->almmgrData.id, 20, subId);
		almmgrData.state = state;
		almmgrData.count = count;
		strcpy_s(this->almmgrData.datetime, 25, DateTimeISO);
	}



	//Funzioni di utilità per utilizzo in C#
	NetworkPacket* CreateNetworkPacket()
	{
		return new NetworkPacket();
	}
	void DestroyNetworkPacket(NetworkPacket* p)
	{
		delete p;
	}
}