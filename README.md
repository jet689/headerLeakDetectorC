# headerLeakDetectorC
this is a simple header based leak detector
# origin
this is based off of https://github.com/namantam1/memory-leak-detector that I helped contribute to with a complete a ground up rewrite, it uses a hash table for near O(1) access while the original had O(n) access where n is the number of allocations done, and provides much more managable errors and warnings, and pushes them to text files to prevent filling the terminal. 
# usage
to use just put ```#include "leak2.h"``` at the very top of the main file of your program
# how it works
it just replaces malloc realloc calloc and free using macros resulting a very drop in system
# output
this is the output of the example test provided

```10
0========================
number of allocations: 26
number of frees: 5
number of reallocs: 2
number of leaks: 21
allocated amount: 128 bytes
freed amount: 24 bytes
leaked amount: 104 bytes
memory allocation table scaled to 14 buckets
========================
leak(s) detected check leak2leaks.txt
========================
warning(s) detected check leak2warnings.txt```

leak2leaks.txt
``` 9 leak(s) at leak2test.c:11 totalling 36 bytes with 0 realloc bytes
9 leak(s) at leak2test.c:16 totalling 36 bytes with 0 realloc bytes
1 leak(s) at leak2test.c:27 totalling 8 bytes with 4 realloc bytes
1 leak(s) at leak2test.c:35 totalling 4 bytes with 0 realloc bytes
1 leak(s) at leak2test.c:40 totalling 8 bytes with 0 realloc bytes```

leak2warnings.txt
```(10x) LEAK-DETECTOR-WARNING::UNKNOWN FREE leak2test.c:22```
# improvements
swapping to open addressing as opposed to seperate chainging would help with cache coherence and potentially lead to faster accessing
