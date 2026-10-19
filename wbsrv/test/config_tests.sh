#!/bin/bash
# Tests Sprint 2 — parser + multi-ports. Règles alignées sur `nginx -t`.
# Grammaire : http { server { ... location ... } }.
# Usage : ./test/config_tests.sh  (depuis la racine du repo)
# Code retour : 0 si tout passe, 1 sinon.

cd "$(dirname "$0")/.." || exit 1
BIN=./webserv
PASS=0
FAIL=0

if [ ! -x "$BIN" ]; then
    echo "build webserv d'abord (make)"
    exit 1
fi

# accept <desc> <conf-contenu> : contenu enveloppé dans http { ... }.
# (printf %b, \n supportés)
accept() {
    printf 'http {\n%b\n}\n' "$2" > /tmp/ws_test.conf
    if timeout 0.5 "$BIN" /tmp/ws_test.conf 2>&1 | grep -q "Loaded"; then
        PASS=$((PASS + 1))
    else
        FAIL=$((FAIL + 1)); echo "RATÉ (aurait dû passer) : $1"
    fi
}

# reject <desc> <conf-contenu> : idem, doit échouer avec "Config error".
reject() {
    printf 'http {\n%b\n}\n' "$2" > /tmp/ws_test.conf
    if timeout 0.5 "$BIN" /tmp/ws_test.conf 2>&1 | grep -q "Config error"; then
        PASS=$((PASS + 1))
    else
        FAIL=$((FAIL + 1)); echo "RATÉ (aurait dû être rejeté) : $1"
    fi
}

# raw_accept / raw_reject : contenu tel quel (tests du validateur syntaxique).
raw_accept() {
    printf "%b" "$2" > /tmp/ws_test.conf
    if timeout 0.5 "$BIN" /tmp/ws_test.conf 2>&1 | grep -q "Loaded"; then
        PASS=$((PASS + 1))
    else
        FAIL=$((FAIL + 1)); echo "RATÉ (aurait dû passer) : $1"
    fi
}

raw_reject() {
    printf "%b" "$2" > /tmp/ws_test.conf
    if timeout 0.5 "$BIN" /tmp/ws_test.conf 2>&1 | grep -q "Config error"; then
        PASS=$((PASS + 1))
    else
        FAIL=$((FAIL + 1)); echo "RATÉ (aurait dû être rejeté) : $1"
    fi
}

echo "== validateur syntaxique (brut, sans enveloppe) =="
raw_accept "http minimal" 'http {\n server {\n listen 19144;\n }\n}\n'
raw_reject "serveur nu sans http" 'server {\n listen 19101;\n}\n'
raw_reject "directive hors bloc" 'listen 19101;\n'
raw_reject "location hors server" 'location / {\n}\n'
raw_reject "point-virgule manquant" 'http {\n server {\n listen 19101\n }\n}\n'
raw_reject "accolade non fermée" 'http {\n server {\n listen 19101;\n'
raw_reject "accolade en trop" 'http {\n server {\n listen 19101;\n }\n }\n}\n'
raw_reject "contenu après http" 'http {\n server {\n listen 19101;\n }\n}\nlisten 1;\n'
raw_reject "double http" 'http {\n}\nhttp {\n}\n'
raw_reject "server sans accolade" 'http {\n server listen 1;\n}\n'
raw_reject "server imbriqué" 'http {\n server {\n listen 19101;\n server {\n }\n }\n}\n'
raw_reject "location sans path" 'http {\n server {\n listen 19101;\n location {\n }\n }\n}\n'
raw_reject "location sans accolade" 'http {\n server {\n listen 19101;\n location / \n }\n}\n'
raw_reject "location imbriquée" 'http {\n server {\n listen 19101;\n location / {\n location /a {\n }\n }\n }\n}\n'
raw_reject "mot-clé inconnu" 'http {\n server {\n listen 19101;\n frobnicate x;\n }\n}\n'
raw_reject "ancien mot-clé methods" 'http {\n server {\n listen 19101;\n location / {\n methods GET;\n }\n }\n}\n'
raw_reject "ancien mot-clé cgi" 'http {\n server {\n listen 19101;\n location / {\n cgi .py /b;\n }\n }\n}\n'
raw_reject "directive dans http" 'http {\n root /x;\n server {\n listen 19101;\n }\n}\n'
raw_reject "point-virgule orphelin" 'http {\n server {\n listen 19101;\n ;\n }\n}\n'
raw_reject "accolade orpheline" 'http {\n server {\n listen 19101;\n {\n }\n}\n'

