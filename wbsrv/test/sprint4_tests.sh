#!/bin/bash

cd "$(dirname "$0")/.." || exit 1

BIN=./webserv
PORT=18090
PORT_FALLBACK=18091
STORE=www/site1/s4up
TMP=/tmp/ws_sprint4_$$
CONF=/tmp/ws_sprint4.conf
LOG=/tmp/ws_sprint4.log
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

assert_not_contains() {
    local desc="$1"
    local needle="$2"
    local haystack="$3"
    if printf '%s' "$haystack" | grep -q -- "$needle"; then
        FAIL=$((FAIL + 1))
        echo "  [FAIL] $desc (mot-clé inattendu: '$needle')"
    else
        PASS=$((PASS + 1))
        echo "  [OK] $desc"
    fi
}

http_code() {
    curl -s -o /dev/null -w '%{http_code}' "$@"
}

location_of() {
    printf '%s\n' "$1" | tr -d '\r' | grep -i '^Location:' \
        | sed 's/^[Ll]ocation: *//'
}

count_files() {
    find "$STORE" -maxdepth 1 -type f | wc -l | tr -d ' '
}

raw_send() {
    local port="$1"
    local payload
    printf -v payload '%b' "$2"
    WS_PAYLOAD="$payload" python3 - "$port" <<'PY'
import os, socket, sys
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.settimeout(3)
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
    rm -rf "$STORE" "$TMP" www/site1/s4_dir www/site1/s4_delete.txt
    rm -f www/site1/s4_link "$CONF" "$LOG"
}
trap cleanup EXIT

rm -rf "$STORE"
mkdir -p "$STORE" "$TMP" www/site1/s4_dir
printf 'delete me\n' > www/site1/s4_delete.txt

cat << 'CFG' > "$CONF"
http {
    server {
        listen 18090;
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

        location /s4up {
            allow_methods GET POST DELETE;
            root www/site1;
            upload_store www/site1/s4up;
            autoindex off;
        }

        location /getonly {
            allow_methods GET;
            root www/site1;
        }

        location /free {
            root www/site1;
        }

        location /nostore {
            allow_methods GET POST;
            root www/site1;
            upload_store www/site1/.missing_s4_store;
        }
    }

    server {
        listen 18091;
        server_name fallback.local;
        root www/site2;
        error_page 404 /errors/absent.html;

        location / {
            allow_methods GET;
            root www/site2;
        }
    }
}
CFG

"$BIN" "$CONF" > "$LOG" 2>&1 &
SRVPID=$!
sleep 0.8

if ! kill -0 "$SRVPID" 2>/dev/null; then
    echo "Erreur : webserv n'a pas pu démarrer :"
    cat "$LOG"
    exit 1
fi

echo "=========================================================="
echo "         TESTS D'INTÉGRATION SPRINT 4 : WEBSERV"
echo "=========================================================="

echo -e "\n1. Régressions GET et pages d'erreur :"
RESP=$(curl -s -i "http://127.0.0.1:$PORT/index.html")
assert_contains "GET /index.html renvoie 200" "HTTP/1.1 200 OK" "$RESP"
assert_contains "Le site 1 est toujours servi" "Site 1" "$RESP"

RESP=$(curl -s -i "http://127.0.0.1:$PORT/inexistant")
assert_contains "404 personnalisée servie" "HTTP/1.1 404 Not Found" "$RESP"
assert_contains "Corps de la 404 personnalisée" "Page introuvable" "$RESP"

RESP=$(curl -s -i "http://127.0.0.1:$PORT_FALLBACK/inexistant")
assert_contains "Fallback 404 sans fichier personnalisé" "HTTP/1.1 404 Not Found" "$RESP"
assert_contains "Corps 404 par défaut" "404 Not Found" "$RESP"

echo -e "\n2. POST généré automatiquement :"
printf 'payload genere\n' > "$TMP/generated.txt"
RESP=$(curl -s -i -X POST --data-binary @"$TMP/generated.txt" \
    "http://127.0.0.1:$PORT/s4up")
assert_contains "POST sans nom renvoie 201" "HTTP/1.1 201 Created" "$RESP"
LOC=$(location_of "$RESP")
assert_contains "Location du fichier généré" "/s4up/upload_" "$LOC"
CODE=$(curl -s -o "$TMP/generated.out" -w '%{http_code}' \
    "http://127.0.0.1:$PORT$LOC")
assert_eq "GET du fichier généré renvoie 200" "200" "$CODE"
cmp -s "$TMP/generated.txt" "$TMP/generated.out"
assert_eq "Contenu généré identique octet pour octet" "0" "$?"

