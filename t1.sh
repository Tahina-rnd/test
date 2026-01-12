#!/bin/bash

# Script de test complet pour détecter les fuites mémoire dans minishell
# Usage: ./test_memory_leaks.sh

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

MINISHELL="./minishell"
VALGRIND="valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --track-fds=yes"

echo "========================================="
echo "Tests de fuites mémoire pour Minishell"
echo "========================================="
echo

if [ ! -f "$MINISHELL" ]; then
    echo -e "${RED}Erreur: minishell non trouvé. Compilez d'abord.${NC}"
    exit 1
fi

test_count=0
leak_count=0

run_test() {
    local test_name="$1"
    local commands="$2"

    ((test_count++))
    echo -e "${YELLOW}Test $test_count: $test_name${NC}"

    # Créer un fichier temporaire avec les commandes
    echo "$commands" > /tmp/minishell_test_$$

    # Exécuter avec valgrind
    $VALGRIND --log-file=/tmp/valgrind_$$.log $MINISHELL < /tmp/minishell_test_$$ 2>&1 > /dev/null

    # Vérifier les fuites
    if grep -q "definitely lost: 0 bytes" /tmp/valgrind_$$.log && \
       grep -q "indirectly lost: 0 bytes" /tmp/valgrind_$$.log; then
        echo -e "${GREEN}✓ Pas de fuite${NC}"
    else
        echo -e "${RED}✗ Fuite détectée!${NC}"
        grep "definitely lost\|indirectly lost" /tmp/valgrind_$$.log
        ((leak_count++))
    fi

    # Nettoyer
    rm -f /tmp/minishell_test_$$ /tmp/valgrind_$$.log
    echo
}

echo "=== 1. TESTS BASIQUES ==="
echo

run_test "Commande simple - echo" "echo hello
exit"

run_test "Commande simple - pwd" "pwd
exit"

run_test "Commande simple - env" "env
exit"

run_test "Ligne vide" "

exit"

run_test "Plusieurs lignes vides" "


exit"

echo "=== 2. TESTS BUILTINS ==="
echo

run_test "cd avec argument" "cd /tmp
pwd
exit"

run_test "cd sans argument" "cd
pwd
exit"

run_test "cd avec trop d'arguments" "cd /tmp /home
exit"

run_test "export sans argument" "export
exit"

run_test "export avec variable" "export TEST=value
env | grep TEST
exit"

run_test "export variable vide" "export EMPTY=
exit"

run_test "unset variable" "export TEST=value
unset TEST
exit"

run_test "echo avec -n" "echo -n hello
exit"

run_test "echo avec -nnn" "echo -nnn hello
exit"

run_test "exit sans argument" "exit"

run_test "exit avec code" "exit 42"

run_test "exit avec argument invalide" "exit abc
exit"

run_test "exit avec trop d'arguments" "exit 1 2
exit"

echo "=== 3. TESTS REDIRECTIONS ==="
echo

run_test "Redirection sortie simple" "echo hello > /tmp/test_out
cat /tmp/test_out
rm /tmp/test_out
exit"

run_test "Redirection entrée" "echo test > /tmp/test_in
cat < /tmp/test_in
rm /tmp/test_in
exit"

run_test "Redirection append" "echo line1 > /tmp/test_append
echo line2 >> /tmp/test_append
cat /tmp/test_append
rm /tmp/test_append
exit"

run_test "Multiples redirections sortie" "echo test > /tmp/out1 > /tmp/out2
rm /tmp/out1 /tmp/out2
exit"

run_test "Redirection vers fichier inexistant (entrée)" "cat < /tmp/fichier_inexistant
exit"

run_test "Redirection ambiguë" "export VAR='file1 file2'
echo test > \$VAR
exit"

echo "=== 4. TESTS HEREDOC ==="
echo

run_test "Heredoc simple" "cat << EOF
line1
line2
EOF
exit"

run_test "Heredoc avec expansion" "export TEST=value
cat << EOF
\$TEST
EOF
exit"

run_test "Heredoc sans expansion (quotes)" "cat << 'EOF'
\$TEST
EOF
exit"

