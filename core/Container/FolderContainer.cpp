/*
  * Generated by cppsrc.sh
  * On 2015-12-10 19:39:52,96
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include "iContainer.h"
#include "FolderContainer.h"
#include <boost/filesystem.hpp>

#ifndef STARVFS_DISABLE_FOLDERCONTAINER
namespace StarVFS {
namespace Containers {

FolderContainer::FolderContainer(FileTableInterface *fti, String Path) :
		iContainer(fti),
		m_Path(std::move(Path)) {

	if (m_Path.back() != '/')
		m_Path += '/';
}

FolderContainer::~FolderContainer() {
}

//-------------------------------------------------------------------------------------------------

String FolderContainer::GetContainerURI() const { return m_Path; }
RWMode FolderContainer::GetRWMode() const { return RWMode::RW; }

//-------------------------------------------------------------------------------------------------

bool FolderContainer::ScanPath() {
	m_FileEntry.clear();
	m_FileEntry.reserve(2048);// because why not
	m_FileEntry.emplace_back(Entry{ FileType::Directory, 0, "", "", 0, });

	std::function<void(const String&, FileType)> handler;

	handler = [this, &handler](const String& path, FileType type) {
		auto subpath = path.substr(m_Path.length() - 1, path.length() - m_Path.length() + 1);

		for (auto &it : subpath)
			if (it == '\\')
				it = '/';

		uint64_t fsize = 0;
		if (type == FileType::File)
			try {
				fsize = (FileSize)boost::filesystem::file_size(path);
			} 
			catch (...) {
			}
		m_FileEntry.emplace_back(Entry{ type, 0, path, subpath, fsize, });
	};

	if (!EnumerateFolder(m_Path, handler)) {
		STARVFSErrorLog("Failed to enumerate folder %s", m_Path.c_str());
		return false;
	}

	m_FileEntry.shrink_to_fit();
	return true;
}

bool FolderContainer::ReloadContainer() {
	auto fcount = GetFileCount();
	auto fti = GetFileTableInterface();
	StarVFSAssert(fti);
	for (FileID cfid = 1, j = fcount; cfid <= j; ++cfid) {
		auto &f = m_FileEntry[cfid];
		fti->DeleteFile(f.m_GlobalFid);
	}
	return ScanPath();
}

bool FolderContainer::RegisterContent() const {
	auto fti = GetFileTableInterface();
	StarVFSAssert(fti);

	auto fcount = GetFileCount();

	if (!fti->EnsureReserve(fcount)) {
		STARVFSErrorLog("Failed to reload folder container");
		return false;
	}

	for (FileID cfid = 1, j = fcount; cfid <= j; ++cfid) {
		auto &f = const_cast<Entry&>(m_FileEntry[cfid]);

		FileID fid = fti->AllocFileID((CString)f.m_SubPath.c_str());
		if (!fid){
			STARVFSErrorLog("Failed to alloc fileid for %s", f.m_SubPath.c_str());
			continue;
		}
		f.m_GlobalFid = fid;

//		bool success;
		switch (f.m_Type) {
		case FileType::Directory:
			/*success =*/ fti->CreateDirectory(fid, cfid);
			break;
		case FileType::File:
			/*success =*/ fti->CreateFile(fid, cfid, static_cast<FileSize>(f.m_FileSize));
			break;
		default:
			STARVFSErrorLog("Invalid FileType value!");
			StarVFSAssert(false);
			continue;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------------------

FileID FolderContainer::GetFileCount() const {
	if (m_FileEntry.empty())
		return 0;
	return static_cast<FileID>(m_FileEntry.size() - 1);//dont count invalid 0-id
}

bool FolderContainer::GetFileData(FileID ContainerFID, ByteTable &out) const {
	out.reset();
	if (ContainerFID >= m_FileEntry.size() || ContainerFID == 0)
		return false;

	auto &f = m_FileEntry[ContainerFID];

	std::ifstream inp(f.m_FullPath.c_str(), std::ios::in | std::ios::binary);
	if (!inp) 
		return false;

	auto size = (size_t)boost::filesystem::file_size(f.m_FullPath);

	out.make_new(size);
	inp.read((char*)out.get(), out.byte_size());
	inp.close();
	return true;
}

bool FolderContainer::SetFileData(FileID ContainerFID, const ByteTable &in) const {
	if (ContainerFID >= m_FileEntry.size() || ContainerFID == 0)
		return false;

	auto &f = m_FileEntry[ContainerFID];
	std::ofstream outf(f.m_FullPath.c_str(), std::ios::out | std::ios::binary);
	outf.write(in.c_str(), in.byte_size());
	outf.close();

	return true;
}

FileID FolderContainer::FindFile(const String& ContainerFileName) const {
	for (auto it = m_FileEntry.begin(), jt = m_FileEntry.end(); it != jt; ++it)
		if (it->m_SubPath == ContainerFileName)
			return static_cast<FileID>(it - m_FileEntry.begin());

	return 0;
}

bool FolderContainer::EnumerateFiles(ContainerFileEnumFunc filterFunc) const {
	for (auto &entry: m_FileEntry) {
		auto cfid = static_cast<FileID>(&entry - &m_FileEntry[0]);
		FileFlags flags;
		flags.intval = 0;
		flags.Directory = entry.m_Type == FileType::Directory;
		flags.Valid = true;
//		using ContainerFileEnumFunc = std::function<bool(ConstCString fname, FileFlags flags, FileID CFid, FileID ParentCFid)>;
		
		if (!filterFunc((ConstCString)entry.m_SubPath.c_str(), flags, cfid, (FileID)0/*parentcfid*/))
			break;
	}
	return true;
}

//-------------------------------------------------------------------------------------------------

template <class T>
bool FolderContainer::EnumerateFolder(const String &Path, T func) {
	using boost::filesystem::directory_iterator;
	using boost::filesystem::recursive_directory_iterator;
	boost::filesystem::path p(Path);
	if (!boost::filesystem::is_directory(Path))
		return false;
	
	try {
		recursive_directory_iterator it(Path);

		for (; it != recursive_directory_iterator();) {
			auto item = it->path();
			FileType type;

			String fullPath = item.string();

			if (boost::filesystem::is_regular_file(item))
				type = FileType::File;
			else
				type = FileType::Directory;

			func(fullPath, type);

			while(it != recursive_directory_iterator())
				try {
					++it;
					break;
				} 
				catch (...) {
					it.no_push();
				}
			}
	}
	catch (const std::exception &e) {
		STARVFSErrorLog("Exception: %s", e.what());
		return true;
	}

	return true;
}

//-------------------------------------------------------------------------------------------------

bool FolderContainer::CanOpen(const String& Location) {
	return boost::filesystem::is_directory(Location);
}

CreateContainerResult FolderContainer::CreateFor(StarVFS *svfs, const String& MountPoint, const String& Location) {
	StarVFSAssert(svfs);
	return svfs->CreateContainer<FolderContainer>(MountPoint, Location);
}

} //namespace Containers 
} //namespace StarVFS 

#endif
