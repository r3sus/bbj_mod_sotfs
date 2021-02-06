# DarkSouls2SOTFSBabyJumpMod
## Description
When Dark Souls II is first opened, this dll is automatically loaded. As long as the Dinput8.dll is in
your game folder when you start the game, the "NoBabyJ (full jump length)" script from the sotfs bbj mode ce table is injected and automatically enabled. It remains active until the game closes. 
This has been written to make it simpler for newer speedrunners to use, to save the hassle 
of opening and enabling CE each time you open the game, and to clear up confusion on which version of the ce table should be used.

## Install
Copy the Dinput8.dll into the sotfs game folder (often found at C:\Program Files (x86)\Steam\steamapps\common\Dark Souls II Scholar of the First Sin\Game).
If a Dinput8.dll file already exists in the game folder, either rename that file to something else, or overwrite it.

## Uninstall
Rename or delete Dinput8.dll from the game folder.

## FAQ
* Does this table work with vanilla DS2?
No, please see the vanilla version at https://github.com/pseudostripy/bbj_mod/releases/latest.

* Why does my Cheat Engine table not work when this mod is installed?
Because it overwrites some of the bytes that some tables use to set up their base pointers. You can just edit the tables to search for the modded array of bytes if you wanted to get them both to work simultaneously.

* Why does this code look so much more complicated that the equivalent vanilla version?
Basically because sotfs is 64-bit.

## Credit
* NEZ64 for a working .dll example found at https://github.com/NEZ64/DarkSoulsOfflineLogoSkip.

* B3LYP for the original baby jump mod code written as a Cheat Engine table and from which this dll is simply a conversion.
