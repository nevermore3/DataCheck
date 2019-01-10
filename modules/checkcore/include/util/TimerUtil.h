#ifndef AUTOHDMAP_DATACHECK_TIMERUTIL_H
#define AUTOHDMAP_DATACHECK_TIMERUTIL_H


#pragma once

#include <chrono>
#include <string>
#include <iostream>


using namespace std;
using namespace std::chrono;

class TimerUtil
{
public:
    TimerUtil() : m_begin(high_resolution_clock::now()) {}
    void reset() { m_begin = high_resolution_clock::now(); }
    /**
     * 输出耗时信息
     * @message 耗时信息前缀
     *
     */
    string elapsed_message(){
        int64_t e=elapsed_milli();
        int64_t S = e / 1000;
        e = e % 1000;
        int64_t M = S / 60;
        S = S % 60;
        int64_t H = M / 60;
        M = M % 60;
        //
        return to_string(H) + "h," + to_string(M) + "m," + to_string(S) + "s," + to_string(e) + "ms.";
    }
    //默认输出毫秒
    int64_t elapsed_milli() const
    {
        return duration_cast<chrono::milliseconds>(high_resolution_clock::now() - m_begin).count();
    }

    //默认输出秒
    double elapsed_second() const
    {
        return duration_cast<duration<double>>(high_resolution_clock::now() - m_begin).count();
    }

    //微秒
    int64_t elapsed_micro() const
    {
        return duration_cast<chrono::microseconds>(high_resolution_clock::now() - m_begin).count();
    }

    //纳秒
    int64_t elapsed_nano() const
    {
        return duration_cast<chrono::nanoseconds>(high_resolution_clock::now() - m_begin).count();
    }

    //秒
    int64_t elapsed_seconds() const
    {
        return duration_cast<chrono::seconds>(high_resolution_clock::now() - m_begin).count();
    }

    //分
    int64_t elapsed_minutes() const
    {
        return duration_cast<chrono::minutes>(high_resolution_clock::now() - m_begin).count();
    }

    //时
    int64_t elapsed_hours() const
    {
        return duration_cast<chrono::hours>(high_resolution_clock::now() - m_begin).count();
    }

private:
    time_point<high_resolution_clock> m_begin;
};

#endif //AUTOHDMAP_DATACHECK_TIMERUTIL_H
