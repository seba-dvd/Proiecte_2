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
  3. Cauta contact
  4. Modifica contact
  5. Sterge contact
  6. Exporta in CSV
  7. Iesire
---------------------------------------------
Alege optiunea: 1

Nume: ion pop
Prenume: maria
Adresa: Str. Florilor 12
Judet: Cluj
Oras: Cluj-Napoca
Tara: Romania
Telefon: +40712345678
Email: MARIA.POP@GMAIL.COM

Contact adaugat: Ion Pop — +40712345678 — maria.pop@gmail.com
Adaugat la: 29.03.2026 14:22:05
```

---

## Functionalitati

- **Adaugare contact** cu validare completa pentru fiecare camp
- **Afisare agenda** sortata cronologic — cele mai recente contacte primele
- **Cautare contact** dupa Nume, Prenume sau Telefon
- **Modificare contact** — editare individuala a unui camp ales (Nume, Telefon, Email sau Adresa)
- **Stergere contact** cu confirmare inainte de executie
- **Export CSV** — genereaza `contacte_export.csv` compatibil cu Excel si Google Sheets
- **Salvare automata** la iesire in fisierul `contacte.txt`
- **Incarcare automata** la pornire — fara configurare manuala
- **Timestamp exact** — data si ora adaugarii fiecarui contact
- **Verificare duplicate** — telefonul si emailul trebuie sa fie unice in agenda

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

| Camp              | Tip       | Descriere                                      |
|-------------------|-----------|------------------------------------------------|
| `Nume`            | `string`  | Numele de familie (Title Case automat)         |
| `Prenume`         | `string`  | Prenumele (Title Case automat)                 |
| `adresa`          | `string`  | Adresa fizica / strada (camp liber)            |
| `judet`           | `string`  | Judetul de resedinta (Title Case automat)      |
| `oras`            | `string`  | Orasul de resedinta (Title Case automat)       |
| `tara`            | `string`  | Tara de resedinta (Title Case automat)         |
| `telefon`         | `string`  | Numar de telefon (minim 10 cifre, unic)        |
| `email`           | `string`  | Adresa de email (validata, lowercase, unica)   |
| `momentAdaugare`  | `time_t`  | Timestamp Unix — momentul exact al adaugarii   |

---

## Referinta functii

| Functie                      | Semnatura                               | Descriere                                                                  |
|------------------------------|-----------------------------------------|----------------------------------------------------------------------------|
| `afiseazaMeniu`              | `void ()`                               | Afiseaza cele 7 optiuni principale in consola                              |
| `adaugaContact`              | `void (vector<Contact>&)`               | Citeste, valideaza si adauga un contact nou cu timestamp                   |
| `afisareAgenda`              | `void (vector<Contact>&)`               | Sorteaza descrescator dupa data si afiseaza contactele cu adresa completa  |
| `cautaContact`               | `void (const vector<Contact>&)`         | Cauta dupa Nume, Prenume sau Telefon si afiseaza potrivirile               |
| `modificaContact`            | `void (vector<Contact>&)`               | Sub-meniu pentru editarea unui camp individual al unui contact             |
| `stergereContact`            | `void (vector<Contact>&)`               | Listeaza contactele, cere confirmare si sterge din vector                  |
| `exportaCSV`                 | `void (const vector<Contact>&)`         | Genereaza `contacte_export.csv` cu toate contactele                        |
| `salveazaInFisier`           | `void (vector<Contact>&)`               | Scrie toate contactele in `contacte.txt`                                   |
| `incarcaDinFisier`           | `void (vector<Contact>&)`               | Citeste contactele salvate la pornire (silentios daca nu exista)           |
| `telefonExista`              | `bool (const vector<Contact>&, string)` | Verifica daca un telefon este deja inregistrat in agenda                   |
| `emailExista`                | `bool (const vector<Contact>&, string)` | Verifica daca un email este deja inregistrat in agenda                     |
| `citesteNumeValid`           | `string (string)`                       | Valideaza litere/spatii si aplica Title Case                               |
| `citesteTelefonValid`        | `string (string)`                       | Valideaza minim 10 cifre, accepta prefix `+`                               |
| `citesteEmailValid`          | `string (string)`                       | Valideaza `@`, domeniu si absenta spatiilor; converteste la lowercase      |
| `citesteJudetValid`          | `string (string)`                       | Valideaza litere/spatii si aplica Title Case (reutilizata pt oras si tara) |
| `ordoneazaCeleMaiNoiPrimele` | `bool (const Contact&, const Contact&)` | Comparator pentru `std::sort` — descrescator dupa timestamp                |

---

## Reguli de validare

### Nume / Prenume / Judet / Oras / Tara
- Accepta doar litere (`a-z`, `A-Z`) si spatii — nu poate fi gol
- **Title Case automat**: `cluj napoca` → `Cluj Napoca`

### Telefon
- Accepta doar cifre; poate incepe cu `+` (prefix international)
- Lungime minima: **10 caractere**
- **Trebuie sa fie unic** — nu poate fi inregistrat la doua contacte diferite

### Email
- Nu poate contine spatii
- Trebuie sa contina `@` — nu la prima sau ultima pozitie
- Trebuie sa contina `.` dupa `@` — nu imediat dupa `@` si nici la final
- **Convertit automat la litere mici**: `MARIA@GMAIL.COM` → `maria@gmail.com`
- **Trebuie sa fie unic** — nu poate fi inregistrat la doua contacte diferite

---

## Persistenta datelor

Datele sunt salvate in `contacte.txt`, in acelasi director cu executabilul. Formatul este text simplu, cu un camp pe linie:

```
Ion
Pop
Str. Florilor 12
Cluj
Cluj-Napoca
Romania
+40712345678
maria.pop@gmail.com
1743254525
```

Ordinea campurilor: `Nume` → `Prenume` → `Adresa` → `Judet` → `Oras` → `Tara` → `Telefon` → `Email` → `Timestamp (Unix)`

> **Atentie:** Fisierele `contacte.txt` salvate de versiunea anterioara (fara campurile judet, oras, tara) **nu sunt compatibile** cu aceasta versiune si vor fi citite incorect.

> Datele se salveaza automat la alegerea optiunii **7 (Iesire)**.  
> La urmatoarea pornire, fisierul este citit automat.  
> Daca fisierul nu exista (prima rulare), agenda porneste goala fara erori.

---

## Fisierul de export CSV

Optiunea **6 (Exporta in CSV)** genereaza fisierul `contacte_export.csv` cu toate contactele. Poate fi deschis direct in Microsoft Excel sau Google Sheets.

**Structura fisierului:**

```
Nume,Prenume,Telefon,Email,Adresa_Strada,Oras,Judet,Tara,Timestamp_Adaugare
Ion,Pop,+40712345678,maria.pop@gmail.com,Str. Florilor 12,Cluj-Napoca,Cluj,Romania,1743254525
```

> Timestamp-ul este exportat ca numar Unix pentru a permite sortare/filtrare numerica in Excel.  
> Campurile care contin virgule (ex: adresa) pot afecta structura CSV — o versiune viitoare va adauga ghilimele de protectie.

---

## Flux de executie

```
Pornire
  └─► incarcaDinFisier()       — citeste contacte.txt (daca exista)
        └─► Afisare meniu principal
              ├─► [1] adaugaContact()     — validare campuri + unicitate + timestamp
              ├─► [2] afisareAgenda()     — sort descrescator + afisare adresa completa
              ├─► [3] cautaContact()      — cautare dupa Nume, Prenume sau Telefon
              ├─► [4] modificaContact()   — sub-meniu editare camp individual
              ├─► [5] stergereContact()   — confirmare + stergere din vector
              ├─► [6] exportaCSV()        — generare contacte_export.csv
              └─► [7] salveazaInFisier()  — scriere contacte.txt + exit
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

- **Adresa strazii** nu este validata (camp liber) — poate contine orice caractere
- Cautarea este **case-sensitive** — `ion` nu gaseste `Ion`
- Fisierul de stocare este **text simplu** — nu este criptat
- Campurile cu virgule in adresa pot **rupe structura CSV** la export
- Aplicatia nu suporta **mai multi utilizatori simultan**
- Fisierele `contacte.txt` din versiunea anterioara **nu sunt compatibile**

---

## Structura proiect

```
agenda/
├── agenda.cpp              # Codul sursa complet
├── contacte.txt            # Fisier de date (generat automat la prima iesire)
├── contacte_export.csv     # Export CSV (generat la optiunea 6)
└── README.md               # Aceasta documentatie
```

---