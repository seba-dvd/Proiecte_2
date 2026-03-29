/**
 * Agenda de Contacte — C++ Console Application
 * =============================================
 * Descriere:
 * Aplicatie de gestiune a contactelor care ruleaza in linia de comanda.
 * Permite adaugarea, afisarea, cautarea, modificarea si stergerea contactelor,
 * exportul in CSV, impreuna cu persistenta automata a datelor.
 *
 * Utilizare:
 * g++ -o agenda agenda.cpp
 * ./agenda          (Linux / macOS)
 * agenda.exe        (Windows)
 *
 * Cerinte:
 * Compilator C++11 sau mai nou. Nu sunt necesare biblioteci externe.
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
 *
 * Campuri noi fata de versiunea anterioara: judet, oras, tara
 * — necesare pentru o adresa completa si pentru exportul CSV structurat.
 */
struct Contact {
    string Nume;              // Numele de familie — Title Case aplicat automat
    string Prenume;           // Prenumele — Title Case aplicat automat
    string adresa;            // Adresa fizica (strada, nr) — camp liber, fara validare stricta
    string judet;             // Judetul de resedinta — validat: doar litere si spatii
    string oras;              // Orasul de resedinta — validat: doar litere si spatii
    string tara;              // Tara de resedinta — validata: doar litere si spatii
    string telefon;           // Numar de telefon — minim 10 cifre, poate incepe cu '+'
    string email;             // Adresa de email — validata sintactic, convertita la lowercase
    time_t momentAdaugare;    // Timestamp Unix (secunde de la 01.01.1970) — setat automat la adaugare
};

// ─────────────────────────────────────────────────────────────────────────────
// 3. DECLARATII FUNCTII (prototipuri)
// Declarate inainte de main() pentru ca compilatorul sa le cunoasca
// inainte de a fi apelate, chiar daca definitiile vin mai jos.
// ─────────────────────────────────────────────────────────────────────────────

void afiseazaMeniu();
void adaugaContact(vector<Contact>& agenda);
void afisareAgenda(vector<Contact>& agenda);   // Referinta non-const: sortarea modifica ordinea in vector
void cautaContact(const vector<Contact>& agenda);  // const: nu modifica agenda, doar o citeste
void modificaContact(vector<Contact>& agenda);     // Modifica un camp al unui contact existent
void stergereContact(vector<Contact>& agenda);
void exportaCSV(const vector<Contact>& agenda);    // const: nu modifica agenda, doar exporta
void salveazaInFisier(vector<Contact>& agenda);
void incarcaDinFisier(vector<Contact>& agenda);

// Prototipuri pentru verificarea duplicatelor in agenda
bool telefonExista(const vector<Contact>& agenda, const string& tel);
bool emailExista(const vector<Contact>& agenda, const string& email);

/**
 * ordoneazaCeleMaiNoiPrimele
 * Functie de comparatie folosita de std::sort pentru sortarea descrescatoare
 * a contactelor dupa timestamp (momentAdaugare).
 *
 * std::sort asteapta o functie care returneaza true daca primul element
 * trebuie sa apara INAINTEA celui de-al doilea in lista finala.
 * Returnam a > b pentru a obtine ordine descrescatoare (cele mai noi primele).
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
    // La iesire (optiunea 7), continutul sau va fi salvat in contacte.txt.
    vector<Contact> agenda;

    // Incarcam datele existente INAINTE de a afisa meniul.
    // Daca fisierul contacte.txt nu exista (prima rulare), functia nu produce erori.
    incarcaDinFisier(agenda);

    bool ruleaza = true;  // Flag care controleaza bucla principala
    int optiune;

    cout << "Bun venit in agenda!\n";

    // Bucla principala — ruleaza pana cand utilizatorul alege optiunea 7 (Iesire)
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
                cautaContact(agenda);    // NOU: cautare dupa nume, prenume sau telefon
                break;
            case 4:
                modificaContact(agenda); // NOU: editare camp individual al unui contact
                break;
            case 5:
                stergereContact(agenda);
                break;
            case 6:
                exportaCSV(agenda);      // NOU: generare fisier contacte_export.csv
                break;
            case 7:
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
 * Afiseaza lista completa de optiuni disponibile in consola.
 * Apelata la inceputul fiecarei iteratii din bucla principala.
 * Meniul a fost extins cu optiunile 3 (Cauta), 4 (Modifica) si 6 (Export CSV).
 */
