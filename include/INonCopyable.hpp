#pragma once

struct INonCopyable
{
    INonCopyable() = default;

    INonCopyable(const INonCopyable&) = delete;

    INonCopyable&
    operator=(const INonCopyable&) = delete;
};