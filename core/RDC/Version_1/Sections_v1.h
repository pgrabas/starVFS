/*
  * Generated by cppsrc.sh
  * On 2016-01-02 11:00:05,98
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef Sections_v1_H
#define Sections_v1_H

#include <sstream>

namespace StarVFS {
namespace RDC {
namespace Version_1 {
namespace Sections {

struct SectionFileBuilderInterface {
	virtual ~SectionFileBuilderInterface() {} ;
	virtual bool WriteBlockAtEnd(const char *data, Size size, DataBlock &blockdesc) = 0;
	virtual bool SubBlockWriteAtEnd(const char *data, Size size, DataBlock &blockdesc, DataBlock &base) = 0;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#define DefineSectionDependency(Name, Type) \
public:	\
	void Set ## Name(Type* sect) { m_ ## Name = sect; }	\
	Type *Get ## Name() const { return m_ ## Name; } \
	SectionIndex Get ## Name ## Index() const { return m_ ## Name ? m_ ## Name->GetIndex( ): 0; } \
private: \
	Type *m_ ## Name = 0;									

struct BaseSection {
	BaseSection(SectionFileBuilderInterface *sfbi, SectionIndex index, SectionType Type) : m_SFBI(sfbi), m_Index(index), m_Type(Type) {}
	virtual ~BaseSection() {}

	SectionIndex GetIndex() const { return m_Index; }
	SectionType GetType() const { return m_Type; }

	const DataBlock& GetSectionDataBlock() const { return m_SectionDataBlock; }

	virtual bool WriteSection()  /* = 0 */ { return true; }

	SectionFileBuilderInterface *GetBuilderInterface() { return m_SFBI; }
private:
	SectionIndex m_Index;
	SectionType m_Type;
	SectionFileBuilderInterface *m_SFBI;
protected:
	DataBlock m_SectionDataBlock;
};

//-----------------------------------------------------------------------------

struct StringTable : public BaseSection {
	StringTable(SectionFileBuilderInterface *sfbi, SectionIndex index) : BaseSection(sfbi, index, SectionType::StringTable) {
		m_data << '\0';
	}

	u32 AllocString(const CString cstr) {
		if (!cstr || !*cstr)
			return 0;
		auto pos = m_data.tellp();
		m_data << cstr << '\0';
		return static_cast<u32>(pos);
	}

	virtual bool WriteSection() override {
		auto str = m_data.str();
		while ((str.length() % 8) != 0)
			str += '\0';
		return GetBuilderInterface()->WriteBlockAtEnd(str.c_str(), str.length(), m_SectionDataBlock);
	}
private:
	std::stringstream m_data;
};

//-----------------------------------------------------------------------------

struct RawDataSection : public BaseSection {
	RawDataSection(SectionFileBuilderInterface *sfbi, SectionIndex index) : BaseSection(sfbi, index, SectionType::RawData) {
		GetBuilderInterface()->WriteBlockAtEnd(nullptr, 0, m_SectionDataBlock);
	}

	virtual bool WriteSection() override {
		return true;
		//auto str = m_data.str();
		//while ((str.length() % 8) != 0)
		//	str += '\0';
		//return GetBuilderInterface()->WriteBlockAtEnd(str.c_str(), str.length(), m_SectionDataBlock);
	}
private:
};

//-----------------------------------------------------------------------------

struct OffsetDataBlockTable : public BaseSection {
	OffsetDataBlockTable(SectionFileBuilderInterface *sfbi, SectionIndex index) : BaseSection(sfbi, index, SectionType::OffsetDataBlockTable) {}

	std::vector<OffsetDataBlock>& GetTable() { return m_Table; }

	virtual bool WriteSection() override {
		if (m_Table.empty()) {
			m_SectionDataBlock.Zero();
			return true;
		}
		return GetBuilderInterface()->WriteBlockAtEnd((char*)&m_Table[0], m_Table.size() * sizeof(m_Table[0]), m_SectionDataBlock);
	}
private:
	std::vector<OffsetDataBlock> m_Table;
};

//-----------------------------------------------------------------------------

struct FileStructureTable : public BaseSection {
	FileStructureTable(SectionFileBuilderInterface *sfbi, SectionIndex index) : BaseSection(sfbi, index, SectionType::FileStructureTable) {}

	std::vector<BaseFileInfo>& GetTable() { return m_Table; }

	virtual bool WriteSection() override {
		if (m_Table.empty()) {
			m_SectionDataBlock.Zero();
			return true;
		}
		return GetBuilderInterface()->WriteBlockAtEnd((char*)&m_Table[0], m_Table.size() * sizeof(m_Table[0]), m_SectionDataBlock);
	}
private:
	std::vector<BaseFileInfo> m_Table;
};

//-----------------------------------------------------------------------------

struct HashTableSection : public BaseSection {
	HashTableSection(SectionFileBuilderInterface *sfbi, SectionIndex index) : BaseSection(sfbi, index, SectionType::HashTable) {}

	std::vector<HashSectionItemType>& GetTable() { return m_Table; }

	virtual bool WriteSection() override {
		if (m_Table.empty()) {
			m_SectionDataBlock.Zero();
			return true;
		}
		bool odd = (m_Table.size() & 1) != 0;
		if (odd)
			m_Table.push_back(0);
		bool ret = GetBuilderInterface()->WriteBlockAtEnd((char*)&m_Table[0], m_Table.size() * sizeof(m_Table[0]), m_SectionDataBlock);
		if (odd)
			m_Table.pop_back();
		return ret;
	}
private:
	std::vector<HashSectionItemType> m_Table;
};

//-----------------------------------------------------------------------------

struct MountEntrySection : public BaseSection {
	DefineSectionDependency(StringTable, StringTable);
	DefineSectionDependency(RawDataSection, RawDataSection);
	DefineSectionDependency(OffsetDataBlockTable, OffsetDataBlockTable);
	DefineSectionDependency(FileStructureTable, FileStructureTable);
	DefineSectionDependency(HashTableSection, HashTableSection);
public:
	MountEntrySection(SectionFileBuilderInterface *sfbi, SectionIndex index) : BaseSection(sfbi, index, SectionType::MountEntry) {}

	u16 m_MountEntryId = 0;

	virtual bool WriteSection() override {
		Headers::MountEntrySection data;
		data.StringTable = GetStringTableIndex();
		data.RawDataSection = GetRawDataSectionIndex();
		data.DataBlockTable = GetOffsetDataBlockTableIndex();
		data.StructureSection = GetFileStructureTableIndex();
		data.HashTable = GetHashTableSectionIndex();
		data.MountEntryId = m_MountEntryId;
		return GetBuilderInterface()->WriteBlockAtEnd((char*)&data, sizeof(data), m_SectionDataBlock);
	}
private:
};

//-----------------------------------------------------------------------------

} //namespace Sections
} //namespace Version_1
} //namespace RDC 
} //namespace StarVFS 

#endif