echo "== structure =="
reject "quote non fermée" 'server {\n listen 19101;\n root "/tmp/x;\n}\n'
accept "quotes + espaces" 'server {\n listen 19102;\n root "/tmp/my dir";\n}\n'
accept "espaces/tabs en trop" '  server   { \n\t listen  19103  ; \n } \n'
reject "server vide" 'server {\n}\n'
reject "fichier vide" ''

echo "== listen =="
accept "port seul" 'server {\n listen 19104;\n}\n'
accept "host:port" 'server {\n listen 127.0.0.1:19105;\n}\n'
accept "*:port" 'server {\n listen *:19106;\n}\n'
reject "port 99999" 'server {\n listen 99999;\n}\n'
reject "port abc" 'server {\n listen abc;\n}\n'
reject "port +80" 'server {\n listen +80;\n}\n'
reject "listen dupliqué sans noms (conflit)" 'server {\n listen 19107;\n}\nserver {\n listen 19107;\n}\n'
accept "même port, noms différents (vhosts)" 'server {\n listen 19158;\n server_name a;\n}\nserver {\n listen 19158;\n server_name b;\n}\n'
accept "même port, un nommé un anonyme" 'server {\n listen 19159;\n}\nserver {\n listen 19159;\n server_name b;\n}\n'
reject "même port, même nom" 'server {\n listen 19160;\n server_name x;\n}\nserver {\n listen 19160;\n server_name x;\n}\n'
reject "même port, mêmes noms multi" 'server {\n listen 19161;\n server_name a b;\n}\nserver {\n listen 19161;\n server_name b a;\n}\n'
accept "même port, noms qui se chevauchent" 'server {\n listen 19162;\n server_name a;\n}\nserver {\n listen 19162;\n server_name a b;\n}\n'
accept "listen default_server" 'server {\n listen 19163 default_server;\n}\n'

echo "== location path =="
accept "slash seul" 'server {\n listen 19108;\n location / {\n }\n}\n'
accept "caractères URI" 'server {\n listen 19109;\n location /=== {\n }\n}\n'
reject "sans slash" 'server {\n listen 19110;\n location foo {\n }\n}\n'
reject "location dupliquée" 'server {\n listen 19111;\n location / {\n }\n location / {\n }\n}\n'

echo "== allow_methods =="
accept "GET POST DELETE" 'server {\n listen 19112;\n location / {\n allow_methods GET POST DELETE;\n }\n}\n'
accept "minuscules" 'server {\n listen 19114;\n location / {\n allow_methods get;\n }\n}\n'
reject "HEAD rejeté (sujet: GET/POST/DELETE)" 'server {\n listen 19113;\n location / {\n allow_methods HEAD;\n }\n}\n'
reject "PUT rejeté" 'server {\n listen 19151;\n location / {\n allow_methods PUT;\n }\n}\n'
reject "méthode inconnue" 'server {\n listen 19115;\n location / {\n allow_methods GET FLY;\n }\n}\n'
reject "allow_methods vide" 'server {\n listen 19116;\n location / {\n allow_methods ;\n }\n}\n'
reject "allow_methods dupliqué" 'server {\n listen 19145;\n location / {\n allow_methods GET;\n allow_methods POST;\n }\n}\n'

