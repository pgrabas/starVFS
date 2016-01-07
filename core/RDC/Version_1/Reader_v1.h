/*
  * Generated by cppsrc.sh
  * On 2016-01-03 22:43:01,77
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef Reader_v1_H
#define Reader_v1_H

namespace StarVFS {
namespace RDC {
namespace Version_1 {

struct MountEntryInfo {
	MountEntrySection m_MountEntry;
	SectionIndex m_SectionIndex;
};

using StringTable = unique_table<char>;
using FileStructureTable = unique_table<BaseFileInfo>;
using OffsetDataBlockTable = unique_table<OffsetDataBlock>;
using HashTable = unique_table<HashSectionItemType>;

class Reader_v1 : public Reader {
public:
 	Reader_v1();
 	virtual ~Reader_v1();

	bool Open(const String& FileName);

	bool ReadBlock(void *data, Size size, const DataBlock &blockdesc) const;
	bool ReadBlock(CharTable &out, Size &out_size, const DataBlock &blockdesc) const;

	bool FindMountEntries(std::vector<MountEntryInfo> &out) const;
	bool LoadStringTable(SectionIndex Index, StringTable &out) const;
	bool LoadFileStructureTable(SectionIndex Index, FileStructureTable &out) const;
	bool LoadOffsetDataBlockTable(SectionIndex Index, OffsetDataBlockTable &out) const;
	bool LoadHashTable(SectionIndex Index, HashTable &out) const;

	static bool TestFooterIntegrity(UniqueBlockFileDevice &device, FileFooter *footer = nullptr);
protected:
private:
	std::vector<SectionDescriptor> m_Sections;

	template<SectionType Type, class TableType>
	bool BaseReadTableBlock(SectionIndex Index, TableType &out) const;
};

} //namespace Version_1 
} //namespace RDC 
} //namespace StarVFS 

#endif
