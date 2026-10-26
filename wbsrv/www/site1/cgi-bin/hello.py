#!/usr/bin/env python3
import os
import sys

query = os.environ.get("QUERY_STRING", "")
method = os.environ.get("REQUEST_METHOD", "")
script_name = os.environ.get("SCRIPT_NAME", "")
path_info = os.environ.get("PATH_INFO", "")
gateway = os.environ.get("GATEWAY_INTERFACE", "")
protocol = os.environ.get("SERVER_PROTOCOL", "")
custom_hdr = os.environ.get("HTTP_X_CUSTOM_TEST", "")

# Lecture d'un fichier en chemin relatif (preuve que chdir() a bien été fait)
rel_data = "absent"
try:
    with open("data.txt", "r") as f:
        rel_data = f.read().strip()
except Exception:
    pass

if "mode=redirect" in query:
    sys.stdout.write("Location: /index.html\r\n\r\n")
    sys.exit(0)

if "mode=status201" in query:
    sys.stdout.write("Status: 201 Created\r\n")
    sys.stdout.write("Content-Type: text/plain; charset=utf-8\r\n\r\n")
    sys.stdout.write("created_via_cgi\n")
    sys.exit(0)

if "mode=badheader" in query:
    sys.stdout.write("ceci_nest_pas_un_header_valide\r\n\r\ncorps\n")
    sys.exit(0)

if "mode=crash" in query:
    sys.exit(1)

sys.stdout.write("Content-Type: text/plain; charset=utf-8\r\n")
sys.stdout.write("X-Cgi-Executed: yes\r\n\r\n")
sys.stdout.write(f"CGI_HELLO_OK\n")
sys.stdout.write(f"METHOD={method}\n")
sys.stdout.write(f"QUERY={query}\n")
sys.stdout.write(f"SCRIPT_NAME={script_name}\n")
sys.stdout.write(f"PATH_INFO={path_info}\n")
sys.stdout.write(f"GATEWAY={gateway}\n")
sys.stdout.write(f"PROTOCOL={protocol}\n")
sys.stdout.write(f"CUSTOM_HEADER={custom_hdr}\n")
sys.stdout.write(f"RELATIVE_DATA={rel_data}\n")
