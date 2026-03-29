# 🔍 SimpleScanner

**Scanner de porturi TCP scris in Python** — identifica rapid porturile deschise ale unui host folosind executie paralela prin thread-uri.

---

## Demo

```
    ================================================
      ____  _                 _   ____
     / ___|(_)_ __ ___  _ __ | | / ___|  ___ __ _ _ __
     \___ \| | '_ ` _ \| '_ \| | \___ \ / __/ _` | '_ \
      ___) | | | | | | | |_) | |  ___) | (_| (_| | | | |
     |____/|_|_| |_| |_| .__/|_| |____/ \___\__,_|_| |_|
                       |_|
    ================================================

Introdu adresa IP sau Domeniul de scanat: scanme.nmap.org
--------------------------------------------------
[*] Tinta confirmata: scanme.nmap.org (45.33.32.156)
[*] Scanarea a inceput la: 14:22:05
--------------------------------------------------
[+] Portul 22  este DESCHIS
[+] Portul 80  este DESCHIS
[+] Portul 443 este DESCHIS
--------------------------------------------------
[*] Scanare finalizata!
[*] Total porturi deschise gasite: 3
[*] Timp total de executie: 6.34 secunde
================================================
```

---

## Functionalitati

- **Rezolvare DNS automata** — accepta atat adrese IP cat si nume de domeniu
- **Scanare paralela** — 100 de thread-uri simultane pentru viteza maxima
- **Acoperire completa** — porturile 1–1024 (porturi well-known / IANA)
- **Rezultate in timp real** — porturile deschise sunt afisate imediat ce sunt detectate
- **Masurarea duratei** — afiseaza timpul total de executie la final
- **Intrerupere eleganta** — suporta `Ctrl+C` fara erori

---

## Utilizare

```bash
python scanner.py
```

La pornire, aplicatia va solicita adresa tinta:

```
Introdu adresa IP sau Domeniul de scanat (ex: scanme.nmap.org):
```

Poti introduce fie un **IP direct** (`192.168.1.1`), fie un **nume de domeniu** (`example.com`).

---

## Interpretarea output-ului

| Prefix | Semnificatie |
|--------|--------------|
| `[*]`  | Mesaj informativ (status normal) |
| `[+]`  | Port deschis detectat |
| `[!]`  | Eroare sau avertisment |

---

## Structura proiect

```
simplescanner/
├── scanner.py          # Fisierul principal
└── README.md           # Aceasta documentatie
```

### Functii principale

#### `scaneaza_port(ip, port)`

Verifica daca un port TCP este deschis pe host-ul tinta.

| Parametru | Tip  | Descriere                        |
|-----------|------|----------------------------------|
| `ip`      | str  | Adresa IPv4 a host-ului tinta    |
| `port`    | int  | Numarul portului (1–65535)       |

**Returneaza:** `int` (numarul portului) daca este deschis, `None` altfel.

#### `afiseaza_banner()`

Afiseaza logo-ul ASCII art la pornirea aplicatiei. Fara parametri, fara valoare returnata.

---

## Performanta

| Parametru              | Valoare               |
|------------------------|-----------------------|
| Thread-uri simultane   | 100                   |
| Timeout per port       | 0.5 secunde           |
| Porturi scanate        | 1 – 1024              |
| Timp estimat (local)   | ~2–5 secunde          |
| Timp estimat (remote)  | ~5–15 secunde         |

---

## Limitari

- Suporta doar protocolul **TCP** (nu UDP)
- Gama de porturi este **fixa** (1–1024), nu se poate configura din CLI
- Nu identifica **serviciile** ce ruleaza pe porturile deschise
- Nu suporta **IPv6** (doar AF_INET)
- Numarul de thread-uri este **hardcodat**

---

## Avertisment legal

> ⚠️ **Foloseste acest instrument EXCLUSIV pe sisteme pentru care ai autorizatie explicita.**
> Scanarea porturilor fara permisiune poate fi ilegala in multe jurisdictii.

Site-uri de test legale:
- [`scanme.nmap.org`](http://scanme.nmap.org) — server oficial Nmap pentru teste
- [`testphp.vulnweb.com`](http://testphp.vulnweb.com) — server Acunetix pentru teste

---

## Cerinte sistem

- Python **3.7** sau mai recent
- Acces la retea catre host-ul tinta
- Permisiuni standard de creare socket TCP

---

## Posibile extensii

- [ ] Argumente CLI (`argparse`) pentru IP, range porturi, numar thread-uri
- [ ] Banner grabbing — identificarea serviciului pe portul deschis
- [ ] Export rezultate in JSON / CSV
- [ ] Suport scanare UDP
- [ ] Suport liste de host-uri sau subnete CIDR
- [ ] Gama configurabila de porturi (ex: 1–65535)

---
