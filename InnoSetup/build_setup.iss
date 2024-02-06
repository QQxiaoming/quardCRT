
#define MyAppName "quardCRT"
#define MyAppVersion "#VERSION#"
#define MyAppVersionInfoVersion "#VERSIONINFOVERSION#"
#define MyAppPublisher "QQxiaoming <2014500726@smail.xtu.edu.cn>"
#define MyAppURL "https://github.com/QQxiaoming/quardCRT"
#define MyAppExeName "quardCRT.exe"
#define MyAppOutputName "quardCRT_setup"

[Setup]
AppId={{4368AAE0-A354-4127-94A9-0C38757B1DE5}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
OutputBaseFilename={#MyAppOutputName}
Compression=lzma
SolidCompression=yes

VersionInfoVersion={#MyAppVersionInfoVersion}
UninstallDisplayIcon={app}/{#MyAppExeName}
InfoBeforeFile=..\InnoSetup\Info.txt
OutPutdir=..\InnoSetup
SetupIconFile=..\icons\ico.ico
PrivilegesRequired=admin
AllowNoIcons=no
DisableProgramGroupPage=yes
WizardStyle=modern

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"
Name: "portuguese"; MessagesFile: "compiler:Languages\Portuguese.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "czech"; MessagesFile: "compiler:Languages\Czech.isl"
Name: "arabic"; MessagesFile: ".\Arabic.isl"
Name: "korean"; MessagesFile: ".\Korean.isl"
Name: "japanese"; MessagesFile: "compiler:Languages\Japanese.isl"
Name: "chinese"; MessagesFile: ".\ChineseSimplified.isl"
Name: "chinese_hk"; MessagesFile: ".\ChineseTraditional.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 6.1; Check: not IsAdminInstallMode

[Files]
Source: "..\InnoSetup\build\quardCRT.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\InnoSetup\build\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{#MyAppName}-mini"; Filename: "{app}\{#MyAppExeName}"; Parameters: "-m true"
Name: "{group}\{cm:ProgramOnTheWeb,{#MyAppName}}"; Filename: "{#MyAppURL}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[Registry]
Root: HKCR; Subkey: "Directory\shell\QuardCRT"; ValueType: string; ValueName: ""; ValueData: "Open with QuardCRT"; Flags: uninsdeletekey ;
Root: HKCR; Subkey: "Directory\shell\QuardCRT"; ValueType: string; ValueName: "Icon"; ValueData: "{app}\{#MyAppExeName},0"; Flags: uninsdeletekey
Root: HKCR; Subkey: "Directory\shell\QuardCRT\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""-m"" ""true"" ""-s"" ""%1"""; Flags: uninsdeletekey
Root: HKCR; Subkey: "Directory\Background\shell\QuardCRT"; ValueType: string; ValueName: ""; ValueData: "Open with QuardCRT"; Flags: uninsdeletekey
Root: HKCR; Subkey: "Directory\Background\shell\QuardCRT"; ValueType: string; ValueName: "Icon"; ValueData: "{app}\{#MyAppExeName},0"; Flags: uninsdeletekey
Root: HKCR; Subkey: "Directory\Background\shell\QuardCRT\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""-m"" ""true"" ""-s"" ""%V"""; Flags: uninsdeletekey
