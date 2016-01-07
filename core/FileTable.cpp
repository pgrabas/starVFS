/*
  * Generated by cppsrc.sh
  * On 2015-12-10 18:00:09,91
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include "StarVFSInternal.h"
#include "FileTable.h"

namespace StarVFS {

FileTable::FileTable(StarVFS *Owner):
		m_Owner(Owner) {
	StarVFSAssert(Owner);

	m_Capacity = m_Allocated = 0;
	m_FileTable = nullptr;
	m_Interfaces.push_back(nullptr);

	m_StringTable = std::make_unique<StringTable>();
	Realloc(Settings::Initial::FileTableSize);
	m_Allocated = 1;//first entry is not valid

	auto *root = AllocNewFile();
	root->m_Flags.Valid = 1;
	root->m_Flags.Directory = 1;
	root->m_Hash = FilePathHashAlgorithm::Hash("/", 1);
	m_HashFileTable.Add(root);
}

FileTable::~FileTable() {
	m_Capacity = m_Allocated = 0;

	m_FileTable = nullptr;
	m_StringTable.reset();
}

//-------------------------------------------------------------------------------------------------

void FileTable::DumpStructure(std::ostream &out) const {
	const char *fmt = "%5s. %3s:%-4s %8s %5s %6s %6s %6s %10s %s%s\n";
	char buf[512];
	sprintf(buf, fmt, "GFID", "CID", "CFID", "HASH", "FLAGS", "PARENT", "FCHILD", "NEXT", "SIZE", "", "NAME");
	out << buf;

	std::function<void(FileID, int)> Printer;
	Printer = [&Printer, this, fmt, &out](FileID id, int level) {
		char buf[512];
		char idbuf[32];
		char containeridbuf[32];
		char containerfileidbuf[32];
		char hashbuf[32];
		char sizebuf[32];
		char parentbuf[32];
		char firstchildbuf[32];
		char nextsyblingbuf[32];
		char levelbuf[128];

		for (int i = 0; i < level; ++i) {
			levelbuf[i * 3 + 0] = '|';
			levelbuf[i * 3 + 1] = ' ';
			levelbuf[i * 3 + 2] = ' ';
		}
		levelbuf[level * 3 + 0] = '}';
	//	levelbuf[level * 3 + 1] = ' ';
		levelbuf[level * 3 + 1] = 0;

		auto &f = m_FileTable[id];
		char flagsbuf[6] = "     ";

		sprintf(idbuf, "%d", id);
		sprintf(containeridbuf, "%d", f.m_ContainerID);
		sprintf(containerfileidbuf, "%d", f.m_ContainerFileID);
		sprintf(hashbuf, "%08x", f.m_Hash);

		sprintf(parentbuf, "%d", f.m_ParentFileID);
		sprintf(firstchildbuf, "%d", f.m_FirstChild);
		sprintf(nextsyblingbuf, "%d", f.m_NextSibling);

		sprintf(sizebuf, "%d", f.m_Size);

		if (f.m_Flags.Valid) flagsbuf[0] = 'V';
		if (f.m_Flags.Directory) flagsbuf[1] = 'D';
		if (f.m_Flags.SymLink) flagsbuf[2] = 'S';

		sprintf(buf, fmt, idbuf, containeridbuf, containerfileidbuf, hashbuf, flagsbuf,
				parentbuf, firstchildbuf, nextsyblingbuf, sizebuf, levelbuf, m_StringTable->Get(f.m_NameStringID));
		out << buf;

		if (f.m_Flags.Directory && f.m_FirstChild) {
			Printer(f.m_FirstChild, level + 1);
		}
		if(f.m_NextSibling)
			Printer(f.m_NextSibling, level);
	};

	Printer(1, 0);
}

void FileTable::DumpFileTable(std::ostream &out) const {
	const char *fmt = "%5s. %3s:%-4s %8s %5s %6s %6s %6s %10s %s\n";

	char buf[512];
	char idbuf[32];
	char containeridbuf[32];
	char containerfileidbuf[32];
	char hashbuf[32];
	char sizebuf[32];
	char parentbuf[32];
	char firstchildbuf[32];
	char nextsyblingbuf[32];
	
	sprintf(buf, fmt, "GFID", "CID", "CFID", "HASH", "FLAGS", "PARENT", "FCHILD", "NEXT", "SIZE", "NAME");
	out << buf;

	for (FileID i = 0; i < m_Allocated; ++i) {
		auto &f = m_FileTable[i];

		char flagsbuf[6] = "     ";

		sprintf(idbuf, "%d", i);
		sprintf(containeridbuf, "%d", f.m_ContainerID);
		sprintf(containerfileidbuf, "%d", f.m_ContainerFileID);
		sprintf(hashbuf, "%08x", f.m_Hash);

		sprintf(parentbuf, "%d", f.m_ParentFileID);
		sprintf(firstchildbuf, "%d", f.m_FirstChild);
		sprintf(nextsyblingbuf, "%d", f.m_NextSibling);

		sprintf(sizebuf, "%d", f.m_Size);

		if (f.m_Flags.Valid) flagsbuf[0] = 'V';
		if (f.m_Flags.Directory) flagsbuf[1] = 'D';
		if (f.m_Flags.SymLink) flagsbuf[2] = 'S';

		sprintf(buf, fmt, idbuf, containeridbuf, containerfileidbuf, hashbuf, flagsbuf, 
				parentbuf, firstchildbuf, nextsyblingbuf, sizebuf, m_StringTable->Get(f.m_NameStringID));
		out << buf;
	}
}

void FileTable::DumpHashTable(std::ostream &out) const {
	out << "Allocated: " << m_Allocated << " files\n";
}

//-------------------------------------------------------------------------------------------------

File* FileTable::AllocNewFile() {
	if (!EnsureCapacity(1))
		return nullptr;

	auto id = m_Allocated++;

	auto *f = m_FileTable.get() + id;
	memset(f, 0, sizeof(*f)); //is this necessary?
	f->m_GlobalFileID = id;

	return f;
}

File* FileTable::AllocNewFile(const CString fullpath) {
	const CString path = strrchr(fullpath, '/');
	if (!path) {
		//TODO: log
		return nullptr;
	}

	size_t len = path - fullpath;

	File *parent = nullptr;
	if (len > 0) {
		auto pid = m_HashFileTable.Lookup(fullpath, len);
		if (pid)
			parent = m_FileTable.get() + pid;
	} else
		parent = GetRoot();

	if (!parent)
		return nullptr;

	return AllocNewFile(parent, FilePathHashAlgorithm::Hash(fullpath, strlen(fullpath)), path + 1);
}

File* FileTable::AllocNewFile(File *Parent, FilePathHash PathHash, const CString FName) {
	if (!Parent)
		return nullptr;

	if (!Parent->m_Flags.Directory)
		return nullptr;

	auto f = AllocNewFile();
	f->m_NameStringID = m_StringTable->Alloc(FName);
	f->m_ParentFileID = Parent->m_GlobalFileID;
	f->m_NextSibling = Parent->m_FirstChild;
	Parent->m_FirstChild = f->m_GlobalFileID;
	f->m_Hash = PathHash;

	m_HashFileTable.Add(f);
	return f;
}

//-------------------------------------------------------------------------------------------------

File* FileTable::AllocFile(const String& InternalFullPath) {
	auto fid = m_HashFileTable.Lookup(InternalFullPath);
	if (fid) {
		return m_FileTable.get() + fid;
	}
	return AllocNewFile((const CString)InternalFullPath.c_str());
}

File* FileTable::AllocFile(FileID Parent, FilePathHash PathHash, const CString FileName) {
	if (!PathHash)
		return nullptr;
	auto fid = Lookup(PathHash);
	if (fid)
		return m_FileTable.get() + fid;
	auto p = GetFile(Parent);
	if (!p)
		return nullptr;

	return AllocNewFile(p, PathHash, FileName);
}

//-------------------------------------------------------------------------------------------------

String FileTable::GetFileFullPath(FileID fid) const {
	std::stringstream ss;
	auto f = GetFile(fid);
	if (!f)
		return String();
	struct T {
		static void Do(std::stringstream &ss, const File *f, const FileTable* This) {
			if (!f || f->m_GlobalFileID <= 1)
				return;
			Do(ss, This->GetFileParent(f), This);
			ss << "/" << This->m_StringTable->Get(f->m_NameStringID);
		}
	};
	T::Do(ss, f, this);
	return ss.str();
}

const CString FileTable::GetFileName(FileID fid) const {
	if (!fid || fid >= m_Allocated)
		return nullptr;
	return (CString)m_StringTable->Get(m_FileTable[fid].m_NameStringID);
}

bool FileTable::GetFileData(FileID fid, CharTable &data, FileSize *fsize) {
	auto f = GetFile(fid);
	if (!fid)
		//TODO: log
		return false;
	if (fsize)
		*fsize = 0;
	auto c = m_Owner->GetContainer(f->m_ContainerID);
	if (!c) {
		STARVFSErrorLog("Invalid cid for file %d", fid);
		return false;
	}
	return c->GetFileData(f->m_ContainerFileID, data, fsize);
}

FileFlags FileTable::GetFileFlags(FileID fid) const {
	auto f = GetFile(fid);
	if (!fid) {
		FileFlags flags;
		flags.intval = 0;
		return flags;
	}
	return f->m_Flags;
}

//-------------------------------------------------------------------------------------------------

bool FileTable::EnsureCapacity(FileID RequiredEmptySpace) {
	if (m_Allocated + RequiredEmptySpace < m_Capacity)
		return true;
	return Realloc(m_Allocated + RequiredEmptySpace);
}

bool FileTable::Realloc(FileID NewCapacity) {
	if (NewCapacity <= m_Capacity)
		return true;

	auto NewMemory = std::unique_ptr<File[]>(new File[NewCapacity]);
	if (!NewMemory)
		return false;

	memset(NewMemory.get(), 0, sizeof(File) * NewCapacity);

	if (m_Allocated > 0) {
		memcpy(NewMemory.get(), m_FileTable.get(), sizeof(File) * m_Allocated);
	}

	STARVFSDebugLog("Reallocated FileTable to %d entries", NewCapacity);
	m_FileTable.swap(NewMemory);
	m_Capacity = NewCapacity;
	 
	return m_HashFileTable.Resize(NewCapacity);
}

//-------------------------------------------------------------------------------------------------

Containers::FileTableInterface *FileTable::AllocateInterface(const String& MountPoint) {
	ContainerID cid = static_cast<ContainerID>(m_Interfaces.size());
	m_Interfaces.emplace_back(std::make_unique<Containers::FileTableInterface>(this, cid));
	return m_Interfaces.back().get();
}

//-------------------------------------------------------------------------------------------------

bool FileTable::RegisterStructureTable(FileStructureInfo &info) {
	if (!info.IsValid()) {
		STARVFSDebugLog("invalid info!");
		return false;
	}

	if (!EnsureCapacity(info.m_Count))
		return false;

	for (FileID i = 1; i < info.m_Count; ++i) {
		auto &basefile = info.m_FileTable[i];

		if (!basefile.m_Flags.Valid)
			continue;

		FilePathHash hash = info.m_PathHashTable[i];

		basefile.m_GlobalIndex = 0;
		
		File *fparent = nullptr;
		if (basefile.m_ParentIndex) {
			if (basefile.m_ParentIndex < info.m_Count && basefile.m_ParentIndex < i) {
				fparent = GetFile(info.m_FileTable[basefile.m_ParentIndex].m_GlobalIndex);
			}
		} else {
			fparent = info.m_Parent;
		}

		auto *f = AllocFile(fparent->m_GlobalFileID, hash, basefile.m_NamePointer);
		if (!f) {
			STARVFSErrorLog("Failed to allocate file!");
			return false;
		}

		bool changeownership = true;
		if (f->m_Flags.Valid) {
			if (f->m_Flags.Directory) {
				changeownership = false;
			} else {
			}
			if (basefile.m_Flags.Directory != f->m_Flags.Directory) {
				//todo: sth?
			}
		} else {
			f->m_Flags.Valid = 1;
			if (basefile.m_Flags.Directory) {
				f->m_Flags.Directory = 1;
			}
		}

		basefile.m_GlobalIndex = f->m_GlobalFileID;
		if (changeownership) {
			f->m_ContainerID = info.m_OwnerContainer;
			f->m_ContainerFileID = i;
		}
	}

	return true;
}

} //namespace StarVFS 
