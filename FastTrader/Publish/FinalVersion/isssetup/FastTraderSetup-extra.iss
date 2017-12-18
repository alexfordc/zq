; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{B62C9C3F-C052-46B9-AC28-6C17C273403B}
AppName=�й������ڻ����Ͻ����ն�(��׼��extra)
AppVerName=�й������ڻ����Ͻ����ն�(��׼��extra) 1.7.2.30
AppPublisher=CIFCO IT SERVICES CO.,LTD
AppPublisherURL=http://www.cifco.net
AppSupportURL=http://www.cifco.net
AppUpdatesURL=http://www.cifco.net
DefaultDirName={pf32}\�й������ڻ����Ͻ����ն�(��׼��extra)
DisableDirPage=no
DefaultGroupName=�й������ڻ����Ͻ����ն�(��׼��extra)
OutputDir=..\output
OutputBaseFilename=FastTraderSetup-extra
Compression=lzma
SolidCompression=yes
VersionInfoCompany=CIFCO ITSER VICES CO.,LTD
VersionInfoTextVersion=1.7.2.30
VersionInfoVersion=1.7.2.30
[Languages]
;Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "chs"; MessagesFile: "compiler:ChineseSimp.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
Source: "..\source\commonfiles\FastTrader.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\source\commonfiles\*"; DestDir: "{app}"; Excludes: ".svn,plugin"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\source\FastTrader-std-extra\*"; DestDir: "{app}"; Excludes: ".svn"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\�й������ڻ����Ͻ����ն�(��׼��extra)"; Filename: "{app}\FastTrader.exe"; WorkingDir:"{app}"
Name: "{group}\{cm:UninstallProgram,�й������ڻ����Ͻ����ն�(��׼��extra)}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\�й������ڻ����Ͻ����ն�(��׼��extra)"; Filename: "{app}\FastTrader.exe"; Tasks: desktopicon; WorkingDir:"{app}"

[Run]
Filename: "{app}\FastTrader.exe"; Description: "{cm:LaunchProgram,�й������ڻ����Ͻ����ն�(��׼��extra)}"; Flags: nowait postinstall skipifsilent runascurrentuser
Filename: "{app}\air.exe"; Parameters: "-silent"; WorkingDir: {app};  StatusMsg: "Installing Adobe AIR ..."
Filename: "{app}\CifcoServicesHall.exe"; Parameters: "-silent -desktopShortcut"; WorkingDir: {app};  StatusMsg: "Installing CifcoServicesHall ..."

[Code]

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);

begin

if CurUninstallStep = usUninstall then
DelTree(ExpandConstant('{app}'), True, True, True);

end;



































