#pragma once

#include <windows.h>

#include <new>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4200)
#endif

template<typename T>
class dynarray
{
    struct priv {
        int size;
        int capacity;
        T   data[0];
    };

    priv *p;

    dynarray(const dynarray &) {}
    void operator=(const dynarray &) {}

    void extend(int new_cap)
    {
        SIZE_T allocsize = sizeof(*p) + new_cap * sizeof(T);

        if (!p) {
            p = (priv *)HeapAlloc(GetProcessHeap(), 0, allocsize);
            p->size = 0;
            p->capacity = new_cap;
        } else {
            void *t = HeapReAlloc(GetProcessHeap(), HEAP_REALLOC_IN_PLACE_ONLY, p, allocsize);
            if (t != NULL) {
                p->capacity = new_cap;
            } else {
                priv *newp = (priv *)HeapAlloc(GetProcessHeap(), 0, allocsize);
                newp->size = p->size;
                newp->capacity = new_cap;
                for (int i = 0; i < p->size; ++i) {
                    new (&newp->data[i]) T(p->data[i]);
                    p->data[i].~T();
                }
                HeapFree(GetProcessHeap(), 0, p);
                p = newp;
            }
        }
    }

public:
    dynarray(): p(NULL)
    {}

    ~dynarray()
    {
        if (p != NULL) {
            for (int i = 0; i < p->size; ++i) {
                p->data[i].~T();
            }

            HeapFree(GetProcessHeap(), 0, p);
            p = NULL;
        }
    }

    T &
    operator[](int index)
    {
        return p->data[index];
    }

    const T &
    operator[](int index) const
    {
        return p->data[index];
    }

    int
    size() const
    {
        if (p != NULL)
            return p->size;
        else
            return 0;
    }

    int
    capacity() const
    {
        if (p != NULL)
            return p->capacity;
        else
            return 0;
    }

    int
    push(const T &e)
    {
        if (capacity() > size()) {
            int i = p->size++;
            new ((char*)&p->data[i]) T(e);
            return i;
        } else {
            int newcap = capacity() * 2;
            if (newcap < 2)
                newcap = 2;

            extend(newcap);
            return push(e);
        }
    }

    // TODO: insert, erase, pop, ...
};


#ifdef _MSC_VER
#pragma warning(pop)
#endif
