/* glibc includes */
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

static void print_usage_and_die(void) __attribute__((__noreturn__));
static void dump_ring(void);
static int  init_ring(void);
static void subtract_ring(void);
static int  not_zero(void);
static void cpy_prev(void);

const char * prog_name = "ring_math";
uint8_t *    this_ring = NULL;
uint8_t *    prev_ring = NULL;
size_t       n_ele = 0;

int
main(int    argc,
     char * argv[])
{
    int n = 0;

    if (argc != 2) {
        print_usage_and_die();
    }

    n = atoi(argv[1]);
    if (n <= 0) {
        fprintf(stderr, "error: need a positive number: %s\n", argv[1]);
    }

    n_ele = (size_t) n;

    printf("nodes = %d\n", n);

    if (init_ring()) {
        return EXIT_FAILURE;
    }

    while (not_zero()) {
        cpy_prev();
        subtract_ring();
        dump_ring();
        sleep(1);
    }

    return EXIT_SUCCESS;
}

static void
print_usage_and_die(void)
{
    printf("usage\n");
    printf(" ./run/%s <number>\n", prog_name);
    exit(EXIT_FAILURE);
}

static int
init_ring(void)
{
    int fd_r = 0;
    fd_r = open("/dev/urandom", O_RDONLY);

    if (fd_r <= 0) {
        int errsv = errno;
        fprintf(stderr, "error: open(\"/dev/urandom\") failed: %d\n", errsv);
        return -1;
    }

    this_ring = malloc(n_ele * sizeof(uint8_t));

    if (!this_ring || this_ring == NULL) {
        fprintf(stderr, "error: malloc(%zu * %zu) failed\n", n_ele,
                sizeof(uint8_t));
        return -1;
    }

    prev_ring = malloc(n_ele * sizeof(uint8_t));

    if (!prev_ring || prev_ring == NULL) {
        fprintf(stderr, "error: malloc(%zu * %zu) failed\n", n_ele,
                sizeof(uint8_t));
        return -1;
    }

    ssize_t n_read = read(fd_r, this_ring, n_ele * sizeof(uint8_t));

    if (n_read <= 0) {
        int errsv = errno;
        fprintf(stderr, "error: read returned %d\n", errsv);
        return -1;
    }

    if ((size_t) n_read != n_ele * sizeof(uint8_t)) {
        fprintf(stderr, "error: tried to read %zu, got %zu\n",
                n_ele * sizeof(uint8_t), n_read);
        return -1;
    }

    return 0;
}

static void
subtract_ring(void)
{
    for (size_t i = 0; i < n_ele - 1; ++i) {
        this_ring[i] = abs(prev_ring[i] - prev_ring[i + 1]);
    }

    this_ring[n_ele - 1] = abs(prev_ring[n_ele - 1] - prev_ring[0]);

    return;
}

static void
dump_ring(void)
{
    for (size_t i = 0; i < n_ele; ++i) {
        printf("%d ", this_ring[i]);
    }
    printf("\n");

    return;
}

static int
not_zero(void)
{
    for (size_t i = 0; i < n_ele; ++i) {
        if (this_ring[i] > 0) {
            return 1;
        }
    }

    return 0;
}

static void
cpy_prev(void)
{
    for (size_t i = 0; i < n_ele; ++i) {
        prev_ring[i] = this_ring[i];
    }
}
