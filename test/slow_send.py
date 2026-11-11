import socket
import time

# --- Configuration ---
HOST = '127.0.0.1'
PORT = 8080          # Remplace par le port de ton webserv
DELAY = 0.5          # Temps de pause entre chaque envoi (en secondes)
CHUNK_SIZE = 5       # Nombre d'octets envoyés à la fois

def test_slow_request():
    # La requête HTTP brute qu'on va découper
    request = (
        "GET / HTTP/1.1\r\n"
        f"Host: {HOST}:{PORT}\r\n"
        "User-Agent: Python-Slow-Client\r\n"
        "Accept: */*\r\n"
        "\r\n"
    )

    try:
        # Création du socket
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        print(f"[*] Connexion à {HOST}:{PORT}...")
        s.connect((HOST, PORT))
        print("[*] Connecté ! Début de l'envoi fragmenté...\n")

        # Envoi de la requête morceau par morceau
        for i in range(0, len(request), CHUNK_SIZE):
            chunk = request[i:i+CHUNK_SIZE]
            s.sendall(chunk.encode('utf-8'))
            print(f"[+] Chunk envoyé : {repr(chunk)}")
            time.sleep(DELAY)

        print("\n[*] Requête envoyée en entier. Attente de la réponse de ton serveur...")
        
        # Attente et affichage de la réponse
        response = s.recv(4096)
        if response:
            print("\n[*] Réponse reçue :")
            print(response.decode('utf-8', errors='replace'))
        else:
            print("\n[!] Le serveur a fermé la connexion sans répondre (ou la réponse est vide).")

    except ConnectionRefusedError:
        print(f"[!] Impossible de se connecter à {HOST}:{PORT}. Ton webserv est-il lancé ?")
    except Exception as e:
        print(f"[!] Erreur inattendue : {e}")
    finally:
        s.close()
        print("\n[*] Socket fermé.")

if __name__ == "__main__":
    test_slow_request()