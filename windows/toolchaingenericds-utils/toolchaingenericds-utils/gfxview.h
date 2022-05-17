#ifndef __gfxview_h__
#define __gfxview_h__

#include <windows.h>

BOOL InitInstance (HINSTANCE, int);
ATOM MyRegisterClass (HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM); //Main window
LRESULT CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM); //About dialog

//Message handler functions
void UpdateScroll(HWND hWnd);
void Invalidate(HWND hWnd, BOOL bErase = FALSE);
void OnInit    (void);
void OnDestroy (void);
void OnPaint   (HWND hWnd, HDC hDC);
void OnResize  (HWND hWnd, WPARAM fwSizeType, int nWidth, int nHeight);
void OnHScroll (HWND hWNd, int nScrollCode, short int nPos, HWND hWndScrollBar);
void OnVScroll (HWND hWnd, int nScrollCode, short int nPos, HWND hWndScrollBar);
BOOL OnCommand (HWND hWnd, WORD wCommand, WORD wNotify, HWND hControl);

BOOL OpenFileDlg (char *pszFilename, int nLen, HWND hWnd);
BOOL SaveFileDlg (char *pszFilename, int nLen, HWND hWnd);

#endif
