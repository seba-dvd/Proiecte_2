# 📒 Agenda de Contacte

**Aplicatie de gestiune a contactelor scrisa in C++** — ruleaza in linia de comanda, cu salvare automata intr-un fisier text local si sortare cronologica.

---

## Demo

```
=============================================
        AGENDA DE CONTACTE
=============================================
  1. Adauga contact
  2. Afiseaza agenda
  3. Sterge contact
  4. Iesire
---------------------------------------------
Alege optiunea: 1

Nume: ion pop
Prenume: maria
Adresa: Str. Florilor 12, Cluj-Napoca
Telefon: +40712345678
Email: MARIA.POP@GMAIL.COM

Contact adaugat: Ion Pop — +40712345678 — maria.pop@gmail.com
Adaugat la: 29.03.2026 14:22:05
```

---

## Functionalitati

- **Adaugare contact** cu validare completa pentru fiecare camp
- **Afisare agenda** sortata cronologic — cele mai recente contacte primele
- **Stergere contact** cu confirmare inainte de executie
- **Salvare automata** la iesire in fisierul `contacte.txt`
- **Incarcare automata** la pornire — fara configurare manuala
- **Timestamp exact** — data si ora adaugarii fiecarui contact

---

## Compilare si rulare

Nu sunt necesare biblioteci externe. Se folosesc exclusiv headere standard C++.

### g++ (Linux / macOS)

```bash
g++ -o agenda agenda.cpp
./agenda
```

### Windows (MinGW)

```bash
g++ -o agenda.exe agenda.cpp
agenda.exe
```

### Visual Studio

Creati un proiect **C++ Console Application**, adaugati `agenda.cpp` si compilati cu `Ctrl+F5`.

### Cerinte

- Compilator **C++11** sau mai nou (g++ 4.8+, MSVC 2015+, Clang 3.3+)
- Nu sunt necesare biblioteci externe

---

## Structura datelor

### `struct Contact`

| Camp              | Tip       | Descriere                                   |
|-------------------|-----------|---------------------------------------------|
| `Nume`            | `string`  | Numele de familie (Title Case automat)      |
| `Prenume`         | `string`  | Prenumele (Title Case automat)              |
| `adresa`          | `string`  | Adresa fizica (camp liber)                  |
| `telefon`         | `string`  | Numar de telefon (minim 10 cifre)           |
| `email`           | `string`  | Adresa de email (validata, lowercase)       |
| `momentAdaugare`  | `time_t`  | Timestamp Unix — momentul exact al adaugarii|

---

## Referinta functii

| Functie                      | Semnatura                              | Descriere                                                         |
|------------------------------|----------------------------------------|-------------------------------------------------------------------|
| `afiseazaMeniu`              | `void ()`                              | Afiseaza optiunile principale in consola                          |
| `adaugaContact`              | `void (vector<Contact>&)`              | Citeste, valideaza si adauga un contact nou cu timestamp          |
| `afisareAgenda`              | `void (vector<Contact>&)`              | Sorteaza descrescator dupa data si afiseaza contactele            |
| `stergereContact`            | `void (vector<Contact>&)`              | Listeaza contactele, cere confirmare si sterge din vector         |
| `salveazaInFisier`           | `void (vector<Contact>&)`              | Scrie toate contactele in `contacte.txt`                          |
| `incarcaDinFisier`           | `void (vector<Contact>&)`              | Citeste contactele salvate la pornire (silentios daca nu exista)  |
| `citesteNumeValid`           | `string (string)`                      | Valideaza litere/spatii si aplica Title Case                      |
| `citesteTelefonValid`        | `string (string)`                      | Valideaza minim 10 cifre, accepta prefix `+`                      |
| `citesteEmailValid`          | `string (string)`                      | Valideaza `@`, domeniu si absenta spatiilor                       |
| `ordoneazaCeleMaiNoiPrimele` | `bool (const Contact&, const Contact&)`| Comparator pentru `std::sort` — descrescator dupa timestamp       |

---

## Reguli de validare

### Nume / Prenume
- Accepta doar litere (`a-z`, `A-Z`) si spatii — nu poate fi gol
- **Title Case automat**: `ion pop` → `Ion Pop`

### Telefon
- Accepta doar cifre; poate incepe cu `+` (prefix international)
- Lungime minima: **10 caractere**

### Email
- Nu poate contine spatii
- Trebuie sa contina `@` — nu la prima sau ultima pozitie
- Trebuie sa contina `.` dupa `@` — nu imediat dupa `@` si nici la final
- **Convertit automat la litere mici**: `MARIA@GMAIL.COM` → `maria@gmail.com`

---

## Persistenta datelor

Datele sunt salvate in `contacte.txt`, in acelasi director cu executabilul. Formatul este text simplu, cu un camp pe linie:

```
Ion
Pop
Str. Florilor 12, Cluj-Napoca
+40712345678
maria.pop@gmail.com
1743254525
```

Ordinea campurilor: `Nume` → `Prenume` → `Adresa` → `Telefon` → `Email` → `Timestamp (Unix)`

> Datele se salveaza automat la alegerea optiunii **4 (Iesire)**.  
> La urmatoarea pornire, fisierul este citit automat.  
> Daca fisierul nu exista (prima rulare), agenda porneste goala fara erori.

---

## Flux de executie

```
Pornire
  └─► incarcaDinFisier()       — citeste contacte.txt (daca exista)
        └─► Afisare meniu principal
              ├─► [1] adaugaContact()     — validare campuri + timestamp
              ├─► [2] afisareAgenda()     — sort descrescator + afisare
              ├─► [3] stergereContact()   — confirmare + stergere din vector
              └─► [4] salveazaInFisier()  — scriere contacte.txt + exit
```

---

## Biblioteci utilizate

| Header        | Utilizare                                              |
|---------------|--------------------------------------------------------|
| `<iostream>`  | Afisare si citire consola (`cout`, `cin`, `getline`)   |
| `<fstream>`   | Citire/scriere fisier (`ifstream`, `ofstream`)         |
| `<string>`    | Tipul `string` si operatii (`find`, `length`, etc.)    |
| `<vector>`    | Stocare dinamica a listei de contacte                  |
| `<cctype>`    | Validare caractere: `isalpha`, `isdigit`, `toupper`    |
| `<ctime>`     | Timestamp sistem: `time`, `localtime`, `strftime`      |
| `<algorithm>` | Sortare vector: `std::sort`                            |

---

## Limitari cunoscute

- Nu exista functie de **cautare sau filtrare** a contactelor
- Nu se verifica **dublurile** — acelasi contact poate fi adaugat de mai multe ori
- **Adresa** nu este validata (camp liber)
- Fisierul de stocare este **text simplu** — nu este criptat
- Aplicatia nu suporta **mai multi utilizatori simultan**

---

## Structura proiect

```
agenda/
├── agenda.cpp       # Codul sursa complet
├── contacte.txt     # Fisier de date (generat automat la prima iesire)
└── README.md        # Aceasta documentatie
```

---
