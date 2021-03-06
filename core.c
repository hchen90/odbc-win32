
void add_listview_column(HWND hWnd,int index,TCHAR* str)
{
	LV_COLUMN lc;
	
	ZeroMemory(&lc,sizeof(LV_COLUMN));
	lc.mask=LVCF_TEXT | LVCF_WIDTH;
	lc.cchTextMax=260;
	lc.pszText=str;
	lc.cx=100;
	SendDlgItemMessage(hWnd,IDC_CHILDLIST,LVM_INSERTCOLUMN,index,(LPARAM)&lc);
}

void add_listview_item(HWND hWnd,int item,int subitem,TCHAR* str)
{
	LV_ITEM li;
	
	ZeroMemory(&li,sizeof(LV_ITEM));
	li.mask=LVIF_TEXT;
	li.cchTextMax=260;
	li.pszText=str;
	if(!subitem){
		li.iItem=item;
		li.iSubItem=0;
		SendDlgItemMessage(hWnd,IDC_CHILDLIST,LVM_INSERTITEM,0,(LPARAM)&li);
	} else {
		li.iItem=item;
		li.iSubItem=subitem;
		SendDlgItemMessage(hWnd,IDC_CHILDLIST,LVM_SETITEM,0,(LPARAM)&li);
	}
}

void database_connect_stmt(HWND hWnd)
{
	SQLHANDLE hStmt;
	SQLRETURN ret;
	SQLSMALLINT	column=0,namesize,datatype,digits,nullable=SQL_NULLABLE;
	SQLINTEGER	row=0;
	SQLULEN	colen,slen;
	TCHAR	buf[260]={0},buf1[260]={0};
	int		i,dt,item=0,subitem=0;
	
	SendDlgItemMessage(hWnd,IDC_CHILDLIST,LVM_DELETEALLITEMS,0,0);
	do {
		dt=SendDlgItemMessage(hWnd,IDC_CHILDLIST,LVM_DELETECOLUMN,0,0);
	} while(dt);
	ret=SQLAllocHandle(SQL_HANDLE_STMT,pSql->hDbc,&hStmt);
	if(ret==SQL_SUCCESS||ret==SQL_SUCCESS_WITH_INFO){
		ret=SQLSetStmtAttr(hStmt,SQL_ATTR_CURSOR_TYPE,(SQLPOINTER)SQL_CURSOR_STATIC,0);
		if(ret==SQL_SUCCESS||ret==SQL_SUCCESS_WITH_INFO||ret==SQL_NO_DATA){
			SendDlgItemMessage(hWnd,IDC_CHILD_EDIT,WM_GETTEXT,sizeof(buf),(LPARAM)buf);
			ret=SQLExecDirect(hStmt,buf,sizeof(buf));
			if(ret==SQL_SUCCESS||ret==SQL_SUCCESS_WITH_INFO){
				ret=SQLNumResultCols(hStmt,&column);
				if(!column){
					ret=SQLRowCount(hStmt,&row);
					if(row!=-1){
						add_output_text(hWnd,TEXT("DDL/DCL done."));
					} else {
						wsprintf(buf,TEXT("DML done[%d]."),row);
						add_output_text(hWnd,buf);
					}
				} else {
					add_output_text(hWnd,TEXT("DQL done."));
					for(i=1;i<=column;i++){
						ret=SQLDescribeCol(hStmt,i,buf,sizeof(buf),&namesize,&datatype,&colen,&digits,&nullable);
						if(ret==SQL_SUCCESS||ret==SQL_SUCCESS_WITH_INFO){
							add_listview_column(hWnd,i-1,buf);
						}
					}
					for(i=1;i<=column;i++){
						ret=SQLBindCol(hStmt,i,SQL_C_CHAR,buf,sizeof(buf),&slen);
						if(ret==SQL_SUCCESS||ret==SQL_SUCCESS_WITH_INFO){
							ret=SQLFetchScroll(hStmt,SQL_FETCH_FIRST,0);
							if(ret==SQL_SUCCESS||ret==SQL_SUCCESS_WITH_INFO){
								//MessageBox(0,buf,buf,0);
								add_listview_item(hWnd,item,subitem,buf);
								item++;
								do{
									switch(SQLFetchScroll(hStmt,SQL_FETCH_NEXT,0)){
									case SQL_SUCCESS:
									case SQL_SUCCESS_WITH_INFO:
										//MessageBox(0,buf,buf,0);
										add_listview_item(hWnd,item,subitem,buf);
										item++;
										dt=1;
									break;
									case SQL_NO_DATA:
										dt=0;
									break;
									}
								} while(dt);
								dt=1;
							}
							SQLFreeStmt(hStmt,SQL_UNBIND);
							item=0;
							subitem++;
						} else {
							MessageBeep(0x30);
							SQLGetDiagRec(SQL_HANDLE_STMT,hStmt,1,buf1,(SQLINTEGER*)&dt,buf,sizeof(buf),(SQLSMALLINT*)&dt);
							add_output_text(hWnd,buf);
						}
					}
				}
			} else {
				MessageBeep(0x30);
				SQLGetDiagRec(SQL_HANDLE_STMT,hStmt,1,buf1,(SQLINTEGER*)&dt,buf,sizeof(buf),(SQLSMALLINT*)&dt);
				add_output_text(hWnd,buf);
			}
			SQLCloseCursor(hStmt);
		}
		SQLFreeHandle(SQL_HANDLE_STMT,hStmt);
	}
}