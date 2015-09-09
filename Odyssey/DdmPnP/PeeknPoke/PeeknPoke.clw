; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CProgressStatusBar
LastTemplate=generic CWnd
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "PeeknPoke.h"
LastPage=0

ClassCount=9
Class1=CPeeknPokeApp
Class2=CPeeknPokeDoc
Class3=CPeeknPokeView
Class4=CMainFrame

ResourceCount=4
Resource1=IDD_ABOUTBOX
Class5=CAboutDlg
Class6=CMsgWnd
Class7=MsgWndView
Resource2=IDR_MAINFRAME
Class8=PortConfig
Resource3=IDD_DIALOG_CONFIG1
Class9=CProgressStatusBar
Resource4=IDD_DIALOG_CONFIG

[CLS:CPeeknPokeApp]
Type=0
HeaderFile=PeeknPoke.h
ImplementationFile=PeeknPoke.cpp
Filter=N
LastObject=CPeeknPokeApp
BaseClass=CWinApp
VirtualFilter=AC

[CLS:CPeeknPokeDoc]
Type=0
HeaderFile=PeeknPokeDoc.h
ImplementationFile=PeeknPokeDoc.cpp
Filter=N
BaseClass=CDocument
VirtualFilter=DC
LastObject=CPeeknPokeDoc

[CLS:CPeeknPokeView]
Type=0
HeaderFile=PeeknPokeView.h
ImplementationFile=PeeknPokeView.cpp
Filter=C
BaseClass=CScrollView
VirtualFilter=VWC
LastObject=CPeeknPokeView

[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
BaseClass=CFrameWnd
VirtualFilter=fWC
LastObject=CMainFrame



[CLS:CAboutDlg]
Type=0
HeaderFile=PeeknPoke.cpp
ImplementationFile=PeeknPoke.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_OPEN
Command2=ID_FILE_SAVE
Command3=ID_COMPILE
Command4=ID_FILE_MRU_FILE1
Command5=ID_APP_EXIT
Command6=ID_EDIT_UNDO
Command7=ID_EDIT_CUT
Command8=ID_EDIT_COPY
Command9=ID_EDIT_PASTE
Command10=ID_VIEW_TOOLBAR
Command11=ID_CONNECT
Command12=ID_DISCONNECT
Command13=ID_CONFIG_PORT
Command14=ID_APP_ABOUT
CommandCount=14

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_UNDO
Command5=ID_EDIT_CUT
Command6=ID_EDIT_COPY
Command7=ID_EDIT_PASTE
Command8=ID_EDIT_UNDO
Command9=ID_EDIT_CUT
Command10=ID_EDIT_COPY
Command11=ID_EDIT_PASTE
Command12=ID_NEXT_PANE
Command13=ID_PREV_PANE
CommandCount=13

[TB:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_OPEN
Command2=ID_FILE_SAVE
Command3=ID_CONFIG_PORT
Command4=ID_CONNECT
Command5=ID_DISCONNECT
Command6=ID_FILE_PRINT
Command7=ID_APP_ABOUT
CommandCount=7

[CLS:CMsgWnd]
Type=0
HeaderFile=MsgWnd.h
ImplementationFile=MsgWnd.cpp
BaseClass=generic CWnd
Filter=W
LastObject=CMsgWnd

[CLS:MsgWndView]
Type=0
HeaderFile=MsgWndView.h
ImplementationFile=MsgWndView.cpp
BaseClass=CEditView
Filter=C
LastObject=MsgWndView
VirtualFilter=VWC

[DLG:IDD_DIALOG_CONFIG]
Type=1
Class=PortConfig
ControlCount=9
Control1=IDC_STATIC,button,1342177287
Control2=IDC_COM1,button,1342373897
Control3=IDC_COM2,button,1342177289
Control4=IDC_COM3,button,1342177289
Control5=IDC_COM4,button,1342177289
Control6=IDC_COM5,button,1342177289
Control7=IDC_COM6,button,1342177289
Control8=IDOK,button,1342242817
Control9=IDCANCEL,button,1342242816

[CLS:PortConfig]
Type=0
HeaderFile=PortConfig.h
ImplementationFile=PortConfig.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=PortConfig

[CLS:CProgressStatusBar]
Type=0
HeaderFile=ProgressStatusBar.h
ImplementationFile=ProgressStatusBar.cpp
BaseClass=CStatusBar
Filter=W
LastObject=CProgressStatusBar

[DLG:IDD_DIALOG_CONFIG1]
Type=1
ControlCount=20
Control1=IDC_STATIC,button,1342177287
Control2=IDC_COM1,button,1342373897
Control3=IDC_COM2,button,1342177289
Control4=IDC_COM3,button,1342177289
Control5=IDC_COM4,button,1342177289
Control6=IDC_BAUD,combobox,1344339971
Control7=IDC_DATA,combobox,1344339971
Control8=IDC_PARITY,combobox,1344339971
Control9=IDC_STOP_BITS,combobox,1344339971
Control10=IDOK,button,1342242817
Control11=IDCANCEL,button,1342242816
Control12=IDC_STATIC,button,1342177287
Control13=IDC_STATIC,static,1342308352
Control14=IDC_STATIC,static,1342308352
Control15=IDC_STATIC,static,1342308352
Control16=IDC_STATIC,static,1342308352
Control17=IDC_STATIC,static,1342308352
Control18=IDC_FLOW_CONTROL,combobox,1344339971
Control19=IDC_COM5,button,1342177289
Control20=IDC_COM6,button,1342177289

