///////////////////////////////
// FILE: except.h
// CGfxExcept class declaration
///////////////////////////////

#ifndef __gfxexcept_h__
#define __gfxexcept_h__

#include <windows.h>

#include "resource.h"

class CGfxExcept
{
public:
   CGfxExcept (int cause = ERR_UNKNOWN) :errIndex(cause) {}
   const char *GetErrMessage (HINSTANCE hInst);

private:
   int errIndex;
   char MsgBuffer[100];

};

#endif