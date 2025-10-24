// test_sensores.c
// Pruebas unitarias para sensor ultrasónico y sensor de turbidez usando Unity

#include <stdio.h>
#include <math.h>
#include "unity.h"
#include "sensor_ultrasonico.h"
#include "sensor_turbidez.h"
#include "esp_log.h"

static const char *TAG = "TESTS";

// Valores esperados (el usuario puede modificar estos valores reales
// para adaptarlos al entorno de prueba)
static const int nivel_real = 50; // distancia esperada en cm (ejemplo)
static const float turbidez_real = 1.5f; // voltaje esperado en V (ejemplo)

// Tolerancias
#define TOLERANCIA_CM 2
#define TOLERANCIA_TURBIDEZ_PERCENT 5.0f

// Test para sensor ultrasónico
void test_sensor_ultrasonico(void)
{
    int medida = medir_distancia();
    if (medida < 0) {
        printf("Fail: sensor ultrasónico (error lectura)\n");
        TEST_FAIL_MESSAGE("Lectura ultrasónica retornó error");
        return;
    }

    int diff = abs(medida - nivel_real);
    if (diff <= TOLERANCIA_CM) {
        printf("Pass: sensor ultrasónico\n");
    } else {
        printf("Fail: sensor ultrasónico (medida=%d cm, esperado=%d cm)\n", medida, nivel_real);
    }

    // Usa macro UNITY para aserción con tolerancia
    TEST_ASSERT_INT_WITHIN(TOLERANCIA_CM, nivel_real, medida);
}

// Test para sensor de turbidez (se basa en voltaje)
void test_sensor_turbidez(void)
{
    float volt = leer_turbidez();

    float allowed = turbidez_real * (TOLERANCIA_TURBIDEZ_PERCENT / 100.0f);
    float diff = fabsf(volt - turbidez_real);

    if (diff <= allowed) {
        printf("Pass: sensor turbidez\n");
    } else {
        printf("Fail: sensor turbidez (medida=%.3f V, esperado=%.3f V)\n", volt, turbidez_real);
    }

    // Aserción Unity con tolerancia relativa en voltios
    TEST_ASSERT_FLOAT_WITHIN(allowed, turbidez_real, volt);
}

// Runner que ejecuta los tests y muestra resultados por consola
void run_tests(void)
{
    UNITY_BEGIN();

    // Ejecuta cada test
    RUN_TEST(test_sensor_ultrasonico);
    // RUN_TEST(test_sensor_turbidez);

    UNITY_END();

    ESP_LOGI(TAG, "Pruebas finalizadas.");
}
