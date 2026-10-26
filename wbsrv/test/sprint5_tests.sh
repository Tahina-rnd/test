#!/bin/bash

cd "$(dirname "$0")/.." || exit 1

BIN=./webserv
PORT=18095
TMP=/tmp/ws_sprint5_$$
CONF=/tmp/ws_sprint5.conf
LOG=/tmp/ws_sprint5.log
SRVPID=
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
    if printf '%s' "$haystack" | grep -q -- "$needle"; then
        PASS=$((PASS + 1))
        echo "  [OK] $desc"
    else
        FAIL=$((FAIL + 1))
        echo "  [FAIL] $desc (mot-clé absent: '$needle')"
    fi
}

http_code() {
    curl -s -o /dev/null -w '%{http_code}' "$@"
}

raw_send() {
    local port="$1"
    local payload
    printf -v payload '%b' "$2"
    WS_PAYLOAD="$payload" python3 - "$port" <<'PY'
import os, socket, sys
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.settimeout(4)
try:
    s.connect(("127.0.0.1", int(sys.argv[1])))
    s.sendall(os.environ["WS_PAYLOAD"].encode("latin1"))
    res = b""
    while True:
        try:
            chunk = s.recv(4096)
        except Exception:
            break
        if not chunk:
            break
        res += chunk
    s.close()
    sys.stdout.write(res.decode("latin1", errors="replace"))
except Exception as e:
    sys.stderr.write(str(e))
PY
}

cleanup() {
    if [ -n "$SRVPID" ] && kill -0 "$SRVPID" 2>/dev/null; then
        kill "$SRVPID" 2>/dev/null
        wait "$SRVPID" 2>/dev/null
    fi
    rm -rf "$TMP" "$CONF" "$LOG"
}
trap cleanup EXIT

mkdir -p "$TMP"

cat << CFG > "$CONF"
http {
    server {
        listen $PORT;
        server_name localhost;
        root www/site1;
        index index.html;
        client_max_body_size 2M;
        error_page 404 /errors/404.html;

        location / {
            allow_methods GET POST DELETE;
            root www/site1;
            autoindex off;
            index index.html;
        }

        location /cgi-bin {
            allow_methods GET POST;
            root www/site1;
            cgi_ext .py;
            cgi_pass /usr/bin/python3;
        }

        location /bad-cgi {
            allow_methods GET POST;
            root www/site1;
            cgi_ext .py;
            cgi_pass /usr/bin/nonexistent_python_binary_xyz;
        }
    }
}
CFG

"$BIN" "$CONF" > "$LOG" 2>&1 &
SRVPID=$!
sleep 0.6

if ! kill -0 "$SRVPID" 2>/dev/null; then
    echo "Erreur : webserv n'a pas pu démarrer :"
    cat "$LOG"
    exit 1
fi

echo "=========================================================="
echo "         TESTS D'INTÉGRATION SPRINT 5 : CGI"
echo "=========================================================="

FD_INITIAL=$(ls -1 "/proc/$SRVPID/fd" 2>/dev/null | wc -l | tr -d ' ')

echo -e "\n1. GET CGI simple, variables CGI/1.1 et accès fichier relatif (chdir) :"
RESP=$(curl -s -i -H "X-Custom-Test: Bravo42" "http://127.0.0.1:$PORT/cgi-bin/hello.py?user=tahina&lang=fr")
assert_contains "GET /cgi-bin/hello.py renvoie 200 OK" "HTTP/1.1 200 OK" "$RESP"
assert_contains "Header Content-Type transmis" "Content-Type: text/plain; charset=utf-8" "$RESP"
assert_contains "Header custom CGI transmis" "X-Cgi-Executed: yes" "$RESP"
assert_contains "Corps CGI contient CGI_HELLO_OK" "CGI_HELLO_OK" "$RESP"
assert_contains "REQUEST_METHOD=GET" "METHOD=GET" "$RESP"
assert_contains "QUERY_STRING transmis" "QUERY=user=tahina&lang=fr" "$RESP"
assert_contains "SCRIPT_NAME=/cgi-bin/hello.py" "SCRIPT_NAME=/cgi-bin/hello.py" "$RESP"
assert_contains "GATEWAY_INTERFACE=CGI/1.1" "GATEWAY=CGI/1.1" "$RESP"
assert_contains "SERVER_PROTOCOL=HTTP/1.1" "PROTOCOL=HTTP/1.1" "$RESP"
assert_contains "Header HTTP_X_CUSTOM_TEST transmis" "CUSTOM_HEADER=Bravo42" "$RESP"
assert_contains "Lecture fichier relatif après chdir()" "RELATIVE_DATA=donnee_locale_cgi_ok" "$RESP"

echo -e "\n2. PATH_INFO dans l'URL CGI :"
RESP=$(curl -s -i "http://127.0.0.1:$PORT/cgi-bin/hello.py/extra/segment?x=1")
assert_contains "GET avec PATH_INFO renvoie 200 OK" "HTTP/1.1 200 OK" "$RESP"
assert_contains "SCRIPT_NAME découpé avant PATH_INFO" "SCRIPT_NAME=/cgi-bin/hello.py" "$RESP"
assert_contains "PATH_INFO=/extra/segment" "PATH_INFO=/extra/segment" "$RESP"

echo -e "\n3. POST CGI avec corps normal et binaire :"
printf 'Bonjour CGI POST 12345\nSeconde ligne\n' > "$TMP/post.txt"
RESP=$(curl -s -i -X POST -H "Content-Type: text/plain" --data-binary @"$TMP/post.txt" \
    "http://127.0.0.1:$PORT/cgi-bin/echo.py")
