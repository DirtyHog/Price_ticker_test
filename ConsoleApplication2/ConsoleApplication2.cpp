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
#define CZAS_ODSWIEZANIA_CEN 4000               //Czas w ms

//Testing testing

bool                                            czy_pobrano = false;
bool                                            czy_pobrano_szczegolowe_dane = false;
int                                             ilosc_par_lista = 0;                                        //ilosc par bez powtorzen
int                                             ilosc_par_lista_szczegolowe = 0;                            //ilosc par dla szczegolowych danych
clock_t                                         start, koniec, czas_pobrania;                               //Czas pobierania danych
std::string                                     dane_wejsciowe[ILOSC_TEST] = { "BTCUSDT","ETHUSDT","SOLUSDT","ADAUSDT","OPUSDT","ENSUSDT","PAXGUSDT","AAVEUSDT" };      //Testowy wektor
std::string                                     url_crypto;                                                 //url do pobierania aktualnych cen do tickera
std::vector<std::string>                        kryptowaluty_lista;                                         //lista krypto ktora bedzie do wyboru
std::vector<std::vector<std::string>>           kryptowaluty_lista_szczegolowe_dane;                        //lista krypto z szczegolami typu (wolumen,%zmiana,cena,...)
std::vector<std::vector<std::string>>           kryptowaluty_aktualne_ceny;                                 //macierz par cena-symbol do odswiezania cen


void inicjuj();                                                                             //Funkcja do testow                                                                       
void stworz_url();                                                                          //Stworz url do pobierania aktualnych cen
void sortuj_lista_szczegolowe_dane(std::string filtr, bool tryb, int ilosc_par);            //sortuj macierz kryptowaluty_lista_szczegolowe_dane po filtrze zgodym z api binance [TRYB - true sortuje rosn¹co/false malej¹co]
void stworz_liste();                                                                        //Tworzy liste samych symboli krypto i usuwa koncowki "USDT","BTC","ETH" itp i powtorzenia z wektora z danymi szczegolowymi
bool sprawdz_powtorzenia(std::vector<std::string> lista1, std::string text, int ilosc_i);   //sprawdza powtorzenia na nowo tworzonej liscie
bool pobierz_dane(std::string text);                                                        //Pobiera dane dla konkretnych wybranych crypto
bool pobierz_dane_szczegolowe();                                                            //Tylko raz pobiera dla wszystkich krypto dane typu wolumen, 24h % zmian itp 



int main()
{
    inicjuj();
    stworz_url();
    std::cout << "Pobieram szczegolowe dane ...";
    while (!czy_pobrano_szczegolowe_dane) { czy_pobrano_szczegolowe_dane = pobierz_dane_szczegolowe();}
    sortuj_lista_szczegolowe_dane("priceChangePercent",false, ilosc_par_lista_szczegolowe);
    std::cout << "  - ok !" << std::endl;
    stworz_liste();
    std::sort(kryptowaluty_lista.begin(), kryptowaluty_lista.end());
    for (int j = 0; j < 1; j++)
    {
        if (j > 0) { Sleep(CZAS_ODSWIEZANIA_CEN - czas_pobrania); }
        std::cout << "Pobieram aktualne ceny ...";
        start = clock();
        while (!czy_pobrano) { czy_pobrano = pobierz_dane(url_crypto); std::cout << ".";}
        koniec = clock();
        czas_pobrania = (koniec - start) * 1000 / CLOCKS_PER_SEC;
        std::cout << "  - ok !" << std::endl;
        for (int i = 0; i < ILOSC_TEST; i++)
        {
            std::cout << kryptowaluty_aktualne_ceny[i][0] << " --> " << kryptowaluty_aktualne_ceny[i][1] << std::endl;
        }
        std::cout << "Czas pobrania: " << czas_pobrania << "ms, odswiezenie cen za: " << CZAS_ODSWIEZANIA_CEN - czas_pobrania << "ms" << std::endl;
        czy_pobrano = false;
    }
}

