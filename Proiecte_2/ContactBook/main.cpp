// 1. Importam bibliotecile
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>
#include <ctime>     // --- NOU: Pentru timpul sistemului ---
#include <algorithm> // --- NOU: Pentru functia sort ---

using namespace std;

// 2. Cream structura pentru contact
struct Contact{
    string Nume;
    string Prenume;
    string adresa;
    string telefon;
    string email;
    time_t momentAdaugare; // --- NOU: Stocheaza secunda exacta cand a fost adaugat ---
};

// 3. Declararea functiilor 
void afiseazaMeniu();
void adaugaContact(vector<Contact>& agenda);
void afisareAgenda(vector<Contact>& agenda); // Nu punem const, pentru ca sortarea modifica ordinea in vector
void stergereContact(vector<Contact>& agenda);
void salveazaInFisier(vector<Contact>& agenda);
void incarcaDinFisier(vector<Contact>& agenda);

// --- NOU: Regula de sortare (cele mai noi primele) ---
bool ordoneazaCeleMaiNoiPrimele(const Contact& a, const Contact& b) {
    return a.momentAdaugare > b.momentAdaugare;
}

// 4. Functia principala
int main()
{
    // cream agenda goala
    vector<Contact> agenda;

    // Incarcam datele din fisier INAINTE de a afisa meniul (daca exista)
    incarcaDinFisier(agenda);

    bool ruleaza = true;
    int optiune;

    cout<<"Bun venit in agenda!\n"; 

    while (ruleaza) {
        afiseazaMeniu();
        cout << "\nAlege o optiune: ";
        cin >> optiune;

        // curatam tasta enter dupa citirea anterioara
        cin.ignore();

       switch(optiune){
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
            // 2. Salvam datele INAINTE de a iesi
            salveazaInFisier(agenda);
            ruleaza = false;
            cout << "\nDatele au fost salvate. La revedere!\n"; 
            break;
        default:
            cout << "\nOptiune invalida! Incercati alta optiune!\n"; 
       }
    }

    return 0;
}

// 5. Crearea functiilor pentru apelare in main

// Meniu
void afiseazaMeniu(){
    cout << "\n--- Meniu ---"<< endl;
    cout << "1. Adauga contact" << endl;
    cout << "2. Afisare agenda" << endl;
    cout << "3. Stergere contact" << endl;
    cout << "4. Iesire" << endl;
}

string citesteNumeValid(string mesajAfisat) {
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
                cout << "Eroare: Numele poate contine doar litere si spatii!\n";
                esteValid = false;
                break; 
            }
        }
    } while (!esteValid);

    bool cuvantNou = true;
    for (int i = 0; i < input.length(); i++) {
        if (isalpha(input[i])) {
            if (cuvantNou) {
                input[i] = toupper(input[i]); 
                cuvantNou = false;
            } else {
                input[i] = tolower(input[i]); 
            }
        } else if (isspace(input[i])) {
            cuvantNou = true; 
        }
    }

    return input; 
}

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

        for (int i = 0; i < input.length(); i++) {
            if (i == 0 && input[i] == '+') {
                continue; 
            }
            if (!isdigit(input[i])) {
                cout << "Eroare: Telefonul poate contine doar cifre (si '+' la inceput)!\n";
                esteValid = false;
                break; 
            }
        }

        if (esteValid && input.length() < 10) {
            cout << "Eroare: Numarul introdus este prea scurt!\n";
            esteValid = false;
        }

    } while (!esteValid);

    return input;
}

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

        for (char c : input) {
            if (isspace(c)) {
                cout << "Eroare: Adresa de email nu poate contine spatii!\n";
                esteValid = false;
                break;
            }
        }
        if (!esteValid) continue; 

        size_t pozitieAt = input.find('@');
        
        if (pozitieAt == string::npos || pozitieAt == 0 || pozitieAt == input.length() - 1) {
            cout << "Eroare: Email-ul trebuie sa contina un '@' valid si nu la margini!\n";
            esteValid = false;
            continue;
        }

        size_t pozitiePunct = input.find('.', pozitieAt);

        if (pozitiePunct == string::npos || pozitiePunct == pozitieAt + 1 || pozitiePunct == input.length() - 1) {
            cout << "Eroare: Email-ul trebuie sa contina un domeniu valid (ex: .com, .ro) dupa '@'!\n";
            esteValid = false;
            continue;
        }

    } while (!esteValid);

    for (int i = 0; i < input.length(); i++) {
        input[i] = tolower(input[i]);
    }

    return input;
}

