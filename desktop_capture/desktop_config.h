#pragma once

#define WEBRTC_WIN 1

// Put this in the declarations for a class to be unassignable.
#define RTC_DISALLOW_ASSIGN(TypeName) \
  TypeName& operator=(const TypeName&) = delete

// A macro to disallow the copy constructor and operator= functions. This should
// be used in the declarations for a class.
#define RTC_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;          \
  RTC_DISALLOW_ASSIGN(TypeName)

#define RTC_CHECK(condition)

#define RTC_LOG_F(sev) std::count 
#define RTC_DCHECK_EQ(x) 
#define RTC_DCHECK_GE(x, y)