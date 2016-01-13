/*
  * Generated by cppsrc.sh
  * On 2015-12-22 21:08:18,29
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef FileTableInterface_H
#define FileTableInterface_H

namespace StarVFS {
namespace Containers {

class FileTableInterface final {
public:
 	FileTableInterface(FileTable *Owner, ContainerID cid);
 	~FileTableInterface();

	struct FileSubStructureInfo {
		FileID m_Count;
		BaseFileInfo* m_FileTable;
		FilePathHash* m_LocalPathHashTable;
	};

	/// all pointers in SubStructure shall be valid at leas until function returns
	bool RegisterFileStructure(FileID Parent, const FileSubStructureInfo& SubStructure);

	bool EnsureReserve(FileID count);

	FileID FindFile(const CString InternalFullPath);
	FileID FindFile(FilePathHash PathHash);

	FileID AllocFileID(const CString InternalFullPath);
	/** Forces full path to be valid. Several directories may be created if necessary. Function is slow, should not be used */
	FileID ForceAllocFileID(const CString InternalFullPath);
	FileID AllocFileID(FileID Parent, FilePathHash PathHash, const CString FileName);

	FileID GetRootID() const { return m_MountPoint; }
	bool IsMoutedToRoot() const { return m_MountPoint == 1; }

	bool CreateFile(FileID fid, FileID cfid, FileSize Size);
	bool CreateDirectory(FileID fid, FileID cfid);
	//bool CreateLink()
	//bool DeleteFile()

	/** function fails if container is not owner of fid pr fid is not a regular file */
	bool UpdateFileSize(FileID fid, FileSize NewSize);

	bool IsFileValid(FileID fid) const;
	bool IsDirectory(FileID fid) const;
	bool IsFile(FileID fid) const;

	const CString GetFileName(FileID fid) const { return m_Owner->GetFileName(fid); }
	String GetFileFullPath(FileID fid) const { return m_Owner->GetFileFullPath(fid); }

	ContainerID GetContainerID() const { return m_CID; }
	FileTable* GetFileTable() { return m_Owner; }
private: 
	FileTable *m_Owner;
	FileID m_MountPoint;
	ContainerID m_CID;
};

} //namespace Containers 
} //namespace StarVFS 

#endif
