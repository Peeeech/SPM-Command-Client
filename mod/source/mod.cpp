#include "mod.h"
#include "commandmanager.h"
#include "commands.h"
#include "evtpatch.h"
#include "patch.h"
#include "msgpatch.h"
#include "netmemoryaccess.h"
#include "network.h"

#include <spm/rel/aa1_01.h>
#include <spm/fontmgr.h>
#include <spm/seqdrv.h>
#include <spm/evt_item.h>
#include <spm/itemdrv.h>
#include <spm/evtmgr.h>
#include <spm/effdrv.h>
#include <spm/eff_sub.h>
#include <spm/memory.h>
#include <spm/seqdef.h>
#include <spm/item_data.h>
#include <spm/mario_pouch.h>
#include <spm/evt_pouch.h>
#include <spm/evt_seq.h>
#include <spm/filemgr.h>
#include <spm/map_data.h>
#include <spm/spmario.h>
#include <wii/os/OSError.h>
#include <msl/stdio.h>

namespace mod {

ItemCheckList itemCheckList[MAX_CHECK_LIST] = 
{
  {
    611, 0x0DF
  },
  {
    612, 0x0DE
  }
};

/*
    Title Screen Custom Text
    Prints "SPM Rel Loader" at the top of the title screen
*/

static spm::seqdef::SeqFunc *seq_titleMainReal;
static void seq_titleMainOverride(spm::seqdrv::SeqWork *wp)
{
    wii::gx::GXColor _colour {0, 255, 0, 255};
    f32 scale = 0.8f;
    char msg[128];
    u32 ip = Mynet_gethostip();
    msl::stdio::snprintf(msg, 128, "%d.%d.%d.%d\n", ip >> 24 & 0xff, ip >> 16 & 0xff, ip >> 8 & 0xff, ip & 0xff);
    spm::fontmgr::FontDrawStart();
    spm::fontmgr::FontDrawEdge();
    spm::fontmgr::FontDrawColor(&_colour);
    spm::fontmgr::FontDrawScale(scale);
    spm::fontmgr::FontDrawNoiseOff();
    spm::fontmgr::FontDrawRainbowColorOff();
    f32 x = -((spm::fontmgr::FontGetMessageWidth(msg) * scale) / 2);
    spm::fontmgr::FontDrawString(x, 200.0f, msg);
    seq_titleMainReal(wp);
}
static void titleScreenCustomTextPatch()
{
    seq_titleMainReal = spm::seqdef::seq_data[spm::seqdrv::SEQ_TITLE].main;
    spm::seqdef::seq_data[spm::seqdrv::SEQ_TITLE].main = &seq_titleMainOverride;
}

  bool ( * pouchAddItem)(s32 itemId);
  spm::itemdrv::ItemEntry * ( * itemEntry)(const char * name, s32 type, s32 behaviour, f32 x, f32 y, f32 z, spm::evtmgr::EvtScriptCode * pickupScript, spm::evtmgr::EvtVar switchNumber);

  bool new_pouchAddItem(s32 itemId)
  {
    if (itemId == 45)
    {
      return true;
    }
    return pouchAddItem(itemId);
  }

  bool itemAdded = false; 
  bool ranOnce = false;

  bool new_itemCollectPouchItem(spm::itemdrv::ItemEntry *item)
  {
    //wii::os::OSReport("GSWF: %d\n", item->switchNumber);
      if (item->switchNumber != 0x0)
        {
          bool ret = spm::itemdrv::itemCollectPouchItem(item);
          if (!itemAdded)
          {
            addToGswfStack(item);
            itemAdded = true;
          }
          if (ret)
          {
            itemAdded = false;
          }
          return ret;
        }
        else {
          if (!ranOnce)
          {
            ranOnce = true;
            wii::os::OSReport("Added item with switch %d to stack\n", item->switchNumber);
            char msg[128];
            msl::stdio::snprintf(msg, 128, "Collected item ID %d\n", item->type);

            NetMemoryAccess::enqueuePacket(0x1001, msg, msl::string::strlen(msg) + 1);

            wii::os::OSReport("Queued one test packet.\n");
          }
        }

        return spm::itemdrv::itemCollectPouchItem(item);
    }

spm::evtmgr::EvtVar convertGswfToIndex(spm::evtmgr::EvtVar switchNumber)
{
  spm::evtmgr::EvtVar gswf = abs(switchNumber);
  gswf -= (abs(GSWF(0)));
  gswf = abs(gswf);
  return gswf;
}

