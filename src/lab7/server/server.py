import http.server
import ssl

# Configuración
server_address = ('0.0.0.0', 8070)
httpd = http.server.HTTPServer(server_address, http.server.SimpleHTTPRequestHandler)

# Envolver el socket con SSL
ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
ctx.load_cert_chain(certfile='./server_cert.pem', keyfile='./server_key.pem')
httpd.socket = ctx.wrap_socket(httpd.socket, server_side=True)

print(f"Servidor HTTPS OTA corriendo en https://<TU_IP_PC>:8070")
httpd.serve_forever()
