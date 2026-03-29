/**
 * Agenda de Contacte — C++ Console Application
 * =============================================
 * Descriere:
 *   Aplicatie de gestiune a contactelor care ruleaza in linia de comanda.
 *   Permite adaugarea, afisarea si stergerea contactelor, cu persistenta
 *   automata a datelor intr-un fisier text local (contacte.txt).
 *
 * Utilizare:
 *   g++ -o agenda agenda.cpp
 *   ./agenda          (Linux / macOS)
 *   agenda.exe        (Windows)
 *
 * Cerinte:
 *   Compilator C++11 sau mai nou. Nu sunt necesare biblioteci externe.
 */

// ─────────────────────────────────────────────────────────────────────────────
// 1. BIBLIOTECI STANDARD
// ─────────────────────────────────────────────────────────────────────────────

#include <iostream>   // cout, cin, getline — afisare si citire de la consola
#include <fstream>    // ifstream, ofstream — citire si scriere fisiere
#include <string>     // tipul string si metodele sale (find, length, empty etc.)
#include <vector>     // vector<T> — lista dinamica pentru stocarea contactelor
#include <cctype>     // isalpha, isdigit, isspace, toupper, tolower — validare caractere
#include <ctime>      // time_t, time(), localtime(), strftime() — lucrul cu data/ora
#include <algorithm>  // std::sort — sortarea vectorului de contacte

using namespace std;

// ─────────────────────────────────────────────────────────────────────────────
// 2. STRUCTURA DE DATE — Contact
// ─────────────────────────────────────────────────────────────────────────────

/**
 * Structura Contact
 * Reprezinta un singur contact din agenda.
 * Fiecare instanta stocheaza toate datele unui contact, inclusiv
 * momentul exact la care a fost adaugat (timestamp Unix).
 */
struct Contact {
    string Nume;              // Numele de familie — Title Case aplicat automat
    string Prenume;           // Prenumele — Title Case aplicat automat
    string adresa;            // Adresa fizica — camp liber, fara validare
    string telefon;           // Numar de telefon — minim 10 cifre, poate incepe cu '+'
    string email;             // Adresa de email — validata sintactic, convertita la lowercase
    time_t momentAdaugare;    // Timestamp Unix (secunde de la 01.01.1970) — setat la adaugare
};

// ─────────────────────────────────────────────────────────────────────────────
// 3. DECLARATII FUNCTII (prototipuri)
// Declarate inainte de main() pentru ca compilatorul sa le cunoasca
// inainte de a fi apelate, chiar daca definitiile vin mai jos.
// ─────────────────────────────────────────────────────────────────────────────

void afiseazaMeniu();
void adaugaContact(vector<Contact>& agenda);
void afisareAgenda(vector<Contact>& agenda);   // Referinta non-const: sortarea modifica ordinea in vector
void stergereContact(vector<Contact>& agenda);
void salveazaInFisier(vector<Contact>& agenda);
void incarcaDinFisier(vector<Contact>& agenda);

/**
 * ordoneazaCeleMaiNoiPrimele
 * Functie de comparatie folosita de std::sort pentru sortarea descrescatoare
 * a contactelor dupa timestamp (momentAdaugare).
 *
 * std::sort asteapta o functie care returneaza true daca primul element
 * trebuie sa apara INAINTEA celui de-al doilea in lista finala.
 * Returnand a > b, obtinem ordinea descrescatoare (cele mai noi primele).
 *
 * Parametri:
 *   a, b — doua contacte de comparat (const&: eficient, fara copiere)
 * Returneaza: true daca 'a' a fost adaugat mai recent decat 'b'
 */
bool ordoneazaCeleMaiNoiPrimele(const Contact& a, const Contact& b) {
    return a.momentAdaugare > b.momentAdaugare;
}

// ─────────────────────────────────────────────────────────────────────────────
// 4. FUNCTIA PRINCIPALA — main()
// ─────────────────────────────────────────────────────────────────────────────

