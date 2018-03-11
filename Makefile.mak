RES=rsrc.res
BIN=code.exe
OBJ=code.obj
MANI=code.exe.embed.manifest
PARAMS_U=/c /nologo /Zc:wchar_t /D UNICODE /D _UNICODE
PARAMS=/c /nologo

$(BIN):$(RES) $(OBJ)
	link /subsystem:windows /nologo /fixed /merge:.rdata=.text /out:"$(BIN)" /merge:.data=.text /section:.text,wer /machine:x86 $(RES) $(OBJ)

.rc.res:
	rc /v $<
	
.c.obj:
	cl  $(PARAMS) $<

clean:
	del /f $(BIN)
	del /f $(RES)
	del /f $(OBJ)
