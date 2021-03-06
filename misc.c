int add_output_text(HWND hWnd,TCHAR* buf)
{
	return SendDlgItemMessage(hWnd,IDC_CHILD_OUTPUT,LB_ADDSTRING,0,(LPARAM)buf);
}

int initialize_dbc(HWND hWnd,TCHAR* filename)
{
	SQLRETURN ret;
	SQLSMALLINT tmp=0;
	TCHAR	buf1[260]={0},buf2[260]={0};
	
	if(pSql=(SQL_HD*)malloc(sizeof(SQL_HD))){
		ret=SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&pSql->hEnv);
		if((ret==SQL_SUCCESS)||(ret==SQL_SUCCESS_WITH_INFO)){
			SQLSetEnvAttr(pSql->hEnv,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)SQL_OV_ODBC3,0);
			ret=SQLAllocHandle(SQL_HANDLE_DBC,pSql->hEnv,&pSql->hDbc);
			if((ret==SQL_SUCCESS)||(ret==SQL_SUCCESS_WITH_INFO)){
				SQLSetConnectAttr(pSql->hDbc,SQL_ATTR_AUTOCOMMIT,SQL_AUTOCOMMIT_OFF,0);
				if(!SendDlgItemMessage(hWnd,IDC_CHILDCOMBO,WM_GETTEXT,sizeof(buf2),(LPARAM)buf2)){
					add_output_text(hWnd,TEXT("please select a driver type!"));
					goto ___error;
				}
				wsprintf(buf1,TEXT("Driver={%s};dbq=%s"),buf2,filename);
				add_output_text(hWnd,buf1);
				SendDlgItemMessage(hWnd,IDC_CHILDEDIT,WM_SETTEXT,0,(LPARAM)filename);
				MessageBeep(0x40);
				EnableWindow(GetDlgItem(hWnd,IDC_CHILD_BTDIS),1);
				buf2[0]=0;
				ret=SQLDriverConnect(pSql->hDbc,hWnd,buf1,sizeof(buf2),buf2,sizeof(buf2),&tmp,SQL_DRIVER_COMPLETE);
				if((ret==SQL_SUCCESS)||(ret==SQL_SUCCESS_WITH_INFO)){
					SendMessage(hWnd,WM_SETTEXT,0,(LPARAM)filename);
					update_status(0,filename);
					update_status(1,TEXT("Connected"));
					EnableWindow(GetDlgItem(hWnd,IDC_CHILD_CONNECT),0);
					EnableWindow(GetDlgItem(hWnd,IDC_CHILD_BT),1);
					add_output_text(hWnd,buf2);
				} else {
					SQLGetDiagRec(SQL_HANDLE_STMT,pSql->hDbc,1,buf1,(SQLINTEGER*)&tmp,buf2,sizeof(buf2),(SQLSMALLINT*)&tmp);
					add_output_text(hWnd,buf2);
					goto ___error;
				}
			} else {
				add_output_text(hWnd,TEXT("Fail allocate Connection Handle!"));
				goto ___error;
			}
		} else {
			add_output_text(hWnd,TEXT("Fail allocate Environment Handle!"));
			goto ___error;
		}
		SetWindowLongPtr(hWnd,GWLP_USERDATA,(LONG_PTR)pSql);
	}
	return 0;
	___error:
	MessageBeep(0x30);
	add_output_text(hWnd,TEXT("Error occurred!"));
	return -1;
}
