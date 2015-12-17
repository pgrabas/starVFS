/*
  * Generated by cppsrc.sh
  * On 2015-03-13 23:11:53.79
  * by Immethis
*/
/*--END OF HEADER BLOCK--*/

#pragma once

namespace MoonGlare {
namespace FileSystem {
namespace MoonGlareContainer {

struct WritterFileNode;
struct WritterContainerMeta;

struct WritterConfiguration {
	Headers::Version Version = Headers::CurrentVersion::Version::Value;
	Headers::Compression Compression;
	Headers::Encryption Encryption;

	bool BufferingEnabled = true;
	Headers::CurrentVersion::Size BufferSizeLimit = 10 * 1024 * 1024;
};


class Writter : public iContainer {
	GABI_DECLARE_STATIC_CLASS(Writter, iContainer)
public:
	Writter(const string &File, const WritterConfiguration *configuration = nullptr);
	virtual ~Writter();

	virtual FileReader GetFileReader(const string& file) const override;
	virtual FileWritter GetFileWritter(const string& file) override;
	virtual bool FileExists(const string& file) const override;

	bool StoreFile(WritterFileNode *file, bool CanBeBuffered = true);

	Headers::CurrentVersion::Size GetBufferedCount();
private: 
	WritterConfiguration m_Configuration;
	string m_ContainerFile;

	std::mutex m_mutex;

	std::list<std::unique_ptr<WritterFileNode>> m_FileList;
	std::list<WritterFileNode*> m_BufferedFiles;
	std::unique_ptr<WritterFileNode> m_RootFile;
	std::unique_ptr<WritterContainerMeta> m_ContainerMeta;
	CFile m_File;

	bool CreateContainer();
	bool CloseContainer();

	WritterFileNode* FindOrAlloc(const string& FileName);

};

} //namespace MoonGlareContainer 
} //namespace FileSystem 
} //namespace MoonGlare 
