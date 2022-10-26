# Laboratorium - współbieżność w systemie operacyjnym: łącza

Do laboratorium zostały dołączone pliki:
- [CMakeLists.txt](https://github.com/Emilo77/SEM5-PW-LAB08/blob/master/CMakeLists.txt)
- [child-fifo.c](https://github.com/Emilo77/SEM5-PW-LAB08/blob/master/child-fifo.c)
- [child-pipe.c](https://github.com/Emilo77/SEM5-PW-LAB08/blob/master/child-pipe.c)
- [err.c](https://github.com/Emilo77/SEM5-PW-LAB08/blob/master/err.c)
- [err.h](https://github.com/Emilo77/SEM5-PW-LAB08/blob/master/err.h)
- [parent-dup.c](https://github.com/Emilo77/SEM5-PW-LAB08/blob/master/parent-dup.c)
- [parent-fifo.c](https://github.com/Emilo77/SEM5-PW-LAB08/blob/master/parent-fifo.c)
- [parent-pipe.c](https://github.com/Emilo77/SEM5-PW-LAB08/blob/master/parent-pipe.c)


## Wprowadzenie

Procesy mają oddzielne przestrzenie adresowe, więc ich synchronizacja i komunikacja wymagają zewnętrznych narzędzi i mechanizmów zapewnionych przez środowisko (np. system operacyjny). Zbiór tych mechanizmów nosi miano komunikacji międzyprocesowej. Takimi mechanizmami są np.:

- łącza nienazwane ([pipe](http://man7.org/linux/man-pages/man7/pipe.7.html)),
- łącza nazwane ([fifo](http://man7.org/linux/man-pages/man7/fifo.7.html)),
- kolejki komunikatów ([msg queues](http://man7.org/linux/man-pages/man7/mq_overview.7.html)),
- pamięć dzielona([shared memory](http://man7.org/linux/man-pages/man7/shm_overview.7.html)),
- semafory ([semaphore](http://man7.org/linux/man-pages/man7/sem_overview.7.html)),
- czy sygnały([signals](http://man7.org/linux/man-pages/man7/signal.7.html)).

Na tych zajęciach skupimy się na łączach.

Większość mechanizmów, które zostaną przedstawione występuje w dwu wersjach tzw. nazwanej i nienazwanej. Wersje nienazwane istnieją tylko w jądrze systemu i ich współdzielenie może odbywać się jedynie poprzez dziedziczenie dowiązań do struktur jądra. Dziedziczenie odbywa się podczas wykonania funkcji `fork()`. Obiekty nienazwane cechuje brak fizycznej reprezentacji obiektu w systemie. Z wersjami nazwanymi są związane pliki specjalne.

## Łącza

### Łącza nienazwane

Łącza nienazwane (`pipe`) to rodzaj buforów istniejących tylko wewnątrz jądra systemu operacyjnego. Nie można ich znaleźć na dysku. Służą one do komunikacji między procesami spokrewnionymi. Dwa procesy są spokrewnione, jeśli mają wspólnego przodka (ojca, dziadka, itd.) lub jeden z nich jest przodkiem drugiego.

Łącza są jednokierunkowe, zatem aby uzyskać komunikację w dwie strony należy utworzyć parę łączy.

Do tworzenia łączy nienazwanych służy funkcja systemowa `pipe(int fds[2])`. Funkcja ta tworzy nowe łącze nienazwane oraz umieszcza w tablicy podanej jako parametr, numery dwóch deskryptorów. Pierwszy deskryptor służy do odczytu, a drugi do zapisu do utworzonego łącza. Aby zamknąć łącze używamy funkcji systemowej `close(int fd)`.

Przypominamy, że nie powinno się zamykać deskryptorów 0, 1, 2, chyba że jest ku temu istotny powód. Natomiast zawsze należy zamknąć deskryptory, które sami otworzyliśmy. Robimy to wtedy, gdy deskryptor nie będzie już używany. W ten sposób zapobiegamy zbędnemu kopiowaniu deskryptorów (przy`fork()`) i nadmiernemu rozrostowi tablic deskryptorów.

### Łącza nazwane

Nazwanym odpowiednikiem `pipe` są łącza nazwane (`fifo`). Różnicą między łączami nazwanymi a nienazwanymi jest istnienie pliku specjalnego w systemie plików, który pozwala na uzyskanie dostępu do łącza. Plik specjalny jest jedynie punktem dostępu do łącza i nie przechowuje żadnych informacji.

Plik specjalny łącza tworzymy funkcją `int mkfifo(const char *pathname, mode_t mode)`, gdzie `pathname` jest ścieżką dostępu do pliku a `mode` jest kodem praw dostępu do pliku.

Aby uzyskać dostęp do łącza nazwanego proces musi otworzyć plik specjalny funkcją `int open(const char *pathname, int flags)`. Otwarcie pliku łącza tworzy w systemie strukturę `pipe`. Z każdym plikiem specjalnym jest związana co najwyżej jedna taka struktura, zatem kolejne próby otwarcia pliku zwracają deskryptory do uprzednio utworzonej struktury.

Łącza są jednokierunkowe, więc by uzyskać prawo do pisania należy otworzyć plik z flagą `O_WRONLY` a do czytania z flagą `O_RDONLY`. Deskryptory nie mogą być używane dopóki oba końce łącza nie będą otwarte. Próba otwarcia jednego końca blokuje proces dopóki drugi koniec nie zostanie otwarty.

### Obsługa łączy

Funkcja `write(int fd, const void *buf, size_t count)` zapisuje do otwartego łącza o deskryptorze `fd` nie więcej niż `count` bajtów znajdujących się w tablicy `buf`. Łącza mają ograniczoną pojemność (nie mniejszą niż 4KB). Proces, który próbuje zapisać do łącza, w którym nie ma miejsca na całą zapisywaną porcję, jest wstrzymany do czasu, aż z łącza zostanie odczytana taka ilość danych by znalazło się miejsce na zapisywane dane. Oznacza to, że `write()` zapisze wszystkie dane albo nic. Jedyny wyjątek od tej reguły występuje wtedy, kiedy próbujemy na raz zapisać do łącza więcej niż rozmiar łącza określony przez stałą `PIPE_BUF`. Wtedy proces zapisuje do łącza tyle ile może i jest wstrzymywany do momentu aż znowu będzie mógł coś do łącza zapisać.

Wynikiem funkcji `write()` jest liczba zapisanych bajtów lub `-1`, jeśli nastąpił błąd.

Zapis do łącza jest możliwy tylko wtedy, gdy jest ono otwarte (przez ten sam lub inny proces) do czytania. Jeśli proces spróbuje pisać do łącza, które nie jest przez żaden proces otwarte do czytania, zostanie przerwany sygnałem `SIGPIPE` (więcej o sygnałach na kolejnych zajęciach). Ten błąd najczęściej objawia się komunikatem `Broken pipe` z poziomu interpretera poleceń.

Do odczytu z łącza używamy funkcji systemowej `read(int fd, void *buf, size_t count)`. Funkcja odczytuje z łącza o deskryptorze `fd` nie więcej niż `count` bajtów do bufora znajdującego się pod adresem `buf`. Jeśli w łączu znajduje się mniej niż `count` bajtów ale nie jest puste, to funkcja `read()` odczytuje tyle danych, ile jest w łączu i kończy się pomyślnie. Odczyt z pustego łącza wstrzymuje proces odczytujący do czasu pojawienia się w łączu jakichkolwiek danych. Istnieje jednak sytuacja, w której 0 może być wynikiem funkcji `read()`. Dzieje się tak przy próbie odczytu z łącza, które nie jest przez żaden proces otwarte do zapisu.

Funkcje `write()` i `read()` są niepodzielne. Oznacza to, że operacje odczytu i zapisu wykonywane jednocześnie na tym samym łączu nie będą się przeplatać — jedna z nich rozpocznie się po zakończeniu drugiej. Kolejność odczytu jest zgodna z kolejnością zapisu — łącza są kolejkami prostymi.

Przykładowy scenariusz użycia łącza jest następujący:

- proces tworzy łącze (`pipe()`)
- następnie tworzy proces potomny (`fork()`)
- proces macierzysty zamyka deskryptor do zapisu
- proces potomny zamyka deskryptor do odczytu (odziedziczony po rodzicu)
- proces macierzysty może wykonać funkcję `read()` — będzie ona wstrzymywać proces aż do chwili, gdy proces potomny zapisze coś w łączu — wtedy rodzic odczyta z łącza wiadomość
- gdy proces potomny zamknie deskryptor do zapisu, `read()` w procesie macierzystym przekaże wartość `0`; w ten sposób proces może wykryć koniec strumienia danych i zakończyć się

### Przykład: łącza nienazwane

Proces rodzica `parent-pipe.c` tworzy proces, który ma wykonać program `child_pipe`. Proces macierzysty komunikuje się z nim za pomocą łącza, do którego pisze krótki komunikat.

Potomek `child-pipe.c` czyta komunikat z deskryptora, którego numer jest przekazywany jako jedyny argument wywołania programu.

Funkcja systemowa `exec()` nie wpływa na postać tablicy deskryptorów. Proces po wykonaniu `exec()` zachowuje otwarte łącza choć nie zna już ich numerów, bo zmienne, które przechowywały deskryptory przestały istnieć w chwili wykonania funkcji `exec()`. Stąd konieczność przekazania numeru deskryptora przez argumenty wywołania programu. Nie jest to jednak zalecana technika, szczególnie w przypadku gdy chcemy wykorzystać program, którego kodu nie możemy modyfikować.

### Podmiana standardowego wejścia/wyjścia

W poprzednim przykładzie proces potomny musiał znać numer deskryptora, jeśli chciał odbierać wiadomości przesyłane przez rodzica. W rozwiązaniu przekazywaliśmy numer tego deskryptora w argumentach funkcji `exec()`. Jednak nie zawsze jest to możliwe, np. jeśli chcemy wywołać program, którego kodu źródłowego nie mamy.

Lepszym sposobem przekazania deskryptorów do łącz jest tak zwana *podmiana standardowego wejścia i wyjścia procesu*. Jest to ważna technika stosowana powszechnie w programach uniksowych. Typowy scenariusz jest następujący:

- proces tworzy łącze, następnie wykonuje fork();
- proces macierzysty zamyka niepotrzebne mu deskryptory, po czym wykonuje się dalej używając otwartego łącza;
- proces potomny
  - zamyka zbędne deskryptory wejścia/wyjścia,
  - duplikuje deskryptory łącza na standardowe wejście lub wyjście w zależności od potrzeb,
  - a następnie zamyka niepotrzebne deskryptory;
- proces potomny wykonuje `exec()`

Na skutek zduplikowania standardowe wejście lub wyjście zostaje przekierowane do utworzonego łącza. Zatem kod programu wywoływanego w funkcji `exec()` można napisać w standardowy sposób — odczyt ze standardowego wejścia a zapis na standardowe wyjście. Przekierowane dokonane przed wywołaniem funkcji `exec()` powoduje, że standardowe deskryptory dotyczyć będą już nie terminala lecz uprzednio utworzonego łącza.

Wspomniana duplikacja deskryptorów polega na stworzeniu tworzenia kopii deskryptora w tablicy deskryptorów. Używamy do tego jednej z dwu funkcji `dup(int oldfd)` lub `dup2(int oldfd, int newfd)`.

Funkcja `dup(int oldfd)` duplikuje deskryptor z pozycji `oldfd` na pierwszą wolną pozycję w tablicy deskryptorów, tj. na wolną pozycję o najmniejszym numerze.

Funkcja `dup2(int oldfd, int newfd)` kopiuje deskryptor z pozycji `oldfd` pozycję `newfd`.

W obydwu przypadkach wynikiem funkcji jest numer nowego deskryptora, a w wypadku błędu `-1` i kod błędu w `errno`. Deskryptor z pozycji `oldfd` nie jest zamykany i może być dalej wykorzystywany.

### Przykład: duplikacja deskryptorów

W przykładzie `parent-dup.c` zostaje utworzony proces potomny, który wykona program wskazany przez argument rodzica np. `./parent-dup ps`, można także podać argumenty czy opcje: `./parent-dup ps -l`. Proces rodzic wysyła komunikat do procesu potomnego. Spróbuj wywołać programy, które czytają coś ze standardowego wejścia np. `./parent-dup cat` i `./parent-dup wc`

### Przykład: łącza nazwane

Jeszcze raz para rodzic-dziecko: `parent-fifo`.c i `child-fifo.c`, tym razem używająca łączy nazwanych. Proces rodzica tworzy proces, który ma wykonać program `child_fifo`. Proces macierzysty komunikuje się z nim za pomocą łącza reprezentowanego przez plik specjalny `"/tmp/fifo_tmp"`.

## Ćwiczenie punktowane 7

Podmiana deskryptorów jest ważną i powszechną techniką wykorzystywaną w Linuksie. Przykładem mechanizmu, który bezpośrednio wykorzystuje tę technikę jest `pipeline`, czyli operator `|`. Wywołanie `A|B` w terminalu powoduje przekierowanie standardowego wyjścia programu `A` na standardowe wejście programu `B`.

Np.: wywołanie

```console
echo "Ala ma kota" | fold -w 1 | grep -ic a
```

policzy liczbę liter `a` w napisie "Ala ma kota"

Dzisiejszym zadaniem jest napisanie uproszczonego odpowiednika operatora `|`. Program `myPipe` ma połączyć rurociągiem argumenty, z którymi został wywołany, czyli

```
./myPipe a1 a2 ... aN
```
ma wykonać
```
a1 | a2 | ... | aN
```
