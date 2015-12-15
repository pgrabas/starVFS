/*
  * Generated by cppsrc.sh
  * On 2015-12-10 17:53:58,21
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#include "StarVFSInternal.h"

#ifdef STARVFS_FOLDER_CONTAINER
#include "FolderContainer.h"
#endif

#include "Modules/RemoteContainer.h"

namespace StarVFS {

StarVFS::StarVFS(unsigned FSFlags) {
	m_FileTable = std::make_unique<FileTable>();
	m_HandleTable = std::make_unique<HandleTable>(m_FileTable.get());
}

StarVFS::~StarVFS() {
	m_Modules.clear();
	m_HandleTable.reset();
	m_FileTable.reset();
}

//-----------------------------------------------------------------------------

void StarVFS::DumpStructure(std::ostream &out) const{
	m_FileTable->DumpStructure(out);
}

void StarVFS::DumpFileTable(std::ostream &out) const {
	m_FileTable->DumpFileTable(out);
}

//-----------------------------------------------------------------------------

bool StarVFS::IsFileValid(FileID fid) const {
	return m_FileTable->IsValid(fid);
}

bool StarVFS::IsFileDirectory(FileID fid) const {
	auto f =  m_FileTable->GetFile(fid);
	if (!f) return false;
	return f->m_Flags.Directory > 0;
}

String StarVFS::GetFullFilePath(FileID fid) const {
	return m_FileTable->GetFileFullPath(fid);
}

const CString StarVFS::GetFileName(FileID fid) const {
	return m_FileTable->GetFileName(fid);
}

FileSize StarVFS::GetFileSize(FileID fid) const {
	auto f = m_FileTable->GetFile(fid);
	if (!f) 
		return 0;
	return f->m_Size;
}

//-----------------------------------------------------------------------------

FileID StarVFS::FindFile(const String& FileName) { return m_FileTable->Lookup(FileName); }

FileHandle StarVFS::OpenFile(const String& FileName, RWMode ReadMode, OpenMode FileMode) {
	FileID fid = 0;
	switch (FileMode) {
//	case OpenMode::CreateNew: //TBD
//		break;
	case OpenMode::OpenExisting:
		fid = FindFile(FileName);
		break;
	default:
		return FileHandle();
	}
	return m_HandleTable->CreateHandle(fid, ReadMode);
	//return OpenFile(FindFile(FileName), ReadMode, FileMode);
}

FileHandle StarVFS::OpenFile(FileID fid, RWMode ReadMode) {
	return m_HandleTable->CreateHandle(fid, ReadMode);
}

//-----------------------------------------------------------------------------

VFSErrorCode StarVFS::OpenContainer(const String& ContainerFile, const String &MountPoint, unsigned ContainerFlags) {
	Container c;
	auto r = CreateContainer(c, ContainerFile, ContainerFlags);
	if (r != VFSErrorCode::Success) {
		//TODO: log
		return r;
	}

	if (!c->ReloadContainer()) {
		//TODO: log
		return VFSErrorCode::ContainerCriticalError;
	}

	if (!m_FileTable->AddLayer(std::move(c))) {
		//TODO: log
		return VFSErrorCode::InternalError;
	}

	return VFSErrorCode::Success;
}

VFSErrorCode StarVFS::CreateContainer(Container& out, const String& ContainerFile, unsigned ContainerFlags) {
	//if (!boost::filesystem::exists(ContainerFile)) {
	//	//AddLogf(Error, "File '%s' does not exists!", FileName.c_str());
	//	return VFSErrorCode::ContainerDoesNotExits;
	//}

	Container c;
	do {
		if (boost::filesystem::is_directory(ContainerFile)) {
#ifndef STARVFS_FOLDER_CONTAINER
			//AddLogf(Error, "File '%s' is an directory!", File.c_str());
#else 
			c.reset(new FolderContainer(ContainerFile, ContainerFlags));
#endif
			break;
		}
		if (!strncmp("tcp://", ContainerFile.c_str(), 6)) {
			String uri = ContainerFile;
			auto port = (char*)strrchr(uri.c_str(), ':');
			auto host = (char*)strrchr(uri.c_str(), '/');

			if (port < host)
				port = 0;

			if (port)
				*port++ = 0;

			if (host) {
				*host++ = 0;
				int intport = port ? strtol(port, nullptr, 10) : 0;
				c = std::make_unique<Modules::RemoteContainer>(host, intport);
			}
		}

//		if (1) {
//			AddLog(TODO, "Consider: test RDC container extension");
//			auto mgc = new MoonGlareContainer::Reader(FileName);
//			c.reset(mgc);
//			if (c->IsReady() && c->IsReadable()) {//try open container
//				return c;
//			}
//		}
//		c.reset();
	} while (0);
//	AddLogf(Error, "Unable to open container for file '%s'!", FileName.c_str());
//	return nullptr;

	if (!c) {
		//TODO: log
		return VFSErrorCode::UnknownContainerFormat;
	}

	STARVFSDebugLog("Created container %s for %s", typeid(*c.get()).name(), ContainerFile.c_str());

	out.swap(c);
	return VFSErrorCode::Success;
}

} //namespace StarVFS 