void afiseazaMeniu() {
    cout << "\n--- Meniu ---"  << endl;
    cout << "1. Adauga contact" << endl;
    cout << "2. Afisare agenda" << endl;
    cout << "3. Cauta contact" << endl;      // NOU
    cout << "4. Modifica contact" << endl;   // NOU
    cout << "5. Stergere contact" << endl;
    cout << "6. Exporta in CSV" << endl;     // NOU
    cout << "7. Iesire" << endl;
}

// ─────────────────────────────────────────────────────────────────────────────
// FUNCTII DE VALIDARE INPUT & DUPLICATE
// ─────────────────────────────────────────────────────────────────────────────

/**
 * telefonExista
 * Parcurge agenda si verifica daca un anumit numar de telefon a fost deja inregistrat.
 * Folosita atat la adaugare cat si la modificare pentru a preveni duplicatele.
 *
 * Parametri:
 *   agenda — vectorul de contacte (const: nu il modifica)
 *   tel    — numarul de telefon de verificat
 *
 * Returneaza: true daca telefonul exista deja, false altfel
 */
bool telefonExista(const vector<Contact>& agenda, const string& tel) {
    for (int i = 0; i < agenda.size(); i++) {
        if (agenda[i].telefon == tel) return true;
    }
    return false;
}

/**
 * emailExista
 * Parcurge agenda si verifica daca un anumit email a fost deja inregistrat.
 * Folosita atat la adaugare cat si la modificare pentru a preveni duplicatele.
 *
 * Parametri:
 *   agenda — vectorul de contacte (const: nu il modifica)
 *   email  — adresa de email de verificat
 *
 * Returneaza: true daca email-ul exista deja, false altfel
 */
bool emailExista(const vector<Contact>& agenda, const string& email) {
    for (int i = 0; i < agenda.size(); i++) {
        if (agenda[i].email == email) return true;
    }
    return false;
}

/**
 * citesteNumeValid
 * Citeste un sir de la tastatura si il valideaza ca nume (litere si spatii).
 * Bucla do-while repeta cererea pana cand inputul este valid.
 * Aplica automat Title Case: 'ion pop' devine 'Ion Pop'.
 *
 * Parametri:
 *   mesajAfisat — textul prompt afisat inainte de citire (ex: "Introduceti Nume: ")
 *
 * Returneaza: sirul validat si formatat Title Case
 */
string citesteNumeValid(string mesajAfisat) {
    string input;
    bool esteValid;
    do {
        esteValid = true;
        cout << mesajAfisat;
        getline(cin, input); // getline() citeste intreaga linie, inclusiv spatii
        
        // Verificam ca inputul nu este gol
        if (input.empty()) {
            cout << "Eroare: Nu ati introdus nimic!\n";
            esteValid = false;
            continue; // Sarim imediat la urmatoarea iteratie a do-while
        }
        
        // Parcurgem fiecare caracter si verificam ca este litera sau spatiu
        for (char litera : input) {
            if (!isalpha(litera) && !isspace(litera)) {
                cout << "Eroare: Numele poate contine doar litere si spatii!\n";
                esteValid = false;
                break; // Iesim din for — nu are rost sa continuam verificarea
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
                input[i] = toupper(input[i]); // Prima litera din cuvant — majuscula
                cuvantNou = false;
            } else {
                input[i] = tolower(input[i]); // Restul literelor — minuscule
            }
        } else if (isspace(input[i])) {
            cuvantNou = true; // Dupa un spatiu, urmatoarea litera va fi majuscula
        }
    }
    return input;
}

