/*
  * Generated by cppsrc.sh
  * On 2015-12-12  9:44:59,49
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#define _WIN32_WINNT 0x0501

#include "../StarVFSInternal.h"
#include "Remote.h"

#include <thread>
#include <chrono>
#include <iostream>
#include <boost/asio.hpp>

#include "RemoteHeaders.h"

using boost::asio::ip::tcp;
using boost::asio::deadline_timer;

namespace StarVFS {
namespace Modules {

Remote::Remote(StarVFS *svfs, int port): iModule(svfs) {
	m_ThreadRunning = false;
	m_CanRun = true;

	std::thread([this](){ 
		m_ThreadRunning = true;
		try {
			ThreadMain();
		}
		catch (std::exception &e) {
			std::cout << "EXCEPTION: " << e.what();
		}
		catch (...) {
		}
		m_ThreadRunning = false;
	}).detach();
}

Remote::~Remote() {
	m_CanRun = false;
	while (m_ThreadRunning)
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

//-------------------------------------------------------------------------------------------------

using BaseConnectionClass = RemoteHeaders::BaseConnection<boost::asio::io_service, tcp::socket>;
struct Remote::Connection : public BaseConnectionClass {
	Remote *m_Owner;
	using MessageBuffer = RemoteHeaders::MessageBuffer;

	Connection(Remote *Owner): BaseConnectionClass() {
		m_Owner = Owner;
	}

	virtual bool CanRun() const override { return m_Owner->m_CanRun; }

	bool ProcessPingPong(MessageBuffer &message) {
		message.Clear();
		auto hdr = message.GetHeader();
		if (hdr->Command == RemoteHeaders::Command::Ping)
			hdr->Command = RemoteHeaders::Command::Pong;
		else
			hdr->Command = RemoteHeaders::Command::Ping;

		return WriteMessage(message);
	}

	bool SendFileTable(MessageBuffer &message) {
		message.Clear();
		auto hdr = message.GetHeader();
		auto vfs = m_Owner->GetVFS();
		auto ft = vfs->GetFileTable();
		
		auto c = ft->GetAllocatedFileCount();
		message.PushString((char*)ft->GetTable(), c * sizeof(File));
		hdr->ElementCount = c;

		return WriteMessage(message);
	}
	bool SendStringTable(MessageBuffer &message) {
		message.Clear();
		auto hdr = message.GetHeader();
		auto vfs = m_Owner->GetVFS();
		auto ft = vfs->GetFileTable();
		auto st = ft->GetStringTable();

		auto c = st->GetAllocatedCount();
		message.PushString((char*)st->GetMemory(), c * sizeof(Char));
		hdr->ElementCount = c;

		return WriteMessage(message);
	}
	bool SendFile(MessageBuffer &message) {
		message.Clear();
//		auto hdr = message.GetHeader();
		auto vfs = m_Owner->GetVFS();
		
		auto request = message.GetAndPull<RemoteHeaders::GetFileRequest>();
		FileHandle h;

		using Mode = RemoteHeaders::GetFileRequest::FindMode;
		switch (request->Mode) {
		case Mode::Hash:
			break;
		case Mode::ID:
			h = vfs->OpenFile(request->ID, request->RWMode);
			break;
		case Mode::Path:
			h = vfs->OpenFile((CString)request->Path, request->RWMode, request->OpenMode);
			break;
		default:
			break;
		}

		message.Clear();
		auto response = message.AllocAndZero<RemoteHeaders::GetFileResponse>();

		CharTable ct;
		if (!h || !h.GetFileData(ct)) {
			response->Result = 1;
		} else {
			response->DataSize = h.GetSize();
			message.PushString(ct.get(), h.GetSize());
		}

		return WriteMessage(message);
	}
	virtual bool ProcessCommand(MessageBuffer &message) override {
		auto hdr = message.GetHeader();

		STARVFSDebugInfoLog("Recived command:%d payload:%d", hdr->Command, hdr->PayLoadSize);

		switch (hdr->Command) {
		case RemoteHeaders::Command::GetFileTable:
			return SendFileTable(message);
		case RemoteHeaders::Command::GetStringTable:
			return SendStringTable(message);
		case RemoteHeaders::Command::GetFile:
			return SendFile(message);
		case RemoteHeaders::Command::Ping:
		case RemoteHeaders::Command::Pong:
			return ProcessPingPong(message);

		case RemoteHeaders::Command::NOP:
		default:
			return true;
		}
	}

	void HandleClient() {
		STARVFSDebugLog("Accepted connection");
		auto Buffer = std::make_unique<MessageBuffer>();
		while (m_Owner->m_CanRun && ReadMessage(*Buffer.get()) && ProcessCommand(*Buffer.get())) {
			//NOP
		}
		STARVFSDebugLog("Client disconnected");
	}
};

//-------------------------------------------------------------------------------------------------

void Remote::ThreadMain() {
	 
//	tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), Settings::Modules::Remote::BasePort);
//	a.open(endpoint.protocol());

//	deadline_timer deadline(io_service);

	Connection c(this);

	int id = 0;
	tcp::acceptor a(c.m_io_service, tcp::endpoint(tcp::v4(), RemoteHeaders::Settings::BasePort));
	while (m_CanRun) {
		//a.async_accept(c.m_Socket, [this, &c](boost::system::error_code ec) {
		//	if (ec) {
		//		STARVFSDebugLog("Accept failed");
		//		return;
		//	}
		//	c.HandleClient();
		//});

		STARVFSDebugLog("Waiting for connection %d", id++);
		boost::system::error_code error;
		c.m_Socket = tcp::socket(c.m_io_service);
		a.accept(c.m_Socket, error);
		c.HandleClient();
		//c.m_io_service.run();

	//	deadline.expires_from_now(boost::posix_time::seconds(1));
	//	io_service.run();
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		//		std::thread(session, std::move(sock)).detach();
	}
}

} //namespace Modules 
} //namespace StarVFS 
