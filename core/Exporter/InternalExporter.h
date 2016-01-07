/*
  * Generated by cppsrc.sh
  * On 2015-12-17 22:12:37,21
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef InternalExporter_H
#define InternalExporter_H

namespace StarVFS {
namespace Exporters {

/*

REHASHING OF PATH HAS TO BE DONE

*/

class InternalExporter : public iExporter {
public:
 	InternalExporter(StarVFS *svfs);
 	virtual ~InternalExporter();

	void SetNamespace(const CString n) { m_Namespace = n; }
	void SetNamespace(const String &n) { m_Namespace = n; }
	const String& GetNamespace() const { return m_Namespace; }

	void SetName(const CString n) { m_Name = n; }
	void SetName(const String &n) { m_Name = n; }
	const String& GetName() const { return m_Name; }

	virtual std::unique_ptr<AttributeMapInstance> GetAttributeMapInstance() const {
		auto atm = CreateAttributeMapInstance<InternalExporter>();
		atm->AddAttrib("Namespace", &InternalExporter::GetNamespace, &InternalExporter::SetNamespace);
		atm->AddAttrib("Name", &InternalExporter::GetName, &InternalExporter::SetName);
		return std::unique_ptr<AttributeMapInstance>(atm.release());
	}
protected:
	virtual ExportResult WriteLocalFile(const String &LocalFileName) override;
private:
	struct ExporterImpl;
	String m_Namespace;
	String m_Name;
};

} //namespace Exporters 
} //namespace StarVFS 

#endif
