#include "commands.h"
#include "stack.hh"
#include <spm/evt_mario.h>
#include <spm/evt_msg.h>
#include <spm/evt_item.h>
#include <spm/mapdrv.h>
#include <spm/itemdrv.h>
#include <spm/evtmgr.h>
#include <spm/effdrv.h>
#include "mod.h"
#include <spm/system.h>
#include <spm/mario.h>
#include <spm/spmario.h>
#include <wii/os.h>
#include <wii/mtx.h>
#include <msl/math.h>
#include <msl/stdio.h>

namespace mod {
u32 itemMax = 0;
char item_name[11];
static u32 * s_lastItemIdx = (u32 *)&spm::spmario::gp->gsw[1000];
static Stack<s32> itemStack;

inline bool isWithinMem1Range(s32 ptr) {
    return (ptr >= 0x80000000 && ptr <= 0x817fffff);
}

inline bool isValidGamePtr(void* ptr)
{
    u32 addr = (u32)ptr;

    return
        (addr >= 0x80000000 && addr < 0x81800000) ||  // MEM1
        (addr >= 0x90000000 && addr < 0x94000000);    // MEM2
}

s32 evt_item_entry_autoname(spm::evtmgr::EvtEntry *evtEntry, bool firstRun)
{
  spm::evtmgr::EvtVar *args = (spm::evtmgr::EvtVar *)evtEntry->pCurData;
  msl::stdio::sprintf(item_name, "i_%d_%d", *s_lastItemIdx, itemMax);
  itemMax++;
  spm::evtmgr_cmd::evtSetValue(evtEntry, args[0], item_name);
  spm::evt_item::evt_item_entry(evtEntry, firstRun);

  spm::evtmgr_cmd::evtSetValue(evtEntry, args[0], spm::itemdrv::itemNameToPtr(item_name)->name);
  return 2;
}

EVT_DECLARE_USER_FUNC(evt_item_entry_autoname, -1)

s32 add_to_gswf_stack(spm::evtmgr::EvtEntry *evtEntry, bool firstRun)
{
  spm::evtmgr::EvtVar *args = (spm::evtmgr::EvtVar *)evtEntry->pCurData;
  const char * name = (const char*)spm::evtmgr_cmd::evtGetValue(evtEntry, args[0]);
  spm::itemdrv::ItemEntry * item = spm::itemdrv::itemNameToPtr(name);
  spm::evtmgr::EvtVar gswf = abs(item->switchNumber);
  gswf -= (abs(GSWF(0)));
  gswf = abs(gswf);
  itemStack.push(gswf);
  wii::os::OSReport("GSWF: %d\n", gswf);
  return 2;
}

void addToGswfStack(spm::itemdrv::ItemEntry * item)
{
  spm::evtmgr::EvtVar gswf = abs(item->switchNumber);
  gswf -= (abs(GSWF(0)));
  gswf = abs(gswf);
  itemStack.push(gswf);
  wii::os::OSReport("GSWF: %d\n", gswf);
  return;
}

EVT_BEGIN(give_ap_item)
USER_FUNC(spm::evt_mario::evt_mario_get_pos, LW(5), LW(6), LW(7))
USER_FUNC(evt_item_entry_autoname, LW(8), LW(0), LW(5), LW(6), LW(7), 0, 0, 0, 0, 0)
USER_FUNC(spm::evt_item::evt_item_flag_onoff, 1, LW(8), 8)
USER_FUNC(spm::evt_item::evt_item_wait_collected, LW(8))
USER_FUNC(spm::evt_mario::evt_mario_key_on)
RETURN()
EVT_END()

//param helper
struct Params
{
    int ints[16];
    float floats[16];
    double doubles[16];
    void* pointers[16];

    const char* strings[16];
    u16 stringLens[16];

    int nInts;
    int nFloats;
    int nDoubles;
    int nStrings;
    int nPtrs;

