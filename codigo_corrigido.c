#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(meu_modulo, LOG_LEVEL_DBG);

// --- Prioridades e tempos ---
#define PRIO_THREAD 7        // Prioridade das threads
#define MAX_COUNT 50000      // Até quanto elas vão ter que contar

volatile int count = 0;      // contador (volatile para evitar otimizações)
bool Terminou_A = false, Terminou_B = false;

/* ---- MUDANÇAS MÍNIMAS AQUI ---- */
/* mutex para proteger `count` */
K_MUTEX_DEFINE(count_mutex);
/* semáforo para sinalizar término das threads A e B (capacidade 2) */
K_SEM_DEFINE(done_sem, 0, 2);
/* ---- fim mudanças mínimas ---- */

void thread_A(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    for (int i = 0; i < MAX_COUNT/2; i++)
    {
        /* proteção mínima: lock apenas ao fazer count++ */
        k_mutex_lock(&count_mutex, K_FOREVER);
        count++;
        k_mutex_unlock(&count_mutex);
    }
    Terminou_A = true;

    LOG_DBG("Thread A terminou!\n");

    /* sinaliza que terminou (menor mudança: give uma vez) */
    k_sem_give(&done_sem);
}

void thread_B(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    for (int i = 0; i < MAX_COUNT/2; i++)
    {
        /* proteção mínima: lock apenas ao fazer count++ */
        k_mutex_lock(&count_mutex, K_FOREVER);
        count++;
        k_mutex_unlock(&count_mutex);
    }
    Terminou_B = true;

    LOG_DBG("Thread B terminou!\n");

    /* sinaliza que terminou */
    k_sem_give(&done_sem);
}

void thread_print(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    /* Espera as duas threads sinalizarem */
    k_sem_take(&done_sem, K_FOREVER); // espera 1ª
    k_sem_take(&done_sem, K_FOREVER); // espera 2ª

    /* Leitura protegida (opcional, mas consistente) */
    k_mutex_lock(&count_mutex, K_FOREVER);
    int final_count = count;
    k_mutex_unlock(&count_mutex);

    LOG_INF("As Threads Contaram: %d, e o valor esperado era %d", final_count, MAX_COUNT);

    /* opcional: parar aqui (evita loops infinitos)
       deixa a thread em sleep indefinido */
    k_sleep(K_FOREVER);
}

K_THREAD_DEFINE(a_tid, 2048, thread_A, NULL, NULL, NULL, PRIO_THREAD, 0, 0);
K_THREAD_DEFINE(b_tid, 2048, thread_B, NULL, NULL, NULL, PRIO_THREAD, 0, 0);
K_THREAD_DEFINE(c_tid, 2048, thread_print, NULL, NULL, NULL, 4, 0, 0);

void main(void)
{
    LOG_INF("Entrou na main thread");

    while (1) {
        k_sleep(K_FOREVER);
    }
}
