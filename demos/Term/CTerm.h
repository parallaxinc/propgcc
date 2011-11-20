#include "term.h"
#include "term_vga.h"
#include "term_tv.h"

class CTerm {
  private:
    TERM *m_term;
  public:
  	CTerm(TERM *term) : m_term(term) {};
  	~CTerm() {}
    void str(char *str)
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

class CVgaTerm : public CTerm {
  private:
    TERM_VGA m_vgaTerm;
  public:
    CVgaTerm(int basepin) : CTerm(&m_vgaTerm.term)
    {
    	vgaTerm_start(&m_vgaTerm, basepin);
    };
    ~CVgaTerm()
    {
    	vgaTerm_stop(&m_vgaTerm.term);
    };
};

class CTvTerm : public CTerm {
  private:
    TERM_TV m_tvTerm;
  public:
    CTvTerm(int basepin) : CTerm(&m_tvTerm.term)
    {
    	tvTerm_start(&m_tvTerm, basepin);
    };
    ~CTvTerm()
    {
    	tvTerm_stop(&m_tvTerm.term);
    };
};