/**
 * citesteTelefonValid
 * Citeste un numar de telefon si il valideaza respectand urmatoarele reguli:
 *   - Poate incepe cu '+' (prefix international, ex: +40)
 *   - Restul caracterelor trebuie sa fie cifre
 *   - Lungime minima: 10 caractere
 *
 * Parametri:
 *   mesajAfisat — textul prompt afisat inainte de citire
 *
 * Returneaza: sirul validat
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
            if (i == 0 && input[i] == '+') continue; 
            
            // Orice alt caracter care nu este cifra este invalid
            if (!isdigit(input[i])) {
                cout << "Eroare: Telefonul poate contine doar cifre (si '+' la inceput)!\n";
                esteValid = false;
                break;
            }
        }
        
        // Verificam lungimea minima (10 caractere)
        if (esteValid && input.length() < 10) {
            cout << "Eroare: Numarul introdus este prea scurt!\n";
            esteValid = false;
        }
    } while (!esteValid);
    return input;
}

/**
 * citesteEmailValid
 * Citeste o adresa de email si o valideaza sintactic respectand regulile:
 *   1. Nu poate contine spatii
 *   2. '@' trebuie sa existe si nu la prima sau ultima pozitie
 *   3. '.' dupa '@' trebuie sa existe in pozitie valida (nu imediat dupa '@' si nici la final)
 * Converteste automat inputul la litere mici (standardizare).
 *
 * Parametri:
 *   mesajAfisat — textul prompt afisat inainte de citire
 *
 * Returneaza: adresa de email validata si convertita la lowercase
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
        if (pozitieAt == string::npos || pozitieAt == 0 || pozitieAt == input.length() - 1) {
            cout << "Eroare: Email-ul trebuie sa contina un '@' valid si nu la margini!\n";
            esteValid = false;
            continue;
        }
        
        // Regula 3: trebuie sa existe '.' dupa '@', in pozitie valida
        // find(char, pozitie) cauta incepand de la pozitia data — cautam dupa '@'
        size_t pozitiePunct = input.find('.', pozitieAt);
        if (pozitiePunct == string::npos || pozitiePunct == pozitieAt + 1 || pozitiePunct == input.length() - 1) {
            cout << "Eroare: Email-ul trebuie sa contina un domeniu valid dupa '@'!\n";
            esteValid = false;
            continue;
        }
    } while (!esteValid);

    // Convertim intregul email la litere mici (standardizare)
    for (int i = 0; i < input.length(); i++) {
        input[i] = tolower(input[i]);
    }
    return input;
}

/**
 * citesteJudetValid
 * Valideaza un sir care reprezinta un judet — aceeasi logica ca la Nume/Prenume:
 * doar litere si spatii, aplicare automata Title Case.
 * Reutilizata si pentru oras si tara prin functii-wrapper de mai jos.
 *
 * Parametri:
 *   mesajAfisat — textul prompt afisat inainte de citire
 *
 * Returneaza: sirul validat si formatat Title Case
 */
string citesteJudetValid(string mesajAfisat) {
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
        for (char litera : input) {
            if (!isalpha(litera) && !isspace(litera)) {
                cout << "Eroare: Poate contine doar litere si spatii!\n";
                esteValid = false;
                break;
            }
        }
    } while (!esteValid);

    // Aplicam Title Case (identic cu citesteNumeValid)
    bool cuvantNou = true;
    for (int i = 0; i < input.length(); i++) {
        if (isalpha(input[i])) {
            if (cuvantNou) { input[i] = toupper(input[i]); cuvantNou = false; } 
            else { input[i] = tolower(input[i]); }
        } else if (isspace(input[i])) {
            cuvantNou = true;
        }
    }
    return input;
}

/**
 * citesteOrasValid / citesteTaraValid
 * Wrapper-uri peste citesteJudetValid — aplica aceeasi validare.
 * Separate semantic pentru claritate la apelare (mesajul prompt difera).
 */
string citesteOrasValid(string mesajAfisat) {
    return citesteJudetValid(mesajAfisat); 
}

string citesteTaraValid(string mesajAfisat) {
    return citesteJudetValid(mesajAfisat); 
}

// ─────────────────────────────────────────────────────────────────────────────
// OPERATII AGENDA
// ─────────────────────────────────────────────────────────────────────────────

/**
 * adaugaContact
 * Cere pe rand datele pentru un contact nou si le valideaza.
 * Campuri noi fata de versiunea anterioara: judet, oras, tara.
 * Se asigura ca email-ul si numarul de telefon sunt unice in vector inainte de salvare.
 *
 * Parametri:
 *   agenda — vectorul de contacte (referinta: se adauga un element nou)
 */
