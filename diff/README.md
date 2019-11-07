# Diff

Makes a `diff` between two strings, pointing out the different characters.

## Example

### In C

As in `main.c`...

```c
#include <stdio.h>
#include "diff.h"

int main(int argc, char **argv)
{
	if (argc != 3) return 1;
	int cost = diff(argv[1], argv[2]);
	if (cost == -1) return 1;
	printf("difference = %d\n", cost);
	return 0;
}
```

### In the command line

``` bash
$ ./diff algorithm algorism
difference = 10
```
