#!/bin/bash

if ! command -v cppcheck &> /dev/null; then
    echo "[BŁĄD] Narzędzie cppcheck nie jest zainstalowane."
    exit 1
fi

echo "[INFO] Uruchamianie analizy Cppcheck (C++23 + GNU extensions)..."

cppcheck --enable=all \
         --std=c++23 \
         -D__GNUC__ \
         --inline-suppr \
         --suppress=missingIncludeSystem \
         -I inc \
         -i freertos \
         -i startup \
         -i src/syscalls.c \
         src inc

echo "[INFO] Analiza zakończona."
