//
// Created by Yuqi Zhou on 2022/11/8.
//

#ifndef GSTORE_DATABASE_OPTIMIZERDEBUG_H_
#define GSTORE_DATABASE_OPTIMIZERDEBUG_H_

#include "../Query/DFSPlan.h"
#include "./PlanGenerator.h"
#include "./Executor.h"

#ifdef TOPK_SUPPORT
#include "../Query/topk/DPBTopKUtil.h"
#endif // TOPK_SUPPORT
#include <cstring>
#include <climits>
#include <algorithm>
#include <list>
#include <cstdlib>
#include <cmath>
#include <random>

//#define FEED_PLAN
// #define OPTIMIZER_DEBUG_INFO

inline long GetTimeDebug(){
#ifdef OPTIMIZER_DEBUG_INFO
  return Util::get_cur_time();
#else
  return 0;
#endif
}

inline void PrintTimeDebug(std::string description, long old_time){
#ifdef OPTIMIZER_DEBUG_INFO
  long t2 = Util::get_cur_time();
  std::cout<<  description << ",  used " << (t2 - old_time) << "ms." <<std::endl;
#endif
}

#define DebugWithLine(x)  (GetLineDescription()+ ": " + x)

#ifdef OPTIMIZER_DEBUG_INFO
  #define PrintDebugInfoLine(x) {std::cout<< DebugWithLine(x)  <<std::endl;}
#else
  #define PrintDebugInfoLine(x) {}
#endif

#define PrintTimeOpRange(x,y,t)  PrintTimeDebug( GetLineDescription()+ ": " + \
  StepOperation::GetString(x)+ std::string(" ") + StepOperation::GetString(y), t)

#endif //GSTORE_DATABASE_OPTIMIZERDEBUG_H_
