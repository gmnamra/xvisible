#ifndef UI_RCAPPCONSTANTS_H
#define UI_RCAPPCONSTANTS_H



#include <rc_uitypes.h>

#include "rc_uiconstants.h"

// Debugging declarations
#if notyet // Not needed now
// Macro to check current stack size
#if defined(__APPLE__)
#define rmStackSize() (::StackSpace())
#else
// Unimplemented for non-Apple platforms
#define rmStackSize() (void)
#endif
#endif

// Declarations for GUI-specific data
const char* const cLicenseFilePath = "/Users/Shared/Library/Reify/visible.rfylic";

// Declarations for application-specific data

// Setting label layout constants
extern const int cUIsettingLabelWidth;
extern const int cUIsettingLabelSpacing;

// Application info
extern const char* const cAppName;
extern const char* const cVersionName;
extern const char* const cAppBuildDate;

#endif // UI_RCAPPCONSTANTS_H
