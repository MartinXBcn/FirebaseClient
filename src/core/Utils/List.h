/*
 * SPDX-FileCopyrightText: 2025 Suwatchai K. <suwatchai@outlook.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef CORE_UTILS_LIST_H
#define CORE_UTILS_LIST_H

#include <Arduino.h>
#include <vector>


// <MS> Logging
#undef MS_LOGGER_LEVEL
#ifdef MS_FIREBASECLIENT_LOGGING
#define MS_LOGGER_LEVEL MS_FIREBASECLIENT_LOGGING
#endif
#include "ESP32Logger.h"


namespace firebase_ns
{
    class List
    {
    public:
        List() {}

        ~List() {}

        void addRemoveList(std::vector<uint32_t> &vec, uint32_t addr, bool add)
        {
          // <MS>
          // Check added if vec.size() is plausible to avoid exceptions due to forbidden memory access.
          // Looks like that AddRemoveList() is called with unvalid pointer [vec].
          if (vec.size() > 6) {
            DBGLOG(Error, "[List] addRemoveList() vector size > 6: %u", vec.size())
          } else {
            for (size_t i = 0; i < vec.size(); i++)
            {
                if (vec[i] == addr)
                {
                    if (add)
                        return;
                    else
                        vec.erase(vec.begin() + i);
                }
            }
            if (add)
                vec.push_back(addr);
          }
        }

        bool existed(const std::vector<uint32_t> &vec, uint32_t addr)
        {
            for (size_t i = 0; i < vec.size(); i++)
            {
                if (vec[i] == addr)
                    return true;
            }
            return false;
        }
    };
}
#endif
