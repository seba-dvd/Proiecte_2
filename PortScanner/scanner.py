"""
SimpleScanner - Scanner de Porturi de Retea
============================================
Descriere:
    Instrument de scanare TCP care verifica porturile 1-1024 ale unui host
    (IP sau domeniu), folosind executie paralela prin thread-uri pentru
    performanta ridicata.

Dependente:
    Doar biblioteci din Standard Library Python (socket, sys, datetime,
    concurrent.futures) — nu necesita instalari suplimentare.

Restrictii legale:
    Folositi exclusiv pe sisteme pentru care aveti autorizatie explicita.
"""

import socket               # Biblioteca pentru comunicatii de retea (TCP/IP)
import sys                  # Necesar pentru sys.exit() — iesire controlata din program
from datetime import datetime       # Folosit pentru a masura durata scanarii
import concurrent.futures   # Ofera ThreadPoolExecutor pentru executie paralela


def scaneaza_port(ip, port):
    """
    Verifica daca un singur port TCP este deschis pe host-ul tinta.

    Aceasta functie este proiectata sa ruleze concurent in mai multe thread-uri
    simultan. Fiecare apel creeaza propriul socket independent, deci nu exista
    probleme de sincronizare sau variabile partajate (thread-safe).

    Mecanism:
        - connect_ex() incearca sa stabileasca o conexiune TCP
        - Returneaza 0 daca conexiunea a reusit (portul este deschis)
        - Returneaza un cod de eroare != 0 daca portul este inchis/filtrat
        - Timeout de 0.5s previne blocarea thread-ului pe host-uri lente

    Parametri:
        ip   (str): Adresa IPv4 a host-ului tinta (ex: '45.33.32.156')
        port (int): Numarul portului de verificat, intre 1 si 65535

    Returneaza:
        int  : Numarul portului, daca acesta este DESCHIS
        None : Daca portul este inchis, filtrat sau a aparut o eroare
    """
    # Cream un socket TCP/IP nou pentru aceasta verificare.
    # AF_INET   = adrese IPv4 (nu IPv6)
    # SOCK_STREAM = protocol TCP (nu UDP)
    scanner = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Setam un timeout de 0.5 secunde.
    # Fara timeout, thread-ul ar astepta indefinit un raspuns de la porturile
    # filtrate de firewall, blocand intreaga scanare.
    scanner.settimeout(0.5)

    try:
        # connect_ex() este versiunea "silentioasa" a connect():
        # - connect()    arunca exceptie OSError daca portul e inchis
        # - connect_ex() returneaza un cod numeric (0 = succes, altceva = eroare)
        # Alegem connect_ex() pentru ca este mai eficient in bucle de scanare.
        rezultat = scanner.connect_ex((ip, port))

        if rezultat == 0:
            # Cod 0 inseamna ca serverul a acceptat conexiunea TCP —
            # portul este DESCHIS si un serviciu asculta pe el.
            return port

        # Orice alt cod (ex: 111 = Connection refused, 110 = Timeout)
        # inseamna ca portul este inchis sau blocat de firewall.
        return None

    except Exception:
        # Prindem orice alta eroare neasteptata (ex: eroare de retea, IP invalid
        # la nivel de socket) si returnam None pentru a nu opri thread-ul.
        return None

    finally:
        # Blocul finally se executa INTOTDEAUNA, indiferent de rezultat sau eroare.
        # Este esential sa inchidem socket-ul pentru a elibera resurse de sistem.
        # Fara acest pas, am epuiza rapid descriptoarele de fisiere disponibile.
        scanner.close()


def afiseaza_banner():
    """
    Afiseaza logo-ul ASCII art al aplicatiei in consola la pornire.

    Nu primeste parametri si nu returneaza nimic (efectul este pur vizual).
    Rolul sau este sa identifice tool-ul si sa ofere o experienta mai
    profesionala utilizatorului.
    """
    # Logo-ul este un sir multilinie (triple quotes).
    # Textul a fost generat cu un font ASCII art de tip "slant".
    print("""
    ================================================
      ____  _                 _   ____                  
     / ___|(_)_ __ ___  _ __ | | / ___|  ___ __ _ _ __  
     \___ \| | '_ ` _ \| '_ \| | \___ \ / __/ _` | '_ \ 
      ___) | | | | | | | |_) | |  ___) | (_| (_| | | | |
     |____/|_|_| |_| |_| .__/|_| |____/ \___\__,_|_| |_|
                       |_|                              
    ================================================
    """)


