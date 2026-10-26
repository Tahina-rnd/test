#!/bin/bash

cd "$(dirname "$0")/.." || exit 1

BIN=./webserv
PASS=0
FAIL=0

if [ ! -x "$BIN" ]; then
    echo "Compiler webserv d'abord (make)"
    exit 1
fi

assert_eq() {
    local desc="$1"
    local expected="$2"
    local actual="$3"
    if [ "$expected" = "$actual" ]; then
        PASS=$((PASS + 1))
        echo "  [OK] $desc"
    else
        FAIL=$((FAIL + 1))
        echo "  [FAIL] $desc (attendu: '$expected', obtenu: '$actual')"
    fi
}

assert_contains() {
    local desc="$1"
    local needle="$2"
    local haystack="$3"
    if echo "$haystack" | grep -q "$needle"; then
        PASS=$((PASS + 1))
        echo "  [OK] $desc"
    else
        FAIL=$((FAIL + 1))
        echo "  [FAIL] $desc (mot-clé absent: '$needle')"
    fi
}

# Helper pour envoyer des octets bruts par socket via Python
raw_send() {
    local host="$1"
    local port="$2"
    local payload="$3"
    python3 -c "
import socket, sys
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.settimeout(2)
try:
    s.connect(('$host', int('$port')))
    s.sendall(b'''$payload''')
    res = b''
    while True:
        try:
            chunk = s.recv(4096)
            if not chunk:
                break
            res += chunk
        except Exception:
            break
    s.close()
    sys.stdout.write(res.decode('latin1', errors='replace'))
except Exception as e:
    sys.stderr.write(str(e))
"
}

echo "=========================================================="
echo "         TESTS D'INTÉGRATION SPRINT 3 : WEBSERV"
echo "=========================================================="

# Création de fichiers d'assets de test dans www/site1
mkdir -p www/site1/assets www/site1/auto_dir www/site1/noauto_dir
echo "body { color: blue; }" > www/site1/assets/style.css
echo "console.log('test');" > www/site1/assets/app.js
echo "Hello plain text" > www/site1/assets/hello.txt
echo "test auto file 1" > www/site1/auto_dir/file1.txt
echo "test auto file 2" > www/site1/auto_dir/file2.txt
echo "secret file" > www/site1/noauto_dir/secret.txt

# Configuration de test pour Sprint 3
cat << 'CFG' > /tmp/ws_sprint3.conf
http {
    server {
        listen 18080;
        server_name alpha.local;
        root www/site1;
        index index.html;
        client_max_body_size 1M;

        location / {
            allow_methods GET;
            root www/site1;
            autoindex off;
            index index.html;
        }

        location /assets {
            allow_methods GET;
            root www/site1;
        }

        location /auto_dir {
            allow_methods GET;
            root www/site1;
            autoindex on;
        }

        location /noauto_dir {
            allow_methods GET;
            root www/site1;
            autoindex off;
        }

        location /postonly {
            allow_methods POST;
            root www/site1;
        }

        location /redirect {
            return 301 http://localhost:18080/assets/hello.txt;
        }
    }

    server {
        listen 18080;
        server_name beta.local;
        root www/site2;
        index index.html;

        location / {
            allow_methods GET;
            root www/site2;
            index index.html;
        }
    }
}
CFG

# Démarrage du serveur de test
"$BIN" /tmp/ws_sprint3.conf > /tmp/ws_sprint3.log 2>&1 &
SRVPID=$!
sleep 0.8

if ! kill -0 $SRVPID 2>/dev/null; then
    echo "Erreur : webserv n'a pas pu démarrer :"
    cat /tmp/ws_sprint3.log
    exit 1
fi