echo "== return =="
accept "301 + URL" 'server {\n listen 19117;\n location / {\n return 301 http://x/;\n }\n}\n'
accept "code seul" 'server {\n listen 19118;\n location / {\n return 404;\n }\n}\n'
accept "URL seule (=302)" 'server {\n listen 19119;\n location / {\n return http://x/;\n }\n}\n'
accept "chemin relatif" 'server {\n listen 19120;\n location / {\n return 302 /here;\n }\n}\n'
reject "URL coquille" 'server {\n listen 19121;\n location / {\n return 301 HHttp:://x/;\n }\n}\n'
reject "chemin relatif seul" 'server {\n listen 19122;\n location / {\n return /here;\n }\n}\n'
reject "code 99" 'server {\n listen 19123;\n location / {\n return 99 http://x/;\n }\n}\n'
reject "code +200" 'server {\n listen 19141;\n location / {\n return +200 http://x/;\n }\n}\n'
reject "return dupliqué" 'server {\n listen 19124;\n location / {\n return 301 http://a/;\n return 302 http://b/;\n }\n}\n'

echo "== index/root/autoindex/upload/cgi =="
accept "index multi" 'server {\n listen 19125;\n location / {\n index a.html b.php;\n }\n}\n'
reject "index vide" 'server {\n listen 19126;\n location / {\n index ;\n }\n}\n'
reject "autoindex maybe" 'server {\n listen 19127;\n location / {\n autoindex maybe;\n }\n}\n'
accept "cgi_ext + cgi_pass" 'server {\n listen 19128;\n location / {\n cgi_ext .py;\n cgi_pass /usr/bin/python3;\n }\n}\n'
accept "cgi_ext multi" 'server {\n listen 19146;\n location / {\n cgi_ext .py .php;\n cgi_pass /b;\n }\n}\n'
reject "cgi sans point" 'server {\n listen 19129;\n location / {\n cgi_ext py;\n cgi_pass /bin/x;\n }\n}\n'
reject "cgi point seul" 'server {\n listen 19130;\n location / {\n cgi_ext .;\n cgi_pass /bin/x;\n }\n}\n'
reject "cgi_ext sans cgi_pass" 'server {\n listen 19147;\n location / {\n cgi_ext .py;\n }\n}\n'
reject "cgi_pass sans cgi_ext" 'server {\n listen 19148;\n location / {\n cgi_pass /b;\n }\n}\n'
reject "cgi_pass dupliqué" 'server {\n listen 19149;\n location / {\n cgi_ext .py;\n cgi_pass /a;\n cgi_pass /b;\n }\n}\n'
reject "cgi ext dupliquée" 'server {\n listen 19150;\n location / {\n cgi_ext .py;\n cgi_ext .py;\n cgi_pass /a;\n }\n}\n'
reject "doublon root location" 'server {\n listen 19131;\n location / {\n root a;\n root b;\n }\n}\n'
reject "doublon root server" 'server {\n listen 19132;\n root a;\n root b;\n}\n'

echo "== server_name/error_page/max_body =="
accept "server_name multi" 'server {\n listen 19133;\n server_name a b *.e.com;\n}\n'
accept "error_page multi-codes" 'server {\n listen 19134;\n error_page 404 500 /e.html;\n}\n'
reject "error_page =code" 'server {\n listen 19135;\n error_page 404 =200 /o.html;\n}\n'
reject "error_page =path" 'server {\n listen 19142;\n error_page 404 =foo;\n}\n'
reject "error_page 200" 'server {\n listen 19136;\n error_page 200 /o.html;\n}\n'
accept "max_body 10M" 'server {\n listen 19138;\n client_max_body_size 10M;\n}\n'
accept "max_body 10K" 'server {\n listen 19152;\n client_max_body_size 10K;\n}\n'
accept "max_body location" 'server {\n listen 19153;\n location / {\n allow_methods GET;\n client_max_body_size 1M;\n }\n}\n'
reject "max_body 0 (doit être > 0)" 'server {\n listen 19137;\n client_max_body_size 0;\n}\n'
reject "max_body minuscule" 'server {\n listen 19154;\n client_max_body_size 10m;\n}\n'
reject "max_body G" 'server {\n listen 19155;\n client_max_body_size 1G;\n}\n'
reject "max_body négatif" 'server {\n listen 19139;\n client_max_body_size -5;\n}\n'
reject "max_body overflow" 'server {\n listen 19140;\n client_max_body_size 99999999999g;\n}\n'

