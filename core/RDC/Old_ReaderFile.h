/*
  * Generated by cppsrc.sh
  * On 2015-03-13 23:11:22.87
  * by Immethis
*/
/*--END OF HEADER BLOCK--*/
#pragma once

namespace MoonGlare {
namespace FileSystem {
namespace MoonGlareContainer {

struct ReaderFile : FileNode {
	std::list<ReaderFile*> Children;
};

struct ReaderLoadMetaStruct {
	Headers::CurrentVersion::SectionHeader *Sections;
	Headers::CurrentVersion::FileTableSection *FileTable;
	std::unique_ptr<Headers::Version_0::StringTableSection> StringTable;

	Headers::CurrentVersion::SectionIndex RawDataIndex = 0;
	Headers::CurrentVersion::SectionIndex StringTableIndex = 0;
	Headers::CurrentVersion::SectionIndex FileTableIndex = 0;

	Headers::CurrentVersion::FileFooter Footer;
	Headers::FileHeader header;

	Headers::CurrentVersion::SectionHeader& GetRawDataSection() { return Sections[RawDataIndex]; }
	Headers::CurrentVersion::SectionHeader& GetStringTableSection() { return Sections[StringTableIndex]; }
	Headers::CurrentVersion::SectionHeader& GetFileTableSection() { return Sections[FileTableIndex]; }

	UniqueByteTable SectionsData;
	UniqueByteTable FileTableData;
};


} //namespace MoonGlareContainer 
} //namespace FileSystem 
} //namespace MoonGlare 