echo -e "\n1. Fichiers statiques et types MIME :"
RESP=$(curl -s -i http://localhost:18080/index.html)
assert_contains "GET /index.html code 200 OK" "HTTP/1.1 200 OK" "$RESP"
assert_contains "MIME text/html" "Content-Type: text/html" "$RESP"
assert_contains "Contenu HTML site 1" "Site 1 — port 8080" "$RESP"

RESP=$(curl -s -i http://localhost:18080/assets/style.css)
assert_contains "MIME text/css" "Content-Type: text/css" "$RESP"
assert_contains "Contenu CSS" "color: blue" "$RESP"

RESP=$(curl -s -i http://localhost:18080/assets/app.js)
assert_contains "MIME application/javascript" "Content-Type: application/javascript" "$RESP"

RESP=$(curl -s -i http://localhost:18080/assets/hello.txt)
assert_contains "MIME text/plain" "Content-Type: text/plain" "$RESP"
assert_contains "Contenu TXT" "Hello plain text" "$RESP"

echo -e "\n2. Routage Virtual Hosts (Host header) :"
RESP_ALPHA=$(curl -s -H "Host: alpha.local" http://localhost:18080/)
assert_contains "Host: alpha.local renvoie Site 1" "Site 1" "$RESP_ALPHA"

RESP_BETA=$(curl -s -H "Host: beta.local" http://localhost:18080/)
assert_contains "Host: beta.local renvoie Site 2" "Site 2" "$RESP_BETA"

RESP_DEFAULT=$(curl -s -H "Host: unknown.local" http://localhost:18080/)
assert_contains "Host inconnu renvoie le serveur par défaut (Site 1)" "Site 1" "$RESP_DEFAULT"

echo -e "\n3. Méthodes autorisées (Allow & 405 Method Not Allowed) :"
RESP=$(curl -s -i -X GET http://localhost:18080/postonly)
assert_contains "GET sur location postonly renvoie 405" "HTTP/1.1 405 Method Not Allowed" "$RESP"
assert_contains "Header Allow présent avec POST" "Allow: POST" "$RESP"

echo -e "\n4. Redirections return 301/302 :"
RESP=$(curl -s -i http://localhost:18080/redirect)
assert_contains "Redirection renvoie 301 Moved Permanently" "HTTP/1.1 301 Moved Permanently" "$RESP"
assert_contains "Header Location présent" "Location: http://localhost:18080/assets/hello.txt" "$RESP"

echo -e "\n5. Sécurité : protection Path Traversal (403 Forbidden) :"
RESP=$(curl --path-as-is -s -i "http://localhost:18080/../../../../etc/passwd")
assert_contains "Tentative traversal renvoie 403 Forbidden" "HTTP/1.1 403 Forbidden" "$RESP"

RESP=$(curl --path-as-is -s -i "http://localhost:18080/assets/../../")
assert_contains "Remontée au-dessus de racine renvoie 403 Forbidden" "HTTP/1.1 403 Forbidden" "$RESP"

echo -e "\n6. Erreur 404 Not Found :"
RESP=$(curl -s -i http://localhost:18080/fichier_inexistant_xyz.html)
assert_contains "Fichier absent renvoie 404 Not Found" "HTTP/1.1 404 Not Found" "$RESP"

echo -e "\n7. Répertoires, slash final et Autoindex :"
RESP=$(curl -s -i http://localhost:18080/assets)
assert_contains "Dossier sans slash renvoie redirection 301" "HTTP/1.1 301 Moved Permanently" "$RESP"
assert_contains "Location redirige vers /assets/" "Location: /assets/" "$RESP"

RESP=$(curl -s -i http://localhost:18080/auto_dir/)
assert_contains "autoindex on renvoie 200 OK" "HTTP/1.1 200 OK" "$RESP"
assert_contains "Listing contient file1.txt" "file1.txt" "$RESP"
assert_contains "Listing contient file2.txt" "file2.txt" "$RESP"

RESP=$(curl -s -i http://localhost:18080/noauto_dir/)
assert_contains "autoindex off sans index renvoie 403 Forbidden" "HTTP/1.1 403 Forbidden" "$RESP"

echo -e "\n8. Conformité HTTP/1.1 et Keep-Alive :"
# Absence de Host en 1.1 -> 400
RESP=$(raw_send "localhost" "18080" "GET / HTTP/1.1\r\n\r\n")
assert_contains "HTTP/1.1 sans Host renvoie 400 Bad Request" "HTTP/1.1 400 Bad Request" "$RESP"

# Version inconnue -> 505
RESP=$(raw_send "localhost" "18080" "GET / HTTP/2.0\r\nHost: alpha.local\r\n\r\n")
assert_contains "Version HTTP/2.0 renvoie 505" "HTTP/1.1 505 HTTP Version Not Supported" "$RESP"

# Keep-Alive : 2 requêtes séquentielles sur le même canal
RESP_MULTI=$(raw_send "localhost" "18080" "GET /assets/hello.txt HTTP/1.1\r\nHost: localhost\r\n\r\nGET /index.html HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n")
OCCURRENCES=$(echo "$RESP_MULTI" | grep -c "HTTP/1.1 200 OK")
assert_eq "Keep-alive traite 2 requêtes sur la même connexion" "2" "$OCCURRENCES"

# Arrêt du serveur de test
kill $SRVPID 2>/dev/null
wait $SRVPID 2>/dev/null
rm -rf www/site1/assets www/site1/auto_dir www/site1/noauto_dir /tmp/ws_sprint3.conf /tmp/ws_sprint3.log

echo -e "\n=========================================================="
echo "RÉSULTATS SPRINT 3 : PASS=$PASS | FAIL=$FAIL"
echo "=========================================================="

[ "$FAIL" -eq 0 ]
