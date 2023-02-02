# Parallel Knapsack Problem

## Descriere generală
Pornind de la implementarea secvențială a problemei rucsacului, s-a implementat un program paralel care rulează un algoritm genetic pentru a rezolva această problemă. Programul este scris în C/C++ și este paralelizat utilizând PThreads.
Implementarea serială se găsește în folderul skel, iar soluția paralelă în rădăcina acestui repo.

## How to use

### Build
    make build

### Run
    ./knapsack_par <fișier_intrare> <numar_generații> <număr_threaduri>
Pe prima linie a fișierului de intrare vom avea numărul de obiecte N pe care le avem la dispoziție și capacitatea rucsacului. Pe următoarele N linii avem câte 2 numere care reprezintă profitul și greutatea fiecărui obiect.


## Informații suplimentare despre implementare
În primul rând, am început prin a distribui numărul de operații în mod egal între threaduri. Astfel, spre deosebire de implementarea serială, toate for-urile din cod nu se vor mai executa de la începutul vectorului de elemente genetice pana la final, ci, în funcție de operația ce urmează să fie executată(inițializare/copiere/mutații/crossover), acestea vor avea efect pe porțiuni din generații pe fiecare thread.

Pentru a ne asigura că vom avea o execuție corectă a algoritmului, am adăugat un număr de bariere pentru a asigura sincronizarea între thread-uri, astfel am introdus bariere pentru:
- a aștepta inițializarea datelor
- în cadrul fiecărei iterații prin generații:
    * a aștepta calculul coeficientului fitness pe întreaga generație
    * a aștepta sortarea după coeficientul fitness în generația curentă
    * a aștepta ca toate copierile, mutațiile și crossover-urile să fie efectuate
    * a aștepta ca indexul pe generația curentă să fie atribuit fiecărui element
- a aștepta ca toate iterațiile să fie finalizate pentru a afișa rezultatul final

Sortările, afișările și verificarea dacă numărul indiviizilor ce urmează să fie folosiți în operația de crossover, urmată de acțiunea necesară în acest caz, vor fi executate doar de thread-ul 0.

Un alt aspect important ce ține de paralelizarea codului din schelet și eficientizarea acestuia este calculul numarului de biti setati pe 1 în șirul de cromozomi. Am constatat faptul că în funcția cmpfunc ce era folosită pentru sortare, de fiecare dată când aceasta era apelată, ea calcula pe loc numarul de biți setați pe 1. Astfel, timpul pierdut era semnificativ, ținând cont că pe un element se itera de mai multe ori prin cromozomii acestuia, prin urmare am ales să introduc un câmp nou în structura individual care să stocheze tocmai acest număr. Calculul numărului de cromozomi setați pe 1 se va face acum o singură dată per iterație în cadrul unei generații, mai exact, în momentul în care se calculează coeficientul fitness.