echo "== conflits & défaut (review externe) =="
reject "double default_server" 'server {\n listen 19166 default_server;\n server_name a;\n}\nserver {\n listen 19166 default_server;\n server_name b;\n}\n'
reject "listen dupliqué même server" 'server {\n listen 19167;\n listen 19167;\n}\n'
reject "listen dupliqué équivalent" 'server {\n listen 19168;\n listen 0.0.0.0:19168;\n}\n'
accept "deux ports différents même server" 'server {\n listen 19169;\n listen 19170;\n}\n'
reject "host invalide" 'server {\n listen foo!!!:19171;\n}\n'
reject "host vide invalide" 'server {\n listen .:::19172;\n}\n'
accept "listen localhost" 'server {\n listen localhost:19173;\n}\n'
reject "host irresolvable" 'server {\n listen nonexistent.invalid:19174;\n}\n'
reject "root vide" 'server {\n listen 19175;\n root "";\n}\n'
reject "upload_store vide" 'server {\n listen 19176;\n location / {\n allow_methods POST;\n upload_store "";\n }\n}\n'
reject "server_name vide" 'server {\n listen 19177;\n server_name "";\n}\n'

echo "== numéros de ligne dans les erreurs =="
printf 'http {\n server {\n listen 99999;\n }\n}\n' > /tmp/ws_test.conf
if timeout 0.5 "$BIN" /tmp/ws_test.conf 2>&1 | grep -q "line 3: invalid port"; then
    PASS=$((PASS + 1));
else
    FAIL=$((FAIL + 1)); echo "RATÉ : ligne d'erreur port";
fi
printf 'http {\n server {\n listen 19143;\n location / {\n allow_methods FLY;\n }\n }\n}\n' > /tmp/ws_test.conf
if timeout 0.5 "$BIN" /tmp/ws_test.conf 2>&1 | grep -q "line 5: unknown method"; then
    PASS=$((PASS + 1));
else
    FAIL=$((FAIL + 1)); echo "RATÉ : ligne d'erreur méthode";
fi
if ./test_tokenizer > /dev/null 2>&1 || (make test-tokenizer > /dev/null 2>&1 && ./test_tokenizer > /dev/null 2>&1); then
    PASS=$((PASS + 1));
else
    FAIL=$((FAIL + 1)); echo "RATÉ : test_tokenizer";
fi
if ./test_config /dev/null > /dev/null 2>&1 || (make test-config > /dev/null 2>&1 && ./test_config /dev/null > /dev/null 2>&1); then
    : # binaire test_config disponible (échec attendu sur /dev/null)
fi

echo "== héritage location <- server =="
printf 'http {\n server {\n listen 19156;\n root /srv;\n index srv.html;\n client_max_body_size 2M;\n location /a {\n allow_methods GET;\n }\n location /b {\n allow_methods POST;\n root /own;\n index own.html;\n client_max_body_size 10K;\n }\n }\n}\n' > /tmp/ws_inh.conf
INH=$(./test_config /tmp/ws_inh.conf 2>&1)
if echo "$INH" | grep -q "LOC\[0.0\] path=/a root=/srv index=srv.html maxbody=2097152"; then
    PASS=$((PASS + 1));
