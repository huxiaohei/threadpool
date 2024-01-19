/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2023/11/23
 * Copyright (c) 2023 虎小黑
 ****************************************************************/

#ifndef __UTILS_H__
#define __UTILS_H__

#include <chrono>

class TimeUtils
{
  public:
    static uint64_t get_current_time_milliseconds()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
            .count();
    }

    static uint16_t get_current_time_seconds()
    {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();
    }
};

#endif // __UTILS_H__