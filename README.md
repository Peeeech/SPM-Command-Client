
# Live Command Injector for Super Paper Mario

#### Note : Current supported versions: eu0, us2 (verified)

## How To Use

Once this mod is running, your private IP address will be displayed on the title screen. The python client provided is an interactive GUI used to select and send commands, as well as to input custom parameters depending on the command called.

## List of commands
(NOTE: IN THIS BUILD PARAMETERS ARE ONLY ACTIVELY PARSED FOR THE BASE (item) COMMAND, AND EFFECT COMMANDS. MAP-INTERACT AND SETIDX ARE UNIMPLEMENTED ATM)
    
    due to setidx being unimplemented, the item command is ran directly without checking for stored idx

### Read
Read IDX: Reads a stored u32 IDX value saved in memory via GSW flags
    
    Params: (N/A)

Read Busy: Reads the current state of the game using the `marioKeyOffChk` function and returns whether the player is unable to send movement inputs at the moment

    Params: (N/A)

### Base
Send Item: Sends an item to the game by string name

    Params: (Str[item-name])

Set IDX: Manually set the IDX stored in memory

    Params: (Int[idx])

Map-Interact: Returns the XYZ Position of a Map Object from it's string-name

    Params: (Str[mapObjName])

### Effect
SPM Recover: Recovers a set amount of health

    Params: float(x) float(y) float(z) int(Hp)
XYZ currently unk

SPM Volt: Spawns the 'electric trickle' effect used by volt shroom / when thunder rage hits an enemy

    Params: float(x) float(y) *int(mode) int(param_4)
XY - used when appended to an entity; not implemented

mode - used for identifying what to attach to; not implemented yet

param_4 - unk

to actually use the effect currently you just need *mode to != 0, and it'll spawn the effect at the room's (0, 0, 0) origin in worldspace

Item Thunder: Has two use-cases: Type 0: Thunder flash. Type 1: Spawns a yellow 'hit bubble' effect and SpmVolt effect -- natively bound to Mario

    Params: (Float(x), Float(y), Float(z), Int[a], Int[b], Int[c], Int[d], Int[e])
a - seems to be a 'type' parameter, where 0 is a global 'whole-screen' thunder flash, and 1 is like the volt shroom zap on mario

    (the rest of the params seem to only apply to type 1 from my testing)
b - seemingly a count for how many times the effect happens

c - seems to be a delay of sorts; the higher the value, the longer between flashes when b > 1

d - unk

e - unk


## Credits

* **JohnP55** for the original SPMMulti repo this was based on
* **PistonMiner**, **Zephiles** and **Seeky** for the SPM rel loader.
* **bushing** for network_wii.c
* **The FreeRTOS team** for their work on coreHTTP and coreJSON.