int main()
{
    // Cream vectorul care va stoca toate contactele in memorie pe durata rularii.
    // La iesire, continutul sau va fi salvat in contacte.txt.
    vector<Contact> agenda;

    // Incarcam datele existente INAINTE de a afisa meniul.
    // Daca fisierul contacte.txt nu exista (prima rulare), functia nu produce erori.
    incarcaDinFisier(agenda);

    bool ruleaza = true;  // Flag care controleaza bucla principala
    int optiune;

    cout << "Bun venit in agenda!\n";

    // Bucla principala — ruleaza pana cand utilizatorul alege optiunea 4 (Iesire)
    while (ruleaza) {
        afiseazaMeniu();
        cout << "\nAlege o optiune: ";
        cin >> optiune;

        // Dupa cin >> optiune, caracterul '\n' (Enter) ramane in buffer.
        // cin.ignore() il consuma, altfel urmatorul getline() ar citi un sir gol.
        cin.ignore();

        switch (optiune) {
            case 1:
                adaugaContact(agenda);
                break;
            case 2:
                afisareAgenda(agenda);
                break;
            case 3:
                stergereContact(agenda);
                break;
            case 4:
                // Salvam datele INAINTE de a inchide aplicatia
                salveazaInFisier(agenda);
                ruleaza = false;
                cout << "\nDatele au fost salvate. La revedere!\n";
                break;
            default:
                cout << "\nOptiune invalida! Incercati alta optiune!\n";
        }
    }

    return 0;  // Cod de iesire 0 = executie cu succes
}

// ─────────────────────────────────────────────────────────────────────────────
// 5. DEFINITIILE FUNCTIILOR
// ─────────────────────────────────────────────────────────────────────────────

/**
 * afiseazaMeniu
 * Afiseaza lista de optiuni disponibile in consola.
 * Apelata la inceputul fiecarei iteratii din bucla principala.
 */
void afiseazaMeniu() {
    cout << "\n--- Meniu ---"  << endl;
    cout << "1. Adauga contact" << endl;
    cout << "2. Afisare agenda" << endl;
    cout << "3. Stergere contact" << endl;
    cout << "4. Iesire" << endl;
}

// ─────────────────────────────────────────────────────────────────────────────
// FUNCTII DE VALIDARE INPUT
// ─────────────────────────────────────────────────────────────────────────────

/**
 * citesteNumeValid
 * Citeste un sir de la tastatura si il valideaza ca nume (litere si spatii).
 * Bucla do-while repeta cererea pana cand inputul este valid.
 * Aplica automat Title Case: 'ion pop' devine 'Ion Pop'.
 *
 * Parametri:
 *   mesajAfisat — textul afisat ca prompt (ex: "Introduceti Nume: ")
 * Returneaza: sirul validat si formatat in Title Case
 */
string citesteNumeValid(string mesajAfisat) {
    string input;
    bool esteValid;

    do {
        esteValid = true;
        cout << mesajAfisat;
        getline(cin, input);  // getline() citeste intreaga linie, inclusiv spatii

        // Verificam ca inputul nu este gol
        if (input.empty()) {
            cout << "Eroare: Nu ati introdus nimic!\n";
            esteValid = false;
            continue;  // Sarim imediat la urmatoarea iteratie a do-while
        }

        // Parcurgem fiecare caracter si verificam ca este litera sau spatiu
        for (char litera : input) {
            if (!isalpha(litera) && !isspace(litera)) {
                cout << "Eroare: Numele poate contine doar litere si spatii!\n";
                esteValid = false;
                break;  // Iesim din for — nu are rost sa continuam verificarea
            }
        }
    } while (!esteValid);

    // Aplicam Title Case: prima litera din fiecare cuvant devine majuscula,
    // restul devin minuscule. Folosim flag-ul cuvantNou pentru a detecta
    // inceputul unui cuvant nou (dupa spatiu).
    bool cuvantNou = true;
    for (int i = 0; i < input.length(); i++) {
        if (isalpha(input[i])) {
            if (cuvantNou) {
                input[i] = toupper(input[i]);  // Prima litera din cuvant — majuscula
                cuvantNou = false;
            } else {
                input[i] = tolower(input[i]);  // Restul literelor — minuscule
            }
        } else if (isspace(input[i])) {
            cuvantNou = true;  // Dupa un spatiu, urmatoarea litera va fi majuscula
        }
    }

    return input;
}

/**
 * citesteTelefonValid
 * Citeste un numar de telefon si il valideaza.
 * Reguli: doar cifre, poate incepe cu '+', lungime minima 10 caractere.
 *
 * Parametri:
 *   mesajAfisat — textul afisat ca prompt
 * Returneaza: sirul validat al numarului de telefon
 */
