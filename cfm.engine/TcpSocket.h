/****************************** Module Header ******************************\
* Module Name:  TcpSocket.h
* Project:      S000
*
*
* Socket class to be removed
*
\***************************************************************************/
#pragma once
#include "framework.h"
#include <winsock2.h>
#include <ws2tcpip.h>

namespace cfm::application {
	using port_t = uint16_t;
	// class WSAInitializer is a part of the Socket class (on win32)
	// as a static instance - so whenever an application uses a Socket,
	// winsock is initialized
	class WSAInitializer // Winsock Initializer
	{
	public:
		WSAInitializer() {
			if (WSAStartup(0x101, &m_wsadata)) {
				exit(-1);
			}
		}
		~WSAInitializer() {
			WSACleanup();
		}
	private:
		WSADATA m_wsadata;
	};

	/** Socket container class, event generator.
\ingroup basic */
	class ISocketHandler {
		friend class Socket;

	};
	class DummyHandler :public ISocketHandler {

	};
	class Socket {
	public:
		Socket(ISocketHandler& h) : m_handler(h){
		}

		virtual ~Socket() {};
	private:
		ISocketHandler& m_handler;
		static	WSAInitializer m_winsock_init; ///< Winsock initialization singleton class
		DummyHandler dh{};
	};

	

	class TcpSocket {
	public:
		TcpSocket() : number(0), m_handler(DummyHandler()){
			
		}
		TcpSocket(ISocketHandler& h) :  m_handler(h), number(0) {
		}
		~TcpSocket() {  }
		int Send(const std::string& m) {
			return 0;
		}
		bool Ready() {
			return true;
		}
	private:
		int number;
		ISocketHandler m_handler;

	};

}
