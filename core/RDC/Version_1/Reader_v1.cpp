/*
  * Generated by cppsrc.sh
  * On 2016-01-03 22:43:01,77
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#include "../nRDC.h"
#include "../../Utils/ZlibCompression.h"

namespace StarVFS {
namespace RDC {
namespace Version_1 {

Reader_v1::Reader_v1() {
}

Reader_v1::~Reader_v1() {
}

//-----------------------------------------------------------------------------

bool Reader_v1::Open(const String& FileName) {
	if (GetDevice())
		return false;

	FileFooter footer;

	{
		auto device = std::make_unique<BlockFileDevice>();
		if (!device->OpenForRead(FileName))
			return false;

		if (!TestHeaderIntegrity(device, &m_Header))
			return false;

		if (m_Header.Version.Major != 1)
			return false;

		if (!TestFooterIntegrity(device, &footer))
			return false;
		SetDevice(std::move(device));
	}

	if (!ReadBlock(m_Sections, footer.SectionTableBlock)) {
		m_Sections.reset();
		SetDevice(nullptr);
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------

template<class TableType>
bool Reader_v1::ReadBlock(TableType &out, const DataBlock &blockdesc) const {
	out.reset();
	ByteTable bt;

	if (!m_BlockProcessor.ReadBlock(GetDevice(), bt, blockdesc)) {
		STARVFSErrorLog("Failed to read block from container!");
		return false;
	}

	out.assign_from(bt);
	return true;
}

bool Reader_v1::ReadBlock(ByteTable &out, const DataBlock &blockdesc) const {
	return ReadBlock<ByteTable>(out, blockdesc);
}

bool Reader_v1::OffsetReadBlock(ByteTable &out, const OffsetDataBlock &offsetblockdesc, const DataBlock &blockdesc) const {
	out.reset();
	ByteTable bt;

	if (!m_BlockProcessor.ReadBlock(GetDevice(), bt, offsetblockdesc, blockdesc)) {
		STARVFSErrorLog("Failed to read block from container!");
		return false;
	}

	out.swap(bt);
	return true;
}

//-----------------------------------------------------------------------------

template<SectionType Type, class TableType>
bool Reader_v1::ReadSectionBlock(SectionIndex Index, TableType &out) const {
	out.reset();

	if (m_Sections.size() <= (size_t)Index) {
		STARVFSErrorLog("Invalid section index! (index: %d, tabletype: %s)", Index, typeid(out).name());
		return false;
	}
	auto &section = m_Sections[Index];
	if (section.Type != Type) {
		STARVFSErrorLog("Invalid section type! want:%d got:%d (index: %d, tabletype: %s)", Type, section.Type, Index, typeid(out).name());
		return false;
	}

	if (!ReadBlock(out, section.SectionBlock)) {
		STARVFSErrorLog("Failed to read block (index: %d, tabletype: %s)", Index, typeid(out).name());
		return false;
	}

	return true;
}

bool Reader_v1::LoadHashTable(SectionIndex Index, HashTable &out) const {
	return ReadSectionBlock<SectionType::HashTable>(Index, out);
}

bool Reader_v1::LoadOffsetDataBlockTable(SectionIndex Index, OffsetDataBlockTable &out) const {
	return ReadSectionBlock<SectionType::OffsetDataBlockTable>(Index, out);
}

bool Reader_v1::LoadFileStructureTable(SectionIndex Index, FileStructureTable &out) const {
	return ReadSectionBlock<SectionType::FileStructureTable>(Index, out);
}

bool Reader_v1::LoadStringTable(SectionIndex Index, StringTable &out) const {
	return ReadSectionBlock<SectionType::StringTable>(Index, out);
}

bool Reader_v1::FindMountEntries(std::vector<MountEntryInfo> &out) const {
	out.clear();
	for (size_t i = 0, j = m_Sections.size(); i < j; ++i) {
		auto &it = m_Sections[i];
		if (it.Type != SectionType::MountEntry)
			continue;

		out.emplace_back();
		auto &item = out.back();
		ByteTable bt;
		if (!ReadBlock(bt, it.SectionBlock)) {
			out.pop_back();
			//todo: not ignore error?
			continue;
		}

		item.m_MountEntry = *((MountEntrySection*)(bt.get()));

		item.m_SectionIndex = static_cast<SectionIndex>(i);
	}

	return true;
}

//-----------------------------------------------------------------------------

bool Reader_v1::TestFooterIntegrity(UniqueBlockFileDevice &device, FileFooter *footer) {
	FileFooter localfooter;
	if (!footer)
		footer = &localfooter;

	if (!device->ReadFromEnd(sizeof(*footer), (char*)footer, sizeof(*footer)))
		return false;

	if (footer->Signature != Signature::Footer) {
		STARVFSDebugInfoLog("Invalid footer signature!");
		return false;
	}

	return true;
}

} //namespace Version_1 
} //namespace RDC 
} //namespace StarVFS 
