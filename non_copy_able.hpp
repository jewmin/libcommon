#ifndef __LIBCOMMON_NON_COPY_ABLE_HPP__
#define __LIBCOMMON_NON_COPY_ABLE_HPP__

class NonCopyAble
{
protected:
    NonCopyAble() = default;
    ~NonCopyAble() = default;

    NonCopyAble(const NonCopyAble &) = delete;
    NonCopyAble & operator = (const NonCopyAble &) = delete;
};

#endif