  // does basically nothing for now but will be useful later for swapping pixls/characters around
  spm::itemdrv::ItemEntry *new_itemEntry(const char * name, s32 type, s32 behaviour, f32 x, f32 y, f32 z, spm::evtmgr::EvtScriptCode * pickupScript, spm::evtmgr::EvtVar switchNumber)
  {
    if (switchNumber != 0x0)
    {
      for (u32 i = 0; i < MAX_CHECK_LIST; i++)
      {
        if (itemCheckList[i].gswfIndex == convertGswfToIndex(switchNumber))
        {
          type = itemCheckList[i].itemid;
          break;
        }
      }
      
      spm::itemdrv::ItemEntry * item = itemEntry(name, type, behaviour, x, y, z, pickupScript, switchNumber);
      if (!item)
      {
        return item;
      }
      return item;
    }
    return itemEntry(name, type, behaviour, x, y, z, pickupScript, switchNumber);
  }


  EVT_BEGIN(insertNop)
    SET(LW(0), LW(0))
  RETURN_FROM_CALL()
  
  EVT_BEGIN(gn4)
    SET(GSW(0), 215)
  RETURN_FROM_CALL()
  
  EVT_BEGIN(gn2)
    SET(GSW(0), 189)
  RETURN_FROM_CALL()
  
  EVT_BEGIN(sp2)
    SET(GSW(0), 142)
  RETURN_FROM_CALL()
  
  EVT_BEGIN(ta2)
    SET(GSW(0), 107)
  RETURN_FROM_CALL()
  
  EVT_BEGIN(ta4)
    SET(GSW(0), 120)
  RETURN_FROM_CALL()
  
  EVT_BEGIN(ls1)
    SET(GSW(0), 358)
  RETURN_FROM_CALL()

  EVT_BEGIN(he2_mi1)
    SET(GSW(0), 20)
  RETURN_FROM_CALL()
  
  EVT_BEGIN(he1)
    SET(GSW(0), 17)
  RETURN_FROM_CALL()

  EVT_BEGIN(mac_02)
    SET(GSW(0), 359)
  RETURN_FROM_CALL()

  // Dialogue to determine quickstart or no
  EVT_BEGIN(determine_quickstart)
  SET(GSW(0), 17)
  USER_FUNC(spm::evt_pouch::evt_pouch_add_item, 50)
  USER_FUNC(spm::evt_pouch::evt_pouch_add_item, 0x0D9)
  USER_FUNC(spm::evt_pouch::evt_pouch_add_item, 0x0DA)
  USER_FUNC(spm::evt_pouch::evt_pouch_add_item, 0x0DB)
  // USER_FUNC(spm::evt_pouch::evt_pouch_add_item, 0x0E0)
  // USER_FUNC(spm::evt_msg::evt_msg_print, 1, PTR(quickstartText), 0, 0)
  // USER_FUNC(spm::evt_msg::evt_msg_select, 1, PTR(quickstartOptions))
  // USER_FUNC(spm::evt_msg::evt_msg_continue)
  // SWITCH(LW(0))
  // END_SWITCH()
  USER_FUNC(spm::evt_seq::evt_seq_set_seq, spm::seqdrv::SEQ_MAPCHANGE, PTR("he1_01"), PTR("doa1_l"))
  RETURN()
  EVT_END()