    bool valid;
};

Params payloadProcess(const u8* payload, size_t payloadLen)
{
    Params params;

    params.nInts = 0;
    params.nFloats = 0;
    params.nDoubles = 0;
    params.nStrings = 0;
    params.nPtrs = 0;
    params.valid = true;

    const u8* p = payload;
    const u8* end = payload + payloadLen;

    while(p < end)
    {
        char t = *p++;

        switch(t)
        {
            case 'i':
            {
                if(p+4 > end) { params.valid=false; return params; }

                params.ints[params.nInts++] =
                (p[0] << 24) |
                (p[1] << 16) |
                (p[2] << 8)  |
                (p[3]);

                p += 4;
                break;
            }

            case 'f':
            {
                if(p+4 > end) { params.valid=false; return params; }

                u32 v =
                (p[0] << 24) |
                (p[1] << 16) |
                (p[2] << 8)  |
                (p[3]);

                params.floats[params.nFloats++] =
                    *(float*)&v;

                p += 4;
                break;
            }

            case 'd':
            {
                if(p+8 > end) { params.valid=false; return params; }

                u64 v =
                ((u64)p[0] << 56) |
                ((u64)p[1] << 48) |
                ((u64)p[2] << 40) |
                ((u64)p[3] << 32) |
                ((u64)p[4] << 24) |
                ((u64)p[5] << 16) |
                ((u64)p[6] << 8)  |
                ((u64)p[7]);

                params.doubles[params.nDoubles++] =
                    *(double*)&v;

                p += 8;
                break;
            }

            case 's':
            {
                if(p+2 > end) { params.valid=false; return params; }

                u16 len = *(u16*)p;
                p += 2;

                if(p+len > end) { params.valid=false; return params; }

                params.strings[params.nStrings] =
                    (char*)p;

                params.stringLens[params.nStrings] =
                    len;

                params.nStrings++;

                p += len;

                break;
            }

            case 'p':
            {
                if(p+4 > end) { params.valid=false; return params; }

                u32 v =
                (p[0] << 24) |
                (p[1] << 16) |
                (p[2] << 8)  |
                (p[3]);

                void* ptr = (void*)v;

                if(!isValidGamePtr(ptr)) { params.valid=false; return params; }

                params.pointers[params.nPtrs++] = ptr;

                p += 4;
                break;
            }

            case 'b':
            {
                if (p + 2 > end) { params.valid = false; return params; }

                u16 len = (p[0] << 8) | p[1];
                p += 2;

                if (p + len > end) { params.valid = false; return params; }

                // Ignore the blob contents for now
                p += len;
                break;
            }

            default:
            {
                params.valid = false;
                return params;
            }
        }
    }

    return params;
}

//Symbols

//Help Command
    COMMAND(CMD_HELP, help, "Returns the arg(s) for command id in Payload. (help categoryId commandId)", 
        {
            if (payloadLen < 2) {
                wii::os::OSReport("CMD_HELP: payload too small (%zu)\n", payloadLen);
                return 0;
            }

            u8 categoryId = payload[0];
            u8 commandId = payload[1];

            CommandId fullId = (CommandId)((categoryId << 8) | commandId);

            auto cmd = CommandManager::Instance()->findCommandById(fullId);

            if (!cmd) {
                return msl::stdio::snprintf(
                    (char*)response,
                    responseSize,
                    "Unknown command %d:%d",
                    categoryId,
                    commandId
                );
            }

            return msl::stdio::snprintf(
                (char*)response,
                responseSize,
                "%s",
                cmd->getHelpMsg()
            );
        });

//Read Commands
    
    COMMAND(CMD_rIDX, ridx, "Reads the current item index. (ridx)", 
        {
            wii::os::OSReport("CMD_rIDX: reading idx\n");

            u32 idx = *s_lastItemIdx;

            msl::string::memcpy((void*)response, &idx, sizeof(u32));
            return sizeof(u32);
        });

    COMMAND(CMD_rBUSY, rbusy, "Reads whether Mario is busy (i.e., loading zone/item-acceptance menu). (rbusy)", 
        {
            wii::os::OSReport("CMD_rBUSY: reading busy\n");

            bool busy = spm::mario::marioKeyOffChk();

            u32 busyValue = busy ? 1 : 0;
            msl::string::memcpy((void*)response, &busyValue, sizeof(u32));
            return sizeof(u32);
        });

//Base Commands
    COMMAND(CMD_ITEM, item, "Gives mario an item. (item itemId)", 
    {
        Params params =
        payloadProcess(payload, payloadLen);

        if(!params.valid || params.nInts < 1) 
            {
                return msl::stdio::snprintf(
                    (char*)response,
                    responseSize,
                    "%s",
                    item.getHelpMsg()
                );
            };

        if (spm::mario::marioKeyOffChk())
            {
                return msl::stdio::snprintf(
                    (char*)response,
                    responseSize,
                    "Mario busy"
                );
            }

        spm::mario::marioKeyOff();

        int itemId = params.ints[0];
        wii::os::OSReport("CMD_ITEM: giving item id=%u\n", itemId);
        spm::evtmgr::EvtEntry* evt = spm::evtmgr::evtEntry(give_ap_item, 0, 0);
        if (!evt)
            {
                return msl::stdio::snprintf(
                    (char*)response,
                    responseSize,
                    "evtEntry failed"
                );
            }

        evt->lw[0] = (u16)itemId;

        char msg[128];
        return msl::stdio::snprintf(
            msg,
            sizeof(msg),
            "CMD_ITEM: received item id=%u\n",
            itemId
        );
    });

