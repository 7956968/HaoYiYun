; �ű��� Inno Setup �ű������ɡ�
; �����ĵ���ȡ���� INNO SETUP �ű��ļ���ϸ����!

[Setup]
AppName={reg:HKLM\Software\HaoYiYun, AppName|��¼��}
AppVerName={reg:HKLM\Software\HaoYiYun, AppVerName|��¼��}
;AppPublisher={reg:HKLM\Software\GMBacklot, AppPublisher|�������Ϳ������ִ�ý�������޹�˾}
;AppPublisherURL={reg:HKLM\Software\GMBacklot, AppPublisherURL|http://www.kuihua.net}

DefaultGroupName={reg:HKLM\Software\HaoYiYun, DefaultGroupName|��¼��}
DefaultDirName={reg:HKLM\Software\HaoYiYun, DefaultDirName|{pf}\��¼��}

Compression=lzma
SolidCompression=yes
UsePreviousAppDir=no
UsePreviousGroup=no
;UsePreviousLanguage=no
AllowCancelDuringInstall=no
OutputDir=..\Product

VersionInfoVersion=1.0.0.1
OutputBaseFilename=��¼��

[Languages]
Name: "chinese"; MessagesFile: "compiler:Default.isl"

[Registry]
Root: HKLM; Subkey: "Software\HaoYiYun"; ValueType: string; ValueName: "DefaultDirName"; ValueData: "{app}"; Flags: uninsdeletekey

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}";

[Files]
Source: "..\Source\bin\HaoYi.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Source\bin\AudioRender.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Source\bin\D3DX9_43.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Source\bin\HCCore.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Source\bin\HCNetSDK.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Source\bin\libcurl.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Source\bin\PlayCtrl.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Source\bin\SuperRender.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Source\bin\zlib1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Source\bin\HCNetSDKCom\HCPreview.dll"; DestDir: "{app}\HCNetSDKCom"; Flags: ignoreversion
Source: "..\Source\bin\HCNetSDKCom\HCCoreDevCfg.dll"; DestDir: "{app}\HCNetSDKCom"; Flags: ignoreversion
Source: "..\Source\bin\HCNetSDKCom\HCGeneralCfgMgr.dll"; DestDir: "{app}\HCNetSDKCom"; Flags: ignoreversion

[Icons]
Name: "{userdesktop}\��¼��"; Filename: "{app}\HaoYi.exe"; Tasks: desktopicon
Name: "{group}\{cm:LaunchProgram,��¼��}"; Filename: "{app}\HaoYi.exe"
Name: "{group}\{cm:UninstallProgram,��¼��}"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\HaoYi.exe"; Description: "{cm:LaunchProgram,��¼��}"; Flags: nowait postinstall skipifsilent