void adaugaContact(vector<Contact>& agenda) {
    Contact contactNou;  
    cout << "\nAdaugare contact nou:\n";

    // Folosim functiile de validare specializate pentru fiecare camp
    contactNou.Nume    = citesteNumeValid("Introduceti Nume: ");
    contactNou.Prenume = citesteNumeValid("Introduceti Prenume: ");
    
    // Adresa strazii nu are o validare stricta (poate contine litere, cifre, cratime)
    cout << "Introduceti adresa (strada, nr, etc.): ";
    getline(cin, contactNou.adresa);

    // Campuri noi: judet, oras, tara — validate prin citesteJudetValid
    contactNou.judet = citesteJudetValid("Introduceti judet: ");
    contactNou.oras  = citesteOrasValid("Introduceti oras: ");
    contactNou.tara  = citesteTaraValid("Introduceti tara: ");

    // Bucla care refuza telefonul daca acesta exista deja la alt contact
    bool telefonUnic = false;
    do {
        contactNou.telefon = citesteTelefonValid("Introduceti telefon: ");
        if (telefonExista(agenda, contactNou.telefon)) {
            cout << "Eroare: Acest numar apartine deja altui contact!\n";
        } else {
            telefonUnic = true;
        }
    } while (!telefonUnic);

    // Bucla care refuza emailul daca acesta exista deja la alt contact
    bool emailUnic = false;
    do {
        contactNou.email = citesteEmailValid("Introduceti e-mail: ");
        if (emailExista(agenda, contactNou.email)) {
            cout << "Eroare: Acest email exista deja in agenda!\n";
        } else {
            emailUnic = true;
        }
    } while (!emailUnic);

    // time(nullptr) returneaza numarul de secunde scurse de la 01.01.1970 (Epoch Unix).
    contactNou.momentAdaugare = time(nullptr);
    
    // push_back() adauga contactul la finalul vectorului (redimensionare automata)
    agenda.push_back(contactNou);
    cout << "Contact adaugat cu succes!\n";
}

/**
 * afisareAgenda
 * Sorteaza vectorul original dupa momentul adaugarii (descrescator) si il afiseaza.
 * Afiseaza adresa completa: strada, oras, judet, tara — campuri noi adaugate.
 *
 * Parametri:
 *   agenda — referinta non-const deoarece std::sort modifica ordinea elementelor
 */
void afisareAgenda(vector<Contact>& agenda) {
    if (agenda.empty()) {
        cout << "\nAgenda este goala. Nu exista contacte de afisat!\n";
        return; 
    }

    // std::sort reorganizeaza elementele intre iteratorii begin() si end()
    // folosind comparatorul ordoneazaCeleMaiNoiPrimele (descrescator dupa timestamp).
    sort(agenda.begin(), agenda.end(), ordoneazaCeleMaiNoiPrimele);
    cout << "\n================ L I S T A   C O N T A C T E ================\n";

    for (int i = 0; i < agenda.size(); i++) {
        // Convertim timestamp-ul Unix (secunde) intr-un text lizibil.
        // localtime() transforma time_t intr-un struct tm cu zi, luna, an, ora etc.
        // strftime() formateaza struct tm ca sir dupa un sablon dat.
        char textTimp[80];
        struct tm* timpInfo = localtime(&agenda[i].momentAdaugare);
        strftime(textTimp, sizeof(textTimp), "%d-%m-%Y %H:%M:%S", timpInfo);

        cout << "Contact #" << (i + 1) << " [Adaugat la: " << textTimp << "]\n";
        cout << "Nume complet : " << agenda[i].Nume << " " << agenda[i].Prenume << "\n";
        cout << "Telefon      : " << agenda[i].telefon << "\n";
        cout << "Email        : " << agenda[i].email << "\n";
        // Adresa completa: strada + oras + judet + tara (campuri noi)
        cout << "Adresa       : " << agenda[i].adresa << ", " << agenda[i].oras
             << ", " << agenda[i].judet << ", " << agenda[i].tara << "\n";
        cout << "-------------------------------------------------------------\n";
    }
}

/**
 * cautaContact                                                            [NOU]
 * Cauta in agenda si afiseaza contactele care contin un anumit text in
 * Nume, Prenume sau Telefon. Cautarea este case-sensitive.
 *
 * Mecanism: string::find() returneaza pozitia primei aparitii a subsirului
 * sau string::npos daca nu il gaseste. Comparam cu npos pentru a detecta potrivirile.
 *
 * Parametri:
 *   agenda — const: cautarea nu modifica niciodata vectorul
 */
