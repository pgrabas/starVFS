/*
  * Generated by cppsrc.sh
  * On 2016-01-01  0:24:38,60
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include "nRDC.h"

namespace StarVFS {
namespace RDC {

void BlockFileDevice::FILEDeleter::operator()(FILE *f) {
	fclose(f);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

BlockFileDevice::BlockFileDevice() {
	Flags._uintvalue = 0;
}

BlockFileDevice::~BlockFileDevice() {
	Close();
}

//-----------------------------------------------------------------------------

bool BlockFileDevice::Close() {
	Flags.Opened = 0;
	Flags.Ok = 0;
	Flags.CanRead = 0;
	Flags.CanWrite = 0;
	m_File.reset();
	return true;
}

bool BlockFileDevice::CreateFile(const String& FileName) {
	if (Flags.Opened) {
		STARVFSErrorLog("Attempt to reopen file!");
		StarVFSAssert(false);
		return false;
	}

	m_File.reset(fopen(FileName.c_str(), "w+b"));
	Flags.Ok = 1;
	Flags.Opened = 1;
	Flags.CanWrite = 1;
	Flags.CanRead = 1;
	return static_cast<bool>(m_File);
}

bool BlockFileDevice::OpenForRead(const String& FileName) {
	if (Flags.Opened) {
		STARVFSErrorLog("Attempt to reopen file!");
		StarVFSAssert(false);
		return false;
	}
	Flags.Ok = 1;
	Flags.Opened = 1;
	Flags.CanWrite = 0;
	Flags.CanRead = 1;
	m_File.reset(fopen(FileName.c_str(), "rb"));
	return static_cast<bool>(m_File);
}

//-----------------------------------------------------------------------------

bool BlockFileDevice::IsOk() const {
	if (Flags.Ok || Flags.Opened) {

	}
	return Flags.Ok && Flags.Opened;
}

size_t BlockFileDevice::GetSize() const {
	if (!Flags.Ok || !Flags.Opened) {
		//log
		return 0;
	}
	if (fseek(m_File.get(), 0, SEEK_END)) {
		STARVFSErrorLog("File seek failed!");
		return 0;
	}
	return ftell(m_File.get());
}

//-----------------------------------------------------------------------------

bool BlockFileDevice::SeekBeg(size_t Offset) const {
	if (fseek(m_File.get(), Offset, SEEK_SET)) {
		STARVFSErrorLog("File seek failed!");
		return false;
	}
	return true;
}

bool BlockFileDevice::SeekEnd(size_t Offset) const {
	if (fseek(m_File.get(), Offset, SEEK_END)) {
		STARVFSErrorLog("File seek failed!");
		return false;
	}
	return true;
}

bool BlockFileDevice::CanWrite() const {
	if (Flags.Ok && Flags.CanWrite) {
		return true;
	}
	//todo: log
	return false;
}

bool BlockFileDevice::CanRead() const {
	if (Flags.Ok && Flags.CanRead) {
		return true;
	}
	//todo: log
	return false;
}

bool BlockFileDevice::RawRead(char *data, size_t ToRead, size_t *read = nullptr) const {
	size_t r = fread(data, 1, ToRead, m_File.get());
	if (read)
		*read = r;
	return r == ToRead;
}

bool BlockFileDevice::RawWrite(const char *data, size_t ToWrite, size_t *written = nullptr) const {
	size_t w = fwrite(data, 1, ToWrite, m_File.get());
	if (w)
		*written = w;
	return w == ToWrite;
}

//-----------------------------------------------------------------------------

bool BlockFileDevice::ReadFromBegining(size_t Offset, char *data, size_t ToRead, size_t *read = nullptr) const {
	if (read)
		*read = 0;
	if (ToRead == 0) {
		return true;
	}
	StarVFSAssert(data);

	return CanRead() && SeekBeg(Offset) && RawRead(data, ToRead, read);
}

bool BlockFileDevice::ReadFromEnd(size_t Offset, char *data, size_t ToRead, size_t *read = nullptr) const {
	if (read)
		*read = 0;
	if (ToRead == 0) {
		return true;
	}
	StarVFSAssert(data);
	return CanRead() && SeekEnd(Offset) && RawRead(data, ToRead, read);
}

//-----------------------------------------------------------------------------

bool BlockFileDevice::WriteAt(size_t Offset, const char *data, size_t ToWrite, size_t *written) const {
	if (written)
		*written = 0;
	if (ToWrite == 0) {
		return true;
	}
	StarVFSAssert(data);
	return CanWrite() && SeekBeg(Offset) && RawWrite(data, ToWrite, written);
}

bool BlockFileDevice::WriteAtEnd(const char *data, size_t ToWrite, size_t *written) const {
	if (written)
		*written = 0;
	if (ToWrite == 0) {
		return true;
	}
	StarVFSAssert(data);
	return CanWrite() && SeekEnd(0) && RawWrite(data, ToWrite, written);
}

} //namespace RDC 
} //namespace StarVFS 
