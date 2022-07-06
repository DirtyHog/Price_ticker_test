#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <time.h>
#include <atlstr.h>
#include <algorithm>    // std::sort

#include <curl/curl.h>
#include <json/json.h>

#define ILOSC_TEST 8

//Testing testing

clock_t                                 start, koniec;                              //Czas pobierania danych
std::string                             url_crypto = "";
std::string                             url_crypto_szczegolowe_dane = "https://api.binance.com/api/v3/ticker/24hr?symbols=%5B%22BTCUSDT%22,%22BNBUSDT%22%5D";
int                                     ilosc_par_lista = 0;
std::vector<std::string>                kryptowaluty_wszystkie;
std::vector<std::string>                kryptowaluty_lista;
std::vector<std::string>                kryptowaluty_lista_z_koncowkami;
std::vector<std::vector<std::string>>   kryptowaluty_lista_szczegolowe_dane;
std::vector<std::vector<std::string>>   kryptowaluty_aktualne_ceny;
std::vector<std::vector<std::string>>   cale_dane;
std::vector<std::string>                para;
std::string                             dane_wejsciowe[ILOSC_TEST] = { "BTCUSDT","ETHUSDT","SOLUSDT","ADAUSDT","OPUSDT","ENSUSDT","PAXGUSDT","AAVEUSDT" };      //Testowy wektor

void inicjuj()
{
    for (int j = 0; j < ILOSC_TEST; j++)
    {
        para.clear();
        //std::cout << dane_wejsciowe[j] << "-->" << j << std::endl;
        para.push_back(dane_wejsciowe[j]);
        para.push_back("0.00");
        cale_dane.push_back(para);
    }
}                                                                                           //Funkcja do testow                                                                       
void stworz_url()                                                                           
{
    std::string url("https://api.binance.com/api/v3/ticker/price?symbols=[");
    for (int i = 0; i < ILOSC_TEST; i++)
    {
        if (i > 0) { url = url + ","; }
        url = url + "\"" + cale_dane[i][0] + "\"";
    }
    url_crypto = url + "]";
}                                                                                           //Stworz url do pobierania aktualnych cen
void stworz_url_szczegoly()
{
    std::string url("https://api.binance.com/api/v3/ticker/24hr?symbols=[");
    for (int i = 0; i < ilosc_par_lista; i++)
    {
        if (i > 0) { url = url + ","; }
        url = url + "\"" + kryptowaluty_lista_z_koncowkami[i] + "\"";
    }
    url_crypto_szczegolowe_dane = url + "]";
}                                                                                           //Stworz url do pobierania wolumenu 24h/% itp
bool pobierz_dane(std::string text);                                                        //Pobiera dane dla konkretnych wybranych crypto
bool pobierz_dane_lista();                                                                  //Tylko raz na poczatku pobiera cala liste wszystkich crypto
bool pobierz_dane_szczegolowe(std::string text);                                            //Tylko raz pobiera dla wszystkich krypto dane typu wolumen, 24h % zmian itp 
void porzadkuj_liste(std::vector<std::string> lista);                                       //Usun koncowki "USDT","BTC","ETH" itp i powtorzenia 
bool sprawdz_powtorzenia(std::vector<std::string> lista1, std::string text, int ilosc_i);   //sprawdza powtorzenia na nowo tworzonej liscie


int main()
{
    bool czy_pobrano = false;
    bool czy_pobrano_liste = false;
    bool czy_pobrano_szczegolowe_dane = false;
    inicjuj();
    stworz_url();
    std::cout << "Pobieram liste kryptowalut ...";
    while (!czy_pobrano_liste) { czy_pobrano_liste = pobierz_dane_lista(); }
    kryptowaluty_lista = kryptowaluty_wszystkie;
    porzadkuj_liste(kryptowaluty_lista);
    std::sort(kryptowaluty_lista.begin(), kryptowaluty_lista.end());
    std::sort(kryptowaluty_lista_z_koncowkami.begin(), kryptowaluty_lista_z_koncowkami.end());
    std::cout << "  - ok !" << std::endl;
    /*TEST
        //std::cout << "Hello World!\n";
        for (int i = 0; i < ilosc_par_lista; i++)
        {
            std::cout << kryptowaluty_lista[i] << std::endl;
            //std::cout << cale_dane[i][0] << " -> " << cale_dane[i][1] << std::endl;
        }
        for (int i = 0; i < ilosc_par_lista; i++)
        {
            std::cout << kryptowaluty_lista_z_koncowkami[i] << std::endl;
            //std::cout << cale_dane[i][0] << " -> " << cale_dane[i][1] << std::endl;
        }
    */
    std::cout << "Pobieram szczegolowe dane ...";
    stworz_url_szczegoly();
    while (!czy_pobrano_szczegolowe_dane) { czy_pobrano_szczegolowe_dane = pobierz_dane_szczegolowe(url_crypto_szczegolowe_dane); }
    /*
    for (int j = 0; j < ilosc_par_lista; j++)
    {
        for (int i = 0; i < 21; i++)
        {
            //TEST
            std::cout << kryptowaluty_lista_szczegolowe_dane[j][i] << std::endl;
        }
    }
    std::cout << url_crypto_szczegolowe_dane << std::endl;
    */
    std::cout << "  - ok !" << std::endl;
    //std::cout << "Ilosc kryptowalut: " << ilosc_par_lista << std::endl;
    std::cout << "Pobieram aktualne ceny ...";
    start = clock();
    while (!czy_pobrano) { czy_pobrano = pobierz_dane(url_crypto); }
    koniec = clock();
    std::cout << "  - ok !" << std::endl;
    //TEST
    for (int i = 0; i < ILOSC_TEST; i++)
    {
        std::cout << kryptowaluty_aktualne_ceny[i][0] << " --> " << kryptowaluty_aktualne_ceny[i][1] << std::endl;
        //std::cout << cale_dane[i][0] << " -> " << cale_dane[i][1] << std::endl;
    }
    std::cout << "Czas pobrania: " << (koniec - start)*1000 / CLOCKS_PER_SEC << "ms" << std::endl;
}