void cautaContact(const vector<Contact>& agenda) {
    if (agenda.empty()) {
        cout << "\nAgenda este goala!\n";
        return;
    }
    
    string termenCautat;
    cout << "\n--- Cautare Contact ---\n";
    cout << "Introduceti Nume, Prenume sau Telefon pentru cautare: ";
    getline(cin, termenCautat);
    
    bool gasit = false;
    cout << "\nRezultatele cautarii:\n";
    cout << "-------------------------------------------------------------\n";
    
    for (int i = 0; i < agenda.size(); i++) {
        // Verificam daca termenul cautat apare in Nume, Prenume SAU Telefon
        if (agenda[i].Nume.find(termenCautat) != string::npos || 
            agenda[i].Prenume.find(termenCautat) != string::npos || 
            agenda[i].telefon.find(termenCautat) != string::npos) {
            
            cout << "Nume complet : " << agenda[i].Nume << " " << agenda[i].Prenume << "\n";
            cout << "Telefon      : " << agenda[i].telefon << "\n";
            cout << "Email        : " << agenda[i].email << "\n";
            cout << "-------------------------------------------------------------\n";
            gasit = true;
        }
    }
    if (!gasit) cout << "Niciun contact nu a fost gasit cu acest criteriu.\n";
}

/**
 * modificaContact                                                         [NOU]
 * Afiseaza lista contactelor si deschide un sub-meniu pentru editarea
 * individuala a unui camp ales de utilizator.
 *
 * Sub-meniu disponibil:
 *   1. Nume si Prenume
 *   2. Telefon (cu verificare unicitate fata de alte contacte)
 *   3. Email (cu verificare unicitate fata de alte contacte)
 *   4. Adresa Completa (strada, judet, oras, tara)
 *
 * La verificarea duplicatelor pentru telefon/email, contactul curent (index)
 * este exclus din comparatie — altfel si-ar detecta propriile date ca duplicate.
 *
 * Parametri:
 *   agenda — referinta: campul selectat va fi suprascris in vector
 */
void modificaContact(vector<Contact>& agenda) {
    if (agenda.empty()) {
        cout << "\nAgenda este goala!\n";
        return;
    }

    cout << "\n--- Modificare Contact ---\n";
    // Afisam lista numerotata (1-based) cu Nume, Prenume si Telefon pentru identificare rapida
    for (int i = 0; i < agenda.size(); i++) {
        cout << i + 1 << ". " << agenda[i].Nume << " " << agenda[i].Prenume
             << " (" << agenda[i].telefon << ")\n";
    }

    int idModificare;
    cout << "\nAlegeti numarul contactului pentru modificare (sau 0 pentru anulare): ";
    cin >> idModificare;
    cin.ignore(); // Consumam '\n' ramas in buffer dupa cin >>

    if (idModificare == 0) return; // Utilizatorul a ales sa anuleze
    if (idModificare < 1 || idModificare > agenda.size()) {
        cout << "Eroare: Numarul introdus nu exista!\n";
        return;
    }

    // Convertim id-ul (1-based) introdus de user la indexul din vector (0-based)
    int index = idModificare - 1;

    cout << "\nCe doriti sa modificati la [" << agenda[index].Nume << " " << agenda[index].Prenume << "]?\n";
    cout << "1. Nume si Prenume\n";
    cout << "2. Telefon\n";
    cout << "3. Email\n";
    cout << "4. Adresa Completa (Strada, Judet, Oras, Tara)\n";
    cout << "Alegere: ";
    
    int optiune;
    cin >> optiune;
    cin.ignore();

    switch(optiune) {
        case 1:
            // Citim si validam noul Nume si Prenume
            agenda[index].Nume    = citesteNumeValid("Nume nou: ");
            agenda[index].Prenume = citesteNumeValid("Prenume nou: ");
            break;

        case 2: {
            // Bucla pana gasim un telefon valid SI unic fata de celelalte contacte
            bool telefonUnic = false;
            do {
                string telNou = citesteTelefonValid("Telefon nou: ");
                bool gasitInAltContact = false;
                // Parcurgem restul agendei. Daca numarul e folosit de ALTcineva (i != index),
                // dam eroare. Nu comparam cu propriul contact curent.
                for(int i = 0; i < agenda.size(); i++) {
                    if (i != index && agenda[i].telefon == telNou) {
                        gasitInAltContact = true;
                        break;
                    }
                }
                if (gasitInAltContact) cout << "Eroare: Telefonul exista deja la alt contact!\n";
                else { agenda[index].telefon = telNou; telefonUnic = true; }
            } while(!telefonUnic);
            break;
        }

        case 3: {
            // Bucla pana gasim un email valid SI unic fata de celelalte contacte
            bool emailUnic = false;
            do {
                string emailNou = citesteEmailValid("Email nou: ");
                bool gasitInAltContact = false;
                for(int i = 0; i < agenda.size(); i++) {
                    if (i != index && agenda[i].email == emailNou) {
                        gasitInAltContact = true;
                        break;
                    }
                }
                if (gasitInAltContact) cout << "Eroare: Email-ul exista deja la alt contact!\n";
                else { agenda[index].email = emailNou; emailUnic = true; }
            } while(!emailUnic);
            break;
        }

        case 4:
            // Actualizam toate campurile adresei complete
            cout << "Adresa strada noua: ";
            getline(cin, agenda[index].adresa);
            agenda[index].judet = citesteJudetValid("Judet nou: ");
            agenda[index].oras  = citesteOrasValid("Oras nou: ");
            agenda[index].tara  = citesteTaraValid("Tara noua: ");
            break;

        default:
            cout << "Optiune invalida. Anulat.\n";
            return;
    }
    cout << "\n[+] Contact modificat cu succes!\n";
}

