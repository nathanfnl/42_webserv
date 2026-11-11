#!/usr/bin/env python3
import os
import sys

def main():
    method = os.environ.get("REQUEST_METHOD", "")
    query = os.environ.get("QUERY_STRING", "")
    content_length = os.environ.get("CONTENT_LENGTH", "0")

    body = ""
    if method == "POST":
        try:
            length = int(content_length)
        except ValueError:
            length = 0
        if length > 0:
            body = sys.stdin.read(length)

    # --- Headers CGI (obligatoire : au moins Content-Type, terminé par ligne vide) ---
    print("Content-Type: text/plain")
    print()  # ligne vide = fin des headers CGI

    # --- Body ---
    print("Hello from CGI\n")
    print(f"REQUEST_METHOD   = {method}")
    print(f"QUERY_STRING     = {query}")
    print(f"CONTENT_LENGTH   = {content_length}")
    print(f"SCRIPT_NAME      = {os.environ.get('SCRIPT_NAME', '')}")
    print(f"SCRIPT_FILENAME  = {os.environ.get('SCRIPT_FILENAME', '')}")
    print(f"SERVER_NAME      = {os.environ.get('SERVER_NAME', '')}")
    print(f"SERVER_PORT      = {os.environ.get('SERVER_PORT', '')}")
    print(f"SERVER_PROTOCOL  = {os.environ.get('SERVER_PROTOCOL', '')}")
    print(f"GATEWAY_INTERFACE= {os.environ.get('GATEWAY_INTERFACE', '')}")

    print("\n--- HTTP_ headers reçus ---")
    for key, value in sorted(os.environ.items()):
        if key.startswith("HTTP_"):
            print(f"{key} = {value}")

    if method == "POST":
        print("\n--- BODY reçu ---")
        print(body)

if __name__ == "__main__":
    main()
