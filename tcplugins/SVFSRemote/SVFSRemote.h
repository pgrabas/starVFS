/*
  * Generated by cppsrc.sh
  * On 
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef SVFSRemote_H
#define SVFSRemote_H

#include "fsplugin.h"

#include <core/nStarVFS.h>
#include "../common/LogVirtualSink.h"

#define TCSearchDataMagic (uint32_t)0x159648ab

struct TCSearchData {
	uint32_t m_Magic = TCSearchDataMagic;
	std::vector<StarVFS::FileID> m_FileIDs;

	static TCSearchData *FromHandle(HANDLE h) {
		TCSearchData *p = (TCSearchData*)h;
		if (p->m_Magic != TCSearchDataMagic)
			return nullptr;
		return p;
	}

	HANDLE AsHandle() const { return (HANDLE)this; }

	TCSearchData() {
		m_Magic = TCSearchDataMagic;
	}
};

class SVFSRemote final {
public:
 	SVFSRemote();
 	~SVFSRemote();

	void Init(int PluginNr, tProgressProc pProgressProc, tLogProc pLogProc, tRequestProc pRequestProc);

	void DoSVFSScan();

	TCSearchData* FindFirst(const char *Path, WIN32_FIND_DATA *FindData);
	bool FindNext(TCSearchData *data, WIN32_FIND_DATA *FindData);
	int FindClose(TCSearchData *data);

	bool GetFile(const char *src, const char *dst);

	void ProgressProc(const char *src, const char *dst, int progress) {
		if (m_ProgressProc)
			m_ProgressProc(m_PluginNr, (char*)src, (char*)dst, progress);
	}
private:
	int m_PluginNr;					//Internal number this plugin was given in Total Commander. Has to be passed as the first parameter in all callback functions so Totalcmd knows which plugin has sent the request.
	tProgressProc m_ProgressProc;	//Pointer to the progress callback function.
	tLogProc m_LogProc;				//Pointer to the logging function
	tRequestProc m_RequestProc;		//Pointer to the request text proc

	std::unique_ptr<::StarVFS::StarVFS> m_SVFS;
};

#endif
