/*
  * Generated by cppsrc.sh
  * On 2015-12-17 21:59:15,19
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef RDCExporter_H
#define RDCExporter_H

namespace StarVFS {
namespace Exporters {

class RDCExporter : public iExporter {
public:
 	RDCExporter(StarVFS *svfs);
 	virtual ~RDCExporter();

	virtual ExportResult DoExport(const String &VFSBase, const String &LocalFileName) const override {
		return ExportResult::FatalError;
	}

protected:
private: 
};

} //namespace Exporters 
} //namespace StarVFS 

#endif