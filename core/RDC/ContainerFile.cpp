/*
  * Generated by cppsrc.sh
  * On 2015-03-15  7:53:40.63
  * by Immethis
*/
/*--END OF HEADER BLOCK--*/
//#include <pch.h>
//#include <nfMoonGlare.h>
//#include "../InternalFileSystem.h"
//#include "nMoonGlareContainer.h"

namespace MoonGlare {
namespace FileSystem {
namespace MoonGlareContainer {
#if 0

CFile::CFile() { 
}

CFile::~CFile() { 
	close(); 
}

bool CFile::open(const char* name) {
	m_file.reset(new std::fstream());
	//m_file->exceptions(std::ios::failbit | std::ios::badbit);
	m_file->open(name, std::ios::out | std::ios::in | std::ios::binary);
	return static_cast<bool>(*this);
}

void CFile::close() {
	m_file.reset();
}

void CFile::seek_read_back(size_t offset) { 
	m_file->seekg(-((long)offset), std::ios::end); 
}

bool CFile::WriteBlock(UniqueByteTable &data, Headers::u32 dataSize, CurrentVersion::DataBlock &block) {
	block.ContainerSize = block.RealSize = dataSize;
	block.FilePointer = WriteLocation();
	return write(data.get(), dataSize);
}

bool CFile::WriteBlock(const void *data, Headers::u32 dataSize, CurrentVersion::DataBlock &block) {
	block.ContainerSize = block.RealSize = dataSize;
	block.FilePointer = WriteLocation();
	return write((char*)data, dataSize);
}

bool CFile::ReadBlock(UniqueByteTable &data, const CurrentVersion::DataBlock &block) {
	data.reset(new char[block.RealSize + 1]);
	data[block.RealSize] = 0;
	seek_read(block.FilePointer);
	return read(data.get(), block.RealSize);
}

#endif
} //namespace MoonGlareContainer 
} //namespace FileSystem 
} //namespace MoonGlare 
