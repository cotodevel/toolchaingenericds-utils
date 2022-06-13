Windows / Linux:
toolchaingenericds-utils [Command] arg0 arg1 arg2 argN

	[Command List]:
	
	[bin2c] Binfile.bin Binfile.c Binfile (optional)SectionName
		__Description__
		Where Binfile.bin is a raw binary blob that turns into BinFile, a C char[] object which can be recompiled and linked later.
		Optionally, if exists, SectionName is the name of the section the raw binary blob will be moved onto. 
		If no argument is specified, it goes into "embeddedBinData" section.

	
	[bin2lzss] command2 filename [filename [...]]
		__Description__
		command2:
			-d ..... decode 'filename'
			-evn ... encode 'filename', VRAM compatible, normal mode (LZ10)
			-ewn ... encode 'filename', WRAM compatible, normal mode
			-evf ... encode 'filename', VRAM compatible, fast mode
			-ewf ... encode 'filename', WRAM compatible, fast mode
			-evo ... encode 'filename', VRAM compatible, optimal mode (LZ-CUE)
			-ewo ... encode 'filename', WRAM compatible, optimal mode (LZ-CUE)
			
			multiple filenames and wildcards are permitted
			the original file is overwritten with the new file