/*
  * Generated by cppsrc.sh
  * On 2015-12-14  7:26:50,12
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef HashFileTable_H
#define HashFileTable_H

namespace StarVFS {

class HashFileTable {
public:
 	HashFileTable();
 	~HashFileTable();
	//bool Resize(FileID NewCapacity);
	//FileID Lookup(FilePathHash Hash);
	//void SortHashTable();
	//void AddToHashTable(File* f);
private:
	//FilePathHash *m_HashTable;   //these tables must be synchronized
	//FileID *m_FileIDTable;	   //these tables must be synchronized
	//FileID m_Allocated;
	//FileID m_Capacity;
	//std::unique_ptr<char[]> m_Memory;
};

} //namespace StarVFS 

#endif

/*

	extract hashfiletable
	add container mountpoints
	add filetable interface for containers

*/
