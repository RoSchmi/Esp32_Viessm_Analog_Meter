#include <Print.h>

#ifndef NULLPRINT_H_
#define NULLPRINT_H_
class NullPrint : public Print 
{
  public: 
  size_t write(uint8_t) override;
};
#endif