#include "term_common.h"
#include "term_serial.h"
#include "term_vga.h"
#include "term_tv.h"

class Term {
  protected:
    TERM *m_term;
  public:
  	Term(TERM *term) : m_term(term) {};
  	~Term() {};
    void str(const char *str)
    {
    	Term_str(m_term, str);
    };
    void dec(int val)
    {
    	Term_dec(m_term, val);
    };
    void hex(int val, int digits = 8)
    {
    	Term_hex(m_term, val, digits);
    };
    void bin(int val, int digits = 32)
    {
    	Term_bin(m_term, val, digits);
    };
};

class DisplayTerm : public Term {
  public:
  	DisplayTerm(TERM *term) : Term(term) {}
  	~DisplayTerm() {};
    void clearScreen()
    {
    	Term_clearScreen(m_term);
    };
	int out(int c)
    {
    	return Term_out(m_term, c);
    };
	void setColorPalette(char *palette)
    {
    	Term_setColorPalette(m_term, palette);
    };
	int getTileColor(int x, int y)
    {
    	return Term_getTileColor(m_term, x, y);
    };
	void setTileColor(int x, int y, int color)
    {
    	Term_setTileColor(m_term, x, y, color);
    };
	void setCurPosition(int x, int y)
    {
    	Term_setCurPosition(m_term, x, y);
    };
	void setCoordPosition(int x, int y)
    {
    	Term_setCoordPosition(m_term, x, y);
    };
	void setXY(int x, int y)
    {
    	Term_setXY(m_term, x, y);
    };
	void setX(int value)
    {
    	Term_setX(m_term, value);
    };
	void setY(int value)
    {
    	Term_setY(m_term, value);
    };
	int getX()
    {
    	return Term_getX(m_term);
    };
	int getY()
    {
    	return Term_getY(m_term);
    };
	void setColors(int value)
    {
    	Term_setColors(m_term, value);
    };
	int getColors()
    {
    	return Term_getColors(m_term);
    };
	int getColumns()
    {
    	return Term_getColumns(m_term);
    };
	int getRows()
    {
    	return Term_getRows(m_term);
    };
	void print(char* s)
    {
    	Term_print(m_term, s);
    };
	int putch(int c)
    {
    	return Term_putchar(m_term, c);
    };
};

class VgaTerm : public DisplayTerm {
  private:
    TERM_VGA m_vgaTerm;
  public:
    VgaTerm(int basepin) : DisplayTerm(&m_vgaTerm.term)
    {
    	vgaTerm_start(&m_vgaTerm, basepin);
    };
    ~VgaTerm()
    {
    	vgaTerm_stop(&m_vgaTerm.term);
    };
};

class TvTerm : public DisplayTerm {
  private:
    TERM_TV m_tvTerm;
  public:
    TvTerm(int basepin) : DisplayTerm(&m_tvTerm.term)
    {
    	tvTerm_start(&m_tvTerm, basepin);
    };
    ~TvTerm()
    {
    	tvTerm_stop(&m_tvTerm.term);
    };
};

class SerialTerm : public Term {
  private:
    TERM_SERIAL m_serialTerm;
  public:
    SerialTerm(FILE *fp) : Term(&m_serialTerm.term)
    {
    	serialTerm_start(&m_serialTerm, fp);
    };
    ~SerialTerm()
    {
    	serialTerm_stop(&m_serialTerm.term);
    };
};