    COMMAND(CMD_IDX, idx, "Sets the current item index. (idx index)", 
        {
            if (payloadLen < sizeof(u32)) {
                wii::os::OSReport("CMD_IDX: payload too small (%zu)\n", payloadLen);
                return 0;
            }

            u32 idx;
            msl::string::memcpy(&idx, payload + 0, sizeof(u32));
            *s_lastItemIdx = idx;

            wii::os::OSReport("CMD_IDX: received idx=%u\n", idx);
            char msg[128];

            return msl::stdio::snprintf(
                msg,
                sizeof(msg),
                "CMD_IDX: received idx=%u\n",
                idx
            );
        });

    COMMAND(CMD_MAP_INTERACT, mapInteract, "Triggers a map interaction. (map_interact interactId)", 
        {
            if (payloadLen < sizeof(u32)) {
                wii::os::OSReport("CMD_MAP_INTERACT: payload too small (%zu)\n", payloadLen);
                return 0;
            }

            u32 messageLen;
            msl::string::memcpy(&messageLen, payload + 0, sizeof(u32));
            if (payloadLen < sizeof(u32) + messageLen) {
                wii::os::OSReport("CMD_MAP_INTERACT: payload too small for message (%zu)\n", payloadLen);
                return 0;
            }
            wii::os::OSReport("CMD_MAP_INTERACT: received message length=%u", messageLen);
            char receivedMessage[256];
            msl::string::memcpy(receivedMessage, payload + sizeof(u32), messageLen);
            receivedMessage[messageLen] = '\0'; // Null-terminate the message
            wii::os::OSReport("\nCMD_MAP_INTERACT: received message=%s", receivedMessage);
            
            wii::mtx::Vec3 pos;

            spm::mapdrv::mapObjGetPos(receivedMessage, &pos);

            wii::os::OSReport("\nPosition of %s: (%f, %f, %f)\n", receivedMessage, (double)pos.x, (double)pos.y, (double)pos.z);

            int type = 0;

            for (int i = 0; receivedMessage[i]; i++)
            {
                if (receivedMessage[i] < '0' || receivedMessage[i] > '9')
                    break;

                type = type * 10 + (receivedMessage[i] - '0');
            }

            //spm::effdrv::EffEntry* heart = spm::effdrv::effNameToPtr("hearts");
            //spm::effdrv::EffEntry* e = spm::effdrv::unk_800a845c(type);
            //wii::os::OSReport("\nEntry: %p\n", e);
            //wii::os::OSReport("\nEntry: %p\n", heart);
            //spm::effdrv::effAssignToMapObj(heart, 0, receivedMessage);

            // response logic
            char msg[128];
            return msl::stdio::snprintf(
                msg,
                sizeof(msg),
                "CMD_MAP_INTERACT: received message length=%u",
                messageLen
            );
        });

//Effect Commands

    COMMAND(CMD_EFF_E_SPMRECOVERY, effSpmRecover, "Spawns the healing effect. (eff_spm_recovery floatX floatY floatZ intHP)", 
        {
            Params params =
            payloadProcess(payload,payloadLen);

            if(!params.valid || params.nInts < 1 || params.nFloats < 3) {
                return msl::stdio::snprintf(
                    (char*)response,
                    responseSize,
                    "%s",
                    effSpmRecover.getHelpMsg()
                );
            };

            int fx = params.floats[0];
            int fy = params.floats[1];
            int fz = params.floats[2];
            int hp = params.ints[0];

            spm::effdrv::EffEntry* entry = spm::effdrv::effSpmRecoveryEntry(fx, fy, fz, hp);
            if(mod::trackedCount < MAX_EFFS)
            {
                mod::TrackedEff* t = &tracked[mod::trackedCount++];

                t->entry = entry;
                t->userWork = nullptr;
                t->mainFunc = entry ? entry->mainFunc : nullptr;
                t->name = entry ? entry->instanceName : nullptr;
                t->cmdName = "CMD_EFFECT_SPMRECOVERY";
            }

            char msg[128];
            return msl::stdio::snprintf(
                msg,
                sizeof(msg),
                "Spawned recovery effect at (%f, %f, %f) with HP %d",
                (double)fx, (double)fy, (double)fz, hp
            );
        });

