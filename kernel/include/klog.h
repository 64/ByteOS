#pragma once

// Don't #include <stdio.h>, it will only increase compile time
extern int printf(const char* __restrict, ...);
extern void abort(void);

#define KLOG_LEVEL_NONE 0
#define KLOG_LEVEL_FATAL 1
#define KLOG_LEVEL_ERROR 2
#define KLOG_LEVEL_WARN 3
#define KLOG_LEVEL_INFO 4
#define KLOG_LEVEL_NOTICE 5
#define KLOG_LEVEL_DETAIL 6

#define ANSI_SEQ_BEGIN "\x1B["
#define KLOG_FATAL_HEADER "[" ANSI_SEQ_BEGIN "1;31mFATAL" ANSI_SEQ_BEGIN "0m]"
#define KLOG_ERROR_HEADER "[" ANSI_SEQ_BEGIN "31mERROR" ANSI_SEQ_BEGIN "0m]"
#define KLOG_WARN_HEADER "["ANSI_SEQ_BEGIN "33mWARNING" ANSI_SEQ_BEGIN "0m]"
#define KLOG_INFO_HEADER "[" ANSI_SEQ_BEGIN "1;34mINFO" ANSI_SEQ_BEGIN "0m]"
#define KLOG_NOTICE_HEADER "[" ANSI_SEQ_BEGIN "36mNOTICE" ANSI_SEQ_BEGIN "0m]"
#define KLOG_DETAIL_HEADER "[" ANSI_SEQ_BEGIN "32mDETAIL" ANSI_SEQ_BEGIN "0m]"

// Everything above this number gets printed
#ifndef KLOG_LEVEL
	#define KLOG_LEVEL KLOG_LEVEL_DETAIL
#endif

#if KLOG_LEVEL >= KLOG_LEVEL_FATAL
	#define klog_fatal(fmt, ...) printf(KLOG_FATAL_HEADER " " fmt, ##__VA_ARGS__)
	#define klog_fatal_nohdr(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
	#define klog_fatal(fmt, ...) do { ((void)0); } while(0)
	#define klog_fatal_nohdr(fmt, ...) do { ((void)0); } while(0)
#endif

#if KLOG_LEVEL >= KLOG_LEVEL_ERROR
	#define klog_error(fmt, ...) printf(KLOG_ERROR_HEADER " " fmt, ##__VA_ARGS__)
	#define klog_error_nohdr(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
	#define klog_error(fmt, ...) do { ((void)0); } while(0)
	#define klog_error_nohdr(fmt, ...) do { ((void)0); } while(0)
#endif

#if KLOG_LEVEL >= KLOG_LEVEL_WARN
	#define klog_warn(fmt, ...) printf(KLOG_WARN_HEADER " " fmt, ##__VA_ARGS__)
	#define klog_warn_nohdr(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
	#define klog_warn(fmt, ...) do { ((void)0); } while(0)
	#define klog_warn_nohdr(fmt, ...) do { ((void)0); } while(0)
#endif

#if KLOG_LEVEL >= KLOG_LEVEL_INFO
	#define klog_info(fmt, ...) printf(KLOG_INFO_HEADER " " fmt, ##__VA_ARGS__)
	#define klog_info_nohdr(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
	#define klog_info(fmt, ...) do { ((void)0); } while(0)
	#define klog_info_nohdr(fmt, ...) do { ((void)0); } while(0)
#endif

#if KLOG_LEVEL >= KLOG_LEVEL_NOTICE
	#define klog_notice(fmt, ...) printf(KLOG_NOTICE_HEADER " " fmt, ##__VA_ARGS__)
	#define klog_notice_nohdr(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
	#define klog_notice(fmt, ...) do { ((void)0); } while(0)
	#define klog_notice_nohdr(fmt, ...) do { ((void)0); } while(0)
#endif

#if KLOG_LEVEL >= KLOG_LEVEL_DETAIL
	#define klog_detail(fmt, ...) printf(KLOG_DETAIL_HEADER " " fmt, ##__VA_ARGS__)
	#define klog_detail_nohdr(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
	#define klog_detail(fmt, ...) do { ((void)0); } while(0)
	#define klog_detail_nohdr(fmt, ...) do { ((void)0); } while(0)
#endif

#define klog_assert(predicate) if (!(predicate)) { klog_fatal("Assertion fail: " #predicate); abort(); }
