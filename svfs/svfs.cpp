/*
  * Generated by cppsrc.sh
  * On 
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include "svfs.h"
#include "luainterface.h"
#include "svfslua.h"

#include <SVFSRegister.h>

SVFS::SVFS(SharedLua Lua) {
	m_Lua = Lua;
}

SVFS::~SVFS() {
}

//-------------------------------------------------------------------------------------------------

bool SVFS::Initialize() {
	if (!m_Lua) {
		return false;
	}

	svfslua::Install(m_Lua->GetState());

	luabridge::getGlobalNamespace(m_Lua->GetState())
		.beginNamespace("api")
			.beginClass<::StarVFS::StarVFS>("StarVFS")
				.addFunction("GetRegister", &::StarVFS::StarVFS::GetRegister)

				.addFunction("GetModule", &SVFS::GetModule)
				.addFunction("GetModuleCount", &SVFS::GetModuleCount)
			.endClass()
			.deriveClass<SVFS, ::StarVFS::StarVFS >("SVFS")
				.addFunction("DumpStructure", &SVFS::CoutDumpStructure)
				.addFunction("DumpFileTable", &SVFS::CoutDumpFileTable)
				.addFunction("DumpHashTable", &SVFS::CoutDumpHashTable)

				.addFunction("ForcePath", &SVFS::RawForcePath)

				.addFunction("OpenContainer", &SVFS::RawOpenContainer)
				.addFunction("OpenFile", &SVFS::RawOpenFile)
				.addFunction("FindFile", &SVFS::RawFindFile)

				.addFunction("GetFullFilePath", &SVFS::GetFullFilePath)
				.addFunction("GetFileName", &SVFS::RawGetFileName)
				.addFunction("GetFileSize", (int(SVFS::*)(int))&SVFS::GetFileSize)
				.addFunction("DeleteFile", (int(SVFS::*)(int))&SVFS::DeleteFile)
				.addFunction("IsFileValid", (int(SVFS::*)(int))&SVFS::IsFileValid)
				.addFunction("IsFileDirectory", &SVFS::RawIsFileDirectory)
	//FileID FindFile(const String& FileName);

			.endClass() 
			 
		.endNamespace()
		;

	luabridge::getGlobalNamespace(m_Lua->GetState())
		.beginNamespace("inst")
			.addPtrVariable<SVFS>("svfs", this)
		.endNamespace()
		;

	return true;
}

//-------------------------------------------------------------------------------------------------