echo -e "\n3. POST nommé, binaire et nom encodé :"
python3 -c 'open("'"$TMP"'/binary.bin", "wb").write(bytes(range(256)))'
RESP=$(curl -s -i -X POST --data-binary @"$TMP/binary.bin" \
    "http://127.0.0.1:$PORT/s4up/binary.bin")
assert_contains "POST /s4up/binary.bin renvoie 201" "HTTP/1.1 201 Created" "$RESP"
assert_contains "Location du fichier nommé" "Location: /s4up/binary.bin" "$RESP"
CODE=$(curl -s -o "$TMP/binary.out" -w '%{http_code}' \
    "http://127.0.0.1:$PORT/s4up/binary.bin")
assert_eq "GET du binaire renvoie 200" "200" "$CODE"
cmp -s "$TMP/binary.bin" "$TMP/binary.out"
assert_eq "Binaire identique octet pour octet" "0" "$?"

printf 'nom avec espace\n' > "$TMP/space.txt"
RESP=$(curl -s -i -X POST --data-binary @"$TMP/space.txt" \
    "http://127.0.0.1:$PORT/s4up/hello%20world.txt")
assert_contains "POST avec nom encodé renvoie 201" "HTTP/1.1 201 Created" "$RESP"
assert_contains "Location ré-encodée correctement" \
    "Location: /s4up/hello%20world.txt" "$RESP"
CODE=$(curl -s -o "$TMP/space.out" -w '%{http_code}' \
    "http://127.0.0.1:$PORT/s4up/hello%20world.txt")
assert_eq "GET du nom encodé renvoie 200" "200" "$CODE"
cmp -s "$TMP/space.txt" "$TMP/space.out"
assert_eq "Contenu du nom encodé identique" "0" "$?"

echo -e "\n4. POST chunked (corps déjà dé-chunké) :"
RESP=$(raw_send "$PORT" "POST /s4up/chunked.txt HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n5\r\nHello\r\n6\r\n World\r\n0\r\n\r\n")
assert_contains "POST chunked renvoie 201" "HTTP/1.1 201 Created" "$RESP"
BODY=$(curl -s "http://127.0.0.1:$PORT/s4up/chunked.txt")
assert_eq "Corps chunked écrit sans tailles hexadécimales" "Hello World" "$BODY"

echo -e "\n5. Refus des noms et chemins dangereux :"
CODE=$(http_code -X POST --data-binary 'x' \
    "http://127.0.0.1:$PORT/s4up/%2e%2e%2fescape.txt")
assert_eq "Nom encodé ../ refusé" "403" "$CODE"
CODE=$(http_code -X POST --data-binary 'x' "http://127.0.0.1:$PORT/s4up/a/b")
assert_eq "Sous-chemin refusé" "403" "$CODE"
CODE=$(http_code -X POST --data-binary 'x' "http://127.0.0.1:$PORT/s4up/a%00b")
assert_eq "Octet NUL dans le nom refusé" "400" "$CODE"
CODE=$(http_code -X POST --data-binary 'x' "http://127.0.0.1:$PORT/s4up/%zz")
assert_eq "Séquence percent invalide refusée" "400" "$CODE"
CODE=$(http_code --path-as-is \
    "http://127.0.0.1:$PORT/%2e%2e/%2e%2e/etc/passwd")
assert_eq "Traversée encodée refusée en GET" "403" "$CODE"
CODE=$(http_code "http://127.0.0.1:$PORT/%00")
assert_eq "Octet NUL encodé refusé en GET" "400" "$CODE"
LONGNAME=$(python3 -c 'print("a" * 256)')
CODE=$(http_code -X POST --data-binary 'x' \
    "http://127.0.0.1:$PORT/s4up/$LONGNAME")
assert_eq "Nom de plus de 255 octets refusé" "403" "$CODE"

echo -e "\n6. POST sans upload_store et méthodes :"
CODE=$(http_code -X POST --data-binary 'x' "http://127.0.0.1:$PORT/free/file.txt")
assert_eq "POST sans upload_store renvoie 403" "403" "$CODE"
CODE=$(http_code -X POST --data-binary 'x' "http://127.0.0.1:$PORT/nostore/file.txt")
assert_eq "upload_store absent du disque renvoie 500" "500" "$CODE"
RESP=$(curl -s -i -X POST --data-binary 'x' "http://127.0.0.1:$PORT/getonly/file.txt")
assert_contains "POST non autorisé renvoie 405" "HTTP/1.1 405 Method Not Allowed" "$RESP"
assert_contains "Allow reflète la location" "Allow: GET" "$RESP"
RESP=$(curl -s -i -X PUT "http://127.0.0.1:$PORT/s4up/x")
assert_contains "PUT renvoie 405" "HTTP/1.1 405 Method Not Allowed" "$RESP"
assert_contains "Allow complet sur /s4up" "Allow: GET, POST, DELETE" "$RESP"
RESP=$(curl -s -i -X PATCH "http://127.0.0.1:$PORT/free/x")
assert_contains "Méthode inconnue sur location libre renvoie 405" \
    "HTTP/1.1 405 Method Not Allowed" "$RESP"
