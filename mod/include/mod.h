#pragma once
#include "evt_cmd.h"
#include <spm/effdrv.h>
namespace mod {

#define MOD_VERSION "SPM-Multi"

extern bool gIsDolphin;

void main();

#define MAX_CHECK_LIST 2

#define EFFDATA_HEAPID 4

#define MAX_EFFS 100
#define MAX_MEM_ALLOC 10

struct TrackedMemAlloc {
  void* ptr;
  int id;
  u32 size;
  void* caller;
};

extern TrackedMemAlloc pendingAllocs[MAX_MEM_ALLOC];
extern u32 pendingAllocCount;

struct TrackedEff {
    char* cmdName; //set manually by cmd injection
    char* name;
    spm::effdrv::EffEntry* entry;
    void* userWork;
    u32 userWorkSize;
    void* mainFunc;

    TrackedMemAlloc memAllocs[MAX_MEM_ALLOC];
    u32 memAllocCount;
  };

extern TrackedEff tracked[MAX_EFFS];
extern u32 trackedCount;

typedef struct ItemCheckList
{
  s32 gswfIndex = 0;
  s32 itemid = 0;
};

}
