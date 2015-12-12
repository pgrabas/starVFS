/*
  * Generated by cppsrc.sh
  * On 2015-12-11 21:48:12,05
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include "StarVFSInternal.h"

namespace StarVFS {

struct HandleTable::HandleData {
	uint16_t m_HID;
	uint16_t m_Generation;
	FileID m_FileID;
	RWMode m_Mode;//todo: value is not checked
	union {
		uint8_t intval;
		struct {
			uint8_t Used : 1;
		};
	} m_Flags;
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

HandleTable::HandleTable(FileTable *FT):	
		m_FileTable(FT),
		m_LastAllocatedID(0) {
	m_HandleTable.resize(Settings::Initial::HandleTableSize);

	for (size_t i = 0, j = m_HandleTable.size(); i < j; ++i) {
		m_HandleTable[i].m_HID = static_cast<uint16_t>(i);
		m_HandleTable[i].m_Generation = 1;
	}
}

HandleTable::~HandleTable() {
}

//-------------------------------------------------------------------------------------------------

HandleTable::HandleData *HandleTable::AllocHandle() {
	for (size_t tries = 0, max = m_HandleTable.size(); tries < max; ++tries) {
		auto hid = m_LastAllocatedID++;
		if (max == m_LastAllocatedID)
			m_LastAllocatedID = 0;
		auto *h = &m_HandleTable[hid];
		if (h->m_Flags.Used)
			continue;

		return h;
	}
	return nullptr;
}

HandleTable::HandleData* HandleTable::GetDataFromHandle(const FileHandle& h) const {
	auto hid = h.GetHandleID();
	if (hid >= m_HandleTable.size())
		return nullptr;

	auto *hd = &m_HandleTable[hid];
	if (!hd->m_Flags.Used)
		return nullptr;

	return const_cast<HandleData*>(hd);
}

//-------------------------------------------------------------------------------------------------

FileHandle HandleTable::CreateHandle(FileID fid, RWMode ReadMode) {
	if (!m_FileTable->IsValid(fid))
		return FileHandle();

	auto h = AllocHandle();
	h->m_Flags.Used = 1;
//	++h->m_Generation;
	h->m_FileID = fid;
	h->m_Mode = ReadMode;
//	h->

	return FileHandle(this, h->m_HID, h->m_Generation);
}

//-------------------------------------------------------------------------------------------------

FileSize HandleTable::HandleGetSize(const FileHandle& h) const {
	auto hd = GetDataFromHandle(h);
	if (!hd) return 0;
	auto f = m_FileTable->GetFile(hd->m_FileID);
	return f ? f->m_Size : 0;
}

String HandleTable::HandleGetFullPath(const FileHandle& h) const {
	auto hd = GetDataFromHandle(h);
	return hd ? m_FileTable->GetFileFullPath(hd->m_FileID) : String();
}

bool HandleTable::HandleGetFileData(const FileHandle& h, CharTable &data) const {
	auto hd = GetDataFromHandle(h);
	if (!hd) return false;
	return m_FileTable->GetFileData(hd->m_FileID, data);
}

RWMode HandleTable::HandleGetRWMode(const FileHandle& h) const {
	auto hd = GetDataFromHandle(h);
	return hd ? hd->m_Mode : RWMode::None;
}

bool HandleTable::HandleIsDirectory(const FileHandle& h) const {
	auto hd = GetDataFromHandle(h);
	if (!hd) return 0;
	auto f = m_FileTable->GetFile(hd->m_FileID);
	return f ? f->m_Flags.Directory == 1 : false;
}

bool HandleTable::HandleIsSymlink(const FileHandle& h) const {
	auto hd = GetDataFromHandle(h);
	if (!hd) return 0;
	auto f = m_FileTable->GetFile(hd->m_FileID);
	return f ? f->m_Flags.SymLink == 1 : false;
}

bool HandleTable::HandleIsHandleValid(const FileHandle& h) const {
	auto hd = GetDataFromHandle(h);
	if (!hd) return false;
	auto f = m_FileTable->GetFile(hd->m_FileID);
	return f != nullptr;
}

void HandleTable::HandleClose(const FileHandle& h) {
	auto hd = GetDataFromHandle(h);
	if (!h)
		return;
	hd->m_FileID = 0;
	++hd->m_Generation;
	hd->m_Mode = RWMode::None;
	hd->m_Flags.Used = 0;
}

} //namespace StarVFS 
