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

class FileTableInterface final {
public:
 	FileTableInterface(FileTable *Owner, ContainerID cid);
 	~FileTableInterface();

//	File* AllocFile(const String& InternalFullPath);
//	File* AllocFile(FileID Parent, FilePathHash PathHash, const CString FileName);

//	AllocDir

	bool IsFileValid(FileID fid) const { return m_Owner->IsValid(fid); }

	const CString GetFileName(FileID fid) const { return m_Owner->GetFileName(fid); }
	String GetFileFullPath(FileID fid) const { return m_Owner->GetFileFullPath(fid); }

	ContainerID GetContainerID() const { return m_CID; }
	FileTable* GetFileTable() { return m_Owner; }
private: 
	FileTable *m_Owner;
	ContainerID m_CID;
};

} //namespace StarVFS 

#endif
