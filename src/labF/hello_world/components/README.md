# ESP-IDF 5.x usa el sistema de componentes gestionados.

## Error de CMake aparece cuando dos componentes distintos con el mismo nombre lógico (icm42670) 
* esp-idf-lib__icm42670 (el de UncleRus / esp-idf-lib)
* espressif__icm42670 (el oficial de Espressif)

ESP-IDF no sabe cuál usar, así que falla.

## Solución: Alternativa: renombrar el componente del registro que quieras usar localmente
Si no quieres copiar todo, clona el repo `esp-idf-lib/icm42670` dentro de `components/` y cambia su name en `idf_component.yml`. Esto lo convierte en componente local y evita la gestión por registro.
