#include <stdint.h>
#include <propeller.h>

class PinGroup {
  private:
    uint32_t m_mask;
    int m_shift;
  public:
    PinGroup(int high, int low);
    ~PinGroup() {};
    void set(uint32_t value);
    uint32_t get();
};
