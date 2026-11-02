/* Common headers compile without CMSIS, even when a target macro is defined. */
#include <uhal/uhal.h>
#include <ucore/time.h>
#include <stdio.h>
#if defined(_WIN32)
#include <windows.h>
#else
#include <pthread.h>
#endif
#define COUNT 20000u
static unsigned counter;
static void work(void) {
    for (unsigned i = 0; i < COUNT; ++i) {
        uhal_critical_t outer = uhal_critical_enter();
        unsigned before = counter;
        uhal_critical_t inner = uhal_critical_enter();
        counter = before + 1u;
        uhal_critical_exit(inner);
        uhal_critical_exit(outer);
    }
}
#if defined(_WIN32)
static DWORD WINAPI worker(LPVOID unused) { (void)unused; work(); return 0; }
#else
static void* worker(void* unused) { (void)unused; work(); return NULL; }
#endif
int main(void) {
#if defined(_WIN32)
    HANDLE threads[4];
    for (unsigned i = 0; i < 4; ++i) { threads[i] = CreateThread(NULL, 0, worker, NULL, 0, NULL); if (!threads[i]) return 1; }
    if (WaitForMultipleObjects(4, threads, TRUE, 30000) != WAIT_OBJECT_0) return 1;
    for (unsigned i = 0; i < 4; ++i) { CloseHandle(threads[i]); }
#else
    pthread_t threads[4];
    for (unsigned i = 0; i < 4; ++i) { if (pthread_create(&threads[i], NULL, worker, NULL)) return 1; }
    for (unsigned i = 0; i < 4; ++i) { if (pthread_join(threads[i], NULL)) return 1; }
#endif
    if (counter != 4u * COUNT) { printf("Critical-section race: %u\n", counter); return 1; }
    if (uhal_irq_enable(0, 0, 0) != UETD_NOT_SUPPORTED) return 1;
    uetd_time_us_t start = uetd_time_now_us();
    uetd_delay_us(1000u);
    if (uetd_time_now_us() - start < 1000u) return 1;
    UHAL_DMB(); UHAL_DSB(); UHAL_ISB(); UHAL_NOP();
    return 0;
}
