#include "stdafx.h"

#include "EnvModel.h"
#include <Report.h>
#include <Maplayer.h>
#include <LULCTREE.H>
#include <PathManager.h>
#include "EnvConstants.h"

bool IsDevelopmentEnvironment()
{
   TCHAR my_path[MAX_PATH];
   my_path[0] = NULL;
   int count = GetModuleFileName(NULL, my_path, MAX_PATH);
   ASSERT(count > 0);
   for (int i = 0; i < lstrlen(my_path); i++) my_path[i] = tolower(my_path[i]);
   TCHAR *debug = _tcsstr(my_path, _T("debug"));
   TCHAR *release = _tcsstr(my_path, _T("release"));
   return(debug || release);
 } // end of IsDevelopmentEnvironment()


bool AddCPLEXenvironmentVar()
{
   size_t requiredSize;
   _tgetenv_s(&requiredSize, NULL, 0, "CPLEX_STUDIO_BINARIES128");
   
   if (requiredSize > 0)
   {
      TCHAR *environ_var = new TCHAR[requiredSize + 256]; environ_var[0] = 0;
      _tgetenv_s(&requiredSize, environ_var, requiredSize, "CPLEX_STUDIO_BINARIES128");
      CString msg; 
      msg.Format("AddCPLEXenvironmentVar() found CPLEX_STUDIO_BINARIES128 already set to %s", environ_var);
      Report::LogMsg(msg);
      delete[] environ_var;
      return(true);
   }

   if (IsDevelopmentEnvironment())
   {
      CString msg;
      msg.Format("AddCPLEXenvironmentVar(): Please install CPLEX and set system environment variables\n"
         "Set CPLEX_STUDIO_DIR128 = C:\\Program Files\\IBM\\ILOG\\CPLEX_Studio128\n"
         "Set CPLEX_STUDIO_BINARIES128 = \n"
         "  C:\\Program Files\\IBM\\ILOG\\CPLEX_Studio128\\opl\\oplide\n"
         "  C:\\Program Files\\IBM\\ILOG\\CPLEX_Studio128\\opl\\bin\\x64_win64\n"
         "  C:\\Program Files\\IBM\\ILOG\\CPLEX_Studio128\\cpoptimizer\\bin\\x64_win64\n"
         "  C:\\Program Files\\IBM\\ILOG\\CPLEX_Studio128\\cplex\\bin\\x64_win64");
         Report::ErrorMsg(msg);
      return(false);
   }
/*
   CString exe_path = PathManager::GetPath(PM_ENVISION_DIR);
   CString cplex_path = exe_path + "CPLEX";
   int err_num = _putenv_s("CPLEX_STUDIO_BINARIES128", cplex_path);
   if (err_num != 0)
   {
      CString msg;
      msg.Format("AddCPLEXenvironmentVar() failed with err_num = %d when attempting to set CPLEX_STUDIO_BINARIES128 = %s", err_num, cplex_path);
      Report::ErrorMsg(msg);
      return(false);
   }

   CString msg;
   msg.Format("AddCPLEXenvironmentVar() set CPLEX_STUDIO_BINARIES128 = %s", cplex_path);
   Report::LogMsg(msg);
*/
   return(true);
} // end of AddCPLEXenvironmentVar()


int AddGDALPath()
{
   CString msg;

   // Get the value of the PATH environment variable.
   size_t requiredSize;
   _tgetenv_s(&requiredSize, NULL, 0, "PATH");
   TCHAR *original_path = new TCHAR[requiredSize + 256]; original_path[0] = 0;
   _tgetenv_s( &requiredSize, original_path, requiredSize, "PATH" );
   
   if (_tcsstr(original_path, "GDAL") != NULL)
   { // GDAL is already in PATH
      msg.Format("AddGDALPath() found 'GDAL' already in PATH:\n %s", original_path);
      Report::ErrorMsg(msg);
      delete[] original_path;
      return(1);
   }
   
   // Attempt to change path. Note that this only affects
   // the environment variable of the current process. The command
   // processor's environment is not changed.
   msg.Format("AddGDALPath() did not find GDAL in PATH.  Original PATH is:\n%s", original_path);
   Report::LogMsg(msg);
 
   // In the normal environment, my_path may look something like "C:\Program Files (x86)\WillametteINFEWSmodel\WillametteINFEWSmodel.exe", and the
   // GDAL stuff would be assumed to be at C:\Program Files (x86)\WillametteINFEWSmodel\GDAL
   // In the development environment, my_path will look something like C:\INFEWSsvn\trunk\Source\x64\Release\WillametteINFEWSmodel.exe", and the
   // GDAL stuff would be assumed to be at C:\INFEWSsvn\trunk\GDAL
   TCHAR my_path[MAX_PATH]; my_path[0] = NULL;
   int count = GetModuleFileName(NULL, my_path, MAX_PATH);
   ASSERT(count > 0);
   TCHAR *end = NULL;
   end = _tcsrchr( my_path, '\\' ); if (end == NULL) end =_tcsrchr(my_path,'/');
   ASSERT(*end != NULL);
   *end = NULL;
   CString gdalPath(my_path); 
   if (IsDevelopmentEnvironment()) gdalPath = gdalPath + "\\..\\..\\..\\GDAL\\";
   else gdalPath = gdalPath + "\\GDAL\\";
   msg.Format("Inserting GDAL at beginning of PATH: %s", gdalPath);
   Report::LogMsg(msg);
   TCHAR * new_path = new TCHAR[requiredSize + 256]; new_path[0] = 0;
   lstrcat(new_path, gdalPath);
   lstrcat(new_path, ";");
   lstrcat(new_path, original_path);
   _putenv_s( "PATH", new_path );

   CString gdalPluginPath = gdalPath + "gdalplugins";
   msg.Format("Setting GDAL_DRIVER_PATH: %s", gdalPluginPath);
   Report::LogMsg(msg);
   _putenv_s( "GDAL_DRIVER_PATH", gdalPluginPath );

   // CString gdalPluginData = gdalPath + "gdal-data";
   //_putenv_s( "GDAL_DATA",        gdalPluginData );
      
   msg.Format("Complete PATH is now: %s", new_path);
   Report::LogMsg(msg);

   delete [] original_path;
   delete [] new_path;
   return 1;
   }