    COMMAND(CMD_EFF_E_VOLTENTRY, effVoltEntry, "Spawns the voltage effect. (eff_volt_entry floatX floatY int-Mode intParam2)", 
        {
            Params params =
            payloadProcess(payload,payloadLen);

            if(!params.valid || params.nInts < 2 || params.nFloats < 2) {
                return msl::stdio::snprintf(
                    (char*)response,
                    responseSize,
                    "%s",
                    effVoltEntry.getHelpMsg()
                );
            };

            EffTargetCtx ctx;

            float fx = params.floats[0];
            float fy = params.floats[1];

            ctx.mode = params.ints[0];
            ctx.id = 0;
            ctx.x = 0.0f;
            ctx.y = 0.0f;
            ctx.z = 0.0f;
            ctx.width = 100.0f;
            ctx.height = 100.0f;

            int p2 = params.ints[1];

            spm::effdrv::EffEntry* entry = spm::effdrv::effSpmVoltEntry(fx, fy, (spm::effdrv::EffEntry*)&ctx, p2);
            if(mod::trackedCount < MAX_EFFS)
            {
                mod::TrackedEff* t = &tracked[mod::trackedCount++];

                t->entry = entry;
                t->userWork = nullptr;
                t->mainFunc = entry ? entry->mainFunc : nullptr;
                t->name = entry ? entry->instanceName : nullptr;
                t->cmdName = "CMD_EFFECT_VOLTENTRY";
            }

            char msg[128];
            return msl::stdio::snprintf(
                msg,
                sizeof(msg),
                "Spawned voltage effect at (%f, %f) with params %d, %d",
                (double)fx, (double)fy, ctx.mode, p2
            );
        });

    COMMAND(CMD_EFF_E_ITEMTHUNDER, effItemThunder, "Spawns a thunder effect. (floatX floatY floatZ intParam1 intParam2 intParam3 intParam4 intParam5)", 
        {
            Params params =
            payloadProcess(payload,payloadLen);

            if(!params.valid || params.nInts < 5 || params.nFloats < 3) {
                return msl::stdio::snprintf(
                    (char*)response,
                    responseSize,
                    "%s",
                    effItemThunder.getHelpMsg()
                );
            };

            int fx = params.floats[0];
            int fy = params.floats[1];
            int fz = params.floats[2];

            int p0 = params.ints[0];
            int p1 = params.ints[1];
            int p2 = params.ints[2];
            int p3 = params.ints[3];
            int p4 = params.ints[4];

            spm::effdrv::EffEntry* entry = spm::effdrv::effItemThunderEntry(fx, fy, fz, p0, p1, p2, p3, p4);
            if(mod::trackedCount < MAX_EFFS)
            {
                mod::TrackedEff* t = &tracked[mod::trackedCount++];

                t->entry = entry;
                t->userWork = nullptr;
                t->mainFunc = entry ? entry->mainFunc : nullptr;
                t->name = entry ? entry->instanceName : nullptr;
                t->cmdName = "CMD_EFFECT_ITEMTHUNDER";
            }

            /*pos.y -= 2.0f;

            spm::effdrv::EffEntry* e2 = spm::effdrv::effDamageStarEntry(pos.x, pos.y, pos.z, 0, 3);
            if (e2) {
                wii::os::OSReport("e2->userWork: %p\n", e2->userWork);
            }*/

            char msg[128];
            return msl::stdio::snprintf(
                msg,
                sizeof(msg),
                "Spawned damage effect at (%f, %f, %f)\nParams: %d, %d, %d, %d, %d",
                (double)fx, (double)fy, (double)fz, p0, p1, p2, p3, p4
            );
        });


EVT_DEFINE_USER_FUNC(evt_deref) {
    s32 addr = spm::evtmgr_cmd::evtGetValue(evt, evt->pCurData[0]);
    SPM_ASSERT(isWithinMem1Range(addr), "evt_deref error");
    s32* ptr = reinterpret_cast<s32*>(addr);
    spm::evtmgr_cmd::evtSetValue(evt, evt->pCurData[1], *ptr);
    return EVT_RET_CONTINUE;
}

}
