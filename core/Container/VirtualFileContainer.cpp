/*
  * Generated by cppsrc.sh
  * On 2015-12-27 21:51:19,30
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include "iContainer.h"
#include "VirtualFileContainer.h"

namespace StarVFS {
namespace Containers {

VirtualFileInterface::VirtualFileInterface() {}
VirtualFileInterface::~VirtualFileInterface() { }
FileSize VirtualFileInterface::GetSize() const { return 0; }
bool VirtualFileInterface::ReadFile(CharTable &out, FileSize *DataSize) const { out.reset();  return false; }

//-------------------------------------------------------------------------------------------------

VirtualFileContainer::VirtualFileContainer(FileTableInterface *fti):
		iContainer(fti), m_InternalIDCounter(0) {
	m_InternalIDCounter = 1;
	m_Files.emplace_back(); //0 is not valid id allways

}

VirtualFileContainer::~VirtualFileContainer() {
}

//-------------------------------------------------------------------------------------------------

bool VirtualFileContainer::GetFileData(FileID ContainerFID, CharTable &out, FileSize *DataSize) const {
	if(!ContainerFID || ContainerFID >= m_Files.size())
		return false;

	auto &f = m_Files[ContainerFID];
	auto ptr = f.GetPtr();
	if (!ptr)
		return false;
	return ptr->ReadFile(out, DataSize);
}

//-------------------------------------------------------------------------------------------------

FileID VirtualFileContainer::GetFileCount() const {
	return static_cast<FileID>(m_Files.size() - 1);//dont count invalid id 0
}

bool VirtualFileContainer::ReloadContainer() {
	return true;
}

bool VirtualFileContainer::RegisterContent() const {
	return true;
}

//-------------------------------------------------------------------------------------------------

bool VirtualFileContainer::RegisterFile(SharedVirtualFileInterface SharedFile, const String& Path, bool ForcePath) {
	if (!SharedFile)
		return false;

	FileInfo fi = {};
	fi.m_FullPath = Path;
	fi.m_InternalID = m_InternalIDCounter++;
	fi.m_WeakPtr = SharedFile;
	m_Files.emplace_back(std::move(fi));
	return ReloadFile(m_Files.back());
}

bool VirtualFileContainer::AddFile(SharedVirtualFileInterface SharedFile, const String& Path, bool ForcePath) {
	if (!SharedFile)
		return false;

	FileInfo fi = {};
	fi.m_FullPath = Path;
	fi.m_InternalID = m_InternalIDCounter++;
	fi.m_SharedPtr = SharedFile;
	fi.m_WeakPtr = SharedFile;
	m_Files.emplace_back(std::move(fi));
	return ReloadFile(m_Files.back());
}

bool VirtualFileContainer::DropFile(SharedVirtualFileInterface SharedFile) {
	return false;
}

//-------------------------------------------------------------------------------------------------

bool VirtualFileContainer::ReloadFile(FileInfo &fi) {
	if (fi.m_GlobalID)
		return true;

	auto fti = GetFileTableInterface();

	auto fid = fti->AllocFileID(fi.m_FullPath);
	if (!fid)
		return false;

	fi.m_GlobalID = fid;
	auto ptr = fi.GetPtr();
	fti->CreateFile(fid, fi.m_InternalID, ptr ? ptr->GetSize() : 0 );

	return true;
}

} //namespace Containers 
} //namespace StarVFS 