string citesteTelefonValid(string mesajAfisat) {
    string input;
    bool esteValid;

    do {
        esteValid = true;
        cout << mesajAfisat;
        getline(cin, input);

        if (input.empty()) {
            cout << "Eroare: Nu ati introdus nimic!\n";
            esteValid = false;
            continue;
        }

        // Parcurgem fiecare caracter al numarului
        for (int i = 0; i < input.length(); i++) {
            // Caracterul '+' este permis DOAR pe prima pozitie (prefix international)
            if (i == 0 && input[i] == '+') {
                continue;  // '+' la inceput este valid — trecem la urmatorul caracter
            }
            // Orice alt caracter care nu este cifra este invalid
            if (!isdigit(input[i])) {
                cout << "Eroare: Telefonul poate contine doar cifre (si '+' la inceput)!\n";
                esteValid = false;
                break;
            }
        }

        // Verificam lungimea minima (10 caractere) — separat de bucla de mai sus,
        // pentru ca ambele conditii trebuie verificate independent
        if (esteValid && input.length() < 10) {
            cout << "Eroare: Numarul introdus este prea scurt!\n";
            esteValid = false;
        }

    } while (!esteValid);

    return input;
}

/**
 * citesteEmailValid
 * Citeste o adresa de email si o valideaza sintactic.
 * Reguli: fara spatii, '@' prezent si nu la margini, '.' dupa '@' in pozitie valida.
 * Converteste automat inputul la litere mici.
 *
 * Parametri:
 *   mesajAfisat — textul afisat ca prompt
 * Returneaza: adresa de email validata, convertita la lowercase
 */
string citesteEmailValid(string mesajAfisat) {
    string input;
    bool esteValid;

    do {
        esteValid = true;
        cout << mesajAfisat;
        getline(cin, input);

        if (input.empty()) {
            cout << "Eroare: Nu ati introdus nimic!\n";
            esteValid = false;
            continue;
        }

        // Regula 1: emailul nu poate contine spatii
        for (char c : input) {
            if (isspace(c)) {
                cout << "Eroare: Adresa de email nu poate contine spatii!\n";
                esteValid = false;
                break;
            }
        }
        if (!esteValid) continue;

        // Regula 2: trebuie sa existe '@', dar nu pe prima sau ultima pozitie
        // string::npos este valoarea returnata de find() cand nu gaseste nimic
        size_t pozitieAt = input.find('@');

        if (pozitieAt == string::npos ||        // Nu exista '@' deloc
            pozitieAt == 0 ||                   // '@' este primul caracter (ex: "@gmail.com")
            pozitieAt == input.length() - 1) {  // '@' este ultimul caracter (ex: "user@")
            cout << "Eroare: Email-ul trebuie sa contina un '@' valid si nu la margini!\n";
            esteValid = false;
            continue;
        }

        // Regula 3: trebuie sa existe '.' dupa '@', in pozitie valida
        // find(char, pozitie) cauta incepand de la pozitia data
        size_t pozitiePunct = input.find('.', pozitieAt);

        if (pozitiePunct == string::npos ||            // Nu exista '.' dupa '@'
            pozitiePunct == pozitieAt + 1 ||           // '.' imediat dupa '@' (ex: "user@.com")
            pozitiePunct == input.length() - 1) {      // '.' la final (ex: "user@gmail.")
            cout << "Eroare: Email-ul trebuie sa contina un domeniu valid (ex: .com, .ro) dupa '@'!\n";
            esteValid = false;
            continue;
        }

    } while (!esteValid);

    // Convertim intregul email la litere mici (standardizare)
    // Ex: "Maria.POP@Gmail.COM" devine "maria.pop@gmail.com"
    for (int i = 0; i < input.length(); i++) {
        input[i] = tolower(input[i]);
    }

    return input;
}

// ─────────────────────────────────────────────────────────────────────────────
// OPERATII AGENDA
// ─────────────────────────────────────────────────────────────────────────────

/**
 * adaugaContact
 * Citeste si valideaza datele unui contact nou, ii atribuie un timestamp
 * si il adauga la sfarsitul vectorului agenda.
 *
 * Parametri:
 *   agenda — referinta la vectorul de contacte (modificat direct)
 */
