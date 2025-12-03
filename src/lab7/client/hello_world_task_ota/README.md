| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C5 | ESP32-C6 | ESP32-C61 | ESP32-H2 | ESP32-H21 | ESP32-H4 | ESP32-P4 | ESP32-S2 | ESP32-S3 | Linux |
| ----------------- | ----- | -------- | -------- | -------- | -------- | --------- | -------- | --------- | -------- | -------- | -------- | -------- | ----- |

# Práctica 7. Over-The-Air Updates (OTA)
El objetivo de esta práctica es familiarizarse con el concepto de OTA, la actualización del firmware de forma remota. Específicamente, usaremos el interfaz simplificado que ofrece ESP-IDF para realizar la actualización de aplicaciones.

Trabajaremos los siguientes aspectos:
* Actualización de firmware mendiante HTTPS.
* Incorporación de certificados en nuestra aplicación.
* Firmado de binarios.

## Crear Certificado
```
openssl req -nodes -newkey rsa:2048 -keyout example.key -out example.crt -x509 -days 365
```
▪ Crea un certificado SSL/TLS auto-firmado
▪ -x509 indica que es self-signed en lugar de hacer un CSR (Certificate Signing Request)
▪ -days indica el tiempo de validez del certificado
▪ CN -> usar la IP del servidor (o nombre de dominio si tenemos)
Similar: 
```
openssl req –nodes -newkey rsa:2048 -keyout ca_key.pem -out ca_cert.pem -x509 -days 365
```
▪ .key is la clave privada
▪ .crt es el certificado firmado
▪ .pem indica que puede ser clave o certificado, pero Base64
❑ openssl genrsa -out my_secure_boot_signing_key.pem 3072
▪ Crea una clave privada
❑ Lanzar un servidor HTTPS en el directorio actual
▪ openssl s_server -WWW -key ca_key.pem -cert ca_cert.pem -port 8070
▪ La clave privada, certificado e imágenes que se quieran descargar deben estar en ese directorio
