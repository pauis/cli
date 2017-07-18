#ifndef __CLITL_HPP__
#define __CLITL_HPP__

#include <cstdio>
#include <iosfwd>
#include <locale>
#include <utility>

#ifdef UNIX
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#elif WIN32
#include <conio.h>
#include <Windows.h>
#endif

namespace clitl {
    /* Color class */
    enum class color {
#ifdef UNIX
        VOIDSPACE = -1,
        DEFAULT = 0,
        BLACK = 30,
        RED = 31,
        GREEN = 32,
        BROWN = 33,
        BLUE = 34,
        MAGENTA = 35,
        CYAN = 36,
#elif WIN32
        VOIDSPACE = -1,
        DEFAULT = 7,
        BLACK = 0,
        RED = 4,
        GREEN = 2,
        BROWN = 7, // Worthless
        BLUE = 1,
        MAGENTA = 7, // Worthless
        CYAN = 9,
#endif
    };

    /* Basic definitions and containers */
    typedef int coord_t;
    typedef int colornum_t;

    template <typename T>
    struct rect {
        std::pair<T, T> origin;
        std::pair<T, T> endpoint;
        color foreground;
    };

    /* Output buffer */
    template <typename charT, typename traits = std::char_traits<charT> >
    class basic_outbuf : public std::basic_streambuf<charT, traits> {
    protected:
        virtual typename traits::int_type
            overflow(typename traits::int_type c)
        {
            if (std::putchar(c) == EOF) {
                return traits::eof();
            }
            return traits::not_eof(c);
        }
    };

    typedef basic_outbuf<char> outbuf;
    typedef basic_outbuf<wchar_t> woutbuf;

    /* Output stream */
    template <typename charT, typename traits = std::char_traits<charT> >
    class basic_ostream : public std::basic_ostream<charT, traits> {
    public:
#ifdef UNIX
        struct winsize wsize;
#endif
#ifdef WIN32
        HANDLE termout_handle;
        CONSOLE_CURSOR_INFO termout_curinfo;
        CONSOLE_SCREEN_BUFFER_INFO termout_sbufinfo;
#endif
        explicit basic_ostream(basic_outbuf<charT, traits>* sb)
            : std::basic_ostream<charT, traits>(sb)
        {
#ifdef UNIX
            wsize = { 0 };
#elif WIN32
            termout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
        }

        basic_ostream<charT, traits>& moveto(const std::pair<coord_t, coord_t>& cord)
        {
            return *this;
        }

        std::pair<coord_t, coord_t> screensize()
        {
            static coord_t column;
            static coord_t row;

#ifdef UNIX
            column = wsize.ws_col;
            row = wsize.ws_row;
#elif WIN32
            GetConsoleScreenBufferInfo(termout_handle, &termout_sbufinfo);
            column = static_cast<coord_t>(termout_sbufinfo.dwSize.X);
            row = static_cast<coord_t>(termout_sbufinfo.dwSize.Y);
#endif

            return std::pair<coord_t, coord_t>(column, row);
        }

        basic_ostream<charT, traits>& operator<<
            (basic_ostream<charT, traits>& (*op)(basic_ostream<charT, traits>&))
        {
            return (*op)(*this);
        }
    };

    template <typename charT, typename traits>
    basic_ostream<charT, traits>& pre_process(basic_ostream<charT, traits>& os)
    {
        os << alternative_system_screenbuffer;
        os << hide_cursor;
        os << clear;
        return os;
    }

    template <typename charT, typename traits>
    basic_ostream<charT, traits>& post_process(basic_ostream<charT, traits>& os)
    {
        os << normal_system_screenbuffer;
        os << show_cursor;
        return os;
    }

    template <typename charT, typename traits>
    basic_ostream<charT, traits>& alternative_system_screenbuffer(basic_ostream<charT, traits>& os)
    {
#if UNIX
        cout << "\033[?1049h"; // Use alternate screen buffer
#endif
        return os;
    }

    template <typename charT, typename traits>
    basic_ostream<charT, traits>& clear(basic_ostream<charT, traits>& os)
    {
#ifdef UNIX
        cout << "\033[2J";
#elif WIN32
        COORD startpoint = { 0, 0 };
        DWORD dw;

        GetConsoleScreenBufferInfo(os.termout_handle,
            &os.termout_sbufinfo);
        FillConsoleOutputCharacterA(os.termout_handle, ' ',
            os.termout_sbufinfo.dwSize.X *
            os.termout_sbufinfo.dwSize.Y,
            startpoint, &dw);
        FillConsoleOutputAttribute(os.termout_handle,
            FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
            os.termout_sbufinfo.dwSize.X *
            os.termout_sbufinfo.dwSize.Y,
            startpoint, &dw);
#endif
        return os;
    }

    template <typename charT, typename traits>
    basic_ostream<charT, traits>& hide_cursor(basic_ostream<charT, traits>& os)
    {
#ifdef UNIX
        cout << "\033[?25l"; // Hide cursor
#elif WIN32
    GetConsoleCursorInfo(os.termout_handle,
        &os.termout_curinfo);
    os.termout_curinfo.bVisible = 0;
    SetConsoleCursorInfo(os.termout_handle,
        &os.termout_curinfo);
#endif
        return os;
    }

    template <typename charT, typename traits>
    basic_ostream<charT, traits>& normal_system_screenbuffer(basic_ostream<charT, traits>& os)
    {
#if UNIX
        cout << "\033[?1049l"; // Use normal screen buffer
#endif
        return os;
    }

    template <typename charT, typename traits>
    basic_ostream<charT, traits>& refresh(basic_ostream<charT, traits>& os)
    {
        fflush(stdout);
        return os;
    }

    template <typename charT, typename traits>
    basic_ostream<charT, traits>& show_cursor(basic_ostream<charT, traits>& os)
    {
#if UNIX
        cout << "\033[?25h"; // Show cursor
#elif WIN32
    CONSOLE_CURSOR_INFO termout_curinfo;
    GetConsoleCursorInfo(os.termout_handle, &termout_curinfo);
    termout_curinfo.bVisible = 1;
    SetConsoleCursorInfo(os.termout_handle, &termout_curinfo);
#endif
        return os;
    }

    typedef basic_ostream<char> ostream;
    typedef basic_ostream<wchar_t> wostream;

    /* Input buffer */

    /* Input stream */
    template <typename charT, typename traits = std::char_traits<charT> >
    class basic_istream : public std::basic_istream<charT, traits> {
        
    };

    typedef basic_istream<char> istream;
    typedef basic_istream<wchar_t> wistream;
}

#endif