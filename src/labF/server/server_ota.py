# Guardar como server_ota.py
import http.server
import ssl

# Configuración
server_address = ('0.0.0.0', 8070)
httpd = http.server.HTTPServer(server_address, http.server.SimpleHTTPRequestHandler)

# Envolver el socket con SSL
try:
    ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    # Reemplaza los nombres de archivo si son diferentes
    ctx.load_cert_chain(certfile='./ca_cert.pem', keyfile='./ca_key.pem') 
    httpd.socket = ctx.wrap_socket(httpd.socket, server_side=True)
except FileNotFoundError:
    print("ERROR: Asegúrate de que ca_cert.pem y ca_key.pem existan en esta carpeta.")
    exit(1)

print(f"Servidor HTTPS OTA corriendo en https://192.168.1.39:8070")
httpd.serve_forever()
