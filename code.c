/*

ODBC .
Copyright (C) 2013 chenxiang.

*/

#include "code.h"

/*
Global variable.
*/

HINSTANCE hInstance;
HWND	hWinMain;
HWND	hClient;
HMENU	hMenu;
HICON	hIcon;

LONG	oldClient;

typedef struct _SQL_HD{
SQLHANDLE	hEnv;
SQLHANDLE	hDbc;
} SQL_HD;

SQL_HD*	pSql;

/*
Global data.
*/

TCHAR	*classname=TEXT("Sql DB Lite");
TCHAR	*childclassname=TEXT("sql_db_child_window");

LRESULT CALLBACK FrameProc(HWND ,UINT ,WPARAM ,LPARAM);
LRESULT	CALLBACK ChildProc(HWND ,UINT ,WPARAM ,LPARAM);
LRESULT	CALLBACK ClientProc(HWND ,UINT ,WPARAM ,LPARAM);

int		initialize_child_window(HWND);
int		resize_child_controls(HWND);
int		add_output_text(HWND,TCHAR*);
int		initialize_dbc(HWND,TCHAR*);
void	database_connect_stmt(HWND);
int		update_status(int,TCHAR*);

int WINAPI	WinMain(HINSTANCE hInst,HINSTANCE hPreInst,LPSTR lpCmdLn,int nCmdShow)
{
	WNDCLASSEX	wc;
	HINSTANCE	hMod=0;
	
	InitCommonControls();
	hMenu=LoadMenu(hInst,MAKEINTRESOURCE(IDM_MENU));
	hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDM_ICON));
	hMod=LoadLibrary(TEXT("RichEd20.dll"));
	
	ZeroMemory(&wc,sizeof(WNDCLASSEX));
	wc.cbSize=sizeof(WNDCLASSEX);
	wc.style=CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc=FrameProc;
	wc.hInstance=hInstance=hInst;
	wc.hCursor=LoadCursor(0,IDC_ARROW);
	wc.hIconSm=wc.hIcon=hIcon;
	wc.lpszMenuName=MAKEINTRESOURCE(IDM_MENU);
	wc.lpszClassName=classname;
	if(RegisterClassEx(&wc)){
		if(hWinMain=CreateWindowEx(0,classname,classname,WS_OVERLAPPEDWINDOW,\
			CW_USEDEFAULT,CW_USEDEFAULT,800,600,0,hMenu,hInst,0)){
			ShowWindow(hWinMain,SW_SHOW);UpdateWindow(hWinMain);
		}
		/*
		Register Child Window Class.
		*/
		wc.lpfnWndProc=ChildProc;
		wc.hIconSm=wc.hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDM_ICONS));
		wc.lpszClassName=childclassname;
		wc.hbrBackground=(HBRUSH)(COLOR_WINDOW);
		if(RegisterClassEx(&wc)){
			while(GetMessage((MSG*)&wc,0,0,0)){
				TranslateMessage((MSG*)&wc);DispatchMessage((MSG*)&wc);
			}
		} else MessageBox(0,TEXT("Fail Register Child Class"),0,0x10);
	} else MessageBox(0,TEXT("Fail Register Parent Class."),0,0x10);
	if(hMod)FreeLibrary(hMod);
	ExitProcess(0);
}

