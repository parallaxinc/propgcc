#include "term.h"
#include "term_vga.h"
#include "term_tv.h"
#include "term_serial.h"

class CTerm {
  protected:
    TERM *m_term;
  public:
  	CTerm(TERM *term) : m_term(term) {};
  	~CTerm() {};
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

class CDisplayTerm : public CTerm {
  public:
  	CDisplayTerm(TERM *term) : CTerm(term) {}
  	~CDisplayTerm() {};
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

class CVgaTerm : public CDisplayTerm {
  private:
    TERM_VGA m_vgaTerm;
  public:
    CVgaTerm(int basepin) : CDisplayTerm(&m_vgaTerm.term)
    {
    	vgaTerm_start(&m_vgaTerm, basepin);
    };
    ~CVgaTerm()
    {
    	vgaTerm_stop(&m_vgaTerm.term);
    };
};

class CTvTerm : public CDisplayTerm {
  private:
    TERM_TV m_tvTerm;
  public:
    CTvTerm(int basepin) : CDisplayTerm(&m_tvTerm.term)
    {
    	tvTerm_start(&m_tvTerm, basepin);
    };
    ~CTvTerm()
    {
    	tvTerm_stop(&m_tvTerm.term);
    };
};

class CSerialTerm : public CTerm {
  private:
    TERM_SERIAL m_serialTerm;
  public:
    CSerialTerm(FILE *fp) : CTerm(&m_serialTerm.term)
    {
    	serialTerm_start(&m_serialTerm, fp);
    };
    ~CSerialTerm()
    {
    	serialTerm_stop(&m_serialTerm.term);
    };
};