// Adaugare contact 
void adaugaContact(vector<Contact>& agenda) {
    Contact contactNou;

    cout << "\n Adaugare contact nou: "<<endl;

    contactNou.Nume = citesteNumeValid("Introduceti Nume: ");
    contactNou.Prenume = citesteNumeValid("Introduceti Prenume: ");

    cout << "Introduceti adresa: ";
    getline(cin, contactNou.adresa);

    contactNou.telefon = citesteTelefonValid("Introduceti telefon: ");

    contactNou.email = citesteEmailValid("Introduceti e-mail: ");

    // --- NOU: Salvam ora si data la care a fost creat contactul ---
    contactNou.momentAdaugare = time(nullptr);

    agenda.push_back(contactNou);

    cout << "Contact adaugat cu succes!\n";
}

void afisareAgenda(vector<Contact>& agenda) {
    if (agenda.empty()) {
        cout << "\nAgenda este goala. Nu exista contacte de afisat!\n";
        return; 
    }

    // --- NOU: Sortam agenda folosind regula noastra inainte de a afisa ---
    sort(agenda.begin(), agenda.end(), ordoneazaCeleMaiNoiPrimele);

    cout << "\n================ L I S T A   C O N T A C T E ================\n";
    
    for (int i = 0; i < agenda.size(); i++) {
        // --- NOU: Traducem secundele in text citibil (Data si Ora) ---
        char textTimp[80];
        struct tm* timpInfo = localtime(&agenda[i].momentAdaugare);
        strftime(textTimp, sizeof(textTimp), "%d-%m-%Y %H:%M:%S", timpInfo);

        cout << "Contact #" << (i + 1) << " [Adaugat la: " << textTimp << "]\n";
        cout << "Nume complet : " << agenda[i].Nume << " " << agenda[i].Prenume << "\n";
        cout << "Telefon      : " << agenda[i].telefon << "\n";
        cout << "Email        : " << agenda[i].email << "\n";
        cout << "Adresa       : " << agenda[i].adresa << "\n";
        cout << "-------------------------------------------------------------\n";
    }
}

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
    cout << "\nIntroduceti numarul contactului pe care doriti sa il stergeti (sau 0 pentru anulare): ";
    cin >> idStergere;
    
    cin.ignore();

    if (idStergere == 0) {
        cout << "Stergere anulata.\n";
        return;
    }

    if (idStergere < 1 || idStergere > agenda.size()) {
        cout << "Eroare: Numarul introdus nu exista in agenda!\n";
        return;
    }

    int indexReala = idStergere - 1; 
    cout << "Sunteti sigur ca doriti sa stergeti contactul [" << agenda[indexReala].Nume << " " << agenda[indexReala].Prenume << "]? (D/N): ";
    
    string confirmare;
    getline(cin, confirmare);

    if (confirmare == "D" || confirmare == "d") {
        agenda.erase(agenda.begin() + indexReala);
        cout << "Contact sters cu succes!\n";
    } else {
        cout << "Stergere anulata.\n";
    }
}
 
void salveazaInFisier(vector<Contact>& agenda) {
    ofstream fisierOut("contacte.txt"); 

    if (!fisierOut.is_open()) {
        cout << "Eroare: Nu s-a putut salva fisierul!\n";
        return;
    }

    for (int i = 0; i < agenda.size(); i++) {
        fisierOut << agenda[i].Nume << "\n";
        fisierOut << agenda[i].Prenume << "\n";
        fisierOut << agenda[i].adresa << "\n";
        fisierOut << agenda[i].telefon << "\n";
        fisierOut << agenda[i].email << "\n";
        fisierOut << agenda[i].momentAdaugare << "\n"; // --- NOU: Salvam si timpul ---
    }

    fisierOut.close(); 
}

void incarcaDinFisier(vector<Contact>& agenda) {
    ifstream fisierIn("contacte.txt");

    if (!fisierIn.is_open()) {
        return; 
    }

    Contact contactTemporar;

    while (getline(fisierIn, contactTemporar.Nume)) {
        getline(fisierIn, contactTemporar.Prenume);
        getline(fisierIn, contactTemporar.adresa);
        getline(fisierIn, contactTemporar.telefon);
        getline(fisierIn, contactTemporar.email);
        
        // --- NOU: Citim timpul din fisier (cu protectie) ---
        string timpText;
        getline(fisierIn, timpText);
        try {
            if (!timpText.empty()) {
                contactTemporar.momentAdaugare = stoll(timpText); // string to long long
            } else {
                contactTemporar.momentAdaugare = time(nullptr);
            }
        } catch (...) {
            // Daca vechiul fisier nu avea timp salvat, punem timpul de acum ca sa nu dea eroare
            contactTemporar.momentAdaugare = time(nullptr);
        }

        agenda.push_back(contactTemporar);
    }

    fisierIn.close(); 
}