LRESULT	CALLBACK	FrameProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	CLIENTCREATESTRUCT	ccs;
	RECT	rc;
	HWND 	hChild;
	int		iParts[3]={500,600,-1};
	switch(uMsg){
	case WM_SIZE:
		if(GetClientRect(hWnd,&rc))MoveWindow(hClient,rc.left,rc.top,rc.right,rc.bottom,1);
		MoveWindow(GetDlgItem(hWnd,100),0,0,0,0,1);
		return 0;
	break;
	case WM_CREATE:
		ccs.hWindowMenu=GetSubMenu(hMenu,1);
		ccs.idFirstChild=IDI_FIRSTCHILD;
		if(!(hClient=CreateWindowEx(0,TEXT("MDICLIENT"),0,WS_CHILD | WS_VISIBLE | WS_VISIBLE | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL,\
			0,0,0,0,hWnd,0,hInstance,&ccs)))SendMessage(hWnd,WM_CLOSE,0,0); /*Dont forget CLIENTCREATESTRUCT.*/
		oldClient=SetWindowLongPtr(hClient,GWL_WNDPROC,(LONG_PTR)ClientProc);
		SendMessage(CreateStatusWindow(SBARS_SIZEGRIP | WS_CHILD | WS_VISIBLE,0,hWnd,100),SB_SETPARTS,3,(LPARAM)iParts);
	break;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDM_FILE_NEW:
			if(hClient)if(hChild=CreateWindowEx(WS_EX_MDICHILD,childclassname,0,MDIS_ALLCHILDSTYLES,\
				CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,hClient,0,hInstance,0)){
				ShowWindow(hChild,SW_SHOW);UpdateWindow(hChild);
			};
		break;
		case IDM_HELP_ABOUT:
			ShellAbout(hWnd,TEXT("About#Sql DB Lite"),TEXT("Copyright(C)2013 chenxiang"),hIcon);
		break;
		case IDM_WINDOW_NEXT:
			SendMessage(hClient,WM_MDINEXT,0,0);
		break;
		case IDM_WINDOW_CASCADE:
			SendMessage(hClient,WM_MDICASCADE,MDITILE_ZORDER,0);
		break;
		case IDM_WINDOW_HORI:
			SendMessage(hClient,WM_MDITILE,MDITILE_HORIZONTAL,0);
		break;
		case IDM_WINDOW_VERT:
			SendMessage(hClient,WM_MDITILE,MDITILE_VERTICAL,0);
		break;
		case IDM_WINDOW_CLOSEALL:
			___closeallchildren:
			while(hChild=(HWND)SendMessage(hClient,WM_MDIGETACTIVE,0,0))SendMessage(hChild,WM_CLOSE,0,0);
		break;
		case IDM_FILE_EXIT:
			SendMessage(hWnd,WM_CLOSE,0,0);
		break;
		}
		//goto __defaultparent; /* MUST ADD ,because Frame will process it.Such as,"minimize"\"maximize"\"close",these button on the top right corner of title.*/
	break;
	case WM_CLOSE:
		if(pSql){
			if(pSql->hEnv||pSql->hDbc){
				if(MessageBox(hWnd,TEXT("Database connection still connectted,force close it now?"),TEXT("Are you sure."),MB_OKCANCEL)==IDOK){
					PostQuitMessage(0);goto ___closeallchildren;
				} else return 0;
			}
		}
		PostQuitMessage(0);
	break;
	}
	return DefFrameProc(hWnd,hClient,uMsg,wParam,lParam);
}

LRESULT	CALLBACK	ChildProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	OPENFILENAME of;
	LONG_PTR	ret;
	TCHAR	buf[260]={0};
	switch(uMsg){
	case WM_CREATE:
		initialize_child_window(hWnd);
	break;
	case WM_MDIACTIVATE:
		ret=GetWindowLongPtr(hWnd,GWLP_USERDATA);
		if(ret){
			pSql=(SQL_HD*)ret;
		}
		if(SendMessage(hWnd,WM_GETTEXT,(WPARAM)sizeof(buf),(LPARAM)buf))update_status(0,buf);
		if(!IsWindowEnabled(GetDlgItem(hWnd,IDC_CHILD_BT)))update_status(1,TEXT("Disconnected"));
		else update_status(1,TEXT("Connected"));
	break;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDC_CHILDBT: /*view filename*/
				ZeroMemory(&of,sizeof(OPENFILENAME));
				GetCurrentDirectory(sizeof(buf),buf);
				lstrcat(buf,TEXT("\\test.mdb"));
				of.lStructSize=sizeof(OPENFILENAME);
				of.hwndOwner=hWnd;
				of.hInstance=hInstance;
				of.lpstrFilter=TEXT("All Files(*.*)\0*.*\0");
				of.lpstrFile=buf;
				of.nMaxFile=260;
				if(GetOpenFileName(&of)){
					initialize_dbc(hWnd,buf);
				}
		break;
		case IDC_CHILD_BT: /*execute statement*/
			database_connect_stmt(hWnd);
		break;
		case IDC_CHILD_CLEAN:
			ret=0;
			SendDlgItemMessage(hWnd,IDC_CHILD_EDIT,WM_SETTEXT,0,(LPARAM)&ret);
		break;
		case IDC_CHILD_CONNECT:
			if(SendDlgItemMessage(hWnd,IDC_CHILDEDIT,WM_GETTEXT,sizeof(buf),(LPARAM)buf)){
				initialize_dbc(hWnd,buf);
			}
		break;
		case IDC_CHILD_BTDIS:
			goto	___disconnect_db;
		break;
		}
	break;
	case WM_SIZE:
		resize_child_controls(hWnd);
	break;
	case WM_CLOSE:
		___disconnect_db:
		if(pSql){
			if(pSql->hDbc){
				SQLEndTran(SQL_HANDLE_DBC,pSql->hDbc,SQL_COMMIT);
				SQLDisconnect(pSql->hDbc);
				SQLFreeHandle(SQL_HANDLE_DBC,pSql->hDbc);
				pSql->hDbc=0;
			}
			if(pSql->hEnv){
				SQLFreeHandle(SQL_HANDLE_ENV,pSql->hEnv);
				pSql->hEnv=0;
			}
		}
		update_status(0,TEXT(" "));
		update_status(1,TEXT("Disconnected"));
		EnableWindow(GetDlgItem(hWnd,IDC_CHILD_BTDIS),0);
		EnableWindow(GetDlgItem(hWnd,IDC_CHILD_BT),0);
		EnableWindow(GetDlgItem(hWnd,IDC_CHILD_CONNECT),1);
	break;
	}
	return DefMDIChildProc(hWnd,uMsg,wParam,lParam);
}

