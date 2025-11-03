#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(meu_modulo, LOG_LEVEL_DBG);

// --- Prioridades e tempos ---
#define PRIO_THREAD 7        // Prioridade das threads
#define MAX_COUNT 5000       // Até quanto elas vão ter que contar

volatile int count = 0;      // contador (volatile para evitar otimizações)
bool Terminou_A = false, Terminou_B = false;

void thread_A(void *p1, void *p2, void *p3)
{
    for (int i = 0; i < MAX_COUNT / 2; i++)
    {
        count++;
    }
    Terminou_A = true;

    LOG_DBG("Thread A terminou!\n");
}

void thread_B(void *p1, void *p2, void *p3)
{
    for (int i = 0; i < MAX_COUNT / 2; i++)
    {
        count++;
    }
    Terminou_B = true;

    LOG_DBG("Thread B terminou!\n");
}

void thread_print(void *p1, void *p2, void *p3)
{
    if (Terminou_A && Terminou_B)
    {
        LOG_DBG("As Threads Contaram: %d, e o valor esperado era %d", count, MAX_COUNT);
        k_msleep(5);
        LOG_INF("Terminando o programa");
    }
    k_msleep(200);
}

K_THREAD_DEFINE(a_tid, 2048, thread_A, NULL, NULL, NULL,
                PRIO_THREAD, 0, 0);

K_THREAD_DEFINE(b_tid, 2048, thread_B, NULL, NULL, NULL,
                PRIO_THREAD, 0, 0);

K_THREAD_DEFINE(c_tid, 2048, thread_print, NULL, NULL, NULL,
                4, 0, 0);

void main(void)
{
    LOG_INF("Entrou na main thread");

    while (1)
    {
        k_sleep(K_FOREVER);
    }
}
