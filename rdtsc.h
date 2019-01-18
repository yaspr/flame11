//
static inline uint64_t rdtsc (void)
{
   uint64_t a, d;

   __asm__ volatile ("rdtsc" : "=a" (a), "=d" (d));

   return (d<<32) | a;
}
