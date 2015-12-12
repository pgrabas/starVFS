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
		//char unused3 : 1;
		//char unused4 : 1;
		//char unused5 : 1;
		//char unused6 : 1;
		//char unused7 : 1;
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
	FileIDHash m_Hash;
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

	FileID Lookup(const String &Path) { return Lookup((const CString)Path.c_str(), Path.length()); }
	FileID Lookup(const CString Path) { return Lookup(Path, strlen(Path)); }
	FileID Lookup(const CString Path, size_t PathLen);

	File* AllocFile(const String& InternalFullPath);
	File* AllocFile(FileID Parent, FileIDHash PathHash, const CString FileName);

	//delete

	bool GetFileData(FileID fid, CharTable &data);
	
	bool IsValid(FileID fid) const {
		if (!fid || fid >= m_Allocated || !m_FileTable[fid].m_Flags.Valid)
			return false;
		return true;
	}
	File* GetFile(FileID fid) const {
		if (!fid || fid >= m_Allocated || !m_FileTable[fid].m_Flags.Valid)
			return nullptr;
		return m_FileTable + fid;
	}

	File* GetFileParent(const File *f) const { return GetFile(f->m_ParentFileID); }
	File* GetFileFirstChild(const File *f) const { return GetFile(f->m_FirstChild); }
	File* GetFileNextSibling(const File *f) const { return GetFile(f->m_NextSibling); }

	iContainer* GetContainer(ContainerID cid) {
		if (cid >= m_Containers.size())
			return nullptr;
		return m_Containers[cid].get();
	}
	const CString GetFileName(FileID fid) const;
	String GetFileFullPath(FileID fid) const;

	const StringTable* GetStringTable() const { return m_StringTable.get(); }
	const File* GetTable() const { return m_FileTable; }
	FileID GetAllocatedFileCount() { return m_Allocated; }
private:
	std::mutex m_Mutex;
	using MutexGuard = std::lock_guard<std::mutex>;

	std::unique_ptr<StringTable> m_StringTable;
	std::vector<std::unique_ptr<iContainer>> m_Containers;

	FileID m_Capacity;
	FileID m_Allocated;
	std::unique_ptr<char[]> m_Memory;
	FileIDHash *m_HashTable;   //these tables must be synchronized
	FileID *m_FileIDTable;	   //these tables must be synchronized
	File *m_FileTable;

//Internal functions, mutex shall be locked before calling them
	bool EnsureCapacity(FileID RequiredEmptySpace);
	bool Realloc(FileID NewCapacity);
	FileID Lookup(FileIDHash Hash);

	File* AllocNewFile();
	File* AllocNewFile(File *Parent, FileIDHash PathHash, const CString FName);
	File* AllocNewFile(const CString fullpath);

	void AddToHashTable(File* f);
	void RebuildHashTable();
	void SortHashTable();

	File* GetRoot() const { return m_FileTable + 1; }
};

} //namespace StarVFS 

#endif
