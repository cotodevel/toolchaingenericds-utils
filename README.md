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
			
	
	[mp4totvs] 
		__Description__
		TGDS-Videoconverter, emits *.TVS video+audio streams playable in ToolchainGenericDS-multimediaplayer (cmd only).
		WIN32 has a script to automate TVS conversion, 
			located at C:\<installpath>\newlib-nds\installer\shared\6.2_2016q4\bin if you install newlib-nds (https://github.com/cotodevel/newlib-nds)
		
		
	[pkgbuilder] [TGDSProjectName] [/baseTargetDecompressorDirectory] [/TGDSLibrarySourceDirectory] [/TGDSProjectSourceDirectory]
		__Description__
		ToolchainGenericDS Packager. 
		Note: TGDSProjectName refers to the TGDS Project's main NTR/TWL binary name.
	
	
	[remotebooter]  [/TGDSProjectSourceDirectory] [NintendoDS IP:xxx.xxx.xxx.xxx format] [ntr_mode/twl_mode] [TGDSProjectName] [baseTargetDecompressorDirectory] [TGDSLibrarySourceDirectory] [overridden TGDS Package Name]
		__Description__
		Packages a destination directory and sends it to ToolchainGenericDS-multiboot remoteboot command
		Note: TGDSProjectName refers to the TGDS Project's main NTR/TWL binary name.
		__Example__
		Sending a TWL executable and its file dependencies remotely:
		toolchaingenericds-utils remotebooter /release 192.168.43.82 twl_mode ToolchainGenericDS-multimediaplayer / C:/toolchain_generic/6.2_2016q4/arm-eabi/lib/newlib-nano-2.1-nds/ remotepackage
		Now run ToolchainGenericDS-multiboot and enter remoteboot mode. Wait until package arrives and executes remotely.
	
	
	[httpserver] [-quit: [Optional] Quits after sending 1 file to client connected.]
		__Description__
		Maps a HTTP Server relative to local directory.


	[untar] [-d] [/targetUntarDirectory] [/tarFilenameSource.tar.gz]
		__Description__
		Unpacks a TAR GZIP file. targetUntarDirectory is destination directory and tarFilenameSource.tar.gz is TAR file source
	
	
	[zipfiles] [zipFilenameToCreate] [file1ToAdd.ext] [file2ToAdd.ext]
		__Description__
		Creates a ZIP (deflated) file from a set of files. Do note depending on the filepath retrieved, these will inherited in the archive
	
	
Other tools:

TGDS filesystem support into ToolchainGenericDS:
	- FatFS FAT16/FAT32/exFAT disk library for the Nintendo DS
	- Enables POSIX -> Newlib File IO Operations such as fopen/fread/fwrite/fclose/fseek/fgetc/etc
	- FileClass layer Implementation: Simulates libfat in the high level layer (about 99%) so it aids in porting legacy NDS Homebrew.
	- TGDS File / Directory Iterator Operator: Instances (Keeping memory footpring low) / Push-Pop operations, preventing to rewrite a lot of useless C code.
	- Libfat-compatible API
	- 	Windows port (Visual Studio 2012) to run TGDS FS drivers natively (debugging purposes) + 32MB FAT Image for running a virtual DLDI layer. 
			Implements TGDS FS (FAT/FAT32) file open/ read / writes from a FAT image which resembles a physical SD mapped to a DLDI driver.
	-	Windows (Visual Studio 2012): TVS video exporter for TGDS-Videoplayer
	
	Folder structure:
	
	misc/build
		- TGDS projects management (Makefile)
		- TGDSPKGBuilderDefaultTemplateNDSHomebrew/ TGDS project template implementing a deployable package to be consumed by ToolchainGenericDS-OnlineApp (download and runs packages from the internet)
		- dsarm/ misc tools to disassemble / assemble / debug ARM binaries from/to librarian formats (.a library of ARM ELF objects, Requires Newlib for NintendoDS before use. Follow: https://github.com/cotodevel/newlib-nds steps to set up requires environment.)		
		- resources/ specification of custom formats used by TGDS software
	
	misc/imaadpcm
		- adpcm encoder / decoder
	
	misc/tcp
		- UDP TGDS Server Companion Program. Used for UDP NIFI (Internet) in TGDS projects
	
	misc/petitfs-tgds.7z
		- git repository having PetitFS + TGDS + DLDI support from VS2012 WIN32 Debugger (ARM7-lighter version of TGDS-Filesystem API implementation)