  /*
      General mod functions
  */

  static void evt_patches()
  {
    spm::map_data::MapData * ls1_md = spm::map_data::mapDataPtr("ls1_01");
    spm::map_data::MapData * he1_md = spm::map_data::mapDataPtr("he1_01");
    spm::map_data::MapData * he2_md = spm::map_data::mapDataPtr("he2_07");
    spm::map_data::MapData * mi1_md = spm::map_data::mapDataPtr("mi1_07");
    spm::map_data::MapData * ta2_md = spm::map_data::mapDataPtr("ta2_04");
    spm::map_data::MapData * ta4_md = spm::map_data::mapDataPtr("ta4_12");
    spm::map_data::MapData * sp2_md = spm::map_data::mapDataPtr("sp2_01");
    spm::map_data::MapData * gn2_md = spm::map_data::mapDataPtr("gn2_02");
    spm::map_data::MapData * gn4_md = spm::map_data::mapDataPtr("gn4_03");
    spm::map_data::MapData * mac_02_md = spm::map_data::mapDataPtr("mac_02");

    evtpatch::hookEvtReplace(spm::aa1_01::aa1_01_mario_house_transition_evt, 10, determine_quickstart);
    evtpatch::hookEvtReplace(ls1_md->initScript, 1, ls1);
    evtpatch::hookEvtReplace(he1_md->initScript, 1, he1);
    evtpatch::hookEvtReplace(he2_md->initScript, 1, he2_mi1);
    evtpatch::hookEvtReplace(mi1_md->initScript, 1, he2_mi1);
    evtpatch::hookEvtReplace(ta2_md->initScript, 1, ta2);
    evtpatch::hookEvtReplace(ta4_md->initScript, 1, ta4);
    evtpatch::hookEvtReplace(sp2_md->initScript, 1, sp2);
    evtpatch::hookEvtReplace(gn2_md->initScript, 1, gn2);
    evtpatch::hookEvtReplace(gn4_md->initScript, 1, gn4);
    evtpatch::hookEvtReplace(mac_02_md->initScript, 1, mac_02);
}

//memory alloc hook for debug
void* (*__memAlloc)(int id, u32 size);
void* memAllocDebug(int id, u32 size)
{
  void* lr;
  asm("mflr %0" : "=r"(lr));
  
  int storedId = id;
  u32 storedSize = size;

  void* ptr = __memAlloc(id, size);

  if (id == EFFDATA_HEAPID){

    if (pendingAllocCount < MAX_MEM_ALLOC)
    {
      pendingAllocs[pendingAllocCount].ptr = ptr;
      pendingAllocs[pendingAllocCount].id = storedId;
      pendingAllocs[pendingAllocCount].size = storedSize;
      pendingAllocs[pendingAllocCount].caller = lr;
      pendingAllocCount++;
    }

    wii::os::OSReport("memAlloc called for EFFDATA with size: %d\nptr: %08X, caller: %08X\n", storedSize, ptr, lr);
  }
  return ptr;
}

//specific entry debug
spm::effdrv::EffEntry * (*effSpmVoltEntry)(double x, double y, spm::effdrv::EffEntry *param_3, int param_4);
spm::effdrv::EffEntry * effSpmVoltEntryDebug(double x, double y, spm::effdrv::EffEntry *param_3, int param_4)
{
  wii::os::OSReport("\neffSpmVoltEntryDebug called with params: x: %f, y: %f, param_3: %08X, param_4: %d\n", x, y, param_3, param_4);
  spm::effdrv::EffEntry* entry = effSpmVoltEntry(x, y, param_3, param_4);
  wii::os::OSReport("Returned entry: %08X\n", entry);
  return entry;
}



//pointer table for tracking effs
TrackedMemAlloc pendingAllocs[MAX_MEM_ALLOC];
u32 pendingAllocCount = 0;

TrackedEff tracked[MAX_EFFS];
u32 trackedCount = 0;

//used for debugging effEntry;
spm::effdrv::EffEntry * trackedEntry = nullptr;

spm::effdrv::EffEntry * (*effEntry)();
spm::effdrv::EffEntry * effEntryDebug()
{
  spm::effdrv::EffEntry* entry = effEntry();

  if(entry->instanceName && entry->instanceName[0] != '\0')
    {
      wii::os::OSReport("\n===================================");
      wii::os::OSReport("\neffEntryDebug called, entry: %08X\n", entry);
      wii::os::OSReport("entry->mainFunc: %08X\n", entry->mainFunc);
      wii::os::OSReport("entry->userWork: %08X\n", entry->userWork);
      wii::os::OSReport("entry->instanceName: %s\n", entry->instanceName);
      wii::os::OSReport("entry->flags: %04X\n", entry->flags);
      wii::os::OSReport("entry->type: %04X\n", entry->type);
      wii::os::OSReport("entry->releaseType: %d\n", entry->releaseType);
      wii::os::OSReport("===================================\n");
    }
    //NOTE: the above will likely never be called, as effEntry is moreso treated like a memory initializer rather than actually creating a fleshed out effect.
    //Full effects seem centralized in eff[effectname]Entry pre-existing functions that will natively take varying parameters.

  trackedEntry = entry;
  return entry;
}

//used for debugging effEntryType, presumably for simpler anims that didn't need full, complex entry structs and just needed a type to determine which effect to spawn
spm::effdrv::EffEntry * (*effEntryType)(s32 type);

spm::effdrv::EffEntry * effEntryTypeDebug(s32 type)
{
  spm::effdrv::EffEntry* entry = effEntryType(type);

  /*if(trackedCount < MAX_EFFS)
    {
      tracked[trackedCount].entry = entry;
      tracked[trackedCount].lastUserWork = nullptr;
      trackedCount++;
    }*/
  /*
  wii::os::OSReport("\n===================================");
  wii::os::OSReport("\neffEntryTypeDebug called, type: %d entry: %08X\n", type, entry);
  wii::os::OSReport("===================================\n");*/

  return entry;
}

//used for tracking internal name set
void (*effSetName)(spm::effdrv::EffEntry * entry, const char * name);

void effSetNameDebug(spm::effdrv::EffEntry * entry, const char * name)
{
  for (int i = 0; i < MAX_EFFS; i++)
    {
      if(tracked[i].entry == entry)
      {
        tracked[i].name = (char*)name;
        wii::os::OSReport("\nSet name for tracked entry %08X: %s\n", entry, name);
        break;
      }
    };
    wii::os::OSReport("Name set for entry %08X: %s\n", entry, name);
    if(entry->mainFunc)
    {
      wii::os::OSReport("MainFunc: %08X\n", entry->mainFunc);
    }
    effSetName(entry, name);
}

const char* lastName = nullptr;

// used for tracking when the pointer name is appended to an effect object
spm::effdrv::EffEntry * (*effNameToPtr)(const char * name);

spm::effdrv::EffEntry * effNameToPtrDebug(const char * name)
{
    spm::effdrv::EffEntry* entry = effNameToPtr(name);

    if(name != lastName)
    {
        wii::os::OSReport("\neffNameToPtrDebug called, name: %s entry: %08X\n",
                          name ? name : "(null)", entry);

        lastName = name;
    }

    return entry;
}

//used for tracking the instanceName called when effects are initialized by the actual orchestration function
spm::effdrv::EffSet * (*effGetSet)(s32 *effectName);

spm::effdrv::EffSet * effGetSetDebug(s32 *effectName)
{
  spm::effdrv::EffSet * set = effGetSet(effectName);
  wii::os::OSReport("effGetSet called for effectName: %s\n", (char*)effectName);
  return set;
}

// used for debugging effSubLoad; called per frame to interpret files and write data into memory for higher level emitter funcs
void (*effSubLoad)(int);

int debugCounter = 3; //startVal
int debugCounterReset = debugCounter; //value to reset to when hits 0

void effDebug(int fileId) //per-frame loop
{
    effSubLoad(fileId);
    if(fileId != 0) //haven't seen any prints yet, worth an explicit print to catch if it happens
    {
    wii::os::OSReport("\n===================================");
    wii::os::OSReport("\neffSubLoad called\n");
    wii::os::OSReport("fileId NOT 0: %d\n\n\n", fileId);
    
    spm::eff_sub::EffSubWork* wp = spm::eff_sub::effsub_wp;

    if(!wp) {
        wii::os::OSReport("effSubLoad: effsub_wp is null\n");
        return;
    }

    spm::filemgr::FileEntry* f = (spm::filemgr::FileEntry*)wp->activeDatFile;
    spm::memory::SmartAllocation* sp = f->sp;
    u32* table = (u32*)sp->data;

    for (int i = 0; i < 16; i++)
    {
        u8* b = (u8*)table[i];

        wii::os::OSReport(
            "table[%02d] = %08X  %02X %02X %02X %02X  %02X %02X %02X %02X\n",
            i,
            table[i],
            b[0], b[1], b[2], b[3],
            b[4], b[5], b[6], b[7]
        );
    }
  }

  else // fileId == 0; this is the one called every frame; we space prints to not flood the console.
  {
    spm::eff_sub::EffSubWork* wp = spm::eff_sub::effsub_wp;

    if(!wp) 
    {
      wii::os::OSReport("effSubLoad: effsub_wp is null\n");
      return;
    }

    int frame = wp->frameCount;
    if (frame % 60 == 0) 
    {
      debugCounter--;
    } 
    if (debugCounter == 0) 
    {
      //wii::os::OSReport("\n===================================\n===================================");
      //wii::os::OSReport("\neffSubLoad called with fileId 0, frameCount: %d\n\n", frame);
      
      for (int i = 0; i < trackedCount; i++)
        {
          auto* entry = tracked[i].entry;
          if (!entry) continue;

          tracked[i].memAllocCount= 0;

          if(entry->mainFunc && entry->userWork)
          {
            if(tracked[i].userWork != entry->userWork)
            {
              wii::os::OSReport("\n===================================");
              if(tracked[i].mainFunc != entry->mainFunc)
              {
                wii::os::OSReport("\n\nEff %08X has new mainFunc: %08X\n", tracked[i].entry, entry->mainFunc);
                tracked[i].name ? tracked[i].name : "(unnamed)";
                tracked[i].entry = nullptr;
                tracked[i] = tracked[trackedCount - 1];
                trackedCount--;
                i--;
                continue;
              }

              wii::os::OSReport("\nentry: %08X\n", entry);
              
              if(tracked[i].cmdName != nullptr)
              {
                wii::os::OSReport("cmd: %s\n", tracked[i].cmdName);
              }
              if(tracked[i].name != nullptr)
              {
                wii::os::OSReport("name: %s\n", tracked[i].name);
              }
              
              wii::os::OSReport("entry->mainFunc: %08X ", entry->mainFunc);
              wii::os::OSReport("entry->userWork: %08X ", entry->userWork);
              wii::os::OSReport("entry->instanceName: %s ", entry->instanceName);
              wii::os::OSReport("\n===================================\n");

              tracked[i].userWork = entry->userWork;
              for (int j = 0; j < pendingAllocCount; j++)
              {
                if (pendingAllocs[j].ptr == entry->userWork)
                {
                    // store the allocation inside the effect tracker
                    if (tracked[i].memAllocCount < MAX_MEM_ALLOC)
                    {
                        tracked[i].memAllocs[tracked[i].memAllocCount++] = pendingAllocs[j];
                    }

                    wii::os::OSReport(
                        "BOUND alloc ptr=%08X size=%u caller=%08X\n",
                        pendingAllocs[j].ptr,
                        pendingAllocs[j].size,
                        pendingAllocs[j].caller
                    );

                    // remove from pending list (swap removal)
                    pendingAllocs[j] = pendingAllocs[pendingAllocCount - 1];
                    pendingAllocCount--;
                    break;
                }
              }
            }
          }
        }


      //wii::os::OSReport("\n===================================\n===================================\n");
      debugCounter = debugCounterReset;
    }
  }
}

//debug funcs, unk, running on subLoadloop
void (*FUN_80091788)(spm::effdrv::EffEntry * entry);
void (*FUN_80091794)(spm::effdrv::EffEntry * entry);
void (*effSpmRecoveryMain)(spm::effdrv::EffEntry * entry);

void (*memcpy)(void * dest, const void * src, u32 size);

void FUN_80091788_debug(spm::effdrv::EffEntry * entry)
{
  wii::os::OSReport("FUN_80091788 called for entry: %08X\n", entry);
  wii::os::OSReport("flags: %04X\n", entry->flags);
  wii::os::OSReport("type: %04X\n", entry->type);
  wii::os::OSReport("releaseType: %d\n", entry->releaseType);
  wii::os::OSReport("MainFunc: %08X\n", entry->mainFunc);
  wii::os::OSReport("UserWork: %08X\n", entry->userWork);
  wii::os::OSReport("instanceName: %s\n", entry->instanceName);
  FUN_80091788(entry);
}

void FUN_80091794_debug(spm::effdrv::EffEntry * entry)
{
  wii::os::OSReport("FUN_80091794 called for entry: %08X\n", entry);
  wii::os::OSReport("MainFunc: %08X\n", entry->mainFunc);
  FUN_80091794(entry);
}

void main()
  {
      wii::os::OSReport("SPM Rel Loader: the mod has ran!\n");
      
      NetMemoryAccess::init();
      evtpatch::evtmgrExtensionInit();
      evt_patches();
      msgpatch::msgpatchMain();
      msgpatch::msgpatchAddEntry("msg_AP_item_name", "AP Item", false);
      msgpatch::msgpatchAddEntry("msg_AP_item_desc", "A valuable object from another dimension.", false);
      spm::item_data::itemDataTable[45].nameMsg = "msg_AP_item_name";
      spm::item_data::itemDataTable[45].descMsg = "msg_AP_item_desc";
      spm::item_data::itemDataTable[45].iconId = 324;
      pouchAddItem = patch::hookFunction(spm::mario_pouch::pouchAddItem, new_pouchAddItem);
      itemEntry = patch::hookFunction(spm::itemdrv::itemEntry, new_itemEntry);

      __memAlloc = patch::hookFunction(spm::memory::__memAlloc, memAllocDebug);

      effSpmVoltEntry = patch::hookFunction(spm::effdrv::effSpmVoltEntry, effSpmVoltEntryDebug);

      effEntry = patch::hookFunction(spm::effdrv::effEntry, effEntryDebug);
      effEntryType = patch::hookFunction(spm::effdrv::effEntryType, effEntryTypeDebug);
      effSetName = patch::hookFunction(spm::effdrv::effSetName, effSetNameDebug);
      effGetSet = patch::hookFunction(spm::effdrv::effGetSet, effGetSetDebug);
      effNameToPtr = patch::hookFunction(spm::effdrv::effNameToPtr, effNameToPtrDebug);
      effSubLoad = patch::hookFunction(spm::effdrv::effSubLoad, effDebug);
      writeBranchLink(spm::itemdrv::itemMain, 0xA18, new_itemCollectPouchItem);

      FUN_80091788 = patch::hookFunction(spm::effdrv::FUN_80091788, FUN_80091788_debug);
      FUN_80091794 = patch::hookFunction(spm::effdrv::FUN_80091794, FUN_80091794_debug);

      titleScreenCustomTextPatch();
  }
}