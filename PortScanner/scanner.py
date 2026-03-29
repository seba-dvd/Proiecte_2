"""
SimpleScanner v2.0 - Scanner de Porturi de Retea
================================================
Descriere:
    Instrument de scanare TCP care verifica porturile 1-1024 ale unui host
    (IP sau domeniu), folosind executie paralela prin thread-uri pentru
    performanta ridicata. Identifica automat serviciile si exporta rezultatele.

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


# --- Functia pentru identificarea serviciilor ---
def obtine_nume_serviciu(port):
    """
    Identifica numele serviciului standard asociat unui port TCP.

    Mecanism:
        - Foloseste socket.getservbyport() pentru a interoga baza de date
          locala a sistemului de operare (ex: fisierul /etc/services pe Linux).
        - Daca portul este cunoscut (ex: 80), returneaza numele (ex: 'http').
        - Daca portul nu este in baza de date, arunca OSError si returnam 'necunoscut'.

    Parametri:
        port (int): Numarul portului gasit deschis.

    Returneaza:
        str: Numele serviciului (ex: 'ssh') sau 'necunoscut'.
    """
    try:
        return socket.getservbyport(port, "tcp")
    except OSError:
        return "necunoscut"


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
    # filtrate de firewall, blocand intreaga scanare.
    scanner.settimeout(0.5)

    try:
        # connect_ex() este versiunea "silentioasa" a connect():
        # - connect()    arunca exceptie OSError daca portul e inchis
        # - connect_ex() returneaza un cod numeric (0 = succes, altceva = eroare)
        # Alegem connect_ex() pentru ca este mai eficient in bucle de scanare.
        rezultat = scanner.connect_ex((ip, port))

        if rezultat == 0:
            # portul este DESCHIS si un serviciu asculta pe el.
            return port
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
    # Acestea sunt porturile "well-known" standardizate de IANA.
    porturi_de_scanat = range(1, 1025)

    # Contor pentru numarul total de porturi deschise gasite
    porturi_deschise = 0
    
    # Lista pentru a stoca rezultatele in vederea exportului ---
    lista_rezultate = [] 

    # Pasul 6: Lansam scanarea paralela cu ThreadPoolExecutor.
    # Folosim try/except exterior pentru a intercepta Ctrl+C (KeyboardInterrupt).
    try:
        # ThreadPoolExecutor creeaza un "pool" de thread-uri reutilizabile.
        # max_workers=100 inseamna ca vor rula simultan maxim 100 de thread-uri.
        with concurrent.futures.ThreadPoolExecutor(max_workers=100) as executor:

            # executor.submit() planifica executia scaneaza_port(ip, port)
            task_uri = [
                executor.submit(scaneaza_port, ip_tinta, port)
                for port in porturi_de_scanat
            ]

            # as_completed() livreaza Future-urile pe masura ce sunt gata
            for task in concurrent.futures.as_completed(task_uri):

                port_gasit = task.result()

                if port_gasit is not None:
                    # --- NOU: Aflam serviciul si salvam datele ---
                    nume_serv = obtine_nume_serviciu(port_gasit)
                    
                    # Am gasit un port deschis — il afisam imediat
                    print(f"[+] Portul {port_gasit} ({nume_serv.upper()}) este DESCHIS")
                    
                    # Actualizam contorul si adaugam tuplul in lista noastra de rezultate
                    porturi_deschise += 1
                    lista_rezultate.append((port_gasit, nume_serv))

    except KeyboardInterrupt:
        # Utilizatorul a apasat Ctrl+C in timpul scanarii.
        # Nu mai oprim programul brusc, ci lasam executia sa continue spre Pasul 7 si 8
        # pentru a genera fisierul cu porturile gasite pana in acest moment.
        print("\n[!] Scanare intrerupta de utilizator (Ctrl+C). Se genereaza raportul partial...")

    # Pasul 7: Calculam si afisam raportul final in consola
    timp_sfarsit = datetime.now()
    durata_totala = timp_sfarsit - timp_start

    print("-" * 50)
    print(f"[*] Scanare finalizata!")
    print(f"[*] Total porturi deschise gasite: {porturi_deschise}")
    print(f"[*] Timp total de executie: {durata_totala.total_seconds():.2f} secunde")
    
    # Pasul 8: Exportul datelor in fisier text ---
    # Verificam daca am gasit macar un port (ca sa nu cream fisiere goale inutile)
    if porturi_deschise > 0:
        nume_fisier = f"scanare_{ip_tinta}.txt"
        
        # Sortam lista crescator dupa numarul portului (indexul 0 din tuplul salvat)
        # deoarece as_completed() le returneaza intr-o ordine aleatorie
        lista_rezultate.sort(key=lambda x: x[0])
        
        try:
            # Deschidem fisierul in modul 'w' (write - scriere). Daca nu exista, il creeaza.
            with open(nume_fisier, "w") as fisier:
                fisier.write(f"=== RAPORT SCANARE: {domeniu_tinta} ({ip_tinta}) ===\n")
                fisier.write(f"Data si ora: {timp_start.strftime('%d-%m-%Y %H:%M:%S')}\n")
                fisier.write("-" * 50 + "\n")
                
                # Iteram prin lista sortata si scriem fiecare linie
                # Folosim :<6 si :<12 pentru a asigura o aliniere tip tabel (padding)
                for port, serviciu in lista_rezultate:
                    fisier.write(f"PORT: {port:<6} | SERVICIU: {serviciu.upper():<12} | STARE: DESCHIS\n")
                
                fisier.write("-" * 50 + "\n")
                fisier.write(f"Total porturi deschise: {porturi_deschise}\n")
                fisier.write(f"Durata scanarii: {durata_totala.total_seconds():.2f} secunde\n")
                
            print(f"[*] RAPORT SALVAT: Rezultatele au fost exportate in '{nume_fisier}'")
        except Exception as eroare:
            print(f"[!] Eroare la salvarea fisierului: {eroare}")

    print("=" * 48)