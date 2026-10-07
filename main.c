#include <stdint.h>

/* ---- Memory-mapped addresses: change to match your SoC/testbench ---- */
#define MAILBOX  (*(volatile uint32_t *)0x10000000u) /* TB watches this: 1 = PASS, 2 = FAIL */
#define UART_TX  (*(volatile uint32_t *)0x10000004u) /* TB prints low byte as a char */

/* ---- Custom MAC instruction: edit opcode/funct to match YOUR accelerator ----
   custom-0 opcode = 0x0B, R-type encoding. rd = rs1 * rs2 + rd_old is a common MAC style;
   adjust the semantics in mac_ref() below to match your hardware. */
#define CUSTOM_MAC(rd, a, b) \
    __asm__ volatile (".insn r 0x0B, 0x0, 0x00, %0, %1, %2" : "=r"(rd) : "r"(a), "r"(b))

static void uart_puts(const char *s) { while (*s) UART_TX = (uint32_t)*s++; }

static int check(const char *name, uint32_t got, uint32_t exp)
{
    uart_puts(name);
    if (got == exp) { uart_puts(": PASS\n"); return 0; }
    uart_puts(": FAIL\n");
    return 1;
}

/* Test 1: sum of an array (loads, adds, loops, branches) */
static int test_array_sum(void)
{
    static const uint32_t a[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint32_t s = 0;
    for (int i = 0; i < 8; i++) s += a[i];
    return check("array_sum", s, 36);
}

/* Test 2: fibonacci (register use, loop control) */
static int test_fib(void)
{
    uint32_t a = 0, b = 1;
    for (int i = 0; i < 10; i++) { uint32_t t = a + b; a = b; b = t; }
    return check("fib10", a, 55);
}

/* Test 3: store/load round trip through memory (data memory path) */
static int test_mem(void)
{
    volatile uint32_t buf[4];
    for (int i = 0; i < 4; i++) buf[i] = 0xA5A50000u + i;
    uint32_t ok = 1;
    for (int i = 0; i < 4; i++) if (buf[i] != 0xA5A50000u + i) ok = 0;
    return check("mem_rw", ok, 1);
}

/* Test 4: custom MAC accelerator vs. software reference */
static uint32_t mac_ref(uint32_t a, uint32_t b) { return a * b; /* edit to match your MAC */ }

static int test_mac(void)
{
    uint32_t r, fails = 0;
    static const uint32_t va[4] = {3, 7, 10, 12};
    static const uint32_t vb[4] = {4, 5, 10, 12};
    for (int i = 0; i < 4; i++) {
        CUSTOM_MAC(r, va[i], vb[i]);
        fails += (r != mac_ref(va[i], vb[i]));
    }
    return check("custom_mac", fails, 0);
}

int main(void)
{
    int fails = 0;
    uart_puts("RV32I bare-metal tests\n");
    fails += test_array_sum();
    fails += test_fib();
    fails += test_mem();
    fails += test_mac();
    MAILBOX = fails ? 2 : 1;   /* tell the testbench we are done */
    while (1) ;
}
