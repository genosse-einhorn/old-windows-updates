#pragma once

#include "string.hpp"

// FIXME! remove max line length limitation

class LineReader {
    HANDLE m_hfile;
    char   m_buf[2000];
    int    m_bufused;
    int    m_eof;

    LineReader(const LineReader &) {}
    void operator=(const LineReader&) {};

public:
    LineReader(const wstr &file): m_hfile(NULL), m_buf(), m_bufused(0), m_eof(0)
    {
        ZeroMemory(m_buf, sizeof(m_buf));
        m_hfile = CreateFile(file.cstr(),
                             GENERIC_READ,
                             FILE_SHARE_READ,
                             NULL,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL);
    }

    ~LineReader()
    {
        CloseHandle(m_hfile);
        m_hfile = NULL;
        m_eof = 1;
    }

    bool
    ok() const
    {
        return m_hfile != NULL && m_hfile != INVALID_HANDLE_VALUE;
    }

    bool
    eof() const
    {
        return m_eof > 0 && m_bufused == 0;
    }

    wstr
    nextline()
    {
        int i;

        for (i = 0; i < m_bufused; ++i) {
            if (m_buf[i] == '\n') {
                wstr retval = wstr::from_utf8(m_buf, i);
                MoveMemory(m_buf, &m_buf[i+1], m_bufused - i - 1);
                m_bufused = m_bufused - i - 1;
                return retval;
            }
        }

        // more data!
        while (m_bufused < (int)sizeof(m_buf)) {
            DWORD space = sizeof(m_buf)-m_bufused;

            DWORD read = 0;
            ReadFile(m_hfile, &m_buf[m_bufused], space, &read, NULL);
            if (read == 0) {
                // eof
                m_eof = 1;
                break;
            }
            m_bufused += read;
        }

        for (i = 0; i < m_bufused; ++i) {
            if (m_buf[i] == '\n') {
                wstr retval = wstr::from_utf8(m_buf, i);
                MoveMemory(m_buf, &m_buf[i+1], m_bufused - i - 1);
                m_bufused = m_bufused - i - 1;
                return retval;
            }
        }

        wstr retval = wstr::from_utf8(m_buf, m_bufused);
        m_bufused = 0;
        return retval;
    }
};
