; Script generated by the Inno Script Studio Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "CW3M_McKenzie"
#define MyAppVersion "McKenzie_0.3.2"
;#define MyAppVersion "x.x.x"
#define MyAppPublisher "Oregon Freshwater Simulations, Inc."
#define MyAppURL "http:/www.freshwatersim.com"
#define MyAppExeName "CW3M.exe"
;#define DataDir "\CW3M"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{A0F76A09-D1B0-4625-BB1D-3E7593933A63}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
OutputBaseFilename=CW3M_Installer_{#MyAppVersion}
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Files]
; NOTE: Don't use "Flags: ignoreversion" on any shared system files
Source: "D:\CW3M.git\trunk\SourceCode\x64\Release\CW3M.exe"; DestDir: "{app}"; DestName: "CW3M.exe"; Flags: ignoreversion
Source: "D:\CW3M.git\trunk\SourceCode\x64\Release\APs.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "D:\CW3M.git\trunk\SourceCode\x64\Release\Flow.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "D:\CW3M.git\trunk\SourceCode\x64\Release\HBV.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "D:\CW3M.git\trunk\SourceCode\x64\Release\Libs.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "D:\CW3M.git\trunk\SourceCode\x64\Release\Modeler.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "D:\CW3M.git\trunk\SourceCode\x64\Release\Reporter.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "D:\CW3M.git\trunk\SourceCode\x64\Release\SpatialAllocator.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "D:\CW3M.git\trunk\SourceCode\x64\Release\UGMFC64.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "D:\CW3M.git\trunk\SourceCode\x64\Release\VegSTM.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "D:\CW3M.git\trunk\SourceCode\x64\Release\WinLibs.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "D:\CW3M.git\trunk\GDAL\*.dll"; DestDir: "{app}\GDAL"; Flags: ignoreversion
Source: "D:\CW3M.git\trunk\GDAL\gdalplugins\*"; DestDir: "{app}\GDAL\gdalplugins"; Flags: ignoreversion
Source: "D:\CW3M.git\trunk\GDAL\gdal-data\*"; DestDir: "{app}\GDAL\gdal-data"; Flags: ignoreversion

