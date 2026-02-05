/*
  Read entire file into memory
  Returns true on success, false on failure
  Caller is responsible for freeing the memory allocated for Dest using VirtualFree
*/
bool os_ReadFile(const char *Filename, void **Dest, unsigned int *FileSize);


bool os_FreeMemory(void *MemoryPtr);

void os_PrintLog(const char *Message);