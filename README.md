# Money Boy
A Game Boy emulator written in C++. It's currenly work-in-progress.

## Todo

* Correct audio implementation
* MBC support (currently only cartridges without a MBC can run)
* Pass more tests
* UI for loading cartridges

## Note

* The implementation is referred to the DMG model among several Game Boy series.
* To avoid copyright concerns, the boot ROM file is not included in the repository.
* The feature of skipping the boot process isn't mature. Though you can run a game without a boot ROM (where the binary is built with PREBOOT defined), the state of the registers wouldn't be correct. For example, the master sound switch might not be on because usually it's turned on during the boot process.
