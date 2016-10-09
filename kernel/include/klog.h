#pragma once

#include <stdio.h>

#define KLOG_LEVEL_FATAL 0
#define KLOG_ERROR 1
#define KLOG_WARN 2
#define KLOG_INFO 3
#define KLOG_NOTICE 4
#define KLOG_DETAIL 5

#define ANSI_SEQ_BEGIN "\x1B["
#define KLOG_FATAL_HEADER "[" ANSI_SEQ_BEGIN "1;31mFATAL" ANSI_SEQ_BEGIN "0m]"
#define KLOG_ERROR_HEADER "[" ANSI_SEQ_BEGIN "31mERROR" ANSI_SEQ_BEGIN "0m]"
#define KLOG_WARN_HEADER "["ANSI_SEQ_BEGIN "33mWARNING" ANSI_SEQ_BEGIN "0m]"
#define KLOG_INFO_HEADER "[" ANSI_SEQ_BEGIN "35mINFO" ANSI_SEQ_BEGIN "0m]"
#define KLOG_NOTICE_HEADER "[" ANSI_SEQ_BEGIN "36mNOTICE" ANSI_SEQ_BEGIN "0m]"
#define KLOG_DETAIL_HEADER "[" ANSI_SEQ_BEGIN "32mDETAIL" ANSI_SEQ_BEGIN "0m]"

// Everything above this number gets printed
#define KLOG_LEVEL KLOG_DETAIL

#if KLOG_LEVEL >= KLOG_LEVEL_FATAL
	#define klog_fatal(fmt, ...) printf(KLOG_FATAL_HEADER " " fmt, ##__VA_ARGS__)
	#if KLOG_LEVEL >= KLOG_LEVEL_ERROR
		#define klog_error(fmt, ...) printf(KLOG_ERROR_HEADER " " fmt, ##__VA_ARGS__)
		#if KLOG_LEVEL >= KLOG_LEVEL_WARN
			#define klog_warn(fmt, ...) printf(KLOG_WARN_HEADER " " fmt, ##__VA_ARGS__)
			#if KLOG_LEVEL >= KLOG_LEVEL_INFO
				#define klog_info(fmt, ...) printf(KLOG_INFO_HEADER " " fmt, ##__VA_ARGS__)
				#if KLOG_LEVEL >= KLOG_LEVEL_NOTICE
					#define klog_notice(fmt, ...) printf(KLOG_NOTICE_HEADER " " fmt, ##__VA_ARGS__)
					#if KLOG_LEVEL >= KLOG_LEVEL_DETAIL
						#define klog_detail(fmt, ...) printf(KLOG_DETAIL_HEADER " " fmt, ##__VA_ARGS__)
					#else
						#define klog_detail(...)
					#endif
				#else
					#define klog_notice(...)
				#endif
			#else
				#define klog_info(...)
			#endif
		#else
			#define klog_warn(...)
		#endif
	#else
		#define klog_error(...)
	#endif
#else
	#define klog_fatal(...)
#endif
