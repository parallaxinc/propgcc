#include "term_common.h"
#include "term_serial.h"
#include "term_vga.h"
#include "term_tv.h"

class Term {
  protected:
    TERM *m_term;
  public:
  	Term(TERM *term);
  	~Term();
    void str(const char *str);
    void dec(int val);
    void hex(int val, int digits = 8);
    void bin(int val, int digits = 32);
};

class SerialTerm : public Term {
  private:
    TERM_SERIAL m_serialTerm;
  public:
    SerialTerm(FILE *fp);
    ~SerialTerm();
};

class DisplayTerm : public Term {
  public:
  	DisplayTerm(TERM *term);
  	~DisplayTerm();
    void clearScreen();
	int out(int c);
	void setColorPalette(char *palette);
	int getTileColor(int x, int y);
	void setTileColor(int x, int y, int color);
	void setCurPosition(int x, int y);
	void setCoordPosition(int x, int y);
	void setXY(int x, int y);
	void setX(int value);
	void setY(int value);
	int getX();
	int getY();
	void setColors(int value);
	int getColors();
	int getColumns();
	int getRows();
	void print(char* s);
	int putch(int c);
};

class VgaTerm : public DisplayTerm {
  private:
    TERM_VGA m_vgaTerm;
  public:
    VgaTerm(int basepin);
    ~VgaTerm();
};

class TvTerm : public DisplayTerm {
  private:
    TERM_TV m_tvTerm;
  public:
    TvTerm(int basepin);
    ~TvTerm();
};

/* inline definitions for Term */

inline Term::Term(TERM *term) : m_term(term)
{
}

inline Term::~Term()
{
}

inline void Term::str(const char *str)
{
    Term_str(m_term, str);
}

inline void Term::dec(int val)
{
    Term_dec(m_term, val);
}

inline void Term::hex(int val, int digits)
{
    Term_hex(m_term, val, digits);
}

inline void Term::bin(int val, int digits)
{
    Term_bin(m_term, val, digits);
}

/* inline definitions for SerialTerm */

inline SerialTerm::SerialTerm(FILE *fp) : Term(&m_serialTerm.term)
{
    serialTerm_start(&m_serialTerm, fp);
}

inline SerialTerm::~SerialTerm()
{
    serialTerm_stop(&m_serialTerm.term);
}

/* inline definitions for DisplayTerm */

inline DisplayTerm::DisplayTerm(TERM *term) : Term(term)
{
}

inline DisplayTerm::~DisplayTerm()
{
}

inline void DisplayTerm::clearScreen()
{
    Term_clearScreen(m_term);
}

inline int DisplayTerm::out(int c)
{
    return Term_out(m_term, c);
}

inline void DisplayTerm::setColorPalette(char *palette)
{
    Term_setColorPalette(m_term, palette);
}

inline int DisplayTerm::getTileColor(int x, int y)
{
    return Term_getTileColor(m_term, x, y);
}

inline void DisplayTerm::setTileColor(int x, int y, int color)
{
    Term_setTileColor(m_term, x, y, color);
}

inline void DisplayTerm::setCurPosition(int x, int y)
{
    Term_setCurPosition(m_term, x, y);
}

inline void DisplayTerm::setCoordPosition(int x, int y)
{
    Term_setCoordPosition(m_term, x, y);
}

inline void DisplayTerm::setXY(int x, int y)
{
    Term_setXY(m_term, x, y);
}

inline void DisplayTerm::setX(int value)
{
    Term_setX(m_term, value);
}

inline void DisplayTerm::setY(int value)
{
    Term_setY(m_term, value);
}

inline int DisplayTerm::getX()
{
    return Term_getX(m_term);
}

inline int DisplayTerm::getY()
{
    return Term_getY(m_term);
}

inline void DisplayTerm::setColors(int value)
{
    Term_setColors(m_term, value);
}

inline int DisplayTerm::getColors()
{
    return Term_getColors(m_term);
}

inline int DisplayTerm::getColumns()
{
    return Term_getColumns(m_term);
}

inline int DisplayTerm::getRows()
{
    return Term_getRows(m_term);
}

inline void DisplayTerm::print(char* s)
{
    Term_print(m_term, s);
}

inline int DisplayTerm::putch(int c)
{
    return Term_putchar(m_term, c);
}

/* inline definitions for VgaTerm */

inline VgaTerm::VgaTerm(int basepin) : DisplayTerm(&m_vgaTerm.term)
{
    vgaTerm_start(&m_vgaTerm, basepin);
}

inline VgaTerm::~VgaTerm()
{
    vgaTerm_stop(&m_vgaTerm.term);
}

/* inline definitions for TvTerm */

inline TvTerm::TvTerm(int basepin) : DisplayTerm(&m_tvTerm.term)
{
    tvTerm_start(&m_tvTerm, basepin);
}

inline TvTerm::~TvTerm()
{
    tvTerm_stop(&m_tvTerm.term);
}
