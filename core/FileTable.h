/*
  * Generated by cppsrc.sh
  * On 2015-12-10 18:00:09,91
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef FileTable_H
#define FileTable_H

namespace StarVFS {

union FileFlags {
	uint8_t intval;
	struct {
		uint8_t Valid : 1;				//Entry is valid
		uint8_t Directory : 1;
		uint8_t SymLink : 1;
		//uint8_t Shadowed : 1;
		//uint8_t Used : 1;
		//uint8_t unused4 : 1;
		//uint8_t unused5 : 1;
		//uint8_t unused6 : 1;
		//uint8_t unused7 : 1;
		//deleted ?
		//used ?
		//shadowed ?
	};
};

static_assert(sizeof(FileFlags) == 1, "File flags may have only 1 byte");

struct File final {
	FileID m_ContainerFileID;
	ContainerID m_ContainerID;
	StringID m_NameStringID;
	FileFlags m_Flags;
	FileSize m_Size;
	FileID m_GlobalFileID;
	FileID m_ParentFileID;
	FileID m_NextSibling;		//ignored when not a directory
	FileID m_FirstChild;
	FileID m_SymLinkIndex;		//valid if SymLink flag
	FilePathHash m_Hash;
};

static_assert(std::is_pod<File>::value, "File structure must be a POD type");

class FileTable final {
public:
 	FileTable(unsigned Flags = 0);
 	~FileTable();

	void DumpStructure(std::ostream &out) const;
	void DumpFileTable(std::ostream &out) const;
	void DumpHashTable(std::ostream &out) const;

	bool AddLayer(Container cin);
	//bool AddMultipleLayers(...);

	File* AllocFile(const String& InternalFullPath);
	File* AllocFile(FileID Parent, FilePathHash PathHash, const CString FileName);
	
	template<class ...ARGS> FileID Lookup(ARGS... args) { return m_HashFileTable.Lookup(std::forward<ARGS>(args)...); }
	//delete

	bool GetFileData(FileID fid, CharTable &data, FileSize *fsize = nullptr);
	
	bool IsValid(FileID fid) const {
		if (!fid || fid >= m_Allocated || !m_FileTable[fid].m_Flags.Valid)
			return false;
		return true;
	}
	File* GetFile(FileID fid) const {
		if (!fid || fid >= m_Allocated || !m_FileTable[fid].m_Flags.Valid)
			return nullptr;
		return &m_FileTable[fid];
	}

	File* GetFileParent(const File *f) const { return GetFile(f->m_ParentFileID); }
	File* GetFileFirstChild(const File *f) const { return GetFile(f->m_FirstChild); }
	File* GetFileNextSibling(const File *f) const { return GetFile(f->m_NextSibling); }

	iContainer* GetContainer(ContainerID cid) {
		if (cid >= m_ContainerTable.size())
			return nullptr;
		return m_ContainerTable[cid].m_Container.get();
	}

	const CString GetFileName(FileID fid) const;
	String GetFileFullPath(FileID fid) const;

	const StringTable* GetStringTable() const { return m_StringTable.get(); }
	const File* GetTable() const { return m_FileTable.get(); }
	FileID GetAllocatedFileCount() { return m_Allocated; }
private:
	struct ContainerInfo {
		std::unique_ptr<iContainer> m_Container;
		std::unique_ptr<FileTableInterface> m_Interface;
	};

	std::unique_ptr<StringTable> m_StringTable;
	std::vector<ContainerInfo> m_ContainerTable;
	HashFileTable m_HashFileTable;
	std::unique_ptr<File[]> m_FileTable;

	FileID m_Capacity, m_Allocated;

//Internal functions, mutex shall be locked before calling them
	bool EnsureCapacity(FileID RequiredEmptySpace);
	bool Realloc(FileID NewCapacity);

	File* AllocNewFile();
	File* AllocNewFile(File *Parent, FilePathHash PathHash, const CString FName);
	File* AllocNewFile(const CString fullpath);

	File* GetRoot() const { return m_FileTable.get() + 1; }
};

} //namespace StarVFS 

#endif
