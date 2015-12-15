/*
  * Generated by cppsrc.sh
  * On 2015-12-10 19:39:52,96
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include "StarVFSInternal.h"
#include "FolderContainer.h"

#ifdef STARVFS_FOLDER_CONTAINER
namespace StarVFS {

#if 0
struct FolderContainerPointer : RawFilePointer {
	string SubPath;
	FileSize Size = 0;

	virtual FileSize GetFileSize() const
	{
		return Size;
	};
};

//-------------------------------------------------------------------------------------------------

class FolderFileReader : public iFileReader {
	GABI_DECLARE_CLASS_NOCREATOR(FolderFileReader, iFileReader);
public:
	FolderFileReader(iContainer *Owner) : BaseClass(Owner)
	{}
	~FolderFileReader()
	{}

	bool Open(const string& FileName)
	{
		std::ifstream inp(FileName.c_str(), std::ios::in | std::ios::binary);
		if (!inp) throw false;
		m_DataLen = static_cast<FileSize>(boost::filesystem::file_size(FileName));
		m_Data.reset(new char[m_DataLen + 1]);
		m_Data[m_DataLen] = 0;
		inp.read(&m_Data[0], m_DataLen);
		inp.close();
		m_FileName = FileName;
		return true;
	}

	virtual FileSize Size() const
	{
		return m_DataLen;
	}
	virtual const char* GetFileData() const
	{
		return &m_Data[0];
	}
	virtual const string& FileName() const
	{
		return m_FileName;
	}
private:
	std::unique_ptr<char[]> m_Data;
	FileSize m_DataLen;
	string m_FileName;
};

GABI_IMPLEMENT_CLASS_NOCREATOR(FolderFileReader);

//-------------------------------------------------------------------------------------------------

class FolderFileWritter : public iFileWritter {
	GABI_DECLARE_CLASS_NOCREATOR(FolderFileWritter, iFileWritter);
public:
	FolderFileWritter(iContainer *Owner, const string& Name) : BaseClass(Owner), m_FileName(Name)
	{}
	~FolderFileWritter()
	{
		std::ofstream inp(m_FileName.c_str(), std::ios::out | std::ios::binary);
		if (!inp) throw false;
		inp.write(&m_Data[0], m_DataLen);
		inp.close();
	}

	bool SetFileData(const char* data, FileSize size) override
	{
		m_DataLen = size;
		m_Data.reset(new char[m_DataLen + 1]);
		m_Data[m_DataLen] = 0;
		memcpy(&m_Data[0], data, m_DataLen);
		return true;
	}

	bool OwnData(std::unique_ptr<char[]> data, FileSize size)
	{
		m_Data.reset();
		m_Data.swap(data);
		m_DataLen = size;
		return true;
	}

	virtual FileSize Size() const
	{
		return m_DataLen;
	}
	virtual const char* GetFileData() const
	{
		return &m_Data[0];
	}
	virtual const string& FileName() const
	{
		return m_FileName;
	}
private:
	std::unique_ptr<char[]> m_Data;
	FileSize m_DataLen;
	string m_FileName;
};

#endif 

//------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

FolderContainer::FolderContainer(const String &Path, unsigned Flags) :
		iContainer(),
		m_Path(Path) {

	if (m_Path.back() != '/')
		m_Path += '/';
}

FolderContainer::~FolderContainer() {
//	m_PtrMap.clear();
//	m_PtrList.clear();
}

//-------------------------------------------------------------------------------------------------

const String& FolderContainer::GetFileName() const { return m_Path; }
RWMode FolderContainer::GetRWMode() const { return RWMode::RW; }

//-------------------------------------------------------------------------------------------------

bool FolderContainer::ReloadContainer() {
	m_FileEntry.clear();

	std::function<void(const String&, const String&, FileType)> handler;

	handler = [this, &handler](const String& path, const String& Subpath, FileType type) {
		m_FileEntry.emplace_back(Entry{ type, path, Subpath });
//		printf("%s -> %s\n", path.c_str(), Subpath.c_str());
		if (type == FileType::Directory)
			EnumerateFolder(path + "/", Subpath + "/", handler);
	};

	if (!EnumerateFolder(m_Path, "/", handler)) {
		STARVFSErrorLog("Failed to enumerate folder %s", m_Path.c_str());
		return false;
	}

	m_FileEntry.shrink_to_fit();
	return true;
}

FileID FolderContainer::GetFileCount() const {
	return static_cast<FileID>(m_FileEntry.size());
}

bool FolderContainer::RegisterFiles(FileTable *table) const {
	if (!table)
		return false;
	for (FileID i = 0, j = (FileID)m_FileEntry.size(); i < j; ++i) {
		auto &f = m_FileEntry[i];

		auto fptr = table->AllocFile(f.m_SubPath);

		if (!fptr)
			return false;

		fptr->m_Flags.Valid = 1;
		fptr->m_ContainerID = GetContainerID();
		fptr->m_ContainerFileID = i;

		if (f.m_Type == FileType::Directory)
			fptr->m_Flags.Directory = 1;
		else
			fptr->m_Size = (FileSize)boost::filesystem::file_size(f.m_FullPath);
	}

	return true;
}

//-------------------------------------------------------------------------------------------------

bool FolderContainer::GetFileData(FileID ContainerFID, CharTable &out, FileSize *DataSize) const {
	out.reset();
	if (DataSize)
		*DataSize = 0;
	if (ContainerFID >= m_FileEntry.size())
		return false;

	auto &f = m_FileEntry[ContainerFID];

	std::ifstream inp(f.m_FullPath.c_str(), std::ios::in | std::ios::binary);
	if (!inp) 
		return false;

	auto size = (size_t)boost::filesystem::file_size(f.m_FullPath);

	out.reset(new char[size + 1]);
	out[size] = '\0';
	inp.read(&out[0], size);
	inp.close();
	if (DataSize)
		*DataSize = size;
	return true;
}

//-------------------------------------------------------------------------------------------------

template <class T>
bool FolderContainer::EnumerateFolder(const String &Path, const String& BaseSubPath, T func) {

	using boost::filesystem::directory_iterator;
	boost::filesystem::path p(Path);
	if (!boost::filesystem::is_directory(Path))
		return false;
	
	for (directory_iterator it(Path); it != directory_iterator(); ++it) {
		auto item = it->path();
		FileType type;

		String fullPath = Path;
		fullPath += item.filename().string();
		String SubPath = BaseSubPath;
		SubPath += item.filename().string();

		if (boost::filesystem::is_regular_file(item))
			type = FileType::File;
		else
			type = FileType::Directory;

		func(fullPath, SubPath, type);
	}

	return true;
}

#if 0

FileWritter FolderContainer::GetFileWritter(const string& file)
{
	string s = FullPath(file);
	boost::filesystem::path p(s);
	boost::filesystem::create_directories(p.parent_path());
	auto ptr = std::make_shared<FolderFileWritter>(const_cast<FolderContainer*>(this), s);
	return ptr;
}

FileReader FolderContainer::GetFileReader(const string& file) const
{
	string s = FullPath(file);
	auto ptr = std::make_shared<FolderFileReader>(const_cast<FolderContainer*>(this));

	try {
		if (!ptr->Open(s))
			throw false;
		return ptr;
	}
	catch (...) {
		AddLogf(Error, "Unable to open file '%s'", s.c_str());
	}
	return nullptr;
}

FileReader FolderContainer::GetFileReader(const RawFilePointer *file) const
{
	const FolderContainerPointer *ptr = dynamic_cast<const FolderContainerPointer*>(file);
	if (!ptr) {
		AddLog(Error, "Invalid file pointer!");
		return FileReader();
	}
	return GetFileReader(ptr->SubPath);
}

FileWritter FolderContainer::GetFileWritter(const RawFilePointer *file)
{
	const FolderContainerPointer *ptr = dynamic_cast<const FolderContainerPointer*>(file);
	if (!ptr) {
		AddLog(Error, "Invalid file pointer!");
		return FileWritter();
	}
	return GetFileWritter(ptr->SubPath);
}

//-------------------------------------------------------------------------------------------------

bool FolderContainer::FileExists(const string& file) const
{
	string s = FullPath(file);
	return boost::filesystem::exists(s);
}

bool FolderContainer::EnumerateFolder(const RawFilePointer *root, FolderEnumerateFunc func) const
{
	if (!func) return false;
	const FolderContainerPointer *ptr = dynamic_cast<const FolderContainerPointer*>(root);
	if (!ptr && root) {
		AddLog(Error, "Invalid file pointer!");
		return false;
	}

	string s;
	if (ptr)
		s = FullPath(ptr->SubPath);
	else
		s = m_Path;

	using boost::filesystem::directory_iterator;
	boost::filesystem::path p(s);
	if (!boost::filesystem::exists(p))
		return false;
	directory_iterator it(p);
	for (; it != directory_iterator(); ++it) {
		auto item = it->path();
		FileType type;
		if (boost::filesystem::is_regular_file(item))
			type = FileType::File;
		else
			type = FileType::Directory;

		string fn = item.filename().string();
		string hash = item.string();
		std::transform(hash.begin(), hash.end(), hash.begin(), [](int c)->int { if (c == '\\') return '/'; return c; });

		FolderContainerPointer *fptr;
		FolderContainerPointer **fptrptr = &m_PtrMap[hash];
		if (!*fptrptr) {
			m_PtrList.emplace_back(std::make_unique<FolderContainerPointer>());
			fptr = m_PtrList.back().get();
			*fptrptr = fptr;
			fptr->SubPath = string(hash.c_str() + m_Path.length());
			if (type == FileType::File)
				fptr->Size = (FileSize)boost::filesystem::file_size(hash);
		} else
			fptr = *fptrptr;

		func(fn, type, fptr);
	}
	return true;
}

const string& FolderContainer::GetFileName() const
{
	return m_Path;
}

#endif

} //namespace StarVFS 
#endif
