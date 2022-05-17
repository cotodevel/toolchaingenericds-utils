// GFX viewer by Artem

#include <windows.h>
#pragma hdrstop

#include "image.h"
#include "gfxview.h"
#include "resource.h"

////////////////////
// Global Variables:
////////////////////
HINSTANCE hInst; //current instance
char szTitle[] = "GFX View by Artem";
char szWindowClass[] = "WND_GFXVIEW_CLASS";
static char szFilter[] = "Windows Bitmap (*.bmp;*.dib;*.rle)\0*.bmp;*.dib;*.rle\0ZSoft Paintbrush (*.pcx)\0*.pcx\0\0";
char szFileName[MAX_PATH];
DWORD nFilterIndex = 1; //selected format filter index on open/save dialog
RECT clntRect; //window client rectangle
int nShiftX = 0, nShiftY = 0; //picture shift
SCROLLINFO HorzInfo, VertInfo; //scroll bars' info
CImageDIB *pImage; //CImageDIB object

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /* hPrevInstance */, char * /* pszCommandLine */, int nCmdShow)
{
   OnInit(); //Initialize

   MSG msg;
   HACCEL hAccelTable;

   MyRegisterClass(hInstance);

   if (!InitInstance (hInstance, nCmdShow))  return FALSE;

   //Load accelerators
   hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDM_MAINMENU));

   while (GetMessage(&msg, NULL, 0, 0)) 
   {
      if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
      {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
      }
   }

   return msg.wParam;
}

////////////////////////
// Register window class
////////////////////////
ATOM MyRegisterClass(HINSTANCE hInstance)
{
   WNDCLASSEX wcex;

   wcex.cbSize = sizeof(WNDCLASSEX); 

   wcex.style = CS_HREDRAW|CS_VREDRAW;
   wcex.lpfnWndProc = (WNDPROC)WndProc;
   wcex.cbClsExtra = 0;
   wcex.cbWndExtra = 0;
   wcex.hInstance = hInstance;
   wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GFXVIEW));
   wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
   wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
   wcex.lpszMenuName  = MAKEINTRESOURCE(IDM_MAINMENU);
   wcex.lpszClassName = LPCWSTR(szWindowClass);
   wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

   return RegisterClassEx(&wcex);
}

//////////////////////////////////////////////
// Save instance handle and create main window
//////////////////////////////////////////////
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance;

   hWnd = CreateWindow(LPCWSTR(szWindowClass), LPCWSTR(szTitle), WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

///////////////////////////////////////
// Process messages for the main window
///////////////////////////////////////
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   PAINTSTRUCT ps;
   HDC hDC;

   switch (message)
   {
   case WM_COMMAND:
      OnCommand(hWnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
      break;

   case WM_SIZE:
      OnResize(hWnd, wParam, LOWORD(lParam), HIWORD(lParam));
      break;

   case WM_HSCROLL:
      OnHScroll(hWnd, (int)LOWORD(wParam), (short int)HIWORD(wParam), (HWND)lParam);
      break;

   case WM_VSCROLL:
      OnVScroll(hWnd, (int)LOWORD(wParam), (short int)HIWORD(wParam), (HWND)lParam);
      break;

   case WM_PAINT:
      hDC = BeginPaint(hWnd, &ps);
      OnPaint(hWnd, hDC);
      EndPaint(hWnd, &ps);
      break;

   case WM_DESTROY:
      OnDestroy();
      PostQuitMessage(0);
      break;

   default:
      return DefWindowProc(hWnd, message, wParam, lParam);
   }

   return 0;
}


////////////////////////////////
// Process mesages for about box
////////////////////////////////
LRESULT CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM /* lParam */)
{
   switch (message)
   {
   case WM_INITDIALOG:
      return TRUE;

   case WM_COMMAND:
      if (LOWORD(wParam) == IDOK)
      {
         EndDialog(hDlg, LOWORD(wParam));
         return TRUE;
      }
      break;
   }

   return FALSE;
}

/////////////////////////////
// WM_COMMAND message handler
/////////////////////////////
BOOL OnCommand (HWND hWnd, WORD wCommand, WORD /* wNotify */, HWND /* hControl */)
{
   switch (wCommand)
   {
   case IDM_OPEN:
      if (OpenFileDlg(szFileName, MAX_PATH, hWnd))
      {
         try
         {
            switch (nFilterIndex)
            {
            case 1:
               pImage->LoadBMP(szFileName);
               break;

            case 2:
               pImage->LoadPCX(szFileName);
               break;
            }

            UpdateScroll(hWnd);
            Invalidate(hWnd, TRUE);
         }
         catch (CGfxExcept e)
         {
            MessageBox(hWnd, LPCWSTR(e.GetErrMessage(hInst)), LPCWSTR("Open error"), MB_OK|MB_ICONSTOP);
         }
      }
      break;

   case IDM_SAVE:
      if (SaveFileDlg(szFileName, MAX_PATH, hWnd))
      {
         try
         {
            switch (nFilterIndex)
            {
            case 1:
               pImage->SaveBMP(szFileName);
               break;

            case 2:
               pImage->SavePCX(szFileName);
               break;
            }
         }
         catch (CGfxExcept e)
         {
            MessageBox(hWnd, LPCWSTR(e.GetErrMessage(hInst)), LPCWSTR("Save error"), MB_OK|MB_ICONSTOP);
         }
      }
      break;

   case IDM_COPY:
      pImage->CopyToClipboard(hWnd);
      break;

   case IDM_FREE:
      pImage->Free();
      UpdateScroll(hWnd);
      Invalidate(hWnd, TRUE);
      break;

   case IDM_ABOUT:
      DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUT), hWnd, (DLGPROC)DlgProc);
      break;

   case IDM_EXIT:
      DestroyWindow(hWnd);
      break;
   }

   return TRUE;
}

