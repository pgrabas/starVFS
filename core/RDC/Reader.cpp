/*
  * Generated by cppsrc.sh
  * On 2016-01-03 22:42:45,89
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#include "nRDC.h"

namespace StarVFS {
namespace RDC {

Reader::Reader() {
}

Reader::~Reader() {
}

//-------------------------------------------------------------------------------------------------

bool Reader::CanOpenFile(const String& FileName) {
	auto device = std::make_unique<BlockFileDevice>();
	device->SetBlockAlignValue(Settings::BlockAlignmentValue);

	if (!device->OpenForRead(FileName))
		return false;

	FileHeader header;
	if (!TestHeaderIntegrity(device, &header))
		return false;

	switch (header.Version.Major) {
	case 1:
		return Version_1::Reader_v1::TestFooterIntegrity(device);
	default:
		return false;
	}
}

bool Reader::TestHeaderIntegrity(UniqueBlockFileDevice &device, FileHeader *header) {
	FileHeader localheader;
	if (!header)
		header = &localheader;

	if (!device->ReadFromBegining(0, (char*)header, sizeof(*header)))
		return false;

	if (header->FileSignature != Signature::Header) {
		STARVFSDebugInfoLog("Invalid header signature!");
		return false;
	}

	return true;
}

} //namespace RDC 
} //namespace StarVFS 