/**
 * stergereContact
 * Sterge un contact prin specificarea indexului sau din lista afisata.
 * Cere confirmare explicita (D/N) inainte de executia stergerii.
 *
 * Mecanism: vector::erase() elimina elementul de la pozitia indicata de
 * iterator; elementele ulterioare sunt deplasate automat spre stanga.
 *
 * Parametri:
 *   agenda — referinta: elementul ales va fi eliminat din vector
 */
void stergereContact(vector<Contact>& agenda) {
    if (agenda.empty()) {
        cout << "\nAgenda este goala. Nu aveti ce sterge!\n";
        return;
    }

    cout << "\n--- Stergere Contact ---\n";
    for (int i = 0; i < agenda.size(); i++) {
        cout << i + 1 << ". " << agenda[i].Nume << " " << agenda[i].Prenume << "\n";
    }

    int idStergere;
    cout << "\nIntroduceti numarul contactului pe care doriti sa il stergeti (sau 0 pt anulare): ";
    cin >> idStergere;
    cin.ignore();  

    if (idStergere == 0) return;
    if (idStergere < 1 || idStergere > agenda.size()) {
        cout << "Eroare: Numarul introdus nu exista in agenda!\n";
        return;
    }

    int indexReala = idStergere - 1;

    // Cerere confirmare inainte de stergere definitiva
    cout << "Sunteti sigur ca doriti sa stergeti contactul ["
         << agenda[indexReala].Nume << " " << agenda[indexReala].Prenume << "]? (D/N): ";

    string confirmare;
    getline(cin, confirmare);

    if (confirmare == "D" || confirmare == "d") {
        // begin() + indexReala: iterator catre elementul de sters
        agenda.erase(agenda.begin() + indexReala);
        cout << "[+] Contact sters cu succes!\n";
    } else {
        cout << "Stergere anulata.\n";
    }
}

/**
 * exportaCSV                                                              [NOU]
 * Genereaza fisierul 'contacte_export.csv' cu toate contactele din agenda.
 * Formatul CSV (Comma-Separated Values) poate fi deschis direct in
 * Microsoft Excel sau Google Sheets pentru vizualizare tabelara.
 *
 * Structura fisierului:
 *   - Linia 1: header cu numele coloanelor
 *   - Liniile 2+: datele fiecarui contact, campuri separate prin virgula
 *   - Timestamp-ul este exportat ca numar Unix (nu formatat) pentru a permite
 *     filtrare/sortare numerica in Excel
 *
 * Atentie: campurile care contin virgule (ex: adresa) pot rupe structura CSV.
 * O versiune viitoare poate adauga ghilimele in jurul campurilor cu virgule.
 *
 * Parametri:
 *   agenda — const: exportul nu modifica niciodata vectorul
 */