///////////////////////////
// WM_PAINT message handler
///////////////////////////
void OnPaint (HWND /* hWnd */, HDC hDC)
{
   pImage->Blit(hDC, nShiftX, nShiftY);
}

////////////////////////////////
// Initialization code goes here
////////////////////////////////
void OnInit ()
{
   pImage = new CImageDIB();
   
   //Default scrolling info
   HorzInfo.cbSize = VertInfo.cbSize = sizeof(SCROLLINFO);
   HorzInfo.nMin = VertInfo.nMin = 0;
   HorzInfo.fMask = VertInfo.fMask = SIF_ALL;
}

/////////////////////////////
// WM_DESTROY message handler
/////////////////////////////
void OnDestroy ()
{
   delete pImage;
}

//////////////////////////
// WM_SIZE message handler
//////////////////////////
void OnResize(HWND hWnd, WPARAM fwSizeType, int /* nWidth */, int /* nHeight */)
{
   if (fwSizeType != SIZE_MINIMIZED)
   {
      GetClientRect(hWnd, &clntRect); //Update size of window's client rectangle
      nShiftX = HorzInfo.nPos = nShiftY = VertInfo.nPos = 0; //Reset scrolling
      UpdateScroll(hWnd);
   }
}

/////////////////////////////
// WM_HSCROLL message handler
/////////////////////////////
void OnHScroll(HWND hWnd, int nScrollCode, short int nPos, HWND /* hWndScrollBar */)
{
   switch (nScrollCode)
   {
   case SB_LINELEFT:
   case SB_PAGELEFT:
      if (HorzInfo.nMin < -nShiftX) nShiftX++;
      break;

   case SB_LINERIGHT:
   case SB_PAGERIGHT:
      if (HorzInfo.nMax > -nShiftX) nShiftX--;
      break;

   case SB_THUMBTRACK:
      nShiftX = -nPos;
      break;

   default:
      break;
   }

   HorzInfo.nPos = -nShiftX;
   SetScrollInfo(hWnd, SB_HORZ, &HorzInfo, TRUE);
   Invalidate(hWnd);
}

/////////////////////////////
// WM_VSCROLL message handler
/////////////////////////////
void OnVScroll(HWND hWnd, int nScrollCode, short int nPos, HWND /* hWndScrollBar */)
{
   switch (nScrollCode)
   {
   case SB_LINEUP:
   case SB_PAGEUP:
      if (VertInfo.nMin < -nShiftY) nShiftY++;
      break;

   case SB_LINEDOWN:
   case SB_PAGEDOWN:
      if (VertInfo.nMax > -nShiftY) nShiftY--;
      break;

   case SB_THUMBTRACK:
      nShiftY = -nPos;
      break;

   default:
      break;
   }

   VertInfo.nPos = -nShiftY;
   SetScrollInfo(hWnd, SB_VERT, &VertInfo, TRUE);
   Invalidate(hWnd);
}

///////////////////////////////
// Force main window to repaint
///////////////////////////////
void Invalidate (HWND hWnd, BOOL bErase)
{
   UINT nFlags = RDW_INVALIDATE;

   if (bErase)
   {
      nFlags |= RDW_ERASE;
   }

   RedrawWindow(hWnd, NULL, NULL, nFlags);
}

/////////////////////
// Update scroll info
/////////////////////
void UpdateScroll (HWND hWnd)
{
   HorzInfo.nMax = pImage->GetWidth() - clntRect.right;
   SetScrollInfo(hWnd, SB_HORZ, &HorzInfo, TRUE);
   VertInfo.nMax = pImage->GetHeight() - clntRect.bottom;
   SetScrollInfo(hWnd, SB_VERT, &VertInfo, TRUE);
}

///////////////////
// Show open dialog
///////////////////
BOOL OpenFileDlg(char *pszFilename, int nLen, HWND hWnd)
{
   OPENFILENAME ofn;
   memset(&ofn, 0, sizeof(OPENFILENAME));
   ofn.lStructSize = 76;  //sizeof(OPENFILENAME) does not seem to work if
                          // compiled with C++ Builder
   ofn.hwndOwner = hWnd;
   ofn.lpstrFilter = LPCWSTR(szFilter);
   ofn.lpstrFile = LPWSTR(pszFilename);
   ofn.lpstrDefExt = LPCWSTR("bmp");
   ofn.nFilterIndex = nFilterIndex;
   ofn.nMaxFile = nLen;
   ofn.Flags = OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;

   BOOL ret = GetOpenFileName(&ofn);

   nFilterIndex = ofn.nFilterIndex;

   return ret;
}

///////////////////
// Show save dialog
///////////////////
BOOL SaveFileDlg(char *pszFilename, int nLen, HWND hWnd)
{
   OPENFILENAME ofn;
   memset(&ofn, 0, sizeof(OPENFILENAME));
   ofn.lStructSize = 76;  //sizeof(OPENFILENAME) does not seem to work if
                          // compiled with C++ Builder
   ofn.hwndOwner = hWnd;
   ofn.lpstrFilter = LPCWSTR(szFilter);
   ofn.lpstrFile = LPWSTR(pszFilename);
   ofn.lpstrDefExt = LPCWSTR("bmp"); //Default extension
   ofn.nFilterIndex = nFilterIndex;
   ofn.nMaxFile = nLen;
   ofn.Flags = OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY;

   BOOL ret = GetSaveFileName(&ofn);

   nFilterIndex = ofn.nFilterIndex;

   return ret;
}
