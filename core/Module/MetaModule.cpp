/*
  * Generated by cppsrc.sh
  * On 2015-12-12  9:44:59,49
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#define _WIN32_WINNT 0x0501

#include "../StarVFSInternal.h"
#include "MetaModule.h"

namespace StarVFS {
namespace Modules {

struct FileTableMetaFile : public Containers::VirtualFileInterface {
	FileTableMetaFile(StarVFS *svfs):
			m_LastSize(0), m_svfs(svfs) {
		StarVFSAssert(svfs);
	}
	virtual FileSize GetSize() const override { return m_LastSize; }

	virtual bool ReadFile(CharTable &out, FileSize *DataSize) const override {
		auto ft = m_svfs->GetFileTable();
		std::stringstream ss;
		ft->DumpFileTable(ss);
		std::string data = ss.str();
		out.reset(new char[data.length() + 1]);
		out[data.length()] = 0;
		memcpy(out.get(), data.c_str(), data.length());
		m_LastSize = data.length() + 1;
		if (DataSize)
			*DataSize = m_LastSize;
		return true;
	}

private:
	mutable FileSize m_LastSize;
	StarVFS *m_svfs;
};

struct FileTableStructureMetaFile : public Containers::VirtualFileInterface {
	FileTableStructureMetaFile(StarVFS *svfs) :
		m_LastSize(0), m_svfs(svfs) {
		StarVFSAssert(svfs);
	}
	virtual FileSize GetSize() const override { return m_LastSize; }

	virtual bool ReadFile(CharTable &out, FileSize *DataSize) const override {
		auto ft = m_svfs->GetFileTable();
		std::stringstream ss;
		ft->DumpStructure(ss);
		std::string data = ss.str();
		out.reset(new char[data.length() + 1]);
		out[data.length()] = 0;
		memcpy(out.get(), data.c_str(), data.length());
		m_LastSize = data.length() + 1;
		if (DataSize)
			*DataSize = m_LastSize;
		return true;
	}

private:
	mutable FileSize m_LastSize;
	StarVFS *m_svfs;
};

struct StatisticsMetaFile : public Containers::VirtualFileInterface {
	StatisticsMetaFile(StarVFS *svfs) :
		m_LastSize(0), m_svfs(svfs) {
		StarVFSAssert(svfs);
	}
	virtual FileSize GetSize() const override { return m_LastSize; }

	void GetStatistics(std::ostream &o) const {
		auto ft = m_svfs->GetFileTable();
		
		o << "Allocated files: " << ft->GetAllocatedFileCount() << "\n";
		o << "Loaded containers: " << (int)m_svfs->GetContainerCount() << "\n";
		o << "Loaded modules: " << (int)m_svfs->GetModuleCount() << "\n";
		o << "\n";
		//	o << "Registered container types:\n";
		//	o << "Registered module types:\n";
		//	o << "Registered exporter types:\n";
	}

	virtual bool ReadFile(CharTable &out, FileSize *DataSize) const override {
		std::stringstream ss;
		GetStatistics(ss);
		std::string data = ss.str();
		out.reset(new char[data.length() + 1]);
		out[data.length()] = 0;
		memcpy(out.get(), data.c_str(), data.length());
		m_LastSize = data.length() + 1;
		if (DataSize)
			*DataSize = m_LastSize;
		return true;
	}

private:
	mutable FileSize m_LastSize;
	StarVFS *m_svfs;
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

MetaModule::MetaModule(StarVFS *svfs): iModule(svfs), m_MetaContainer(nullptr){
}

MetaModule::~MetaModule() {
	Disable();
}

//-------------------------------------------------------------------------------------------------

bool MetaModule::Enable() {
	if (m_MetaContainer)
		return true;

	auto ret = GetVFS()->CreateContainer<Containers::VirtualFileContainer>("/");
	if (!ret.second)
		return false;
	m_MetaContainer = dynamic_cast<Containers::VirtualFileContainer*>(ret.second);
	if (!m_MetaContainer)
		return false;

	{
		auto f = std::make_shared<FileTableMetaFile>(GetVFS());
		m_MetaFiles.emplace_back(f);
		m_MetaContainer->AddFile(f, "/$FileTable");
	}
	{
		auto f = std::make_shared<FileTableStructureMetaFile>(GetVFS());
		m_MetaFiles.emplace_back(f);
		m_MetaContainer->AddFile(f, "/$Structure");
	}
	{
		auto f = std::make_shared<StatisticsMetaFile>(GetVFS());
		m_MetaFiles.emplace_back(f);
		m_MetaContainer->AddFile(f, "/$Statistics");
	}
	return true;
}

bool MetaModule::Disable() {
	if (!m_MetaContainer)
		return true;

	m_MetaFiles.clear();

//	m_MetaContainer = nullptr;
	return false;
}

} //namespace Modules 
} //namespace StarVFS 