assert_contains "POST /cgi-bin/echo.py renvoie 200 OK" "HTTP/1.1 200 OK" "$RESP"
assert_contains "X-Echo-Method: POST" "X-Echo-Method: POST" "$RESP"
assert_contains "X-Echo-Length exact" "X-Echo-Length: 37" "$RESP"
assert_contains "Corps POST restitué par le CGI" "Bonjour CGI POST 12345" "$RESP"

python3 -c 'open("'"$TMP"'/bin.dat", "wb").write(bytes(range(256)) * 20)'
CODE=$(curl -s -o "$TMP/bin.out" -w '%{http_code}' -X POST \
    -H "Content-Type: application/octet-stream" \
    --data-binary @"$TMP/bin.dat" "http://127.0.0.1:$PORT/cgi-bin/echo.py")
assert_eq "POST binaire 5 Ko vers CGI renvoie 200" "200" "$CODE"
cmp -s "$TMP/bin.dat" "$TMP/bin.out"
assert_eq "Sortie binaire du CGI identique octet pour octet" "0" "$?"

echo -e "\n4. POST CGI chunked (corps dé-chunké avant stdin CGI) :"
RESP=$(raw_send "$PORT" "POST /cgi-bin/echo.py HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\n6\r\nChunkA\r\n7\r\n-ChunkB\r\n0\r\n\r\n")
assert_contains "POST chunked CGI renvoie 200 OK" "HTTP/1.1 200 OK" "$RESP"
assert_contains "Taille dé-chunkée = 13" "X-Echo-Length: 13" "$RESP"
assert_contains "Corps dé-chunké reçu sans tailles hex" "ChunkA-ChunkB" "$RESP"

echo -e "\n5. En-têtes CGI Status: et Location: :"
RESP=$(curl -s -i "http://127.0.0.1:$PORT/cgi-bin/hello.py?mode=status201")
assert_contains "Status: 201 Created interprété" "HTTP/1.1 201 Created" "$RESP"
assert_contains "Corps 201 présent" "created_via_cgi" "$RESP"

RESP=$(curl -s -i "http://127.0.0.1:$PORT/cgi-bin/hello.py?mode=redirect")
assert_contains "Location CGI sans Status produit 302 Found" "HTTP/1.1 302 Found" "$RESP"
assert_contains "Header Location présent" "Location: /index.html" "$RESP"

echo -e "\n6. Cas d'erreur CGI (404, 405, 502) :"
CODE=$(http_code "http://127.0.0.1:$PORT/cgi-bin/inexistant.py")
assert_eq "Script CGI absent renvoie 404" "404" "$CODE"

RESP=$(curl -s -i -X DELETE "http://127.0.0.1:$PORT/cgi-bin/hello.py")
assert_contains "DELETE sur /cgi-bin renvoie 405" "HTTP/1.1 405 Method Not Allowed" "$RESP"
assert_contains "Allow: GET, POST" "Allow: GET, POST" "$RESP"

CODE=$(http_code "http://127.0.0.1:$PORT/cgi-bin/hello.py?mode=badheader")
assert_eq "Sortie CGI malformée renvoie 502" "502" "$CODE"

CODE=$(http_code "http://127.0.0.1:$PORT/cgi-bin/hello.py?mode=crash")
assert_eq "Script CGI qui exit(1) renvoie 502" "502" "$CODE"

echo -e "\n7. Keep-alive sur requêtes CGI successives :"
RESP=$(raw_send "$PORT" "GET /cgi-bin/hello.py?req=1 HTTP/1.1\r\nHost: localhost\r\n\r\nGET /cgi-bin/hello.py?req=2 HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n")
COUNT_OK=$(printf '%s' "$RESP" | grep -c "HTTP/1.1 200 OK")
assert_eq "2 requêtes CGI en keep-alive sur 1 connexion" "2" "$COUNT_OK"
assert_contains "Première réponse CGI présente" "QUERY=req=1" "$RESP"
assert_contains "Seconde réponse CGI présente" "QUERY=req=2" "$RESP"

echo -e "\n8. Non-blocage pendant un CGI lent + Timeout 10s (504) + Zéro zombie :"
curl -s -o /dev/null -w '%{http_code}' --max-time 15 \
    "http://127.0.0.1:$PORT/cgi-bin/slow.py" > "$TMP/slow_code" &
SLOW_CURL_PID=$!
sleep 0.5

# Pendant que slow.py tourne, le serveur DOIT répondre immédiatement aux autres clients
STATIC_CODE=$(http_code --max-time 2 "http://127.0.0.1:$PORT/index.html")
assert_eq "Serveur non bloqué pendant slow.py (GET statique -> 200)" "200" "$STATIC_CODE"

FAST_CGI_CODE=$(http_code --max-time 2 "http://127.0.0.1:$PORT/cgi-bin/hello.py")
assert_eq "Autre CGI rapide répond 200 pendant que slow.py tourne" "200" "$FAST_CGI_CODE"

wait "$SLOW_CURL_PID"
SLOW_CODE=$(cat "$TMP/slow_code")
assert_eq "CGI lent (>10s) tué et renvoie 504 Gateway Timeout" "504" "$SLOW_CODE"

sleep 0.3
ZOMBIES=$(ps -o pid,ppid,stat,comm --ppid "$SRVPID" 2>/dev/null | grep -c 'Z')
assert_eq "Aucun processus enfant zombie (<defunct>)" "0" "$ZOMBIES"

FD_FINAL=$(ls -1 "/proc/$SRVPID/fd" 2>/dev/null | wc -l | tr -d ' ')
assert_eq "Aucune fuite de descripteur de fichier (pipes/sockets)" "$FD_INITIAL" "$FD_FINAL"

echo -e "\n=========================================================="
echo "RÉSULTATS SPRINT 5 : PASS=$PASS | FAIL=$FAIL"
echo "=========================================================="

[ "$FAIL" -eq 0 ]