Source: "D:\CW3M.git\trunk\DataCW3M\Observations\*"; DestDir: "\CW3M_{#MyAppVersion}\Observations"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Observations\jd_csv\*"; DestDir: "\CW3M_{#MyAppVersion}\Observations\jd_csv"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Observations\IncludingLeapDays\*"; DestDir: "\CW3M_{#MyAppVersion}\Observations\IncludingLeapDays"; 

Source: "D:\CW3M.git\trunk\DataCW3M\Reservoirs\Area_Capacity_Curves\*"; DestDir: "\CW3M_{#MyAppVersion}\Reservoirs\Area_Capacity_Curves"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Reservoirs\ControlPoints\*"; DestDir: "\CW3M_{#MyAppVersion}\Reservoirs\ControlPoints"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Reservoirs\Output_from_ResSim\*"; DestDir: "\CW3M_{#MyAppVersion}\Reservoirs\Output_from_ResSim"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Reservoirs\Rel_Cap\*"; DestDir: "\CW3M_{#MyAppVersion}\Reservoirs\Rel_Cap"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Reservoirs\Rule_Curves\*"; DestDir: "\CW3M_{#MyAppVersion}\Reservoirs\Rule_Curves"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Reservoirs\Rules_BR\*"; DestDir: "\CW3M_{#MyAppVersion}\Reservoirs\Rules_BR"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Reservoirs\Rules_CG\*"; DestDir: "\CW3M_{#MyAppVersion}\Reservoirs\Rules_CG"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Reservoirs\Rules_Cougar\*"; DestDir: "\CW3M_{#MyAppVersion}\Reservoirs\Rules_Cougar"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Reservoirs\Rules_Detroit\*"; DestDir: "\CW3M_{#MyAppVersion}\Reservoirs\Rules_Detroit"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Reservoirs\Rules_Dorena\*"; DestDir: "\CW3M_{#MyAppVersion}\Reservoirs\Rules_Dorena"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Reservoirs\Rules_FC\*"; DestDir: "\CW3M_{#MyAppVersion}\Reservoirs\Rules_FC"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Reservoirs\Rules_Foster\*"; DestDir: "\CW3M_{#MyAppVersion}\Reservoirs\Rules_Foster"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Reservoirs\Rules_FR\*"; DestDir: "\CW3M_{#MyAppVersion}\Reservoirs\Rules_FR"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Reservoirs\Rules_GP\*"; DestDir: "\CW3M_{#MyAppVersion}\Reservoirs\Rules_GP"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Reservoirs\Rules_HC\*"; DestDir: "\CW3M_{#MyAppVersion}\Reservoirs\Rules_HC"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Reservoirs\Rules_Lookout\*"; DestDir: "\CW3M_{#MyAppVersion}\Reservoirs\Rules_Lookout"; 

Source: "D:\CW3M.git\trunk\DataCW3M\APs.xml"; DestDir: "\CW3M_{#MyAppVersion}"; 
Source: "D:\CW3M.git\trunk\DataCW3M\cooling_cost.csv"; DestDir: "\CW3M_{#MyAppVersion}"; 
Source: "D:\CW3M.git\trunk\DataCW3M\cropchoice.csv"; DestDir: "\CW3M_{#MyAppVersion}"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Crops.csv"; DestDir: "\CW3M_{#MyAppVersion}"; 
Source: "D:\CW3M.git\trunk\DataCW3M\deterministic_transition_lookup.csv"; DestDir: "\CW3M_{#MyAppVersion}"; 
Source: "D:\CW3M.git\trunk\DataCW3M\FullCostUrb_commercial_industrial_prices.csv"; DestDir: "\CW3M_{#MyAppVersion}"; 
Source: "D:\CW3M.git\trunk\DataCW3M\FullCostUrb_residential_prices.csv"; DestDir: "\CW3M_{#MyAppVersion}"; 
Source: "D:\CW3M.git\trunk\DataCW3M\HBV.csv"; DestDir: "\CW3M_{#MyAppVersion}"; 
Source: "D:\CW3M.git\trunk\DataCW3M\HBV_PEST.csv"; DestDir: "\CW3M_{#MyAppVersion}"; 
Source: "D:\CW3M.git\trunk\DataCW3M\HRU.xml"; DestDir: "\CW3M_{#MyAppVersion}"; 
Source: "D:\CW3M.git\trunk\DataCW3M\IDU.xml"; DestDir: "\CW3M_{#MyAppVersion}"; 
Source: "D:\CW3M.git\trunk\DataCW3M\lulc.xml"; DestDir: "\CW3M_{#MyAppVersion}"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Modeler.xml"; DestDir: "\CW3M_{#MyAppVersion}"; 
Source: "D:\CW3M.git\trunk\DataCW3M\probability_transition_lookup.csv"; DestDir: "\CW3M_{#MyAppVersion}"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Reach.xml"; DestDir: "\CW3M_{#MyAppVersion}"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Reporter.xml"; DestDir: "\CW3M_{#MyAppVersion}"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Scenarios.xml"; DestDir: "\CW3M_{#MyAppVersion}"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Scenarios_PEST.xml"; DestDir: "\CW3M_{#MyAppVersion}"; 
Source: "D:\CW3M.git\trunk\DataCW3M\SpatialAllocator.xml"; DestDir: "\CW3M_{#MyAppVersion}"; 
Source: "D:\CW3M.git\trunk\DataCW3M\VegSTM.xml"; DestDir: "\CW3M_{#MyAppVersion}"; 
Source: "D:\CW3M.git\trunk\DataCW3M\wr_pods.csv"; DestDir: "\CW3M_{#MyAppVersion}"; 
Source: "D:\CW3M.git\trunk\DataCW3M\wr_pous.csv"; DestDir: "\CW3M_{#MyAppVersion}"; 

Source: "D:\CW3M.git\trunk\DataCW3M\CW3MdigitalHandbook\*"; DestDir: "\CW3M_{#MyAppVersion}\CW3MdigitalHandbook";
Source: "D:\CW3M.git\trunk\DataCW3M\GriddedRecentWeather\*"; DestDir: "\CW3M_{#MyAppVersion}\GriddedRecentWeather"; 

Source: "D:\CW3M.git\trunk\DataCW3M\ScenarioData\*"; DestDir: "\CW3M_{#MyAppVersion}\ScenarioData"; 
Source: "D:\CW3M.git\trunk\DataCW3M\ScenarioData\Demo\*"; DestDir: "\CW3M_{#MyAppVersion}\ScenarioData\Demo";
Source: "D:\CW3M.git\trunk\DataCW3M\ScenarioData\Baseline\*"; DestDir: "\CW3M_{#MyAppVersion}\ScenarioData\Baseline";
Source: "D:\CW3M.git\trunk\DataCW3M\ScenarioData\365dayBaseline\*"; DestDir: "\CW3M_{#MyAppVersion}\ScenarioData\365dayBaseline";
Source: "D:\CW3M.git\trunk\DataCW3M\ScenarioData\HadGEM-ES_20th_century\*"; DestDir: "\CW3M_{#MyAppVersion}\ScenarioData\HadGEM-ES_20th_century";
Source: "D:\CW3M.git\trunk\DataCW3M\ScenarioData\HadGEM-ES_rcp85\*"; DestDir: "\CW3M_{#MyAppVersion}\ScenarioData\HadGEM-ES_rcp85";
Source: "D:\CW3M.git\trunk\DataCW3M\ScenarioData\MIROC5_20th_century\*"; DestDir: "\CW3M_{#MyAppVersion}\ScenarioData\MIROC5_20th_century";
Source: "D:\CW3M.git\trunk\DataCW3M\ScenarioData\MIROC5_rcp85\*"; DestDir: "\CW3M_{#MyAppVersion}\ScenarioData\MIROC5_rcp85";
Source: "D:\CW3M.git\trunk\DataCW3M\ScenarioData\PEST\*"; DestDir: "\CW3M_{#MyAppVersion}\ScenarioData\PEST";
Source: "D:\CW3M.git\trunk\DataCW3M\ScenarioData\PopulationAndIncomeScenarios\BaselinePopulationAndIncome\*"; DestDir: "\CW3M_{#MyAppVersion}\ScenarioData\PopulationAndIncomeScenarios\BaselinePopulationAndIncome";

;Source: "D:\CW3M.git\trunk\DataCW3M\CW3M_WRB.envx"; DestDir: "\CW3M_{#MyAppVersion}"; 
;Source: "D:\CW3M.git\trunk\DataCW3M\Flow_WRB_31yr_spinup_2010.ic"; DestDir: "\CW3M_{#MyAppVersion}"; 
;;Source: "D:\CW3M.git\trunk\DataCW3M\Flow_WRB_MIROC5_56yr_spinup_2006.ic"; DestDir: "\CW3M_{#MyAppVersion}"; 
;;Source: "D:\CW3M.git\trunk\DataCW3M\Flow_WRB_HadGEM-ES_56yr_spinup_2006.ic"; DestDir: "\CW3M_{#MyAppVersion}"; 
;Source: "D:\CW3M.git\trunk\DataCW3M\Flow.xml"; DestDir: "\CW3M_{#MyAppVersion}"; 
;Source: "D:\CW3M.git\trunk\DataCW3M\HRU_CW3M*"; DestDir: "\CW3M_{#MyAppVersion}"; 
;Source: "D:\CW3M.git\trunk\DataCW3M\IDU_CW3M.cpg"; DestDir: "\CW3M_{#MyAppVersion}"; 
;Source: "D:\CW3M.git\trunk\DataCW3M\IDU_CW3M.dbf"; DestDir: "\CW3M_{#MyAppVersion}"; 
;Source: "D:\CW3M.git\trunk\DataCW3M\IDU_CW3M.prj"; DestDir: "\CW3M_{#MyAppVersion}"; 
;Source: "D:\CW3M.git\trunk\DataCW3M\IDU_CW3M.sbn"; DestDir: "\CW3M_{#MyAppVersion}"; 
;Source: "D:\CW3M.git\trunk\DataCW3M\IDU_CW3M.sbx"; DestDir: "\CW3M_{#MyAppVersion}"; 
;Source: "D:\CW3M.git\trunk\DataCW3M\IDU_CW3M.shp"; DestDir: "\CW3M_{#MyAppVersion}"; 
;Source: "D:\CW3M.git\trunk\DataCW3M\IDU_CW3M.shp.xml"; DestDir: "\CW3M_{#MyAppVersion}"; 
;Source: "D:\CW3M.git\trunk\DataCW3M\IDU_CW3M.shx"; DestDir: "\CW3M_{#MyAppVersion}"; 
;Source: "D:\CW3M.git\trunk\DataCW3M\Reach_CW3M*"; DestDir: "\CW3M_{#MyAppVersion}"; 

Source: "D:\CW3M.git\trunk\DataCW3M\CW3M_Clackamas.envx"; DestDir: "\CW3M_{#MyAppVersion}"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Clackamas\Flow_Clackamas.xml"; DestDir: "\CW3M_{#MyAppVersion}\Clackamas"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Clackamas\Flow_Clackamas2010.ic"; DestDir: "\CW3M_{#MyAppVersion}\Clackamas"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Clackamas\HRU_Clackamas*"; DestDir: "\CW3M_{#MyAppVersion}\Clackamas"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Clackamas\IDU_Clackamas.cpg"; DestDir: "\CW3M_{#MyAppVersion}\Clackamas"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Clackamas\IDU_Clackamas.dbf"; DestDir: "\CW3M_{#MyAppVersion}\Clackamas"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Clackamas\IDU_Clackamas.prj"; DestDir: "\CW3M_{#MyAppVersion}\Clackamas"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Clackamas\IDU_Clackamas.sbn"; DestDir: "\CW3M_{#MyAppVersion}\Clackamas"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Clackamas\IDU_Clackamas.sbx"; DestDir: "\CW3M_{#MyAppVersion}\Clackamas"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Clackamas\IDU_Clackamas.shp"; DestDir: "\CW3M_{#MyAppVersion}\Clackamas"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Clackamas\IDU_Clackamas.shp.xml"; DestDir: "\CW3M_{#MyAppVersion}\Clackamas"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Clackamas\IDU_Clackamas.shx"; DestDir: "\CW3M_{#MyAppVersion}\Clackamas"; 
Source: "D:\CW3M.git\trunk\DataCW3M\Clackamas\Reach_Clackamas*"; DestDir: "\CW3M_{#MyAppVersion}\Clackamas"; 
;Source: "D:\CW3M.git\trunk\DataCW3M\Clackamas\Reporter_Clackamas.xml"; DestDir: "\CW3M_{#MyAppVersion}\Clackamas"; 

Source: "D:\CW3M.git\trunk\DataCW3M\CW3M_McKenzie.envx"; DestDir: "\CW3M_{#MyAppVersion}"; 
;Source: "D:\CW3M.git\trunk\DataCW3M\CW3M_McKenzie_PEST.envx"; DestDir: "\CW3M_{#MyAppVersion}"; 
Source: "D:\CW3M.git\trunk\DataCW3M\McKenzie\Flow_McKenzie.xml"; DestDir: "\CW3M_{#MyAppVersion}\McKenzie"; 
;Source: "D:\CW3M.git\trunk\DataCW3M\McKenzie\Flow_McKenzie_PEST.xml"; DestDir: "\CW3M_{#MyAppVersion}\McKenzie"; 
Source: "D:\CW3M.git\trunk\DataCW3M\McKenzie\Flow_McKenzie2010.ic"; DestDir: "\CW3M_{#MyAppVersion}\McKenzie"; 
;Source: "D:\CW3M.git\trunk\DataCW3M\McKenzie\Flow_McKenzie_31yr_spinup_2010.ic"; DestDir: "\CW3M_{#MyAppVersion}\McKenzie"; 
;Source: "D:\CW3M.git\trunk\DataCW3M\McKenzie\Flow_McKenzie_MIROC5_56yr_spinup_2006.ic"; DestDir: "\CW3M_{#MyAppVersion}\McKenzie"; 
;Source: "D:\CW3M.git\trunk\DataCW3M\McKenzie\Flow_McKenzie_HadGEM-ES_56yr_spinup_2006.ic"; DestDir: "\CW3M_{#MyAppVersion}\McKenzie"; 
Source: "D:\CW3M.git\trunk\DataCW3M\McKenzie\HRU_McKenzie*"; DestDir: "\CW3M_{#MyAppVersion}\McKenzie"; 
Source: "D:\CW3M.git\trunk\DataCW3M\McKenzie\IDU_McKenzie.cpg"; DestDir: "\CW3M_{#MyAppVersion}\McKenzie"; 
Source: "D:\CW3M.git\trunk\DataCW3M\McKenzie\IDU_McKenzie.dbf"; DestDir: "\CW3M_{#MyAppVersion}\McKenzie"; 
Source: "D:\CW3M.git\trunk\DataCW3M\McKenzie\IDU_McKenzie.prj"; DestDir: "\CW3M_{#MyAppVersion}\McKenzie"; 
Source: "D:\CW3M.git\trunk\DataCW3M\McKenzie\IDU_McKenzie.sbn"; DestDir: "\CW3M_{#MyAppVersion}\McKenzie"; 
Source: "D:\CW3M.git\trunk\DataCW3M\McKenzie\IDU_McKenzie.sbx"; DestDir: "\CW3M_{#MyAppVersion}\McKenzie"; 
Source: "D:\CW3M.git\trunk\DataCW3M\McKenzie\IDU_McKenzie.shp"; DestDir: "\CW3M_{#MyAppVersion}\McKenzie"; 
Source: "D:\CW3M.git\trunk\DataCW3M\McKenzie\IDU_McKenzie.shp.xml"; DestDir: "\CW3M_{#MyAppVersion}\McKenzie"; 
Source: "D:\CW3M.git\trunk\DataCW3M\McKenzie\IDU_McKenzie.shx"; DestDir: "\CW3M_{#MyAppVersion}\McKenzie"; 
Source: "D:\CW3M.git\trunk\DataCW3M\McKenzie\Reach_McKenzie*"; DestDir: "\CW3M_{#MyAppVersion}\McKenzie"; 
;Source: "D:\CW3M.git\trunk\DataCW3M\McKenzie\PEST\*"; DestDir: "\CW3M_{#MyAppVersion}\McKenzie\PEST";
Source: "D:\CW3M.git\trunk\DataCW3M\McKenzie\Reporter_McKenzie.xml"; DestDir: "\CW3M_{#MyAppVersion}\McKenzie"; 

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