run_test "Multiples heredocs" "cat << EOF1 << EOF2
first
EOF1
second
EOF2
exit"

echo "=== 5. TESTS PIPES ==="
echo

run_test "Pipe simple" "echo hello | cat
exit"

run_test "Pipe double" "echo hello | cat | cat
exit"

run_test "Pipe triple" "echo hello | cat | cat | cat
exit"

run_test "Pipe avec builtin" "export TEST=value | cat
exit"

run_test "Pipe vers builtin" "echo test | cd /tmp
exit"

echo "=== 6. TESTS EXPANSION ==="
echo

run_test "Expansion variable simple" "export VAR=test
echo \$VAR
exit"

run_test "Expansion \$?" "false
echo \$?
exit"

run_test "Expansion dans quotes" "export VAR=test
echo \"\$VAR\"
exit"

run_test "Expansion variable inexistante" "echo \$INEXISTANT
exit"

run_test "Expansion \$\$ (pas supporté)" "echo \$\$
exit"

run_test "Word splitting" "export VAR='a b c'
echo \$VAR
exit"

run_test "Pas de word splitting avec quotes" "export VAR='a b c'
echo \"\$VAR\"
exit"

echo "=== 7. TESTS QUOTES ==="
echo

run_test "Simple quotes" "echo 'hello world'
exit"

run_test "Double quotes" "echo \"hello world\"
exit"

run_test "Quotes imbriquées" "echo \"test 'word' end\"
exit"

run_test "Variable dans quotes" "export VAR=test
echo '\$VAR'
echo \"\$VAR\"
exit"

echo "=== 8. TESTS COMMANDES EXTERNES ==="
echo

run_test "ls simple" "ls
exit"

run_test "/bin/ls absolu" "/bin/ls
exit"

run_test "Commande inexistante" "commandeinexistante
exit"

run_test "Commande avec arguments" "ls -la
exit"

run_test "grep avec pipe" "echo test | grep test
exit"

echo "=== 9. TESTS COMBINÉS ==="
echo

run_test "Pipe + redirections" "echo hello | cat > /tmp/test_combined
cat /tmp/test_combined
rm /tmp/test_combined
exit"

run_test "Multiples commandes complexes" "export VAR=test
echo \$VAR | cat > /tmp/test
cat < /tmp/test
rm /tmp/test
exit"

run_test "Heredoc + pipe" "cat << EOF | grep test
test line
other line
EOF
exit"

run_test "Export + unset + expansion" "export A=1 B=2 C=3
echo \$A \$B \$C
unset B
echo \$A \$B \$C
exit"

echo "=== 10. TESTS EDGE CASES ==="
echo

run_test "Commande vide avec redirections" "> /tmp/empty_redir
rm /tmp/empty_redir
exit"

run_test "Point seul" ".
exit"

run_test "Deux points" "..
exit"

run_test "Chemin relatif" "./minishell --version
exit"

run_test "Fichier non exécutable" "/etc/passwd
exit"

run_test "Répertoire comme commande" "/tmp
exit"

run_test "Très longue ligne" "echo aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
exit"

run_test "Beaucoup d'espaces" "echo          hello          world
exit"

run_test "Tabs" "echo	hello	world
exit"

echo "=== 11. TESTS SIGNAUX (Ctrl+C simulation) ==="
echo
echo "Note: Ces tests doivent être faits manuellement avec Ctrl+C"

echo "=== 12. TESTS MÉMOIRE SPÉCIFIQUES ==="
echo

run_test "Boucle d'allocations" "export A=1
export B=2
export C=3
export D=4
export E=5
unset A B C D E
exit"

run_test "Multiples heredocs sans utilisation" "cat << EOF
test
EOF
cat << EOF2
test2
EOF2
exit"

run_test "Pipes sans fin" "echo a | echo b | echo c | echo d | echo e
exit"

echo "========================================="
echo "RÉSUMÉ DES TESTS"
echo "========================================="
echo -e "Tests exécutés: $test_count"
if [ $leak_count -eq 0 ]; then
    echo -e "${GREEN}Fuites détectées: $leak_count ✓${NC}"
else
    echo -e "${RED}Fuites détectées: $leak_count ✗${NC}"
fi
echo
