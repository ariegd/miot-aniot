# Práctica 7. Over-The-Air Updates (OTA)
1. Generar los certificados de tu servidor. Importante cuando soliciten el FQND colocar la IP de tu ordenador
**Nota: Una vez generado el certificado, no se puede cambiar el nombre** 
```
openssl req -nodes -newkey rsa:2048 -keyout ca_key.pem -out ca_cert.pem -x509 -days 365
```
2. En el ejemplo `simple_ota_example` de ESP, en el directorio `simple_ota_example/server_certs` copiar el certificado generado. 
**Nota: Si es preciso en el `main/CMakeLists.txt` cambiar el nombre al certificado que generaste.**

3. Seguir las indicaciones del pdf agregado.

4. El servidor python `server/server_ota.py` ejecutar en el directorio donde se encuentre el certificado y el *.bin