else
    FAIL=$((FAIL + 1)); echo "RATÉ : héritage /a ($INH)";
fi
if echo "$INH" | grep -q "LOC\[0.1\] path=/b root=/own index=own.html maxbody=10240"; then
    PASS=$((PASS + 1));
else
    FAIL=$((FAIL + 1)); echo "RATÉ : valeurs propres /b ($INH)";
fi
printf 'http {\n server {\n listen 19157;\n location / {\n allow_methods GET;\n }\n }\n}\n' > /tmp/ws_def.conf
if ./test_config /tmp/ws_def.conf 2>&1 | grep -q "LOC\[0.0\] path=/ root=. index=index.html maxbody=1048576"; then
    PASS=$((PASS + 1));
else
    FAIL=$((FAIL + 1)); echo "RATÉ : défauts";
fi

echo "== serveur par défaut =="
printf 'http {\n server {\n listen 19164;\n server_name a;\n }\n server {\n listen 19164;\n server_name b;\n }\n}\n' > /tmp/ws_def2.conf
DEF=$(./test_config /tmp/ws_def2.conf 2>&1)
if echo "$DEF" | grep -q "SERVER\[0\] default=Y" && echo "$DEF" | grep -q "SERVER\[1\] default=N"; then
    PASS=$((PASS + 1));
else
    FAIL=$((FAIL + 1)); echo "RATÉ : défaut = premier ($DEF)";
fi
printf 'http {\n server {\n listen 19165;\n server_name a;\n }\n server {\n listen 19165 default_server;\n server_name b;\n }\n}\n' > /tmp/ws_def3.conf
DEF=$(./test_config /tmp/ws_def3.conf 2>&1)
if echo "$DEF" | grep -q "SERVER\[0\] default=N" && echo "$DEF" | grep -q "SERVER\[1\] default=Y"; then
    PASS=$((PASS + 1));
else
    FAIL=$((FAIL + 1)); echo "RATÉ : flag default_server ($DEF)";
fi

echo "== smoke multi-ports =="
# Ports 18280/18281 : 8080 peut être occupé par l'environnement.
printf 'http {\n server {\n listen 18280;\n root www/site1;\n location / {\n allow_methods GET;\n }\n}\nserver {\n listen 18281;\n root www/site2;\n location / {\n allow_methods GET;\n }\n}\n}\n' > /tmp/ws_smoke.conf
"$BIN" /tmp/ws_smoke.conf > /tmp/ws_smoke.log 2>&1 &
SRVPID=$!
sleep 0.7
if curl -s --max-time 2 http://localhost:18280/ | grep -q "Hello" \
&& curl -s --max-time 2 http://localhost:18281/ | grep -q "Hello"; then
    PASS=$((PASS + 1))
else
    FAIL=$((FAIL + 1)); echo "RATÉ : smoke multi-ports"
fi
kill $SRVPID 2>/dev/null; wait 2>/dev/null

echo "== smoke port partagé (vhosts, 1 socket) =="
printf 'http {\n server {\n listen 18302;\n server_name a;\n root www/site1;\n location / {\n allow_methods GET;\n }\n}\n server {\n listen 18302;\n server_name b;\n root www/site2;\n location / {\n allow_methods GET;\n }\n}\n}\n' > /tmp/ws_shared.conf
"$BIN" /tmp/ws_shared.conf > /tmp/ws_shared.log 2>&1 &
SRVPID=$!
sleep 0.7
if grep -q "(default)" /tmp/ws_shared.log \
&& curl -s --max-time 2 http://localhost:18302/ | grep -q "Hello"; then
    PASS=$((PASS + 1))
else
    FAIL=$((FAIL + 1)); echo "RATÉ : smoke port partagé"
fi
kill $SRVPID 2>/dev/null; wait 2>/dev/null

echo ""
echo "PASS=$PASS FAIL=$FAIL"
[ "$FAIL" -eq 0 ]
