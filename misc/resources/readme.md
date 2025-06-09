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

-

Nintendo DS GX Notes:

21.070 How do texture objects work?
Texture objects store texture maps and their associated texture parameter state. They allow switching between textures with a single call to glBindTexture().
Texture objects were introduced in OpenGL 1.1. Prior to that, an application changed textures by calling glTexImage*(), a rather expensive operation. Some OpenGL 1.0 implementations simulated texture object functionality for texture maps that were stored in display lists.
Like display lists, a texture object has a GLuint identifier (the textureName parameter to glBindTexture()). OpenGL supplies your application with texture object names when your application calls glGenTextures(). Also like display lists, texture objects can be shared across rendering contexts.
Unlike display lists, texture objects are mutable. When a texture object is bound, changes to texture object state are stored in the texture object, including changes to the texture map itself.
The following functions affect and store state in texture objects: glTexImage*(), glTexSubImage*(), glCopyTexImage*(), glCopyTexSubImage*(), glTexParameter*(), and glPrioritizeTextures(). 


Typical OpenGL drawing:
Get a textureName from glGenTextures(). You'll want one name for each of the texture objects you plan to use.
Initially bind a texture object with glBindTexture(). Specify the texture map, and any texture parameters. Repeat this for all texture objects your application uses.
Before rendering texture mapped geometry, call glBindTexture() with the desired textureName. OpenGL will use the texture map and texture parameter state stored in that object for rendering.


NOTE: 
Display Lists:
GX Display Lists Port can only receive blocks of memory through DMA from 4MB EWRAM, which is an extension of the Display Lists OpenGL 1.0 standard. 
As it is, this implementation require transactions between Client(RAM) and Server(VRAM) sided rendering.

Texture objects: 
NOT supported by NintendoDS hardware, but can be added in software, albeit it's slower. Because these define mutable Texture, Vertices and Color arrays directly running on the Server(VRAM)







--------------------------------------------------------------------------------------------------------------------------------------------------------------------
[[[[[[[[Steps to perform standard OpenGL 1.0 rendering on NintendoDS GX pipeline using Textures + Normals + 1 light source + materials on standard vertices ]]]]]]]]
--------------------------------------------------------------------------------------------------------------------------------------------------------------------


glBindTexture( 0, textureArrayNDS[0]); //bind the texture to-be used

//Reset The View towards the Model and *project* translation on current matrix
glLoadIdentity();
glTranslatef(1.4f+(float(xloop)*2.8f)-(float(yloop)*1.4f),((6.0f-float(yloop))*2.4f)-7.3f,-20.0f + camMov);

//Rotation because NintendoDS seems to emit 45° rotated polygons, this set's them back to defaults (as a normal OpenGL driver would do)
glRotatef(45.0f-(2.0f*yloop)+xrot,1.0f,0.0f,0.0f);
glRotatef(45.0f+yrot,0.0f,1.0f,0.0f);

//Enable Light0 + Material Parameters
#define GX_LIGHT0 (1 << 0)
glPolyFmt(GX_LIGHT0 | POLY_ALPHA(31) | POLY_CULL_NONE);
glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
glLightfv(GL_LIGHT0, GL_POSITION, light_position);
glMaterialf(GL_AMBIENT_AND_DIFFUSE, RGB15(31,31,31));	

//Base Color which blends into final texture color
glColor3fv(boxcol[yloop-1]);

//Normals (base surface vector coordinates to cause light emission towards current Light Direction Vector using current Light Color + Texture Color + Base color)
glNormal3f( 1.0f, 1.0f,-1.0f);

//2D Texture Coordinates
glTexCoord2f(1.0f, 0.0f); 

//Finally 3-Vertices (a Triangle in the 3D ModelViewProjection space)
glVertex3f(-1.0f, -1.0f, -1.0f);

Example: https://github.com/cotodevel/snakegl







--------------------------------------------------------------------------------------------------------------------------------------------------------------------
[[[[[[[[Steps to perform standard OpenGL 1.0 rendering on NintendoDS GX pipeline using Textures + Normals + 1 light source + materials on Display Lists ]]]]]]]]
--------------------------------------------------------------------------------------------------------------------------------------------------------------------

glBindTexture( 0, textureArrayNDS[0]); //bind the texture to-be used

//Reset The View towards the Model and *project* translation on current matrix
glLoadIdentity();
glTranslatef(1.4f+(float(xloop)*2.8f)-(float(yloop)*1.4f),((6.0f-float(yloop))*2.4f)-7.3f,-20.0f + camMov);

//Rotation because NintendoDS seems to emit 45ï¿½ rotated polygons, this set's them back to defaults (as a normal OpenGL driver would do)
glRotatef(45.0f-(2.0f*yloop)+xrot,1.0f,0.0f,0.0f);
glRotatef(45.0f+yrot,0.0f,1.0f,0.0f);

//Enable Light0 + Material Parameters
#define GX_LIGHT0 (1 << 0)
glPolyFmt(GX_LIGHT0 | POLY_ALPHA(31) | POLY_CULL_NONE);
glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
glLightfv(GL_LIGHT0, GL_POSITION, light_position);
glMaterialf(GL_AMBIENT_AND_DIFFUSE, RGB15(31,31,31));	

//Open GL 1.1 Display List 
glColor3fv(boxcol[yloop-1]);
glCallList(box);
glColor3fv(topcol[yloop-1]);
glCallList(top);

Example: https://github.com/cotodevel/toolchaingenericds-unittest

![ToolchainGenericDS](https://bytebucket.org/Coto88/ndsdisplaylistutils/raw/02aca38abbf1fec3cd145c7f43950920ff99f4cb/img/ndsdisplaylistutils_nds.png)







--------------------------------------------------------------------------------------------------------------------------------------------------------------------
[[[[[[[[  Useful guide for porting Coloured Triangle Example   ]]]]]]]]
[[[[[[[[  from OpenGL 1.0 unpacked calls into OpenGL 1.1 Vertex Buffer Array format  ]]]]]]]]
--------------------------------------------------------------------------------------------------------------------------------------------------------------------


//////////////OGL 1.0 version: Use 3 vertices//////////////
glBegin(GL_TRIANGLES);
glColor3f( 1, 0, 0 ); // red
glVertex2f( -0.9, -0.9 );
glColor3f( 0, 1, 0 ); // green
glVertex2f( 0.9, -0.9 );
glColor3f( 0, 0, 1 ); // blue
glVertex2f( 0, 0.7 );
glEnd();


//////////////VBO version//////////////
float coords[6] = { -0.9,-0.9,  0.9,-0.9,  0,0.7 }; // two coords per vertex.
float colors[9] = { 1,0,0,  0,1,0,  0,0,1 };  // three RGB values per vertex.

glVertexPointer( 2, GL_FLOAT, 0, coords );  // Set data type and location.
glColorPointer( 3, GL_FLOAT, 0, colors );

glEnableClientState( GL_VERTEX_ARRAY );  // Enable use of arrays.
glEnableClientState( GL_COLOR_ARRAY );

glDrawArrays( GL_TRIANGLES, 0, 3 ); // Use 3 vertices, starting with vertex 0.

glDisableClientState(GL_VERTEX_ARRAY);
glDisableClientState(GL_COLOR_ARRAY);