#pragma once

///* Usage:  By design, use THROW, TRY, CATCH.  
//           But Standard C++ usage is, e.g.:
//               throw EnvException();
//               catch (EnvException & e) { /* do someting */ }
//*/

#define ENV_ASSERT(f) \
   if ( !(f) )\
      { \
      ASSERT(0); \
      CString str;\
      str.Format( "FAILED ASSERT(%s):\n  File: %s\n  Line: %d", #f, __FILE__, __LINE__ ); \
      throw new EnvFatalException( str ); \
      }

class EnvException : public CException
   {
   DECLARE_DYNAMIC(EnvException)

   public:
      EnvException();
      EnvException( LPCSTR errorStr );
      virtual ~EnvException();

   private:
      CString m_errorStr;

   public:
      virtual BOOL GetErrorMessage(LPTSTR lpszError, UINT nMaxError, PUINT pnHelpContext = NULL);
      virtual const char * what() const;
   };

class EnvRuntimeException : public EnvException
   {
   public:
      EnvRuntimeException() : EnvException( "Unspecified Runtime Error" ) {}
      EnvRuntimeException( LPCSTR errorStr ) : EnvException( errorStr ) {}
   };

class EnvFatalException : public EnvException
   {
   public:
      EnvFatalException() : EnvException( "Unspecified Fatal Error" ) {}
      EnvFatalException( LPCSTR errorStr ) : EnvException( errorStr ) {}
   };