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

	virtual std::unique_ptr<AttributeMapInstance> GetAttributeMapInstance() const {
		auto atm = CreateAttributeMapInstance<RDCExporter>();
		//atm->AddAttrib("Namespace", &InternalExporter::GetNamespace, &InternalExporter::SetNamespace);
		//atm->AddAttrib("Name", &InternalExporter::GetName, &InternalExporter::SetName);
		return std::unique_ptr<AttributeMapInstance>(atm.release());
	}

private: 
	virtual ExportResult WriteLocalFile(const String &LocalFileName) override;
	struct Impl;
};

} //namespace Exporters 
} //namespace StarVFS 

#endif
