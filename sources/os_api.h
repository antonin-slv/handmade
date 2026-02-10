#ifndef OS_API_H
#define OS_API_H

/*
  Read entire file into transient memory (will be freed at the end of the frame).
  Returns true on success, false on failure
*/
bool os_ReadFile(const char *Filename, void **Dest, unsigned int *FileSize);

void os_PrintLog(const char *Message);

/* This objects represents the memory of the game.
* Memory is divided in 2 categories : permanent storage and transient storage.

- Permanent storage is meant to store data that should be kept during the whole execution of the game,
  such as loaded assets, game state, etc. It is allocated once at the start of the game and should not
  be freed until the game exits.
- transient storage is meant to store temporary data that is only needed during a single frame,
  such as temporary buffers, etc. It is allocated once at the start of each frame and should be freed at the end of the frame.
*/

struct ScratchArena
{
  uint8_t *base;
  size_t capacity;
  size_t used;
};


/*
  returns a pointer to a block of memory of the given size allocated from the arena.
  The memory is not initialized, so it may contain garbage data.
*/
void *PushSize(ScratchArena *arena, size_t size);
/*
  this should only be done once at the start of the game, to initialize the arenas with the appropriate sizes and base pointers.
  the permanent storage should be big enough to hold all the data that needs to be kept during the whole execution of the game.
  The transient storage should be big enough to hold all the temporary data that is needed during a single frame.
*/
void InitArena(ScratchArena *arena, uint8_t *base, size_t capacity);

struct HandmadeMemory
{
  bool IsInitialized;

  size_t TotalSize;
  void* BasePointer;

  ScratchArena Permanent;
  ScratchArena Transient;
  ScratchArena Backbuffer;
};

static HandmadeMemory GlobalMemory;


#endif // OS_API_H