#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#ifdef HAS_BACKTRACE
#include <execinfo.h>
#else
#include <assert.h>
#include <sys/ucontext.h>
typedef struct
{
	const char *dli_fname;
	void *dli_fbase;
	const char *dli_sname;
	void *dli_saddr;
} Dl_info;

static int backtrace(void **array, int size)
{
	if (size <= 0)
	{
		return 0;
	}

	int *fp = 0, *next_fp = 0;
	int cnt = 0;
	int ret = 0;

	__asm__ __volatile__(
		"mov %0, fp\n"
		: "=r"(fp)
	);

	array[cnt++] = (void *)(*(fp-1));
	next_fp = (int *)(*(fp-3));
	while ((cnt <= size) && (next_fp != 0))
	{
		array[cnt++] = (void *)*(next_fp - 1);
		next_fp = (int *)(*(next_fp-3));
	}

	ret = ((cnt <= size) ? cnt : size);
	printf("--->>Backstrace (%d deep)\n", ret);

	return ret;
}

static char ** backtrace_symbols(void *const *array, int size)
{
    #define WORD_WIDTH 8
	Dl_info info[size];
	int status[size];
	int cnt;
	size_t total = 0;
	char **result;

	for (cnt = 0; cnt < size; ++cnt)
	{
		status[cnt] = dladdr(array[cnt], &info[cnt]);
		if (status[cnt] && info[cnt].dli_fname && info[cnt].dli_fname[0] != '\0')
		{
    		/* <file-name>(<sym-name>) [+offset].  */
    		total += (strlen (info[cnt].dli_fname ?: "")
    			+ (info[cnt].dli_sname ? strlen (info[cnt].dli_sname) + 3 + WORD_WIDTH + 3 : 1)
    			+ WORD_WIDTH + 5);
		}
		else
		{
			total += 5 + WORD_WIDTH;
		}
	}

	result = (char **) malloc (size * sizeof (char *) + total);
	if (result != NULL)
	{
		char *last = (char *) (result + size);

		for (cnt = 0; cnt < size; ++cnt)
		{
			result[cnt] = last;
			if (status[cnt] && info[cnt].dli_fname && info[cnt].dli_fname[0] != '\0')
			{
				char buf[20];
				if (array[cnt] >= (void *) info[cnt].dli_saddr)
				{
					sprintf (buf, "+%#lx", (unsigned long)(array[cnt] - info[cnt].dli_saddr));
				}
				else
				{
					sprintf (buf, "-%#lx", (unsigned long)(info[cnt].dli_saddr - array[cnt]));
				}

				last += 1 + sprintf (last, "%s%s%s%s%s[%p]",
					info[cnt].dli_fname ?: "",
					info[cnt].dli_sname ? "(" : "",
					info[cnt].dli_sname ?: "",
					info[cnt].dli_sname ? buf : "",
					info[cnt].dli_sname ? ") " : " ",
					array[cnt]);
			}
			else
			{
				last += 1 + sprintf (last, "[%p]", array[cnt]);
			}
		}
		assert (last <= (char *) result + size * sizeof (char *) + total);
	}

	return result;
}
#endif

void dump_stack(unsigned int sn , siginfo_t  *si , void *ptr)
{
	if (NULL != ptr)
	{
		printf("------------>>unhandled page fault (%d) at: 0x%08x<<------------\n", si->si_signo, si->si_addr);

        #define SIZE 100
		void *buffer[SIZE];
		int i, nptrs;
		char **strings;
		nptrs = backtrace(buffer, SIZE);
		strings = backtrace_symbols(buffer, nptrs);
		if (strings == NULL)
		{
		    return;
		}

		for (i = 0; i < nptrs; i++)
		{
			printf("--->>%d: %s\n", i+1, strings[i]);
		}
		free(strings);
	}
	else
	{
		printf("--->>dump_stack error!\n");
	}

    exit(-1);
}

static void signal_handler(void)
{
    struct sigaction s;
    s.sa_flags = SA_SIGINFO;
    s.sa_sigaction = (void *)dump_stack;
    sigaction(SIGSEGV, &s, NULL);
}

int main(int argc, char **argv)
{
	signal_handler();
	return 0;
}