LRESULT CALLBACK	ClientProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg){
	default:
		return CallWindowProc((WNDPROC)oldClient,hWnd,uMsg,wParam,lParam);
	}
	return 0;
}

int initialize_child_window(HWND hWnd)
{
	LOGFONT	font;
	HFONT	hf;
	
	SendMessage(hWnd,WM_SETTEXT,0,(LPARAM)TEXT("New SQL Task"));
	if(hWnd){
		CreateWindowEx(0,TEXT("COMBOBOX"),0,CBS_HASSTRINGS | CBS_AUTOHSCROLL | CBS_DROPDOWN | WS_VSCROLL | WS_CHILD | WS_VISIBLE,0,0,0,120,hWnd,(HMENU)IDC_CHILDCOMBO,hInstance,0);
		CreateWindowEx(0,TEXT("EDIT"),0,WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL,0,0,0,0,hWnd,(HMENU)IDC_CHILDEDIT,hInstance,0);
		CreateWindowEx(0,TEXT("BUTTON"),TEXT("..."),WS_VISIBLE | WS_CHILD,0,0,0,0,hWnd,(HMENU)IDC_CHILDBT,hInstance,0);
		CreateWindowEx(0,RICHEDIT_CLASS,0,ES_MULTILINE | WS_VISIBLE | WS_CHILD | WS_HSCROLL | WS_VSCROLL,0,0,0,0,hWnd,(HMENU)IDC_CHILD_EDIT,hInstance,0);
		CreateWindowEx(0,TEXT("BUTTON"),TEXT("E&xecute"),WS_DISABLED | WS_VISIBLE | WS_CHILD,0,0,0,0,hWnd,(HMENU)IDC_CHILD_BT,hInstance,0);
		CreateWindowEx(0,TEXT("BUTTON"),TEXT("&Disjoin"),WS_DISABLED | WS_VISIBLE | WS_CHILD,0,0,0,0,hWnd,(HMENU)IDC_CHILD_BTDIS,hInstance,0);
		CreateWindowEx(0,TEXT("BUTTON"),TEXT("&Clean"),WS_VISIBLE | WS_CHILD,0,0,0,0,hWnd,(HMENU)IDC_CHILD_CLEAN,hInstance,0);
		CreateWindowEx(0,TEXT("BUTTON"),TEXT("C&onnect"),WS_VISIBLE | WS_CHILD,0,0,0,0,hWnd,(HMENU)IDC_CHILD_CONNECT,hInstance,0);
		CreateWindowEx(0,TEXT("SysListView32"),0,WS_VISIBLE | WS_CHILD | WS_HSCROLL | WS_VSCROLL | LVS_REPORT,0,0,0,0,hWnd,(HMENU)IDC_CHILDLIST,hInstance,0);
		CreateWindowEx(0,TEXT("LISTBOX"),0,LBS_HASSTRINGS	| WS_VSCROLL | WS_HSCROLL | WS_VISIBLE | WS_CHILD,0,0,0,0,hWnd,(HMENU)IDC_CHILD_OUTPUT,hInstance,0);
	}
	ZeroMemory(&font,sizeof(LOGFONT));
	font.lfHeight=-13;
	font.lfWeight=500;
	lstrcpy((TCHAR*)&font.lfFaceName,TEXT("Consolas"));
	if(hf=CreateFontIndirect(&font)){
		SendDlgItemMessage(hWnd,IDC_CHILDCOMBO,WM_SETFONT,(WPARAM)hf,1);
		SendDlgItemMessage(hWnd,IDC_CHILDEDIT,WM_SETFONT,(WPARAM)hf,1);
		SendDlgItemMessage(hWnd,IDC_CHILDBT,WM_SETFONT,(WPARAM)hf,1);
		SendDlgItemMessage(hWnd,IDC_CHILD_EDIT,WM_SETFONT,(WPARAM)hf,1);
		SendDlgItemMessage(hWnd,IDC_CHILD_BT,WM_SETFONT,(WPARAM)hf,1);
		SendDlgItemMessage(hWnd,IDC_CHILD_BTDIS,WM_SETFONT,(WPARAM)hf,1);
		SendDlgItemMessage(hWnd,IDC_CHILDLIST,WM_SETFONT,(WPARAM)hf,1);
		SendDlgItemMessage(hWnd,IDC_CHILD_OUTPUT,WM_SETFONT,(WPARAM)hf,1);
		SendDlgItemMessage(hWnd,IDC_CHILD_CLEAN,WM_SETFONT,(WPARAM)hf,1);
		SendDlgItemMessage(hWnd,IDC_CHILD_CONNECT,WM_SETFONT,(WPARAM)hf,1);
	}
	{
		SendDlgItemMessage(hWnd,IDC_CHILDCOMBO,CB_ADDSTRING,0,(LPARAM)TEXT("Microsoft Access Driver (*.mdb)"));
		SendDlgItemMessage(hWnd,IDC_CHILDCOMBO,CB_SETCURSEL,0,0);
	}
	SendDlgItemMessage((HWND)hWnd,IDC_CHILDLIST,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP | LVS_EX_GRIDLINES);
	return 0;
}

