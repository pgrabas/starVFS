/*
  * Generated by cppsrc.sh
  * On 2016-01-01 18:40:09,38
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include "../nRDC.h"

#include "../../Utils/ZlibCompression.h"

namespace StarVFS {
namespace RDC {
namespace Version_1 {

struct Builder_v1::PrivateData : public Sections::SectionFileBuilderInterface {
//	std::vector<SectionDescriptor> m_Sections;

	virtual BlockProcessingResult WriteBlockAtEnd(const ByteTable &in, DataBlock &blockdesc) override {
		ByteTable cin;
		cin.make_copy(in);
		return m_BlockProcessor.WriteBlock(m_Owner->GetDevice(), std::move(cin), blockdesc);
	}

	virtual BlockProcessingResult OffsetBlockWriteAtEnd(const ByteTable &in, OffsetDataBlock &blockdesc, DataBlock &base) override {
		ByteTable cin;
		cin.make_copy(in);
		return m_BlockProcessor.WriteBlock(m_Owner->GetDevice(), std::move(cin), blockdesc, base);
	}

	virtual BlockProcessingResult WriteBlockAtEndOwning(ByteTable in, DataBlock &blockdesc) override {
		return m_BlockProcessor.WriteBlock(m_Owner->GetDevice(), std::move(in), blockdesc);
	}
	virtual BlockProcessingResult OffsetBlockWriteAtEndOwning(ByteTable in, OffsetDataBlock &blockdesc, DataBlock &base) override {
		return m_BlockProcessor.WriteBlock(m_Owner->GetDevice(), std::move(in), blockdesc, base);
	}

	PrivateData(Builder_v1 *Owner) : m_Owner(Owner) { }

	std::vector<std::unique_ptr<Sections::BaseSection>> m_Sections;

	template <class T, class ... ARGS>
	T* CreateSection(ARGS ...args) {
		static_assert(std::is_base_of<Sections::BaseSection, T>::value, "invalid base");
		SectionIndex idx = static_cast<SectionIndex>(m_Sections.size() + 1);
		auto ptr = std::make_unique<T>(this, idx, std::forward<ARGS>(args)...);
		auto rawptr = ptr.get();
		m_Sections.emplace_back(std::move(ptr));
		return rawptr;
	}

	bool WriteSectionTable(DataBlock &SectionTableBlock) {
		unique_table<SectionDescriptor> RawSections;
		RawSections.make_new(m_Sections.size() + 1);
		//RawSections.memset(0);

		for (size_t i = 0, j = m_Sections.size(); i < j; ++i) {
			auto &it = m_Sections[i];
			auto &out = RawSections[i + 1];

			out.SectionBlock = it->GetSectionDataBlock();
			out.Type = it->GetType();
		}

		ByteTable bt;
		bt.assign_from(RawSections);
		if (!WriteBlockAtEndOwning(std::move(bt), SectionTableBlock)) {
			STARVFSErrorLog("Unable to write section table!");
			return false;
		}
		return true;
	}
private:
	Builder_v1 *m_Owner;
	BlockProcessor m_BlockProcessor;
};

Builder_v1::Builder_v1() {
	Reset();
}

Builder_v1::~Builder_v1() {
}

void Builder_v1::Reset() {
	m_Data = std::make_unique<PrivateData>(this);
	m_RawDataSectionCrated = false;
}

//-----------------------------------------------------------------------------

bool Builder_v1::WriteFileFooter() {
	FileFooter footer;

	if (!m_Data->WriteSectionTable(footer.SectionTableBlock)) {
		STARVFSErrorLog("An error has occur during writting of section table!");
		return false;
	}

	footer.SectionCount = static_cast<SectionIndex>(m_Data->m_Sections.size());
	return GetDevice()->WriteAtEnd((char*)&footer, sizeof(footer));
}

bool Builder_v1::WriteSections() {
	for (auto &it : m_Data->m_Sections) {
		auto ret = it->WriteSection();
		if (!ret) {
			STARVFSErrorLog("WriteSection failed!");
			return false;
		}

		if (ret.m_Compression == Compression::CompressionResult::UnableToReduceSize) {
			STARVFSDebugLog("Section %d (Type: %02x, class: %s) not compressed. Reason: Unable to reduce size",
						(int)it->GetIndex(), (int)it->GetType(), typeid(*it.get()).name());
		}
	}
	return true;
}

//-----------------------------------------------------------------------------

Sections::StringTable* Builder_v1::CreateStringTableSection() {
	return m_Data->CreateSection<Sections::StringTable>();
}

Sections::MountEntrySection* Builder_v1::CreateMountEntrySection() {
	return m_Data->CreateSection<Sections::MountEntrySection>();
}

Sections::OffsetDataBlockTable* Builder_v1::CreateOffsetDataBlockTable() {
	return m_Data->CreateSection<Sections::OffsetDataBlockTable>();
}

Sections::RawDataSection* Builder_v1::CreateRawDataSection() {
	if (m_RawDataSectionCrated) {
		//todo: log
		return nullptr;
	}
	m_RawDataSectionCrated = true;
	return m_Data->CreateSection<Sections::RawDataSection>();
}

Sections::FileStructureTable* Builder_v1::CreateFileStructureTable() {
	return m_Data->CreateSection<Sections::FileStructureTable>();
}

Sections::HashTableSection* Builder_v1::CreateHashTable() {
	return m_Data->CreateSection<Sections::HashTableSection>();
}

//-----------------------------------------------------------------------------

void Builder_v1::ForEachSection(SectionEnumerateFunc func) {
	for (auto &it : m_Data->m_Sections) {
		if(it->ValidSection())
			func(it.get());
	}
}

} //namespace Version_1
} //namespace RDC 
} //namespace StarVFS 
