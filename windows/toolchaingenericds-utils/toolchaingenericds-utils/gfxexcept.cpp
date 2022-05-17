//////////////////////////////////
// FILE: except.cpp
// CGfxExcept class implementation
//////////////////////////////////

#include "gfxexcept.h"

//////////////////////////
// Error message retrieval
//////////////////////////
const char *CGfxExcept::GetErrMessage(HINSTANCE hInst)
{
   if (LoadString(hInst, errIndex, LPWSTR(MsgBuffer), sizeof(MsgBuffer)))
      return (char *)MsgBuffer;
   else
      return "Unknown error.";
}