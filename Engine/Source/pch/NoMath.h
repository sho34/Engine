#pragma once

inline bool IsPowerOfTwo(unsigned int n) { return (n > 0) && ((n & (n - 1)) == 0); }
inline unsigned int PrevPowerOfTwo(unsigned int x) { x = x | (x >> 1); x = x | (x >> 2); x = x | (x >> 4); x = x | (x >> 8); x = x | (x >> 16); return x - (x >> 1); }
