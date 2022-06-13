vc++ toolchaingenericds-utils (windows):
toolchaingenericds-utils.exe [Command] arg0 arg1 arg2 argN

	[Command List]:
	
	Binfile.bin Binfile.c Binfile (optional)SectionName
		Description:
		Where Binfile.bin is a raw binary blob that turns into BinFile, a C char[] object which can be recompiled and linked later.
		Optionally, if exists, SectionName is the name of the section the raw binary blob will be moved onto. 
		If no argument is specified, it goes into "embeddedBinData" section.

	...