assert_contains "Allow par défaut sans liste" "Allow: GET, POST, DELETE" "$RESP"

echo -e "\n7. DELETE :"
RESP=$(curl -s -i -X DELETE "http://127.0.0.1:$PORT/s4up/binary.bin")
assert_contains "DELETE d'un fichier renvoie 204" "HTTP/1.1 204 No Content" "$RESP"
assert_not_contains "204 sans Content-Length" "Content-Length" "$RESP"
CODE=$(http_code "http://127.0.0.1:$PORT/s4up/binary.bin")
assert_eq "Fichier supprimé devient 404" "404" "$CODE"
CODE=$(http_code -X DELETE "http://127.0.0.1:$PORT/s4up/binary.bin")
assert_eq "DELETE d'un fichier absent renvoie 404" "404" "$CODE"
CODE=$(http_code -X DELETE "http://127.0.0.1:$PORT/s4up")
assert_eq "DELETE du dossier d'upload renvoie 403" "403" "$CODE"
CODE=$(http_code -X DELETE "http://127.0.0.1:$PORT/")
assert_eq "DELETE de la racine renvoie 403" "403" "$CODE"
CODE=$(http_code -X DELETE "http://127.0.0.1:$PORT/s4_dir")
assert_eq "DELETE d'un dossier renvoie 403" "403" "$CODE"
CODE=$(http_code -X DELETE "http://127.0.0.1:$PORT/s4_delete.txt")
assert_eq "DELETE d'un fichier statique renvoie 204" "204" "$CODE"
CODE=$(http_code "http://127.0.0.1:$PORT/s4_delete.txt")
assert_eq "Fichier statique supprimé devient 404" "404" "$CODE"
printf 'cible precieuse\n' > "$TMP/target.txt"
ln -sf "$TMP/target.txt" www/site1/s4_link
CODE=$(http_code -X DELETE "http://127.0.0.1:$PORT/s4_link")
assert_eq "DELETE d'un lien symbolique renvoie 204" "204" "$CODE"
[ -f "$TMP/target.txt" ]
assert_eq "La cible du lien symbolique reste intacte" "0" "$?"
CODE=$(http_code --path-as-is -X DELETE \
    "http://127.0.0.1:$PORT/%2e%2e/%2e%2e/etc/passwd")
assert_eq "DELETE avec traversée encodée refusé" "403" "$CODE"

echo -e "\n8. Keep-alive après POST :"
RESP=$(raw_send "$PORT" "POST /s4up/keep.txt HTTP/1.1\r\nHost: localhost\r\nContent-Length: 3\r\n\r\nabcGET /s4up/keep.txt HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n")
assert_eq "POST puis GET sur la même connexion" "1" \
    "$(printf '%s' "$RESP" | grep -c 'HTTP/1.1 201 Created')"
assert_eq "GET keep-alive après POST" "1" \
    "$(printf '%s' "$RESP" | grep -c 'HTTP/1.1 200 OK')"
assert_contains "GET keep-alive renvoie le bon corps" "abc" "$RESP"

echo -e "\n9. Corps trop volumineux :"
BEFORE=$(count_files)
RESP=$(raw_send "$PORT" "POST /s4up HTTP/1.1\r\nHost: localhost\r\nContent-Length: 2100000\r\nConnection: close\r\n\r\n")
assert_contains "POST au-delà de client_max_body_size renvoie 413" \
    "HTTP/1.1 413 Payload Too Large" "$RESP"
assert_eq "Aucun fichier écrit après 413" "$BEFORE" "$(count_files)"

echo -e "\n10. Concurrence d'uploads générés :"
BEFORE=$(count_files)
: > "$TMP/codes"
PIDS=()
for i in $(seq 1 10); do
    curl -s -o /dev/null -w '%{http_code}\n' -X POST --data-binary 'concurrent' \
        "http://127.0.0.1:$PORT/s4up" >> "$TMP/codes" &
    PIDS+=($!)
done
for pid in "${PIDS[@]}"; do
    wait "$pid"
done
assert_eq "10 uploads concurrents réussis" "10" "$(grep -c '^201$' "$TMP/codes")"
assert_eq "10 noms générés distincts" "$((BEFORE + 10))" "$(count_files)"

echo -e "\n=========================================================="
echo "RÉSULTATS SPRINT 4 : PASS=$PASS | FAIL=$FAIL"
echo "=========================================================="

[ "$FAIL" -eq 0 ]