void adaugaContact(vector<Contact>& agenda) {
    Contact contactNou;  // Cream un contact temporar in care stocam datele citite

    cout << "\nAdaugare contact nou:\n";

    // Folosim functiile de validare specializate pentru fiecare camp
    contactNou.Nume    = citesteNumeValid("Introduceti Nume: ");
    contactNou.Prenume = citesteNumeValid("Introduceti Prenume: ");

    // Adresa nu are validare — acceptam orice sir de caractere
    cout << "Introduceti adresa: ";
    getline(cin, contactNou.adresa);

    contactNou.telefon = citesteTelefonValid("Introduceti telefon: ");
    contactNou.email   = citesteEmailValid("Introduceti e-mail: ");

    // time(nullptr) returneaza numarul de secunde scurse de la 01.01.1970 (Epoch Unix).
    // Acest timestamp va fi folosit ulterior pentru sortarea cronologica.
    contactNou.momentAdaugare = time(nullptr);

    // push_back() adauga contactul la finalul vectorului (redimensionare automata)
    agenda.push_back(contactNou);

    cout << "Contact adaugat cu succes!\n";
}

/**
 * afisareAgenda
 * Sorteaza contactele descrescator dupa data adaugarii si le afiseaza formatat.
 * Sorteaza vectorul original (de aceea parametrul nu este const).
 *
 * Parametri:
 *   agenda — referinta la vectorul de contacte (sortarea modifica ordinea)
 */
void afisareAgenda(vector<Contact>& agenda) {
    if (agenda.empty()) {
        cout << "\nAgenda este goala. Nu exista contacte de afisat!\n";
        return;  // Iesim din functie inainte — nu are sens sa continuam
    }

    // Sortam vectorul folosind regula noastra personalizata.
    // std::sort reorganizeaza elementele intre iteratorii begin() si end().
    // Al treilea argument este functia de comparatie — determina ordinea.
    sort(agenda.begin(), agenda.end(), ordoneazaCeleMaiNoiPrimele);

    cout << "\n================ L I S T A   C O N T A C T E ================\n";

    for (int i = 0; i < agenda.size(); i++) {
        // Convertim timestamp-ul Unix (secunde) intr-un text lizibil.
        // localtime() transforma time_t intr-un struct tm cu zi, luna, an, ora etc.
        // strftime() formateaza struct tm ca sir dupa un sablon dat.
        char textTimp[80];
        struct tm* timpInfo = localtime(&agenda[i].momentAdaugare);
        strftime(textTimp, sizeof(textTimp), "%d-%m-%Y %H:%M:%S", timpInfo);
        // Rezultat exemplu: "29-03-2026 14:22:05"

        cout << "Contact #" << (i + 1) << " [Adaugat la: " << textTimp << "]\n";
        cout << "Nume complet : " << agenda[i].Nume << " " << agenda[i].Prenume << "\n";
        cout << "Telefon      : " << agenda[i].telefon << "\n";
        cout << "Email        : " << agenda[i].email << "\n";
        cout << "Adresa       : " << agenda[i].adresa << "\n";
        cout << "-------------------------------------------------------------\n";
    }
}

/**
 * stergereContact
 * Afiseaza lista de contacte, cere numarul celui de sters,
 * solicita confirmare si il elimina din vector.
 *
 * Parametri:
 *   agenda — referinta la vectorul de contacte (modificat prin stergere)
 */
