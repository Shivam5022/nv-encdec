#include "../include/DecHeader.h"


int main()
{
  DecodeProc dec(0, "/home/satyam/dev/nv_encdec/input.mp4", OutputFormat::bgra64, "/home/satyam/dev/nv_encdec/output");
  int print_times = 5;
  while (true) {
    auto f = dec.getNext();
    if (print_times-- > 0) {
      std::cout << "Address of first character: " << static_cast<const void*>(f) << std::endl;
    }
    if (f == nullptr) {
      break;
    }
  }

  // DecodeProc2(0, "/home/satyam/dev/nv_encdec/input.mp4", OutputFormat::bgra64, "/home/satyam/dev/nv_encdec/output");
}
