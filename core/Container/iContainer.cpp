/*
  * Generated by cppsrc.sh
  * On 2015-12-10 18:06:02,09
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include "../StarVFSInternal.h"
#include "iContainer.h"

namespace StarVFS {
namespace Containers {

iContainer::iContainer(FileTableInterface  *fti): m_FTI(fti) {
}

iContainer::~iContainer() {
}

ConstCString iContainer::GetFileName(FileID ContainerFID) const  {
	return nullptr;
}

bool iContainer::EnumerateFiles(ContainerFileEnumFunc filterFunc) const {
	return false;
}

} //namespace Containers 
} //namespace StarVFS 
