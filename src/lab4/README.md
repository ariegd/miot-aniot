# Lee los datos del sensor
```
i2cset -c 0x40 0xE5
I (232216) cmd_i2ctools: Write OK
```
# Comando de Lectura Corregido (Largo)
Vamos a intentar decirle directamente que queremos leer 2 bytes usando el flag -l:
```
i2c-tools> i2cset -c 0x40 0xE5
I (1302536) cmd_i2ctools: Write OK
i2c-tools> i2cget -c 0x40 -l 2
0x00 0x00 


i2c-tools> i2cconfig --sda=21 --scl=22 --freq=100000
i2cdetect
i2cget -c 0x40 -r 0xE7 -l 1
```
