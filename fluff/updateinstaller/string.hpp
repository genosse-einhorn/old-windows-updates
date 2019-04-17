#pragma once

#include <windows.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4200)
#endif

class wstr {
    struct header {
#ifdef _MSC_VER
        long  refcnt;
#else
        ULONG refcnt;
#endif
        int   length;
        WCHAR data[0];
    };

    struct header *ptr;

    void clear() {
        if (ptr != NULL) {
            InterlockedDecrement(&ptr->refcnt);
            if (0 == ptr->refcnt) {
                HeapFree(GetProcessHeap(), 0, ptr);
            }
            ptr = NULL;
        }
    }

public:
    wstr(): ptr(NULL)
    {
        //noop
    }

    wstr(const WCHAR *lpwsz, int len = -1): ptr(NULL)
    {
        if (!lpwsz)
            return; // empty string as NULL is allowed

        if (len == -1)
            len = lstrlenW(lpwsz);

        ptr = (struct header *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                                         sizeof(struct header) + (len + 1) * sizeof(WCHAR));
        ptr->refcnt = 1;
        ptr->length = len;
        CopyMemory(ptr->data, lpwsz, len*sizeof(WCHAR));
    }

    wstr(const wstr &other): ptr(other.ptr)
    {
        if (ptr != NULL) {
            InterlockedIncrement(&ptr->refcnt);
        }
    }

    void
    operator=(const wstr &other)
    {
        clear();
        ptr = other.ptr;
        if (ptr != NULL) {
            InterlockedIncrement(&ptr->refcnt);
        }
    }

    ~wstr()
    {
        clear();
    }

    int
    size() const
    {
        if (ptr == NULL)
            return 0;
        else
            return ptr->length;
    }

    const WCHAR *
    cstr() const
    {
        if (ptr == NULL) {
            return L"";
        } else {
            return ptr->data;
        }
    }

    const WCHAR &
    operator[](const int index) const
    {
        return cstr()[index];
    }

    wstr
    operator+(const wstr &other) const
    {
        wstr ret;

        ULONG retlen = size() + other.size();
        if (retlen == 0)
            return ret;

        ret.ptr = (struct header*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                            sizeof(struct header) + (retlen + 1)*sizeof(WCHAR));
        ret.ptr->refcnt = 1;
        ret.ptr->length = retlen;
        CopyMemory(&ret.ptr->data[0], cstr(), size()*sizeof(WCHAR));
        CopyMemory(&ret.ptr->data[size()], other.cstr(), other.size()*sizeof(WCHAR));

        return ret;
    }

    wstr
    trimmed() const
    {
        int left = 0;
        int right = 0;
        int i;
        for (i = 0; i < size(); ++i) {
            int c = (*this)[i];
            if (c != ' ' && c != '\t' && c != '\r' && c != '\n' && c != '\v')
                break;

            left = i + 1;
        }

        for (i = size()-1; i > left; --i) {
            int c = (*this)[i];
            if (c != ' ' && c != '\t' && c != '\r' && c != '\n' && c != '\v')
                break;

            right = size() - i;
        }

        return substr(left, size() - left - right);
    }

    wstr
    substr(int start, int count=2147483647) const
    {
        if (size() == 0)
            return wstr();

        if (start < 0)
            start = size() + start;
        if (start < 0)
            start = 0;

        if (start >= size())
            return wstr();

        if (count >= size() - start)
            count = size() - start;

        if (start == 0 && count == size())
            return *this;

        return wstr(&cstr()[start], count);
    }

    bool
    startswith(const wstr &other) const
    {
        if (other.size() > size())
            return false;

        for (int i = 0; i < other.size(); ++i) {
            if ((*this)[i] != other[i])
                return false;
        }

        return true;
    }

    bool
    startswith(const WCHAR *other) const
    {
        for (int i = 0; i < size(); ++i) {
            if (other[i] == 0)
                return true;

            if ((*this)[i] != other[i])
                return false;
        }

        return false;
    }

    bool
    endswith(const wstr &other) const
    {
        if (other.size() > size())
            return false;

        for (int i = 0; i < other.size(); ++i) {
            if ((*this)[size() - other.size() + i] != other[i])
                return false;
        }

        return true;
    }

    static wstr
    from_utf8(const char *lpsz, int length = -1)
    {
        wstr retval;

        if (length < 0)
            length = lstrlenA(lpsz);

        int widelen = MultiByteToWideChar(CP_UTF8, 0, lpsz, length, NULL, 0);
        retval.ptr = (struct header*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                               sizeof(struct header) + (widelen + 1)*sizeof(WCHAR));
        retval.ptr->refcnt = 1;
        retval.ptr->length = widelen;

        MultiByteToWideChar(CP_UTF8, 0, lpsz, length, retval.ptr->data, widelen);

        return retval;
    }
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif
