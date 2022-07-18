Resources:
LLVM + ARM porting:
https://interrupt.memfault.com/blog/arm-cortexm-with-llvm-clang

-
Set up git globally:

git config user.name "Your Name"
git config user.email "Your email"
git config --global credential.helper store
-

/release folder is distributed, then packages are forked.

ToolchainGenericDS software package format:
TGDSPKG_[TGDS Projectname].tar.gz

Format:
.tar.gz file: 

PKG Structure:
[Compressed File Contents] descriptor.txt

Text file: descriptor.txt:
- mainApp: [full-appname.nds] . Entrypoint.
- mainAppCRC32: [crc32generated]
- TGDS-SDKCRC32: [takes the sum of crc32: libcnano7.a, libcnano9.a, libtoolchaingen7.a, libtoolchaingen9.a]. There will be an automated script, uploading through FTP, proper TGDS-SDKCRC32 whitelists.
- baseTargetPath: "/path/to/unzip/the/package". If none exists, root("/") is preset. All file and folder structures will be decompressed here.

Todo:
add new TGDS project rule:
build TGDSPackage. (C++) App does the above right after the binary is built.
-


Upgrade VideoGL Nintendo DS  OpenGL (1.0 -> 1.1):

21.070 How do texture objects work?

Texture objects store texture maps and their associated texture parameter state. They allow switching between textures with a single call to glBindTexture().

Texture objects were introduced in OpenGL 1.1. Prior to that, an application changed textures by calling glTexImage*(), a rather expensive operation. Some OpenGL 1.0 implementations simulated texture object functionality for texture maps that were stored in display lists.

Like display lists, a texture object has a GLuint identifier (the textureName parameter to glBindTexture()). OpenGL supplies your application with texture object names when your application calls glGenTextures(). Also like display lists, texture objects can be shared across rendering contexts.

Unlike display lists, texture objects are mutable. When a texture object is bound, changes to texture object state are stored in the texture object, including changes to the texture map itself.

The following functions affect and store state in texture objects: glTexImage*(), glTexSubImage*(), glCopyTexImage*(), glCopyTexSubImage*(), glTexParameter*(), and glPrioritizeTextures(). 

Here is a summary of typical texture object usage:

Get a textureName from glGenTextures(). You'll want one name for each of the texture objects you plan to use.
Initially bind a texture object with glBindTexture(). Specify the texture map, and any texture parameters. Repeat this for all texture objects your application uses.
Before rendering texture mapped geometry, call glBindTexture() with the desired textureName. OpenGL will use the texture map and texture parameter state stored in that object for rendering.


Workflow:
1)Model export phase: 
Blender model exporter exports 2 objects:

1.1) NDSCallLists
1.2) NDSCallList descriptor which defines:

How many textures: n
texture index (n):
texture size

2)NDSProgram: NDSLoad (and not NDS CallLists rendering):
2.1) Script to read all textures from a polygon
2.2) Read texture coordinates, each index represent a position in texture memory, and generate a texture object context.

3)NDSProgram: Rendering
3.1) Develop render-to-texture example
3.2) Add other upcoming texture object features...

-


TGDS TWL Binaries:

Gamecode: TGDS
Makercode: NN
Title: DS.TinyFB

The default method for launching TGDS TWL Binaries is TWiLightMenu++, thus the Title reserved is "DS.TinyFB" and binaries must be copied in SD:/ndsi/ folder (create a new one if doesn't exist).
This way, TWL binaries are loaded natively, in TWL mode.


-

.TVS file format (ToolchainGenericDS VideoStream)
Implementation: ToolchainGenericDS-Videoplayer (client) / ToolchainGenericDS-Helper/misc/vs2012TGDS-FS (encoder)

- Video: A sequence of LZSS (WRAM) compressed, compatible with NDS/NDSi Bios decompression functions's 15bit RGB Bitmap videoFrames, running at fixated 10 FramesPerSecond 
- Audio: Intel IMA-ADPCM
- Also both Video + Audio tracks are synchronized to timestamps, so if the SD card speed can keep up the framerate, you'll never get an out of sync video again ;-)