# ─────────────────────────────────────────────────────────────────────────────
# PUNCT DE INTRARE PRINCIPAL
# Blocul `if __name__ == "__main__"` garanteaza ca acest cod se executa DOAR
# cand fisierul este rulat direct (python scanner.py), nu si cand este importat
# ca modul intr-un alt script Python.
# ─────────────────────────────────────────────────────────────────────────────
if __name__ == "__main__":

    # Pasul 1: Afisam logo-ul la pornirea aplicatiei
    afiseaza_banner()

    # Pasul 2: Citim adresa tinta de la utilizator.
    # input() blocheaza executia pana cand utilizatorul apasa Enter.
    # Acceptam atat IP-uri directe (ex: 192.168.1.1) cat si domenii (ex: google.com).
    domeniu_tinta = input("Introdu adresa IP sau Domeniul de scanat (ex: scanme.nmap.org): ")

    # Pasul 3: Rezolvam domeniul la adresa IP reala (DNS lookup).
    # Chiar daca utilizatorul a introdus direct un IP, gethostbyname() il
    # valideaza si il returneaza nemodificat. Daca domeniul nu exista in DNS,
    # functia arunca exceptia socket.gaierror.
    try:
        ip_tinta = socket.gethostbyname(domeniu_tinta)
    except socket.gaierror:
        # Eroare de rezolvare DNS: domeniu inexistent sau fara conexiune la internet.
        # Informam utilizatorul si oprim programul cu sys.exit() (cod implicit 0).
        print("\n[!] Eroare: Domeniul introdus nu a putut fi gasit. Verifica daca ai scris corect.")
        sys.exit()

    # Afisam confirmarea tintei — util pentru a verifica ca s-a rezolvat IP-ul corect
    print("-" * 50)
    print(f"[*] Tinta confirmata: {domeniu_tinta} ({ip_tinta})")

    # Pasul 4: Marcam timpul de start pentru a calcula durata totala la final
    timp_start = datetime.now()
    print(f"[*] Scanarea a inceput la: {timp_start.strftime('%H:%M:%S')}")
    print("-" * 50)

    # Pasul 5: Definim gama de porturi de scanat.
    # range(1, 1025) genereaza porturile 1 pana la 1024 (inclusiv).
    # Acestea sunt porturile "well-known" standardizate de IANA (ex: 80=HTTP,
    # 443=HTTPS, 22=SSH, 21=FTP, 25=SMTP etc.).
    porturi_de_scanat = range(1, 1025)

    # Contor pentru numarul total de porturi deschise gasite
    porturi_deschise = 0

    # Pasul 6: Lansam scanarea paralela cu ThreadPoolExecutor.
    # Folosim try/except exterior pentru a intercepta Ctrl+C (KeyboardInterrupt).
    try:
        # ThreadPoolExecutor creeaza un "pool" de thread-uri reutilizabile.
        # max_workers=100 inseamna ca vor rula simultan maxim 100 de thread-uri.
        # Cuvantul cheie `with` garanteaza ca toate thread-urile se termina
        # inainte de a continua (join automat la iesirea din bloc).
        with concurrent.futures.ThreadPoolExecutor(max_workers=100) as executor:

            # Trimitem toate task-urile in pool dintr-o data (non-blocking).
            # executor.submit() planifica executia scaneaza_port(ip, port) pe
            # un thread disponibil si returneaza un obiect Future (promisiune
            # de rezultat viitor). Cream lista de Future-uri pentru toti portii.
            task_uri = [
                executor.submit(scaneaza_port, ip_tinta, port)
                for port in porturi_de_scanat
            ]

            # as_completed() este un iterator care "livreaza" Future-urile
            # pe masura ce thread-urile le finalizeaza — NU in ordinea initiala.
            # Acest lucru ne permite sa afisam porturile deschise IMEDIAT ce
            # sunt descoperite, fara sa asteptam terminarea tuturor scanarilor.
            for task in concurrent.futures.as_completed(task_uri):

                # task.result() extrage valoarea returnata de scaneaza_port():
                # - un numar intreg daca portul e deschis
                # - None daca portul e inchis sau a aparut o eroare
                port_gasit = task.result()

                if port_gasit is not None:
                    # Am gasit un port deschis — il afisam imediat si actualizam contorul
                    print(f"[+] Portul {port_gasit} este DESCHIS")
                    porturi_deschise += 1

    except KeyboardInterrupt:
        # Utilizatorul a apasat Ctrl+C in timpul scanarii.
        # Afisam un mesaj politicos si iesim curat din program.
        # ThreadPoolExecutor se va inchide automat (context manager).
        print("\n[!] Scanare intrerupta de utilizator (Ctrl+C).")
        sys.exit()

    # Pasul 7: Calculam si afisam raportul final
    timp_sfarsit = datetime.now()

    # Scadem timpul de start din timpul de sfarsit pentru a obtine un obiect
    # timedelta, din care extragem durata in secunde (cu zecimale).
    durata_totala = timp_sfarsit - timp_start

    print("-" * 50)
    print(f"[*] Scanare finalizata!")
    print(f"[*] Total porturi deschise gasite: {porturi_deschise}")
    # :.2f formateaza numarul float la exact 2 zecimale (ex: 6.34)
    print(f"[*] Timp total de executie: {durata_totala.total_seconds():.2f} secunde")
    print("=" * 48)