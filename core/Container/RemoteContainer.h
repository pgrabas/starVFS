/*
  * Generated by cppsrc.sh
  * On 2015-12-12 12:02:12,55
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef RemoteContainer_H
#define RemoteContainer_H

namespace StarVFS {
namespace Containers {

class RemoteContainer : public iContainer {
public:
	RemoteContainer(FileTableInterface  *fti);
	RemoteContainer(FileTableInterface  *fti, int port, const String& host);
	virtual ~RemoteContainer();

	virtual bool ReloadContainer() override;

	virtual FileID GetFileCount() const override;
	virtual bool RegisterContent() const override;

	virtual const String& GetFileName() const override;

	virtual RWMode GetRWMode() const override;

	virtual bool GetFileData(FileID ContainerFID, CharTable &out, FileSize *DataSize) const override;

	static bool CanOpen(const String& Location);
	static CreateContainerResult CreateFor(StarVFS *svfs, const String& MountPoint, const String& Location);
private:
	String m_Host;
	int m_Port;

	struct Connection;
	std::unique_ptr<Connection> m_Connection;
};
} //namespace Containers 
} //namespace StarVFS 

#endif
