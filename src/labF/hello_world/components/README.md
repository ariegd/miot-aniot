# ESP-IDF 5.x usa el sistema de componentes gestionados.

## Error de CMake aparece cuando dos componentes distintos con el mismo nombre lógico (icm42670) 
* esp-idf-lib__icm42670 (el de UncleRus / esp-idf-lib)
* espressif__icm42670 (el oficial de Espressif)

ESP-IDF no sabe cuál usar, así que falla.

## Solución: Alternativa: renombrar el componente del registro que quieras usar localmente
Si no quieres copiar todo, clona el repo `esp-idf-lib/icm42670` dentro de `components/` y cambia su name en `idf_component.yml`. Esto lo convierte en componente local y evita la gestión por registro.

## Crear un componente i2c_comp el Circuito integrado (I2C) compartiendo
* , incluir únicamente el driver nuevo. Adactar el hello_world_main.c para utilizar el i2c_comp que llame a sht3c_comp.c y icm_comp.c. Tener en cuenta que el icm_comp.c tiene la dependencia espressif/icm42670 que internamente utiliza el driver nuevo (driver/i2c_master.h)
* El objetivo es crear un "Singleton" o un recurso compartido para el bus I2C, inicializarlo una sola vez en el main, y pasar ese handle (manejador) a los componentes sht3c y icm.
