// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__4A442736_3BA3_420F_A77E_FE69AC6133CE__INCLUDED_)
#define AFX_STDAFX_H__4A442736_3BA3_420F_A77E_FE69AC6133CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <stdio.h>
#include <windows.h>

#pragma comment (lib, "WpdPack\\Lib\\wpcap.lib")
#pragma comment (lib, "Ws2_32.lib")

#ifdef WIN32
#pragma warning(disable: 4514 4786)
#endif

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__4A442736_3BA3_420F_A77E_FE69AC6133CE__INCLUDED_)
