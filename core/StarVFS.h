/*
  * Generated by cppsrc.sh
  * On 2015-12-10 17:53:58,21
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef StarVFS_H
#define StarVFS_H

namespace StarVFS {

class StarVFS {
public:
 	StarVFS(unsigned FSFlags = 0);
 	virtual ~StarVFS();

	template<class T>
	Containers::iContainer* CreateContainer(const String &MountPoint) {
		static_assert(std::is_base_of<Containers::iContainer, T>::value , "Invalid container class!");
		auto c = std::make_unique<T>(NewFileTableInterface());
		auto cptr = c.get();
		auto r = MountContainer(c, MountPoint);
		if (r != VFSErrorCode::Success)
			return nullptr;
		return cptr;
	}

	VFSErrorCode OpenContainer(const String& ContainerFile, const String &MountPoint = "/");

	/** Debug function. Prints all files in human-readable format. */
	void DumpStructure(std::ostream &out) const;
	void DumpFileTable(std::ostream &out) const;

	bool ForcePath(const String& path);

	FileID FindFile(const String& FileName);
	bool IsFileValid(FileID fid) const;
	bool IsFileDirectory(FileID fid) const;
	String GetFullFilePath(FileID fid) const;
	const CString GetFileName(FileID fid) const;
	FileSize GetFileSize(FileID fid) const;

	FileHandle OpenFile(const String& FileName, RWMode ReadMode = RWMode::R, OpenMode FileMode = OpenMode::OpenExisting);
	FileHandle OpenFile(FileID fid, RWMode ReadMode = RWMode::R);

	template<class T, class ...ARGS>
	Modules::iModule* AddModule(ARGS ...args) {
		static_assert(std::is_base_of<Modules::iModule, T>::value , "Invalid module class!");
		return InsertModule(std::make_unique<T>(this, std::forward<ARGS>(args)...));
	}
	size_t GetModuleCount() const;
	Modules::iModule* GetModule(size_t mid);

	template<class T, class ...ARGS>
	std::unique_ptr<T> CreateExporter(ARGS ...args) {
		static_assert(std::is_base_of<Exporters::iExporter, T>::value, "Invalid exporter class!");
		return std::make_unique<T>(this, std::forward<ARGS>(args)...);
	}

	FileTable* GetFileTable() { return m_FileTable.get(); }
	HandleTable* GetHandleTable();

	Register* GetRegister();
protected:
//	virtual bool CanLoadContainer(iContainer *container);
//	virtual void OnContaierLoaded(iContainer *container);
private:
	unsigned m_Flags;
	struct Internals;
	std::unique_ptr<Internals> m_Internals;
	std::unique_ptr<FileTable> m_FileTable;

	VFSErrorCode CreateContainer(Container& out, const String& ContainerFile);
	VFSErrorCode MountContainer(Container c, String MountPoint);
	VFSErrorCode ReloadContainer(ContainerID cid);
	Containers::FileTableInterface* NewFileTableInterface();
	Modules::iModule* InsertModule(std::unique_ptr<Modules::iModule> module);
};

} //namespace StarVFS 

#endif
