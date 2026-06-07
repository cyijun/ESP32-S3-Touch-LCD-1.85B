#ifndef DEBUG_TRACE_H
#define DEBUG_TRACE_H

#include <Arduino.h>

// ===== 模块开关（编译时控制，关闭时代码被优化掉，零开销）=====
#define DEBUG_DETAIL   1
#define DEBUG_SELECT   1
#define DEBUG_INTERACT 1
#define DEBUG_STORAGE  1
#define DEBUG_MAIN     1
#define DEBUG_NETWORK  1
#define DEBUG_KIMI     1

// ===== 基础宏 =====
#define _TRACE_BASE(module, fmt, ...) \
    do { Serial.printf("[%s] %s:%d " fmt "\n", #module, __FUNCTION__, __LINE__, ##__VA_ARGS__); } while(0)

#define _TRACE_ENTER(module) \
    do { Serial.printf("[%s] >>> ENTER %s\n", #module, __FUNCTION__); } while(0)

#define _TRACE_EXIT(module) \
    do { Serial.printf("[%s] <<< EXIT  %s heap=%u\n", #module, __FUNCTION__, ESP.getFreeHeap()); } while(0)

#define _TRACE_HEAP(module) \
    do { Serial.printf("[%s] HEAP free=%u min=%u\n", #module, ESP.getFreeHeap(), ESP.getMinFreeHeap()); } while(0)

// ===== DETAIL =====
#if DEBUG_DETAIL
  #define TRACE_DETAIL(fmt, ...)  _TRACE_BASE(DETAIL, fmt, ##__VA_ARGS__)
  #define TRACE_DETAIL_ENTER()    _TRACE_ENTER(DETAIL)
  #define TRACE_DETAIL_EXIT()     _TRACE_EXIT(DETAIL)
  #define TRACE_DETAIL_HEAP()     _TRACE_HEAP(DETAIL)
#else
  #define TRACE_DETAIL(fmt, ...)  ((void)0)
  #define TRACE_DETAIL_ENTER()    ((void)0)
  #define TRACE_DETAIL_EXIT()     ((void)0)
  #define TRACE_DETAIL_HEAP()     ((void)0)
#endif

// ===== SELECT =====
#if DEBUG_SELECT
  #define TRACE_SELECT(fmt, ...)  _TRACE_BASE(SELECT, fmt, ##__VA_ARGS__)
  #define TRACE_SELECT_ENTER()    _TRACE_ENTER(SELECT)
  #define TRACE_SELECT_EXIT()     _TRACE_EXIT(SELECT)
  #define TRACE_SELECT_HEAP()     _TRACE_HEAP(SELECT)
#else
  #define TRACE_SELECT(fmt, ...)  ((void)0)
  #define TRACE_SELECT_ENTER()    ((void)0)
  #define TRACE_SELECT_EXIT()     ((void)0)
  #define TRACE_SELECT_HEAP()     ((void)0)
#endif

// ===== INTERACT =====
#if DEBUG_INTERACT
  #define TRACE_INTERACT(fmt, ...)  _TRACE_BASE(INTERACT, fmt, ##__VA_ARGS__)
  #define TRACE_INTERACT_ENTER()    _TRACE_ENTER(INTERACT)
  #define TRACE_INTERACT_EXIT()     _TRACE_EXIT(INTERACT)
  #define TRACE_INTERACT_HEAP()     _TRACE_HEAP(INTERACT)
#else
  #define TRACE_INTERACT(fmt, ...)  ((void)0)
  #define TRACE_INTERACT_ENTER()    ((void)0)
  #define TRACE_INTERACT_EXIT()     ((void)0)
  #define TRACE_INTERACT_HEAP()     ((void)0)
#endif

// ===== STORAGE =====
#if DEBUG_STORAGE
  #define TRACE_STORAGE(fmt, ...)  _TRACE_BASE(STORAGE, fmt, ##__VA_ARGS__)
  #define TRACE_STORAGE_ENTER()    _TRACE_ENTER(STORAGE)
  #define TRACE_STORAGE_EXIT()     _TRACE_EXIT(STORAGE)
  #define TRACE_STORAGE_HEAP()     _TRACE_HEAP(STORAGE)
#else
  #define TRACE_STORAGE(fmt, ...)  ((void)0)
  #define TRACE_STORAGE_ENTER()    ((void)0)
  #define TRACE_STORAGE_EXIT()     ((void)0)
  #define TRACE_STORAGE_HEAP()     ((void)0)
#endif

// ===== MAIN =====
#if DEBUG_MAIN
  #define TRACE_MAIN(fmt, ...)  _TRACE_BASE(MAIN, fmt, ##__VA_ARGS__)
  #define TRACE_MAIN_ENTER()    _TRACE_ENTER(MAIN)
  #define TRACE_MAIN_EXIT()     _TRACE_EXIT(MAIN)
  #define TRACE_MAIN_HEAP()     _TRACE_HEAP(MAIN)
#else
  #define TRACE_MAIN(fmt, ...)  ((void)0)
  #define TRACE_MAIN_ENTER()    ((void)0)
  #define TRACE_MAIN_EXIT()     ((void)0)
  #define TRACE_MAIN_HEAP()     ((void)0)
#endif

// ===== NETWORK =====
#if DEBUG_NETWORK
  #define TRACE_NETWORK(fmt, ...)  _TRACE_BASE(NETWORK, fmt, ##__VA_ARGS__)
  #define TRACE_NETWORK_ENTER()    _TRACE_ENTER(NETWORK)
  #define TRACE_NETWORK_EXIT()     _TRACE_EXIT(NETWORK)
  #define TRACE_NETWORK_HEAP()     _TRACE_HEAP(NETWORK)
#else
  #define TRACE_NETWORK(fmt, ...)  ((void)0)
  #define TRACE_NETWORK_ENTER()    ((void)0)
  #define TRACE_NETWORK_EXIT()     ((void)0)
  #define TRACE_NETWORK_HEAP()     ((void)0)
#endif

// ===== KIMI =====
#if DEBUG_KIMI
  #define TRACE_KIMI(fmt, ...)  _TRACE_BASE(KIMI, fmt, ##__VA_ARGS__)
  #define TRACE_KIMI_ENTER()    _TRACE_ENTER(KIMI)
  #define TRACE_KIMI_EXIT()     _TRACE_EXIT(KIMI)
  #define TRACE_KIMI_HEAP()     _TRACE_HEAP(KIMI)
#else
  #define TRACE_KIMI(fmt, ...)  ((void)0)
  #define TRACE_KIMI_ENTER()    ((void)0)
  #define TRACE_KIMI_EXIT()     ((void)0)
  #define TRACE_KIMI_HEAP()     ((void)0)
#endif

#endif
