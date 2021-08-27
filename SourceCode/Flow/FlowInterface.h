#pragma once

/* Tokens used in the TMIN_GROW and TMINGROAVG attributes 
to distinguish which IDUs are treated as part of the ag basin.
Token values should be outside the range of meaningful values for temperatures.
Note that TMIN_GROW and TMINGROAVG represent real numbers, not integers,
so tests of equality on the tokens are insufficient.
*/
#define TOKEN_NOT_IN_AG_BASIN -99.f
#define TOKEN_INIT_EXP_AVG -97.f
#define IDU_IN_AG_BASIN(x) (true)
#define INITIALIZE_EXP_AVG(x) ((1.f + TOKEN_NOT_IN_AG_BASIN) < x && x < (1.f + TOKEN_INIT_EXP_AVG))

// Water Right use bitwise codes http://www.oregon.gov/owrd/pages/wr/wrisuse.aspx
enum WR_USE 
   { 
   // WRU_NULL = 0,
   WRU_UNKNOWN=1, 
   // WRU_MINING=2, 
   // WRU_AG=4,
   // WRU_DOMESTIC=8, 
   WRU_IRRIGATION=16,
   // WRU_COMMERCIAL=32,
   // WRU_RECREATION=64,
   // WRU_POWER=128, 
   // WRU_FISH=256,
   // WRU_LIVESTOCK=512,
   WRU_MUNICIPAL=1024,
   WRU_INSTREAM=2048,
   // WRU_MISC=4096,
   // WRU_WILDLIFE=8192
   }; 

// Water Right permit bitwise codes http://www.oregon.gov/owrd/pages/wr/wrisuse.aspx
enum WR_PERMIT 
   { 
// WRT_NULL = 0,
   WRP_UNKNOWN=1,
   WRP_SURFACE=2,
   WRP_GROUNDWATER=4, 
   WRP_RESERVOIR=8,
   WRP_ENLARGEMENT=16,
   WRP_UNDERGROUND=32 
   };

// WREXISTS utility functions
inline   unsigned __int8 GetPermit(__int32 wrexists_arg)                    { return (wrexists_arg & 0x000000FF); }
inline   unsigned __int16 GetUse(__int32 wrexists_arg)                      { return (wrexists_arg & 0x00FFFF00) >> 8; }
inline   int GetMuni(__int32 wrexists_arg)                                  { return ((wrexists_arg >> 24) & 0xFF); }
inline __int32 SetPermit(__int32 wrexists_arg, unsigned __int8 permit_arg)  { return (wrexists_arg & ~0x000000FF) | (permit_arg & 0x000000FF); }
inline __int32 SetUse(__int32 wrexists_arg, unsigned __int16 use_flags_arg) { return (wrexists_arg & ~0x00FFFF00) | ((use_flags_arg & 0x0000FFFF) << 8); }
inline __int32 SetMuni(__int32 wrexists_arg, int muni_arg)                  { return (wrexists_arg & 0x00FFFFFF) | (muni_arg << 24); }

//methods for decoding WREXIST
inline bool IsMunicipal(unsigned __int16 flags)  { return ((flags & WRU_MUNICIPAL) > 0) ? true : false; }
inline bool IsIrrigation(unsigned __int16 flags) { return ((flags & WRU_IRRIGATION) > 0) ? true : false; }

//for use with WREXISTS PERMITS)
inline bool IsUnknownPermit(unsigned __int8 flags){ return ((flags & WRP_UNKNOWN) > 0) ? true : false; }
inline bool IsSurface(unsigned __int8 flags)      { return ((flags & WRP_SURFACE) > 0) ? true : false; }
inline bool IsGroundwater(unsigned __int8 flags)  { return ((flags & WRP_GROUNDWATER) > 0) ? true : false; }
inline bool IsReservoir(unsigned __int8 flags)    { return ((flags & WRP_RESERVOIR) > 0) ? true : false; }
inline bool IsEnlargement(unsigned __int8 flags)  { return ((flags & WRP_ENLARGEMENT) > 0) ? true : false; }
inline bool IsUnderground(unsigned __int8 flags)  { return ((flags & WRP_UNDERGROUND) > 0) ? true : false; }