int resize_child_controls(HWND hWnd)
{
	RECT	rc;
	HWND	hCtrl=0;
	if(GetClientRect(hWnd,&rc)){
		if(hCtrl=GetDlgItem(hWnd,IDC_CHILDCOMBO))MoveWindow(hCtrl,rc.left+3,rc.top+3,100,22,1);
		if(hCtrl=GetDlgItem(hWnd,IDC_CHILDEDIT))MoveWindow(hCtrl,rc.left+106,rc.top+3,rc.right-40-106,22,1);
		if(hCtrl=GetDlgItem(hWnd,IDC_CHILDBT))MoveWindow(hCtrl,rc.right-36,rc.top+3,30,22,1);
		if(hCtrl=GetDlgItem(hWnd,IDC_CHILD_EDIT))MoveWindow(hCtrl,rc.left+3,rc.top+28,rc.right-160,47,1);
		if(hCtrl=GetDlgItem(hWnd,IDC_CHILD_BT))MoveWindow(hCtrl,rc.right-76,rc.top+28,70,22,1);
		if(hCtrl=GetDlgItem(hWnd,IDC_CHILD_BTDIS))MoveWindow(hCtrl,rc.right-76,rc.top+53,70,22,1);
		if(hCtrl=GetDlgItem(hWnd,IDC_CHILD_CLEAN))MoveWindow(hCtrl,rc.right-152,rc.top+28,70,22,1);
		if(hCtrl=GetDlgItem(hWnd,IDC_CHILD_CONNECT))MoveWindow(hCtrl,rc.right-152,rc.top+53,70,22,1);
		if(hCtrl=GetDlgItem(hWnd,IDC_CHILDLIST))MoveWindow(hCtrl,rc.left+3,rc.top+78,rc.right-6,rc.bottom-(rc.top+50)-3-83,1);
		if(hCtrl=GetDlgItem(hWnd,IDC_CHILD_OUTPUT))MoveWindow(hCtrl,rc.left+3,rc.top+78+rc.bottom-(rc.top+50)-3-80,rc.right-6,50,1);
	}
	return 0;
}

int update_status(int index,TCHAR* str)
{
	SendMessage(GetDlgItem(hWinMain,100),SB_SETTEXT,(WPARAM)index,(LPARAM)str);
	return 0;
}

#include "misc.c"
#include "core.c"
