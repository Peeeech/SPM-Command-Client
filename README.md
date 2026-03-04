
# Live Command Injector for Super Paper Mario

#### Note : Current supported versions: eu0, us2 (verified)

## How To Use

Once this mod is running, your private IP address will be displayed on the title screen. The python client provided is an interactive GUI used to select and send commands, as well as to input custom parameters depending on the command called.

## List of commands
(NOTE: IN THIS BUILD PARAMETERS ARE ONLY ACTIVELY PARSED FOR THE EFFECT COMMAND. READ COMMANDS *DO* WORK, BUT BASE COMMANDS CURRENTLY *DO NOT*.)

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
Effect Test: Currently binded to the function `effItemThunderEntry` as it was used to test parameter functions. Usage varies by parameters

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