void stergereContact(vector<Contact>& agenda) {
    if (agenda.empty()) {
        cout << "\nAgenda este goala. Nu aveti ce sterge!\n";
        return;
    }

    cout << "\n--- Stergere Contact ---\n";

    // Afisam lista numerotata pentru ca utilizatorul sa stie ce numar sa introduca
    for (int i = 0; i < agenda.size(); i++) {
        cout << i + 1 << ". " << agenda[i].Nume << " " << agenda[i].Prenume << "\n";
    }

    int idStergere;
    cout << "\nIntroduceti numarul contactului pe care doriti sa il stergeti (sau 0 pentru anulare): ";
    cin >> idStergere;
    cin.ignore();  // Curatam '\n' din buffer dupa cin >>

    // Optiunea 0 anuleaza operatia fara modificari
    if (idStergere == 0) {
        cout << "Stergere anulata.\n";
        return;
    }

    // Validam ca numarul introdus exista in lista (intre 1 si dimensiunea agendei)
    if (idStergere < 1 || idStergere > agenda.size()) {
        cout << "Eroare: Numarul introdus nu exista in agenda!\n";
        return;
    }

    // Convertim numarul afisat (1-based) la indexul real din vector (0-based)
    // Ex: utilizatorul alege "3" → index in vector = 3 - 1 = 2
    int indexReala = idStergere - 1;

    // Cerem confirmare inainte de stergere definitiva
    cout << "Sunteti sigur ca doriti sa stergeti contactul ["
         << agenda[indexReala].Nume << " " << agenda[indexReala].Prenume
         << "]? (D/N): ";

    string confirmare;
    getline(cin, confirmare);

    if (confirmare == "D" || confirmare == "d") {
        // erase() elimina elementul de la pozitia data.
        // agenda.begin() + indexReala calculeaza iteratorul exact al elementului.
        agenda.erase(agenda.begin() + indexReala);
        cout << "Contact sters cu succes!\n";
    } else {
        cout << "Stergere anulata.\n";
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// PERSISTENTA DATE — Fisier
// ─────────────────────────────────────────────────────────────────────────────

/**
 * salveazaInFisier
 * Scrie toate contactele din vector in fisierul "contacte.txt".
 * Fisierul este suprascris complet la fiecare salvare (nu se adauga la final).
 * Formatul: un camp pe linie, 6 linii per contact.
 *
 * Parametri:
 *   agenda — referinta la vectorul de contacte de salvat
 */
void salveazaInFisier(vector<Contact>& agenda) {
    // ofstream deschide fisierul pentru scriere.
    // Daca fisierul exista deja, continutul sau este sters (comportament implicit).
    ofstream fisierOut("contacte.txt");

    if (!fisierOut.is_open()) {
        cout << "Eroare: Nu s-a putut salva fisierul!\n";
        return;
    }

    // Scriem fiecare contact: 6 campuri, fiecare pe linie separata.
    // Acest format este citit linie cu linie de incarcaDinFisier().
    for (int i = 0; i < agenda.size(); i++) {
        fisierOut << agenda[i].Nume             << "\n";
        fisierOut << agenda[i].Prenume          << "\n";
        fisierOut << agenda[i].adresa           << "\n";
        fisierOut << agenda[i].telefon          << "\n";
        fisierOut << agenda[i].email            << "\n";
        fisierOut << agenda[i].momentAdaugare   << "\n";  // Salvat ca numar intreg (Unix timestamp)
    }

    fisierOut.close();  // Eliberam resursa — buna practica, desi destructorul o face automat
}

/**
 * incarcaDinFisier
 * Citeste contactele salvate anterior din "contacte.txt" si le adauga in vector.
 * Daca fisierul nu exista (prima rulare), functia iese silentios fara erori.
 * Include protectie pentru fisiere vechi fara camp de timestamp.
 *
 * Parametri:
 *   agenda — referinta la vectorul de contacte (populat cu datele citite)
 */
void incarcaDinFisier(vector<Contact>& agenda) {
    // ifstream deschide fisierul pentru citire.
    // Daca fisierul nu exista, is_open() returneaza false.
    ifstream fisierIn("contacte.txt");

    if (!fisierIn.is_open()) {
        return;  // Prima rulare — nici o eroare, pornim cu agenda goala
    }

    Contact contactTemporar;  // Contact reutilizat la fiecare iteratie

    // getline() din conditia while citeste primul camp (Nume).
    // Cand fisierul s-a terminat, getline() returneaza false si bucla se opreste.
    while (getline(fisierIn, contactTemporar.Nume)) {
        getline(fisierIn, contactTemporar.Prenume);
        getline(fisierIn, contactTemporar.adresa);
        getline(fisierIn, contactTemporar.telefon);
        getline(fisierIn, contactTemporar.email);

        // Citim timestamp-ul cu protectie pentru fisiere vechi (fara camp de timp)
        string timpText;
        getline(fisierIn, timpText);

        try {
            if (!timpText.empty()) {
                // stoll() = string to long long — convertim textul in numar intreg
                contactTemporar.momentAdaugare = stoll(timpText);
            } else {
                // Camp lipsa (fisier vechi) — atribuim timpul curent ca fallback
                contactTemporar.momentAdaugare = time(nullptr);
            }
        } catch (...) {
            // Catch-all (...) prinde orice exceptie (ex: stoll pe sir invalid).
            // In loc sa cadem, atribuim timpul curent si continuam incarcarea.
            contactTemporar.momentAdaugare = time(nullptr);
        }

        agenda.push_back(contactTemporar);
    }

    fisierIn.close();  // Eliberam resursa
}