void exportaCSV(const vector<Contact>& agenda) {
    if (agenda.empty()) {
        cout << "\nNu exista date de exportat!\n";
        return;
    }

    ofstream fisierCSV("contacte_export.csv");
    if (!fisierCSV.is_open()) {
        cout << "[!] Eroare: Nu s-a putut crea fisierul CSV!\n";
        return;
    }

    // Header-ul tabelului (numele coloanelor) — include si campurile noi: Oras, Judet, Tara
    fisierCSV << "Nume,Prenume,Telefon,Email,Adresa_Strada,Oras,Judet,Tara,Timestamp_Adaugare\n";

    // Datele contactelor, fiecare camp delimitat prin virgula
    for (int i = 0; i < agenda.size(); i++) {
        fisierCSV << agenda[i].Nume     << ","
                  << agenda[i].Prenume  << ","
                  << agenda[i].telefon  << ","
                  << agenda[i].email    << ","
                  << agenda[i].adresa   << ","
                  << agenda[i].oras     << ","
                  << agenda[i].judet    << ","
                  << agenda[i].tara     << ","
                  << agenda[i].momentAdaugare << "\n"; // Timestamp Unix brut
    }

    fisierCSV.close();
    cout << "\n[+] Datele au fost exportate cu succes in fisierul 'contacte_export.csv'!\n";
    cout << "[*] Poti da dublu-click pe el in folderul proiectului pentru a-l deschide cu Excel.\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// PERSISTENTA DATE — Fisier
// ─────────────────────────────────────────────────────────────────────────────

/**
 * salveazaInFisier
 * Scrie toate contactele din vector in fisierul text principal 'contacte.txt'.
 * Fisierul este suprascris complet la fiecare operatie de salvare (modul 'w').
 * Ordinea campurilor trebuie sa fie identica cu cea din incarcaDinFisier.
 *
 * Format fisier (un camp per linie, 9 linii per contact):
 *   Nume / Prenume / Adresa / Judet / Oras / Tara / Telefon / Email / Timestamp
 *
 * Parametri:
 *   agenda — vectorul de contacte de salvat
 */
void salveazaInFisier(vector<Contact>& agenda) {
    ofstream fisierOut("contacte.txt");
    if (!fisierOut.is_open()) return; // Daca fisierul nu poate fi creat, iesim silentios

    for (int i = 0; i < agenda.size(); i++) {
        fisierOut << agenda[i].Nume             << "\n";
        fisierOut << agenda[i].Prenume          << "\n";
        fisierOut << agenda[i].adresa           << "\n";
        fisierOut << agenda[i].judet            << "\n";  // Camp nou
        fisierOut << agenda[i].oras             << "\n";  // Camp nou
        fisierOut << agenda[i].tara             << "\n";  // Camp nou
        fisierOut << agenda[i].telefon          << "\n";
        fisierOut << agenda[i].email            << "\n";
        fisierOut << agenda[i].momentAdaugare   << "\n";
    }
    fisierOut.close();  
}

/**
 * incarcaDinFisier
 * Populeaza vectorul de contacte la pornire citind 'contacte.txt' linie cu linie.
 * Ordinea campurilor trebuie sa fie identica cu cea din salveazaInFisier.
 * Are protectie pentru timestamp lipsa sau corupt (fallback la timpul curent).
 *
 * Compatibilitate: fisierele salvate de versiunea veche (fara judet/oras/tara)
 * NU sunt compatibile cu aceasta versiune — campurile ar fi citite gresit.
 *
 * Parametri:
 *   agenda — vectorul in care se adauga contactele incarcate
 */
void incarcaDinFisier(vector<Contact>& agenda) {
    ifstream fisierIn("contacte.txt");
    if (!fisierIn.is_open()) return;  // Prima rulare: fisierul nu exista inca

    Contact contactTemporar;  

    // getline citeste primul camp (Nume). Cand fisierul se termina, while se opreste.
    while (getline(fisierIn, contactTemporar.Nume)) {
        getline(fisierIn, contactTemporar.Prenume);
        getline(fisierIn, contactTemporar.adresa);
        getline(fisierIn, contactTemporar.judet);   // Camp nou
        getline(fisierIn, contactTemporar.oras);    // Camp nou
        getline(fisierIn, contactTemporar.tara);    // Camp nou
        getline(fisierIn, contactTemporar.telefon);
        getline(fisierIn, contactTemporar.email);

        // Timestamp-ul este stocat ca text si trebuie convertit inapoi la time_t
        string timpText;
        getline(fisierIn, timpText);

        try {
            if (!timpText.empty()) {
                // stoll() (string to long long) converteste textul in numar intreg
                contactTemporar.momentAdaugare = stoll(timpText);
            } else {
                // Timestamp lipsa: folosim timpul curent ca fallback
                contactTemporar.momentAdaugare = time(nullptr);
            }
        } catch (...) {
            // Un catch-all (...) captureaza orice exceptie la conversie (ex: text corupt)
            contactTemporar.momentAdaugare = time(nullptr);
        }

        agenda.push_back(contactTemporar);
    }
    fisierIn.close();  
}