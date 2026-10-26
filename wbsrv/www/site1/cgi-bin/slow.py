#!/usr/bin/env python3
import time
import sys

# Dort volontairement plus longtemps que CGI_TIMEOUT_SEC (10 s)
# pour vérifier que le serveur tue le processus et répond 504 Gateway Timeout
# sans bloquer les autres clients ni laisser de zombie.
time.sleep(15)
sys.stdout.write("Content-Type: text/plain\r\n\r\nToo late\n")