namespace
{
    std::size_t callback(
        const char* in,
        std::size_t size,
        std::size_t num,
        std::string* out)
    {
        const std::size_t totalBytes(size * num);
        out->append(in, totalBytes);
        return totalBytes;
    }
}

bool pobierz_dane(std::string text)
{
    const std::string url=text;
    CURL* curl = curl_easy_init();

    // Set remote URL.
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // Don't bother trying IPv6, which would increase DNS resolution time.
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

    // Don't wait forever, time out after 1 seconds.
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1);

    // Follow HTTP redirects if necessary.
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    // Hook up data handling function.
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);

    // Response information.
    //long httpCode(0);
    std::unique_ptr<std::string> httpData(new std::string());

    // Hook up data container (will be passed as the last parameter to the
    // callback handling function).  Can be any pointer type, since it will
    // internally be passed as a void pointer.
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());

    // Run our HTTP GET command, capture the HTTP response code, and clean up.
    curl_easy_perform(curl);
    //curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_easy_cleanup(curl);
    Json::Value jsonData;
    Json::Reader jsonReader;
    std::vector<std::string> tymczasowy;
    kryptowaluty_aktualne_ceny.clear();

    if (jsonReader.parse(*httpData.get(), jsonData))
    {
        for (Json::Value::ArrayIndex i = 0; i != jsonData.size(); i++)
        {
            tymczasowy.push_back(jsonData[i]["symbol"].asString());
            tymczasowy.push_back(jsonData[i]["price"].asString());
            kryptowaluty_aktualne_ceny.push_back(tymczasowy);
            tymczasowy.clear();
        }
        return true;
    }
    else
    {
        return false;
    }
}
bool pobierz_dane_lista()
{
    const std::string url("https://api.binance.com/api/v3/ticker/price");
    CURL* curl = curl_easy_init();

    // Set remote URL.
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // Don't bother trying IPv6, which would increase DNS resolution time.
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

    // Don't wait forever, time out after 1 seconds.
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1);

    // Follow HTTP redirects if necessary.
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    // Hook up data handling function.
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);

    // Response information.
    //long httpCode(0);
    std::unique_ptr<std::string> httpData(new std::string());

    // Hook up data container (will be passed as the last parameter to the
    // callback handling function).  Can be any pointer type, since it will
    // internally be passed as a void pointer.
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());

    // Run our HTTP GET command, capture the HTTP response code, and clean up.
    curl_easy_perform(curl);
    //curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_easy_cleanup(curl);
    Json::Value jsonData;
    Json::Reader jsonReader;

    int ilosc = 0;

    if (jsonReader.parse(*httpData.get(), jsonData))
    {
        for (Json::Value::ArrayIndex i = 0; i != jsonData.size(); i++)
        {
            kryptowaluty_wszystkie.push_back(jsonData[i]["symbol"].asString());
            ilosc++;
        }
        ilosc_par_lista = ilosc;
        return true;
    }
    else
    {
        return false;
    }
}
bool pobierz_dane_szczegolowe(std::string text)
{
    const std::string url = text;
    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
    std::unique_ptr<std::string> httpData(new std::string());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    Json::Value jsonData;
    Json::Reader jsonReader;
    std::vector<std::string> tymczasowy;

    if (jsonReader.parse(*httpData.get(), jsonData))
    {
        for (Json::Value::ArrayIndex i = 0; i != jsonData.size(); i++)
        {
            tymczasowy.push_back(jsonData[i]["symbol"].asString());
            tymczasowy.push_back(jsonData[i]["priceChange"].asString());
            tymczasowy.push_back(jsonData[i]["priceChangePercent"].asString());
            tymczasowy.push_back(jsonData[i]["weightedAvgPrice"].asString());
            tymczasowy.push_back(jsonData[i]["prevClosePrice"].asString());
            tymczasowy.push_back(jsonData[i]["lastPrice"].asString());
            tymczasowy.push_back(jsonData[i]["lastQty"].asString());
            tymczasowy.push_back(jsonData[i]["bidPrice"].asString());
            tymczasowy.push_back(jsonData[i]["bidQty"].asString());
            tymczasowy.push_back(jsonData[i]["askPrice"].asString());
            tymczasowy.push_back(jsonData[i]["askQty"].asString());
            tymczasowy.push_back(jsonData[i]["openPrice"].asString());
            tymczasowy.push_back(jsonData[i]["highPrice"].asString());
            tymczasowy.push_back(jsonData[i]["lowPrice"].asString());
            tymczasowy.push_back(jsonData[i]["volume"].asString());
            tymczasowy.push_back(jsonData[i]["quoteVolume"].asString());
            tymczasowy.push_back(jsonData[i]["openTime"].asString());
            tymczasowy.push_back(jsonData[i]["closeTime"].asString());
            tymczasowy.push_back(jsonData[i]["firstId"].asString());
            tymczasowy.push_back(jsonData[i]["lastId"].asString());
            tymczasowy.push_back(jsonData[i]["count"].asString());
            kryptowaluty_lista_szczegolowe_dane.push_back(tymczasowy);
            tymczasowy.clear();
        }
        return true;
    }
    else
    {
        return false;
    }
}
void porzadkuj_liste(std::vector<std::string> lista)
{
    std::vector<std::string> tymczasowy;
    int szukaj = 0;
    int ilosc_par_nowe = 0;
    for (int i = 0; i < ilosc_par_lista; i++)   //usun "DOWN" "UP" "AUCTION" "BULL" "BEAR"
    {
        szukaj = lista[i].find("DOWN");
        if (szukaj > 0) { lista[i]=""; }
        szukaj = lista[i].find("UP");
        if (szukaj > 0) { lista[i]=""; }
        szukaj = lista[i].find("BULL");
        if (szukaj > 0) { lista[i]=""; }
        szukaj = lista[i].find("BEAR");
        if (szukaj > 0) { lista[i]=""; }
        szukaj = lista[i].find("AUCTION");
        if (szukaj > 0) { lista[i]=""; }
        szukaj = lista[i].find("USDT");         //usun koncowke USDT
        if (szukaj > 0) 
        { 
            lista[i].replace(szukaj, 4, ""); 
            if (sprawdz_powtorzenia(tymczasowy, lista[i], ilosc_par_nowe))
            {
                tymczasowy.push_back(lista[i]);                                         //tymczasowy wektor do zapisu wszytskich krypto bez koncowek
                kryptowaluty_lista_z_koncowkami.push_back(kryptowaluty_lista[i]);       //zapas do pobierania wolumenu i innych
                ilosc_par_nowe++;
            }
        }
        szukaj = lista[i].find("BTC");          //usun koncowke BTC
        if (szukaj > 0) 
        { 
            lista[i].replace(szukaj, 3, "");
            if (sprawdz_powtorzenia(tymczasowy, lista[i], ilosc_par_nowe))
            {
                tymczasowy.push_back(lista[i]);
                kryptowaluty_lista_z_koncowkami.push_back(kryptowaluty_lista[i]);
                ilosc_par_nowe++;
            }
        }
        szukaj = lista[i].find("ETH");          //usun koncowke ETH
        if (szukaj > 0) 
        { 
            lista[i].replace(szukaj, 3, ""); 
            if (sprawdz_powtorzenia(tymczasowy, lista[i], ilosc_par_nowe))
            {
                tymczasowy.push_back(lista[i]);
                kryptowaluty_lista_z_koncowkami.push_back(kryptowaluty_lista[i]);
                ilosc_par_nowe++;
            }
        }
    }
    ilosc_par_lista = ilosc_par_nowe;
    kryptowaluty_lista.clear();
    kryptowaluty_lista = tymczasowy;
    tymczasowy.clear();
}
bool sprawdz_powtorzenia(std::vector<std::string> lista1, std::string text, int ilosc_i)
{
    int zmienna = -1;
    for (int i = 0; i < ilosc_i; i++)
    {
        zmienna = lista1[i].find(text);
        if (zmienna >= 0) { return false; i = ilosc_i; }
    }
    return true;
}
