// Custom encoding using a-z A-Z 0-9 as charset. Random song (2:41): https://abstract.land/tidal/redirect/79691533

#include <string>
#include <vector>
#include <utility>
#include <math.h>

std::string encode(unsigned long long in)
{
  std::string out;
  std::vector<std::uint16_t> nums;
  std::uint16_t mod;
  do
  {
    mod = in % 0x3E;
    in = in / 0x3E;
    nums.push_back(mod);
  } while (in > 0);
  std::vector<std::uint16_t> reversed(nums.rbegin(), nums.rend());
  for (auto &i : reversed)
  {
    char conv = i + 0x3D;
    if (i < 0xA)
      conv = i + 0x30;
    if (i > 0x9 && i < 0x24)
      conv = i + 0x37;
    out += conv;
  }
  return out;
};

unsigned long long decode(std::string in)
{
  unsigned long long out = 0;
  std::string rev(in.rbegin(), in.rend());
  for (int i = 0; i < rev.length(); i++)
  {
    int c = rev[i] - 0x30;
    if (c > 0x9)
      c -= 0x7;
    if (c > 0x23)
      c -= 0x6;
    out += c * std::pow(0x3E, i);
  }
  return out;
}

// Example
#include <chrono>
int main()
{
  int epoch = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  std::string enc = encode(epoch);
  auto dec = decode(enc);
  printf("Epoch:   %d\nEncoded: %s\nDecoded: %d\n", epoch, enc.c_str(), dec);
}