void inicjuj()
{
    std::vector<std::string> para;
    for (int j = 0; j < ILOSC_TEST; j++)
    {
        para.push_back(dane_wejsciowe[j]);
        para.push_back("0.00");
        kryptowaluty_aktualne_ceny.push_back(para);
        para.clear();
    }
}
void stworz_url()
{
    std::string url("https://api.binance.com/api/v3/ticker/price?symbols=[");
    for (int i = 0; i < ILOSC_TEST; i++)
    {
        if (i > 0) { url = url + ","; }
        url = url + "\"" + kryptowaluty_aktualne_ceny[i][0] + "\"";
    }
    url_crypto = url + "]";
}
void sortuj_lista_szczegolowe_dane(std::string filtr, bool tryb, int ilosc_par)
{
    int licznik = 0;
    std::string tymczasowy[21] = { "symbol" ,"priceChange" ,"priceChangePercent" ,"weightedAvgPrice",
    "prevClosePrice" , "lastPrice" ,"lastQty","bidPrice","bidQty","askPrice","askQty","openPrice",
    "highPrice" ,"lowPrice" ,"volume" ,"quoteVolume" ,"openTime" ,"closeTime" ,"firstId" ,"lastId","count" };
    std::vector<std::string> tymczasowy1;
    std::vector<double>      tymczasowy11;
    std::vector<std::vector<std::string>> zapas;
    for (int i = 0; i < 21; i++)                                                                                //21 - to stala wynikajaca z ilosci argumentow binance api
    {
        if (filtr == tymczasowy[i]) { licznik = i; i = 21;}
    }
    for (int i = 0; i < ilosc_par; i++)
    {
        if(licznik==0) { tymczasowy1.push_back(kryptowaluty_lista_szczegolowe_dane[i][licznik]); }       //inna zmienna dla zmiennej string inczaej dla double ||  Je¿eli "symbol" wtedy sortowanie stringow/ inaczej sortowanie liczb    
        else { tymczasowy11.push_back(stod(kryptowaluty_lista_szczegolowe_dane[i][licznik])); }
    }
    if (tryb) 
    { 
        if (licznik == 0) { std::sort(tymczasowy1.begin(), tymczasowy1.end()); }
        else { std::sort(tymczasowy11.begin(), tymczasowy11.end()); }
    }
    else 
    { 
        if (licznik == 0) { std::sort(tymczasowy1.begin(), tymczasowy1.end(), std::greater<std::string>()); }
        else { std::sort(tymczasowy11.begin(), tymczasowy11.end(), std::greater<double>()); }
    }
    for (int i = 0; i < ilosc_par; i++)
    {
        for (int j = 0; j < ilosc_par; j++)
        {
            if (licznik == 0)
            {
                if (tymczasowy1[i] == kryptowaluty_lista_szczegolowe_dane[j][licznik])
                {
                    zapas.push_back(kryptowaluty_lista_szczegolowe_dane[j]);
                    j = ilosc_par;
                }
            }
            else
            {
                if (tymczasowy11[i] == stod(kryptowaluty_lista_szczegolowe_dane[j][licznik]))
                {
                    zapas.push_back(kryptowaluty_lista_szczegolowe_dane[j]);
                    j = ilosc_par;
                }
            }
        }
    }
    kryptowaluty_lista_szczegolowe_dane.clear();
    kryptowaluty_lista_szczegolowe_dane = zapas;
    tymczasowy1.clear();
    zapas.clear();
}
void stworz_liste()
{
    std::vector<std::string> lista;
    std::vector<std::string> tymczasowy;
    int szukaj;
    ilosc_par_lista = 0;
    kryptowaluty_lista.clear();
    for (int i = 0; i < ilosc_par_lista_szczegolowe; i++)   //Stworz wektor nazw wszystkich krypto
    {
        lista.push_back(kryptowaluty_lista_szczegolowe_dane[i][0]);
    }
    for (int i = 0; i < ilosc_par_lista_szczegolowe; i++)   //usun "DOWN" "UP" "AUCTION" "BULL" "BEAR"
    {
        szukaj = lista[i].find("DOWN");
        if (szukaj > 0) { lista[i] = ""; }
        szukaj = lista[i].find("UP");
        if (szukaj > 0) { lista[i] = ""; }
        szukaj = lista[i].find("BULL");
        if (szukaj > 0) { lista[i] = ""; }
        szukaj = lista[i].find("BEAR");
        if (szukaj > 0) { lista[i] = ""; }
        szukaj = lista[i].find("AUCTION");
        if (szukaj > 0) { lista[i] = ""; }
        szukaj = lista[i].find("USDT");         //usun koncowke USDT
        if (szukaj > 0)
        {
            lista[i].replace(szukaj, 4, "");
            if (sprawdz_powtorzenia(tymczasowy, lista[i], ilosc_par_lista))
            {
                tymczasowy.push_back(lista[i]);                                         //tymczasowy wektor do zapisu wszytskich krypto bez koncowek
                ilosc_par_lista++;
            }
        }
        szukaj = lista[i].find("BTC");          //usun koncowke BTC
        if (szukaj > 0)
        {
            lista[i].replace(szukaj, 3, "");
            if (sprawdz_powtorzenia(tymczasowy, lista[i], ilosc_par_lista))
            {
                tymczasowy.push_back(lista[i]);
                ilosc_par_lista++;
            }
        }
        szukaj = lista[i].find("ETH");          //usun koncowke ETH
        if (szukaj > 0)
        {
            lista[i].replace(szukaj, 3, "");
            if (sprawdz_powtorzenia(tymczasowy, lista[i], ilosc_par_lista))
            {
                tymczasowy.push_back(lista[i]);
                ilosc_par_lista++;
            }
        }
    }
    kryptowaluty_lista = tymczasowy;
    tymczasowy.clear();
    lista.clear();
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
    const std::string url = text;
    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    long httpCode(0);
    std::unique_ptr<std::string> httpData(new std::string());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());
    curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_easy_cleanup(curl);
    if (httpCode == 200)
    {
        Json::Value jsonData;
        Json::Reader jsonReader;
        std::vector<std::string>    tymczasowy;

        if (jsonReader.parse(*httpData.get(), jsonData))
        {
            kryptowaluty_aktualne_ceny.clear();
            for (Json::Value::ArrayIndex i = 0; i != jsonData.size(); i++)
            {
                tymczasowy.push_back(jsonData[i]["symbol"].asString());
                tymczasowy.push_back(jsonData[i]["price"].asString());
                kryptowaluty_aktualne_ceny.push_back(tymczasowy);
                tymczasowy.clear();
            }
            return true;
        }
        else { return false; }  
    }
    else
    {
        return false;
    }
}
bool pobierz_dane_szczegolowe()
{
    const std::string url("https://api.binance.com/api/v3/ticker/24hr");
    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    long httpCode(0);
    std::unique_ptr<std::string> httpData(new std::string());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());
    curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_easy_cleanup(curl);
    if (httpCode == 200)
    {
        Json::Value jsonData;
        Json::Reader jsonReader;
        std::vector<std::string>    tymczasowy;
        if (jsonReader.parse(*httpData.get(), jsonData))
        {
            kryptowaluty_lista_szczegolowe_dane.clear();
            ilosc_par_lista_szczegolowe = 0;
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
                ilosc_par_lista_szczegolowe++;
                tymczasowy.clear();
            }
            return true;
        }
        else { return false; }
    }
    else
    {
        return false;
    }
}