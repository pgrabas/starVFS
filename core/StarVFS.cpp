/*
  * Generated by cppsrc.sh
  * On 2015-12-10 17:53:58,21
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#include "StarVFSInternal.h"
#include <SVFSRegister.h>

namespace StarVFS {

#ifdef STARVFS_LOG_TO_SINK

static void DefaultStarVFSLogSink(const char *file, const char *function, unsigned line, const char *log, const char *type)  { }
void(*StarVFSLogSink)(const char *file, const char *function, unsigned line, const char *log, const char *type) = DefaultStarVFSLogSink;

#endif

//-----------------------------------------------------------------------------

struct StarVFS::Internals {

	struct ContainerInfo {
		Container m_Container;
		String m_MountPoint;

		void Release() {
			m_Container.release();
			m_MountPoint.clear();
		}

		ContainerInfo() { }
		ContainerInfo(const ContainerInfo&) = delete;
		ContainerInfo(ContainerInfo&& oth) : m_Container(std::move(oth.m_Container)), m_MountPoint(std::move(oth.m_MountPoint)) { }
	};

	std::unique_ptr<HandleTable> m_HandleTable;
	std::vector<std::unique_ptr<Modules::iModule>> m_Modules;
	std::vector<ContainerInfo> m_Containers;
#ifndef STARVFS_DISABLE_REGISTER
	std::unique_ptr<Register> m_Register;
#endif

	~Internals() {
		m_HandleTable.reset();
		m_Modules.clear();
		m_Containers.clear();
	}
};

//-----------------------------------------------------------------------------

StarVFS::StarVFS(unsigned FSFlags): 
		m_Callback(nullptr) {
	m_FileTable = std::make_unique<FileTable>(this);
	m_Internals = std::make_unique<Internals>();
	m_Internals->m_HandleTable = std::make_unique<HandleTable>(m_FileTable.get());
	m_Internals->m_Containers.emplace_back();//cid:0 is not valid
}

StarVFS::~StarVFS() {
	m_Internals.reset();
	m_FileTable.reset();
}

StarVFSCallback* StarVFS::SetCallback(StarVFSCallback *newone) {
	auto prv = m_Callback;
	m_Callback = newone;
	return prv;
}

//-----------------------------------------------------------------------------

void StarVFS::DumpStructure(std::ostream &out) const{
	m_FileTable->DumpStructure(out);
}

void StarVFS::DumpFileTable(std::ostream &out) const {
	m_FileTable->DumpFileTable(out);
}

void StarVFS::DumpHashTable(std::ostream &out) const {
	m_FileTable->DumpHashTable(out);
}

//-----------------------------------------------------------------------------

bool StarVFS::IsFileValid(FileID fid) const {
	return m_FileTable->IsValid(fid);
}

bool StarVFS::IsFileDirectory(FileID fid) const {
	auto f =  m_FileTable->GetFile(fid);
	if (!f) return false;
	return f->m_Flags.Directory;
}

String StarVFS::GetFullFilePath(FileID fid) const {
	return m_FileTable->GetFilePath(fid, 0);
}

String StarVFS::GetFilePath(FileID fid, FileID ParentFID) const {
	return m_FileTable->GetFilePath(fid, ParentFID);
}

CString StarVFS::GetFileName(FileID fid) const {
	return m_FileTable->GetFileName(fid);
}

FileSize StarVFS::GetFileSize(FileID fid) const {
	auto f = m_FileTable->GetFile(fid);
	if (!f) 
		return 0;
	return f->m_Size;
}

bool StarVFS::GetFileData(FileID fid, ByteTable &data) {
	return m_FileTable->GetFileData(fid, data);
}

Containers::iContainer* StarVFS::GetContainer(ContainerID cid) {
	if (cid >= m_Internals->m_Containers.size())
		return nullptr;
	return m_Internals->m_Containers[cid].m_Container.get();
}

bool StarVFS::CloseContainer(ContainerID cid) {
	if (cid >= m_Internals->m_Containers.size())
		return false;

	m_FileTable->InvalidateCID(cid);
	m_Internals->m_HandleTable->InvalidateCID(cid);
	m_Internals->m_Containers[cid].Release();

	return true;
}

//-----------------------------------------------------------------------------

FileID StarVFS::FindFile(const String& FileName) { return m_FileTable->Lookup(FileName); }

FileHandle StarVFS::OpenFile(const String& FileName, RWMode ReadMode, OpenMode FileMode) {
	FileID fid = 0;
	switch (FileMode) {
	case OpenMode::OpenExisting:
		fid = FindFile(FileName);
		break;
	case OpenMode::CreateNew: //TBD
		STARVFSErrorLog("OpenMode::CreateNew is not implemented!");
	default:
		return FileHandle();
	}
	return m_Internals->m_HandleTable->CreateHandle(fid, ReadMode);
	//return OpenFile(FindFile(FileName), ReadMode, FileMode);
}

FileHandle StarVFS::OpenFile(FileID fid, RWMode ReadMode) {
	return m_Internals->m_HandleTable->CreateHandle(fid, ReadMode);
}

//-----------------------------------------------------------------------------

VFSErrorCode StarVFS::OpenContainer(const String& ContainerFile, const String &MountPoint) {
	//if (!boost::filesystem::exists(ContainerFile)) {
	//	//AddLogf(Error, "File '%s' does not exists!", FileName.c_str());
	//	return VFSErrorCode::ContainerDoesNotExits;
	//}

	auto r = Containers::CreateContainer(ContainerFile, MountPoint, this);
	return r.first;

# if 0

	std::pair<VFSErrorCode, Containers::iContainer*>  result = std::make_pair(VFSErrorCode::InternalError, nullptr);
	do {
		if (boost::filesystem::is_directory(ContainerFile)) {
#ifdef STARVFS_DISABLE_FOLDERCONTAINER
			//AddLogf(Error, "File '%s' is an directory!", File.c_str());
#else 
			result = CreateContainer<Containers::FolderContainer>(MountPoint, ContainerFile);
//			c.reset(new Containers::FolderContainer(ContainerFile));
#endif
			break;
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

	if (result.second)
		STARVFSDebugLog("Created container %s for %s", typeid(*result.second).name(), ContainerFile.c_str());

	return result.first;
#endif
}

VFSErrorCode StarVFS::MountContainer(Container c, String MountPoint) {
	StarVFSAssert(c);

	if (!c->Initialize()) {
		STARVFSErrorLog("container initialization failed!");
		return VFSErrorCode::InternalError;
	}

	if (m_Callback) {
		auto res = m_Callback->BeforeContainerMount(c.get(), MountPoint);
		switch (res) {
		case StarVFSCallback::BeforeContainerMountResult::Mount:
			break;
		case StarVFSCallback::BeforeContainerMountResult::Cancel:
			if (!c->Finalize()) {
				STARVFSErrorLog("container finalization failed!");
			}
			return VFSErrorCode::NotAllowed;
		default:
			STARVFSErrorLog("Invalid enum %s value %d", typeid(res).name(), (int)res);
			break;
		}
	}

	m_Internals->m_Containers.emplace_back();
	Internals::ContainerInfo &ci = m_Internals->m_Containers.back();
	ci.m_Container.swap(c);
	ci.m_MountPoint.swap(MountPoint);

	auto cid = ci.m_Container->GetContainerID();

	return ReloadContainer(cid, true);
}

//-----------------------------------------------------------------------------

ContainerID StarVFS::GetContainerCount() const {
	return static_cast<ContainerID>(m_Internals->m_Containers.size() - 1); //dont count first entry
}

VFSErrorCode StarVFS::ReloadContainer(ContainerID cid, bool FirstMount) {
	StarVFSAssert(cid < m_Internals->m_Containers.size());
	auto &ci = m_Internals->m_Containers[cid];
	if (!ci.m_Container->ReloadContainer()) {
		STARVFSErrorLog("Failed to reload cid: %d", cid);
		return VFSErrorCode::ContainerCriticalError;
	}

	if (!ci.m_Container->RegisterContent()) {
		STARVFSErrorLog("Failed to register container content cid: %d", cid);
		return VFSErrorCode::InternalError;
	}

	if (m_Callback) {
		if(FirstMount)
			m_Callback->AfterContainerMounted(ci.m_Container.get());
	}

	return VFSErrorCode::Success;
}

//-----------------------------------------------------------------------------

Containers::FileTableInterface* StarVFS::NewFileTableInterface(const String &MountPoint, bool Force) {
	return m_FileTable->AllocateInterface(MountPoint);
}

//-----------------------------------------------------------------------------
 
Register* StarVFS::GetRegister() {
#ifndef STARVFS_DISABLE_REGISTER
	if (!m_Internals->m_Register)
		m_Internals->m_Register = std::make_unique<Register>(this);
	return m_Internals->m_Register.get();
#else
	return nullptr;
#endif
}

//-----------------------------------------------------------------------------

HandleTable* StarVFS::GetHandleTable() { 
	return m_Internals->m_HandleTable.get(); 
}

//-----------------------------------------------------------------------------

size_t StarVFS::GetModuleCount() const { 
	return m_Internals->m_Modules.size(); 
}

Modules::iModule* StarVFS::GetModule(size_t mid) {
	if (mid >= GetModuleCount())  return nullptr;
	return m_Internals->m_Modules[mid].get();
}

Modules::iModule* StarVFS::InsertModule(std::unique_ptr<Modules::iModule> module){
	m_Internals->m_Modules.emplace_back(std::move(module));
	return m_Internals->m_Modules.back().get();
}

} //namespace StarVFS 
